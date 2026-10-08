// Copyright SAEVUS. Todos los derechos reservados.

#include "Audio/SAEVUSMusica.h"
#include "Math/RandomStream.h"

namespace SAEVUSMusica
{
	constexpr float MaximoInt16 = 32767.f;
	// Pico de la pista tras normalizar: deja margen para sumar varias a la vez.
	constexpr float PicoNormalizado = 0.7f;

	// Duraciones: exploración 4 acordes de 8 s; combate 8 compases a 120 ppm; jefe 8 compases a 140 ppm.
	constexpr float DuracionExploracion = 32.f;
	constexpr float PulsosPorMinutoCombate = 120.f;
	constexpr float PulsosPorMinutoJefe = 140.f;
	constexpr int32 CompasesBucle = 8;
	constexpr int32 PulsosPorCompas = 4;
	// Cola de reverberación que se suma al principio del bucle.
	constexpr float Cola = 5.f;

	float Frecuencia(float NotaMidi)
	{
		return 440.f * FMath::Pow(2.f, (NotaMidi - 69.f) / 12.f);
	}

	/** Sierra suave (con un poco de seno para quitarle aristas). */
	float Sierra(float Fase)
	{
		const float Ciclo = FMath::Frac(Fase / UE_TWO_PI);
		return 2.f * Ciclo - 1.f;
	}

	float Triangulo(float Fase)
	{
		const float Ciclo = FMath::Frac(Fase / UE_TWO_PI);
		return 4.f * FMath::Abs(Ciclo - 0.5f) - 1.f;
	}

	/** Paso bajo de un polo. */
	struct FPasoBajo
	{
		float Valor = 0.f;
		float Filtrar(float Entrada, float Corte)
		{
			const float Alfa = 1.f - FMath::Exp(-UE_TWO_PI * Corte / Muestreo);
			Valor += Alfa * (Entrada - Valor);
			return Valor;
		}
	};

	/** Lienzo de la pista: suma de voces sobre un búfer con cola; se cierra en bucle al final. */
	struct FLienzo
	{
		TArray<float> Muestras;
		int32 Largo = 0;

		explicit FLienzo(float Duracion)
		{
			Largo = FMath::RoundToInt32(Duracion * Muestreo);
			Muestras.SetNumZeroed(Largo + FMath::RoundToInt32(Cola * Muestreo));
		}

		int32 Indice(float Tiempo) const { return FMath::Clamp(FMath::RoundToInt32(Tiempo * Muestreo), 0, Muestras.Num() - 1); }
	};

	/**
	 * Una nota de cuerdas o coro: dos sierras desafinadas por un paso bajo, con ataque y caída lentos y un
	 * vibrato suave. Inicio y Duracion en segundos.
	 */
	void Colchon(FLienzo& Lienzo, float NotaMidi, float Inicio, float Duracion, float Volumen, float Ataque, float Caida, float Corte)
	{
		const float Base = Frecuencia(NotaMidi);
		const int32 Desde = Lienzo.Indice(Inicio);
		const int32 Hasta = Lienzo.Indice(Inicio + Duracion + Caida);
		float FaseA = 0.f;
		float FaseB = 0.f;
		FPasoBajo Filtro;
		for (int32 Muestra = Desde; Muestra < Hasta; ++Muestra)
		{
			const float Tiempo = static_cast<float>(Muestra - Desde) / Muestreo;
			const float Envolvente = Tiempo < Ataque ? Tiempo / Ataque
				: (Tiempo < Duracion ? 1.f : FMath::Max(0.f, 1.f - (Tiempo - Duracion) / Caida));
			const float Vibrato = 1.f + 0.003f * FMath::Sin(UE_TWO_PI * 4.5f * Tiempo);
			FaseA += UE_TWO_PI * Base * 1.0023f * Vibrato / Muestreo;
			FaseB += UE_TWO_PI * Base * 0.9977f * Vibrato / Muestreo;
			const float Bruto = 0.5f * (Sierra(FaseA) + Sierra(FaseB));
			Lienzo.Muestras[Muestra] += Filtro.Filtrar(Bruto, Corte) * Envolvente * Volumen;
		}
	}

