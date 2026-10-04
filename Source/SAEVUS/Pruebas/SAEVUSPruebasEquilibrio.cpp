// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Armas/SAEVUSTiposArma.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Combate/SAEVUSTiposCombate.h"
#include "Enemigos/SAEVUSTiposEnemigo.h"
#include "SAEVUS.h"

/**
 * Equilibrio de primera pasada: tablas calculadas a partir de los Data Assets reales (daño por
 * segundo de cada técnica, tiempo para derrotar a cada enemigo, amenaza de cada ataque enemigo).
 * Las tablas van al log con el prefijo "[SAEVUS equilibrio]"; unas pocas reglas de diseño se
 * comprueban para que un cambio de datos no rompa el ritmo sin que nadie se entere.
 */
namespace SAEVUSPruebasEquilibrio
{
	const TCHAR* const RutaRepertorio = TEXT("/Game/SAEVUS/Datos/DA_RepertorioMelee_Base.DA_RepertorioMelee_Base");
	const TCHAR* const RutasArmas[] = {
		TEXT("/Game/SAEVUS/Datos/DA_Arma_Pistola.DA_Arma_Pistola"),
		TEXT("/Game/SAEVUS/Datos/DA_Arma_Rifle.DA_Arma_Rifle"),
		TEXT("/Game/SAEVUS/Datos/DA_Arma_Escopeta.DA_Arma_Escopeta"),
	};
	const TCHAR* const RutasEnemigos[] = {
		TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Rastrero.DA_Enemigo_Rastrero"),
		TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Saltador.DA_Enemigo_Saltador"),
		TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Escupidor.DA_Enemigo_Escupidor"),
		TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Revoloteador.DA_Enemigo_Revoloteador"),
		TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Acorazado.DA_Enemigo_Acorazado"),
		TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Jefe_Guardian.DA_Jefe_Guardian"),
		TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Jefe_MadreAlada.DA_Jefe_MadreAlada"),
	};

	// Reglas de diseño de primera pasada (se revisan al equilibrar).
	/** Aviso mínimo de cualquier ataque enemigo, en cualquier fase: tiempo de reacción humano. */
	constexpr float AvisoMinimo = 0.25f;
	/** Golpes de un mismo ataque o contacto enemigo que hacen falta para tumbar al jugador (aguanta tres). */
	constexpr int32 GolpesMinimosParaCaer = 4;
	/** Un enemigo menor cae con este número de combos de suelo como mucho. */
	constexpr int32 CombosMaximosEnemigoMenor = 2;
	/** Un jefe aguanta al menos este número de combos de suelo. */
	constexpr int32 CombosMinimosJefe = 4;
	/** En la recuperación de cada ataque enemigo cabe al menos este número de tajos. */
	constexpr int32 TajosMinimosEnCastigo = 1;

	/** Una técnica del jugador: daño que hace un ciclo y lo que tarda ese ciclo. */
	struct FTecnica
	{
		FString Nombre;
		float Danio = 0.f;
		float Tiempo = 0.f;

		float DanioPorSegundo() const { return Tiempo > 0.f ? Danio / Tiempo : 0.f; }
	};

	/** Una cadena encadenando en cuanto se puede: hasta EncadenarDesde de cada ataque y el último entero. */
	FTecnica Cadena(const FString& Nombre, const TArray<FSAEVUSDefinicionAtaque>& Ataques)
	{
		FTecnica Tecnica{Nombre};
		for (int32 Indice = 0; Indice < Ataques.Num(); ++Indice)
		{
			Tecnica.Danio += Ataques[Indice].Danio;
			Tecnica.Tiempo += Indice + 1 < Ataques.Num() ? Ataques[Indice].EncadenarDesde : Ataques[Indice].ObtenerTiempoTotal();
		}
		return Tecnica;
	}

	FTecnica Suelto(const FString& Nombre, const FSAEVUSDefinicionAtaque& Ataque)
	{
		return FTecnica{Nombre, Ataque.Danio, Ataque.ObtenerTiempoTotal()};
	}