	/** Campana: parciales inarmónicos que se apagan solos. */
	void Campana(FLienzo& Lienzo, float NotaMidi, float Inicio, float Volumen, float Caida)
	{
		const float Base = Frecuencia(NotaMidi);
		const float Parciales[3][2] = { { 1.f, 1.f }, { 2.76f, 0.4f }, { 5.4f, 0.18f } };
		const int32 Desde = Lienzo.Indice(Inicio);
		const int32 Hasta = Lienzo.Indice(Inicio + Caida * 5.f);
		for (int32 Muestra = Desde; Muestra < Hasta; ++Muestra)
		{
			const float Tiempo = static_cast<float>(Muestra - Desde) / Muestreo;
			float Suma = 0.f;
			for (const auto& Parcial : Parciales)
			{
				// Los parciales altos se apagan antes.
				Suma += Parcial[1] * FMath::Sin(UE_TWO_PI * Base * Parcial[0] * Tiempo) * FMath::Exp(-Tiempo * Parcial[0] / Caida);
			}
			const float Golpe = FMath::Min(1.f, Tiempo / 0.004f);
			Lienzo.Muestras[Muestra] += Suma * Golpe * Volumen;
		}
	}

	/** Tambor grave (taiko): seno que cae de tono con un chasquido de ruido al golpe. */
	void Tambor(FLienzo& Lienzo, FRandomStream& Aleatorio, float Inicio, float Volumen, float TonoAlto, float TonoBajo, float Caida)
	{
		const int32 Desde = Lienzo.Indice(Inicio);
		const int32 Hasta = Lienzo.Indice(Inicio + Caida * 5.f);
		float Fase = 0.f;
		FPasoBajo Ruido;
		for (int32 Muestra = Desde; Muestra < Hasta; ++Muestra)
		{
			const float Tiempo = static_cast<float>(Muestra - Desde) / Muestreo;
			const float Tono = FMath::Lerp(TonoBajo, TonoAlto, FMath::Exp(-Tiempo / 0.045f));
			Fase += UE_TWO_PI * Tono / Muestreo;
			const float Cuerpo = FMath::Sin(Fase) * FMath::Exp(-Tiempo / Caida);
			const float Chasquido = Ruido.Filtrar(Aleatorio.FRandRange(-1.f, 1.f), 1800.f) * FMath::Exp(-Tiempo / 0.012f);
			Lienzo.Muestras[Muestra] += (Cuerpo + 0.6f * Chasquido) * Volumen;
		}
	}

	/** Nota pulsada (cuerdas graves en ostinato, metales): sierra con el filtro que se abre al golpe y se cierra. */
	void Pulsada(FLienzo& Lienzo, float NotaMidi, float Inicio, float Duracion, float Volumen, float CorteBase, float CorteGolpe)
	{
		const float Base = Frecuencia(NotaMidi);
		const int32 Desde = Lienzo.Indice(Inicio);
		const int32 Hasta = Lienzo.Indice(Inicio + Duracion + 0.08f);
		float Fase = 0.f;
		FPasoBajo Filtro;
		for (int32 Muestra = Desde; Muestra < Hasta; ++Muestra)
		{
			const float Tiempo = static_cast<float>(Muestra - Desde) / Muestreo;
			const float Envolvente = FMath::Min(1.f, Tiempo / 0.006f) * (Tiempo < Duracion ? FMath::Exp(-Tiempo / (Duracion * 0.8f)) : FMath::Max(0.f, 1.f - (Tiempo - Duracion) / 0.08f));
			Fase += UE_TWO_PI * Base / Muestreo;
			const float Corte = CorteBase + CorteGolpe * FMath::Exp(-Tiempo / 0.08f);
			Lienzo.Muestras[Muestra] += Filtro.Filtrar(Sierra(Fase), Corte) * Envolvente * Volumen;
		}
	}

	/** Bordón continuo de toda la pista (sin principio ni fin: enlaza en bucle). */
	void Bordon(FLienzo& Lienzo, float NotaMidi, float Volumen, float Corte, float CiclosRespiracion)
	{
		const float Base = Frecuencia(NotaMidi);
		float Fase = 0.f;
		float FaseQuinta = 0.f;
		FPasoBajo Filtro;
		for (int32 Muestra = 0; Muestra < Lienzo.Muestras.Num(); ++Muestra)
		{
			const float Tiempo = static_cast<float>(Muestra) / Muestreo;
			Fase += UE_TWO_PI * Base / Muestreo;
			FaseQuinta += UE_TWO_PI * Base * 1.4983f / Muestreo;
			// La respiración del bordón da ciclos enteros en el bucle.
			const float Respira = 0.8f + 0.2f * FMath::Sin(UE_TWO_PI * CiclosRespiracion * Tiempo * Muestreo / Lienzo.Largo);
			const float Bruto = 0.7f * Sierra(Fase) + 0.3f * Triangulo(FaseQuinta);
			Lienzo.Muestras[Muestra] += Filtro.Filtrar(Bruto, Corte) * Respira * Volumen;
		}
	}

	/** Viento: ruido grave que sube y baja (ciclos enteros en el bucle). */
	void Viento(FLienzo& Lienzo, FRandomStream& Aleatorio, float Volumen, float Corte, float CiclosRafaga)
	{
		FPasoBajo Filtro;
		FPasoBajo Segundo;
		for (int32 Muestra = 0; Muestra < Lienzo.Muestras.Num(); ++Muestra)
		{
			const float Fraccion = static_cast<float>(Muestra) / Lienzo.Largo;
			const float Rafaga = 0.5f + 0.5f * FMath::Sin(UE_TWO_PI * CiclosRafaga * Fraccion);
			const float Ruido = Segundo.Filtrar(Filtro.Filtrar(Aleatorio.FRandRange(-1.f, 1.f), Corte), Corte);
			Lienzo.Muestras[Muestra] += Ruido * Rafaga * Volumen;
		}
	}

	/** Reverberación de Schroeder (cuatro peines y dos pasatodo): sala de piedra grande. */
	void Reverberar(FLienzo& Lienzo, float Mezcla, float Realimentacion)
	{
		const int32 Peines[4] = { 1116 / 2, 1188 / 2, 1277 / 2, 1356 / 2 };
		const int32 Pasatodos[2] = { 556 / 2, 225 / 2 };
		const int32 Numero = Lienzo.Muestras.Num();
		TArray<float> Humedo;
		Humedo.SetNumZeroed(Numero);
		for (const int32 Retardo : Peines)
		{
			TArray<float> Linea;
			Linea.SetNumZeroed(Retardo);
			int32 Posicion = 0;
			float Amortiguado = 0.f;
			for (int32 Muestra = 0; Muestra < Numero; ++Muestra)
			{
				const float Salida = Linea[Posicion];
				// La sala se come los agudos en cada vuelta.
				Amortiguado = Salida * 0.6f + Amortiguado * 0.4f;
				Linea[Posicion] = Lienzo.Muestras[Muestra] + Amortiguado * Realimentacion;
				Posicion = (Posicion + 1) % Retardo;
				Humedo[Muestra] += Salida * 0.25f;
			}
		}
		for (const int32 Retardo : Pasatodos)
		{
			TArray<float> Linea;
			Linea.SetNumZeroed(Retardo);
			int32 Posicion = 0;
			for (int32 Muestra = 0; Muestra < Numero; ++Muestra)
			{
				const float Retrasada = Linea[Posicion];
				const float Entrada = Humedo[Muestra];
				const float Salida = -Entrada + Retrasada;
				Linea[Posicion] = Entrada + Retrasada * 0.5f;
				Posicion = (Posicion + 1) % Retardo;
				Humedo[Muestra] = Salida;
			}
		}
		for (int32 Muestra = 0; Muestra < Numero; ++Muestra)
		{
			Lienzo.Muestras[Muestra] = FMath::Lerp(Lienzo.Muestras[Muestra], Humedo[Muestra], Mezcla);
		}
	}