	/**
	 * Ataque cargado: la pulsación saca el primer tajo, la carga crece en su recuperación y al
	 * soltar sale el giro. Ciclo = tajo hasta su fase de recuperación + carga + giro entero.
	 */
	FTecnica Cargado(const FSAEVUSRepertorioMelee& Repertorio)
	{
		const FSAEVUSDefinicionAtaque& Primero = Repertorio.ComboSuelo[0];
		const FSAEVUSDefinicionAtaque& Giro = Repertorio.AtaqueCargado;
		return FTecnica{TEXT("Ataque cargado (tajo + carga + giro)"), Primero.Danio + Giro.Danio,
			Primero.TiempoPreparacion + Primero.TiempoActivo + Repertorio.TiempoCarga + Giro.ObtenerTiempoTotal()};
	}

	/** Tajos del combo de suelo que golpean (empiezan su fase activa) dentro de una ventana. */
	int32 TajosQueCabenEn(float Ventana, const TArray<FSAEVUSDefinicionAtaque>& Combo)
	{
		int32 Tajos = 0;
		float Inicio = 0.f;
		for (const FSAEVUSDefinicionAtaque& Ataque : Combo)
		{
			if (Inicio + Ataque.TiempoPreparacion > Ventana)
			{
				break;
			}
			++Tajos;
			Inicio += Ataque.EncadenarDesde;
		}
		return Tajos;
	}

	/** Escala de tiempos más dura entre las fases que usan el ataque (1 si no hay fases). */
	float EscalaTiemposMinima(const FSAEVUSAjusteEnemigo& Enemigo, int32 IndiceAtaque)
	{
		float Escala = 1.f;
		bool bUsado = Enemigo.Fases.IsEmpty();
		for (const FSAEVUSFaseEnemigo& Fase : Enemigo.Fases)
		{
			if (Fase.Ataques.Contains(IndiceAtaque))
			{
				Escala = bUsado ? FMath::Min(Escala, Fase.EscalaTiempos) : Fase.EscalaTiempos;
				bUsado = true;
			}
		}
		return Escala;
	}