	/** Cierra el bucle (la cola suena sobre el principio), normaliza y pasa a 16 bits. */
	TArray<int16> Cerrar(FLienzo& Lienzo)
	{
		for (int32 Muestra = Lienzo.Largo; Muestra < Lienzo.Muestras.Num(); ++Muestra)
		{
			Lienzo.Muestras[Muestra - Lienzo.Largo] += Lienzo.Muestras[Muestra];
		}
		float Pico = UE_KINDA_SMALL_NUMBER;
		for (int32 Muestra = 0; Muestra < Lienzo.Largo; ++Muestra)
		{
			Pico = FMath::Max(Pico, FMath::Abs(Lienzo.Muestras[Muestra]));
		}
		const float Ganancia = PicoNormalizado / Pico;
		TArray<int16> Resultado;
		Resultado.SetNumUninitialized(Lienzo.Largo);
		for (int32 Muestra = 0; Muestra < Lienzo.Largo; ++Muestra)
		{
			Resultado[Muestra] = static_cast<int16>(FMath::Clamp(Lienzo.Muestras[Muestra] * Ganancia, -1.f, 1.f) * MaximoInt16);
		}
		return Resultado;
	}

	// Acordes (MIDI): re menor, si bemol, sol menor y la mayor (la sensible da la tensión de la menor armónica).
	const float Acordes[4][3] = { { 50.f, 53.f, 57.f }, { 46.f, 50.f, 53.f }, { 43.f, 46.f, 50.f }, { 45.f, 49.f, 52.f } };
	// Pentatónica de re menor aguda, para las campanas.
	const float NotasCampana[5] = { 74.f, 77.f, 79.f, 81.f, 84.f };

	TArray<int16> Exploracion()
	{
		FLienzo Lienzo(DuracionExploracion);
		FRandomStream Aleatorio(1971);
		Bordon(Lienzo, 38.f, 0.16f, 260.f, 2.f);
		const float DuracionAcorde = DuracionExploracion / 4.f;
		for (int32 Acorde = 0; Acorde < 4; ++Acorde)
		{
			for (const float Nota : Acordes[Acorde])
			{
				Colchon(Lienzo, Nota, Acorde * DuracionAcorde, DuracionAcorde, 0.07f, 2.5f, 3.f, 900.f);
			}
			// Una voz de coro una octava arriba, a ratos.
			if (Acorde % 2 == 0)
			{
				Colchon(Lienzo, Acordes[Acorde][0] + 12.f, Acorde * DuracionAcorde + 1.f, DuracionAcorde - 2.f, 0.035f, 3.f, 3.f, 1400.f);
			}
		}
		// Campanas lejanas y escasas: siempre en los mismos sitios (semilla fija).
		for (float Tiempo = 1.5f; Tiempo < DuracionExploracion; Tiempo += 4.f)
		{
			if (Aleatorio.FRand() < 0.65f)
			{
				Campana(Lienzo, NotasCampana[Aleatorio.RandRange(0, 4)], Tiempo + Aleatorio.FRandRange(0.f, 1.f), 0.09f, 1.6f);
			}
		}
		Viento(Lienzo, Aleatorio, 0.05f, 380.f, 2.f);
		Reverberar(Lienzo, 0.38f, 0.84f);
		return Cerrar(Lienzo);
	}

	/** Pelea o jefe: tambores, ostinato grave y colchón de tensión sobre los acordes. */
	TArray<int16> Pelea(float PulsosPorMinuto, bool bJefe)
	{
		const float Pulso = 60.f / PulsosPorMinuto;
		const float Duracion = Pulso * PulsosPorCompas * CompasesBucle;
		const float Semicorchea = Pulso / 4.f;
		FLienzo Lienzo(Duracion);
		FRandomStream Aleatorio(bJefe ? 666 : 1213);
		// El jefe va en re frigio: el segundo acorde es mi bemol (lo más oscuro del modo).
		const float AcordesJefe[4][3] = { { 50.f, 53.f, 57.f }, { 51.f, 55.f, 58.f }, { 50.f, 53.f, 57.f }, { 48.f, 52.f, 55.f } };
		const float (&Usados)[4][3] = bJefe ? AcordesJefe : Acordes;
		const float DuracionAcorde = Duracion / 4.f;

		// Tambores: patrón sincopado de semicorcheas por compás (el jefe golpea más).
		const int32 PatronCombate[] = { 0, 6, 8, 11, 14 };
		const int32 PatronJefe[] = { 0, 3, 4, 8, 10, 12, 14, 15 };
		for (int32 Compas = 0; Compas < CompasesBucle; ++Compas)
		{
			const float InicioCompas = Compas * Pulso * PulsosPorCompas;
			const TConstArrayView<int32> Patron = bJefe ? TConstArrayView<int32>(PatronJefe) : TConstArrayView<int32>(PatronCombate);
			for (int32 Paso = 0; Paso < Patron.Num(); ++Paso)
			{
				const bool bFuerte = Patron[Paso] % 8 == 0;
				Tambor(Lienzo, Aleatorio, InicioCompas + Patron[Paso] * Semicorchea, bFuerte ? 0.55f : 0.32f, bFuerte ? 120.f : 160.f, bFuerte ? 42.f : 60.f, bFuerte ? 0.32f : 0.18f);
			}
		}
		// Ostinato de cuerdas graves en corcheas: raíz, raíz, tercera, raíz, segunda, raíz, séptima baja, raíz.
		const float Patron[8] = { 0.f, 0.f, 3.f, 0.f, 2.f, 0.f, -2.f, 0.f };
		const int32 Corcheas = PulsosPorCompas * CompasesBucle * 2;
		for (int32 Corchea = 0; Corchea < Corcheas; ++Corchea)
		{
			const float Tiempo = Corchea * Pulso * 0.5f;
			const int32 Acorde = FMath::Min(3, static_cast<int32>(Tiempo / DuracionAcorde));
			const float Raiz = Usados[Acorde][0] - 12.f;
			Pulsada(Lienzo, Raiz + Patron[Corchea % 8], Tiempo, Pulso * 0.45f, 0.12f, 300.f, 1500.f);
		}
		// Colchón de tensión y, al cambiar de acorde, un golpe de metales graves.
		for (int32 Acorde = 0; Acorde < 4; ++Acorde)
		{
			for (const float Nota : Usados[Acorde])
			{
				Colchon(Lienzo, Nota, Acorde * DuracionAcorde, DuracionAcorde, bJefe ? 0.05f : 0.04f, 0.6f, 1.2f, bJefe ? 1600.f : 1100.f);
			}
			Pulsada(Lienzo, Usados[Acorde][0] - 12.f, Acorde * DuracionAcorde, Pulso * 1.5f, bJefe ? 0.2f : 0.14f, 250.f, 2600.f);
			if (bJefe)
			{
				// Coro arriba y metales a contratiempo.
				Colchon(Lienzo, Usados[Acorde][2] + 12.f, Acorde * DuracionAcorde, DuracionAcorde, 0.04f, 0.4f, 1.f, 1800.f);
				Pulsada(Lienzo, Usados[Acorde][1], Acorde * DuracionAcorde + Pulso * 2.5f, Pulso * 0.6f, 0.1f, 400.f, 3000.f);
			}
		}
		Reverberar(Lienzo, bJefe ? 0.25f : 0.22f, 0.8f);
		return Cerrar(Lienzo);
	}
}

float SAEVUSMusica::DuracionPista(ESAEVUSPistaMusica Pista)
{
	switch (Pista)
	{
	case ESAEVUSPistaMusica::Combate:
		return 60.f / PulsosPorMinutoCombate * PulsosPorCompas * CompasesBucle;
	case ESAEVUSPistaMusica::Jefe:
		return 60.f / PulsosPorMinutoJefe * PulsosPorCompas * CompasesBucle;
	case ESAEVUSPistaMusica::Exploracion:
	default:
		return DuracionExploracion;
	}
}

TArray<int16> SAEVUSMusica::GenerarPista(ESAEVUSPistaMusica Pista)
{
	switch (Pista)
	{
	case ESAEVUSPistaMusica::Combate:
		return Pelea(PulsosPorMinutoCombate, false);
	case ESAEVUSPistaMusica::Jefe:
		return Pelea(PulsosPorMinutoJefe, true);
	case ESAEVUSPistaMusica::Exploracion:
	default:
		return Exploracion();
	}
}