	void Linea(const FString& Texto)
	{
		UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS equilibrio] %s"), *Texto);
	}

	const FSAEVUSRepertorioMelee* CargarRepertorio(FAutomationTestBase& Prueba)
	{
		const USAEVUSDatosRepertorioMelee* Datos = LoadObject<USAEVUSDatosRepertorioMelee>(nullptr, RutaRepertorio);
		Prueba.TestNotNull(TEXT("Repertorio cuerpo a cuerpo"), Datos);
		return Datos && Prueba.TestTrue(TEXT("El combo de suelo tiene tajos"), !Datos->Repertorio.ComboSuelo.IsEmpty()) ? &Datos->Repertorio : nullptr;
	}

	float VidaJugador()
	{
		return GetDefault<ASAEVUSPersonaje>()->ObtenerVida()->ObtenerVidaMaxima();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEquilibrioTecnicas, "SAEVUS.Equilibrio.Tecnicas", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEquilibrioTecnicas::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEquilibrio;

	const FSAEVUSRepertorioMelee* Repertorio = CargarRepertorio(*this);
	if (!Repertorio)
	{
		return false;
	}
	TArray<FTecnica> Tecnicas = {
		Cadena(TEXT("Combo de suelo"), Repertorio->ComboSuelo),
		Cadena(TEXT("Combo aéreo"), Repertorio->ComboAire),
		Suelto(TEXT("Tajo hacia arriba"), Repertorio->TajoArriba),
		Suelto(TEXT("Tajo hacia abajo (sin contar el rebote)"), Repertorio->TajoAbajo),
		Suelto(TEXT("Tajo en carrera (sin contar el dash)"), Repertorio->TajoEnCarrera),
		Cargado(*Repertorio),
	};
	TArray<FString> LineasMunicion;
	for (const TCHAR* Ruta : RutasArmas)
	{
		const USAEVUSDefinicionArma* Arma = LoadObject<USAEVUSDefinicionArma>(nullptr, Ruta);
		if (!TestNotNull(Ruta, Arma))
		{
			continue;
		}
		const FSAEVUSAjusteArmaFuego& Municion = Arma->ArmaFuego;
		const int32 Balas = Municion.TamanoCargador + FMath::Max(Municion.MunicionReserva, 0);
		LineasMunicion.Add(FString::Printf(TEXT("| %s | %d | %s | %.0f | %s |"), *Arma->NombreVisible.ToString(), Municion.TamanoCargador,
			Municion.TieneReservaInfinita() ? TEXT("infinita") : *FString::FromInt(Municion.MunicionReserva),
			Municion.Danio * Municion.PerdigonesPorDisparo * Balas,
			Municion.ReservaPorGolpeCuerpoACuerpo > 0 ? *FString::FromInt(FMath::DivideAndRoundUp(FMath::Max(Municion.MunicionReserva, 0), Municion.ReservaPorGolpeCuerpoACuerpo)) : TEXT("—")));
		// Las armas ya no son una fuente infinita de daño: la reserva se gana con la espada.
		TestFalse(FString::Printf(TEXT("%s tiene la reserva limitada"), *Arma->NombreVisible.ToString()), Municion.TieneReservaInfinita());
		const FSAEVUSAjusteArmaFuego& Fuego = Arma->ArmaFuego;
		const float DanioDisparo = Fuego.Danio * Fuego.PerdigonesPorDisparo;
		const FString Nombre = Arma->NombreVisible.ToString();
		Tecnicas.Add(FTecnica{Nombre + TEXT(" (ráfaga, todos los perdigones)"), DanioDisparo, Fuego.IntervaloDisparo});
		Tecnicas.Add(FTecnica{Nombre + TEXT(" (sostenido, con recarga)"), DanioDisparo * Fuego.TamanoCargador,
			Fuego.IntervaloDisparo * Fuego.TamanoCargador + Fuego.TiempoRecarga});
	}

	Linea(TEXT("| Técnica | Daño por ciclo | Ciclo (s) | Daño/s |"));
	Linea(TEXT("|---|---|---|---|"));
	for (const FTecnica& Tecnica : Tecnicas)
	{
		Linea(FString::Printf(TEXT("| %s | %.0f | %.2f | %.1f |"), *Tecnica.Nombre, Tecnica.Danio, Tecnica.Tiempo, Tecnica.DanioPorSegundo()));
		TestTrue(FString::Printf(TEXT("%s hace daño"), *Tecnica.Nombre), Tecnica.DanioPorSegundo() > 0.f);
	}

	Linea(TEXT("| Arma | Cargador | Reserva | Daño con todo lleno | Golpes de espada para llenar la reserva |"));
	Linea(TEXT("|---|---|---|---|---|"));
	for (const FString& Fila : LineasMunicion)
	{
		Linea(Fila);
	}

	// El giro cargado compromete más, así que cada golpe suyo vale más que cualquier tajo del combo.
	for (const FSAEVUSDefinicionAtaque& Tajo : Repertorio->ComboSuelo)
	{
		TestTrue(FString::Printf(TEXT("El giro cargado pega más que %s"), *Tajo.Nombre.ToString()), Repertorio->AtaqueCargado.Danio > Tajo.Danio);
	}
	// El tajo en carrera premia el dash: pega más que el primer tajo del combo.
	TestTrue(TEXT("El tajo en carrera pega más que el primer tajo"), Repertorio->TajoEnCarrera.Danio > Repertorio->ComboSuelo[0].Danio);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEquilibrioEnemigos, "SAEVUS.Equilibrio.Enemigos", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEquilibrioEnemigos::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEquilibrio;

	const FSAEVUSRepertorioMelee* Repertorio = CargarRepertorio(*this);
	if (!Repertorio)
	{
		return false;
	}
	const FTecnica Combo = Cadena(TEXT("Combo de suelo"), Repertorio->ComboSuelo);
	const float DanioPrimerTajo = Repertorio->ComboSuelo[0].Danio;
	const float Vida = VidaJugador();
	Linea(FString::Printf(TEXT("Vida del jugador: %.0f"), Vida));

	Linea(TEXT("| Enemigo | Vida | Primeros tajos | Combos de suelo | Tiempo con el combo (s) |"));
	Linea(TEXT("|---|---|---|---|---|"));
	TArray<TPair<FString, const FSAEVUSAjusteEnemigo*>> Enemigos;
	for (const TCHAR* Ruta : RutasEnemigos)
	{
		const USAEVUSDatosEnemigo* Datos = LoadObject<USAEVUSDatosEnemigo>(nullptr, Ruta);
		if (!TestNotNull(Ruta, Datos))
		{
			continue;
		}
		const FSAEVUSAjusteEnemigo& Enemigo = Datos->Enemigo;
		const FString Nombre = Enemigo.NombreVisible.ToString();
		Enemigos.Emplace(Nombre, &Enemigo);
		const int32 Combos = FMath::CeilToInt32(Enemigo.VidaMaxima / Combo.Danio);
		Linea(FString::Printf(TEXT("| %s | %.0f | %d | %d | %.1f |"), *Nombre, Enemigo.VidaMaxima,
			FMath::CeilToInt32(Enemigo.VidaMaxima / DanioPrimerTajo), Combos, Enemigo.VidaMaxima / Combo.DanioPorSegundo()));
		if (Enemigo.bEsJefe)
		{
			TestTrue(FString::Printf(TEXT("%s aguanta al menos %d combos"), *Nombre, CombosMinimosJefe), Combos >= CombosMinimosJefe);
		}
		else
		{
			TestTrue(FString::Printf(TEXT("%s cae en %d combos como mucho"), *Nombre, CombosMaximosEnemigoMenor), Combos <= CombosMaximosEnemigoMenor);
		}
	}

	Linea(TEXT("| Enemigo | Ataque | Aviso más corto (s) | Daño | Golpes para tumbar al jugador | Castigo (s) | Tajos que caben |"));
	Linea(TEXT("|---|---|---|---|---|---|---|"));
	for (const TPair<FString, const FSAEVUSAjusteEnemigo*>& Par : Enemigos)
	{
		const FSAEVUSAjusteEnemigo& Enemigo = *Par.Value;
		const int32 GolpesContacto = FMath::CeilToInt32(Vida / FMath::Max(Enemigo.DanioContacto, KINDA_SMALL_NUMBER));
		Linea(FString::Printf(TEXT("| %s | contacto | — | %.0f | %d | — | — |"), *Par.Key, Enemigo.DanioContacto, GolpesContacto));
		TestTrue(FString::Printf(TEXT("Hacen falta %d contactos de %s para tumbar al jugador"), GolpesMinimosParaCaer, *Par.Key), GolpesContacto >= GolpesMinimosParaCaer);

		for (int32 Indice = 0; Indice < Enemigo.Ataques.Num(); ++Indice)
		{
			const FSAEVUSAtaqueEnemigo& Ataque = Enemigo.Ataques[Indice];
			const float Escala = EscalaTiemposMinima(Enemigo, Indice);
			const float Aviso = Ataque.TiempoTelegrafiar * Escala;
			const float Castigo = Ataque.TiempoRecuperacion * Escala;
			const int32 Golpes = FMath::CeilToInt32(Vida / FMath::Max(Ataque.Danio, KINDA_SMALL_NUMBER));
			const int32 Tajos = TajosQueCabenEn(Castigo, Repertorio->ComboSuelo);
			const FString Nombre = FString::Printf(TEXT("%s, %s"), *Par.Key, *Ataque.Nombre.ToString());
			Linea(FString::Printf(TEXT("| %s | %s | %.2f | %.0f | %d | %.2f | %d |"), *Par.Key, *Ataque.Nombre.ToString(), Aviso, Ataque.Danio, Golpes, Castigo, Tajos));
			TestTrue(FString::Printf(TEXT("%s: aviso de al menos %.2f s"), *Nombre, AvisoMinimo), Aviso >= AvisoMinimo);
			TestTrue(FString::Printf(TEXT("%s: hacen falta %d golpes para tumbar al jugador"), *Nombre, GolpesMinimosParaCaer), Golpes >= GolpesMinimosParaCaer);
			TestTrue(FString::Printf(TEXT("%s: cabe un castigo"), *Nombre), Tajos >= TajosMinimosEnCastigo);
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
