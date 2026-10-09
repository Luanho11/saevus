// Copyright SAEVUS. Todos los derechos reservados.

#include "Animacion/SAEVUSCapaSecundaria.h"

namespace SAEVUSSecundaria
{
	// Muelles auxiliares: ladeo de las curvas y adelanto de la cabeza llegan sin rebotar casi.
	constexpr float FrecuenciaLadeo = 3.f;
	constexpr float AmortiguacionLadeo = 0.75f;
	constexpr float AmortiguacionCabeza = 0.8f;
	// En una acción la cadera alcanza el giro mucho antes.
	constexpr float AceleracionCaderaEnAccion = 3.f;
	// Corriendo, las piernas siguen el giro antes que quieto.
	constexpr float AceleracionCaderaCorriendo = 1.2f;
	// Respiración más corta corriendo (el pecho ya va con la zancada).
	constexpr float RespiracionEnMovimiento = 0.45f;
	// El reposo vivo entra poco a poco tras pararse.
	constexpr float EsperaQuietud = 0.3f;
	constexpr float RampaQuietud = 1.2f;
	// Por debajo de esta fracción de la velocidad de referencia se está quieto.
	constexpr float FraccionQuieto = 0.03f;
	// Los gestos de reposo entran y salen suaves en este tramo de su duración.
	constexpr float BordeGestoReposo = 0.2f;
	// Un giro de guiñada mayor que esto en un fotograma es una rotación forzada, no el jugador girando (grados).
	constexpr float MaximoGiroPorFotograma = 100.f;

	float Suavizar(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return X * X * (3.f - 2.f * X);
	}
}

void FSAEVUSCapaSecundaria::Reiniciar()
{
	*this = FSAEVUSCapaSecundaria();
}

void FSAEVUSCapaSecundaria::Aterrizar(const FSAEVUSAjusteSecundario& Ajuste, float Fuerza)
{
	Rebote.Impulsar(Ajuste.ImpulsoAterrizaje * FMath::Max(Fuerza, 0.f));
	// El tronco sigue hacia delante y abajo con la caída.
	Inercia.Impulsar(FVector(Ajuste.ImpulsoAterrizaje * 0.12f * Fuerza, 0.f, 0.f));
}

void FSAEVUSCapaSecundaria::Empujar(const FVector2D& GiroTronco, float Hundimiento)
{
	Inercia.Impulsar(FVector(GiroTronco.X, GiroTronco.Y, 0.f));
	Rebote.Impulsar(Hundimiento);
}

void FSAEVUSCapaSecundaria::Actualizar(const FSAEVUSAjusteSecundario& Ajuste, const FSAEVUSEntradaSecundaria& Entrada)
{
	using namespace SAEVUSSecundaria;
	const float Dt = Entrada.DeltaTime;
	if (Dt <= 0.f)
	{
		return;
	}
	// Un cuerpo más pesado responde más despacio: los muelles bajan de frecuencia.
	const float Lentitud = 1.f / FMath::Sqrt(FMath::Max(Entrada.Peso, 0.1f));
	Tiempo += Dt;

	// Ejes del cuerpo en el mundo (UE: X adelante, Y derecha; la guiñada positiva gira hacia la derecha).
	const float GuinadaRad = FMath::DegreesToRadians(Entrada.Guinada);
	const FVector2D Adelante(FMath::Cos(GuinadaRad), FMath::Sin(GuinadaRad));
	const FVector2D Derecha(-Adelante.Y, Adelante.X);
	const FVector2D VelocidadCuerpo(FVector2D::DotProduct(Entrada.Velocidad, Derecha), FVector2D::DotProduct(Entrada.Velocidad, Adelante));
	const float Rapidez = static_cast<float>(Entrada.Velocidad.Size());
	const float Avance = FMath::Clamp(Rapidez / FMath::Max(Entrada.VelocidadReferencia, 1.f), 0.f, 1.5f);

	float DeltaGuinada = bIniciada ? FMath::FindDeltaAngleDegrees(GuinadaAnterior, Entrada.Guinada) : 0.f;
	// Un giro de golpe mayor que esto no es el jugador girando sino una rotación forzada (sentarse, salir de un portal,
	// reaparecer): se ignora, si no la cadera quedaba retorcida y luego se «desenroscaba» (ANIMVFX-23).
	if (FMath::Abs(DeltaGuinada) > MaximoGiroPorFotograma)
	{
		DeltaGuinada = 0.f;
	}
	const float VelocidadGiro = DeltaGuinada / Dt;
	// Aceleración en el mundo, pasada a los ejes del cuerpo: girar la cámara corriendo de lado no es acelerar.
	const FVector2D Aceleracion = bIniciada ? (Entrada.Velocidad - VelocidadAnterior) / Dt : FVector2D::ZeroVector;
	const FVector2D AceleracionCuerpo(FVector2D::DotProduct(Aceleracion, Derecha), FVector2D::DotProduct(Aceleracion, Adelante));
	GuinadaAnterior = Entrada.Guinada;
	VelocidadAnterior = Entrada.Velocidad;
	bIniciada = true;

	const bool bQuieto = Avance < FraccionQuieto;
	TiempoQuieto = Entrada.bReposoLibre && bQuieto ? TiempoQuieto + Dt : 0.f;
	const float Quietud = Suavizar((TiempoQuieto - EsperaQuietud) / RampaQuietud);

	// --- Giro por capas. La cadera se queda donde estaba en el mundo y el muelle la trae; la cabeza se adelanta. ---
	RetrasoCadera.Valor = FMath::Clamp(RetrasoCadera.Valor - DeltaGuinada, -Ajuste.MaxRetrasoCadera, Ajuste.MaxRetrasoCadera);
	TiempoPasoGiro = FMath::Max(0.f, TiempoPasoGiro - Dt);
	if (TiempoPasoGiro <= 0.f && bQuieto && Entrada.bEnSuelo && !Entrada.bAccion && FMath::Abs(RetrasoCadera.Valor) > Ajuste.UmbralPasoGiro)
	{
		// Quieto y muy torcido: los pies dan un paso hacia donde mira el cuerpo.
		TiempoPasoGiro = Ajuste.DuracionPasoGiro * (1.f / Lentitud);
		LadoPasoGiro = RetrasoCadera.Valor < 0.f ? 1.f : -1.f;
	}
	float FrecuenciaCadera = (TiempoPasoGiro > 0.f ? Ajuste.FrecuenciaPasoGiro : Ajuste.FrecuenciaCadera) * (1.f + AceleracionCaderaCorriendo * FMath::Min(Avance, 1.f));
	if (Entrada.bAccion)
	{
		FrecuenciaCadera *= AceleracionCaderaEnAccion;
	}
	// En el aire las piernas no tienen dónde apoyarse para girar: la cadera va con el cuerpo casi sin retraso.
	if (!Entrada.bEnSuelo)
	{
		FrecuenciaCadera *= AceleracionCaderaEnAccion;
	}
	RetrasoCadera.Avanzar(0.f, FrecuenciaCadera * Lentitud, Ajuste.AmortiguacionCadera, Dt);
	AdelantoCabeza.Avanzar(FMath::Clamp(VelocidadGiro * Ajuste.AdelantoCabeza, -Ajuste.MaxAdelantoCabeza, Ajuste.MaxAdelantoCabeza),
		Ajuste.FrecuenciaCabeza * Lentitud, AmortiguacionCabeza, Dt);

	const float Retraso = RetrasoCadera.Valor * (Entrada.bAccion ? Ajuste.FraccionRetrasoEnAccion : 1.f);
	const float RetrasoPecho = Retraso * Ajuste.FraccionRetrasoPecho;
	Salida.GiroCadera = Retraso;
	// El tronco reparte la vuelta entre la cadera y el pecho: la columna lleva más.
	Salida.GiroColumna = (RetrasoPecho - Retraso) * 0.6f;
	Salida.GiroPecho = (RetrasoPecho - Retraso) * 0.4f;
	const float GiroCabezaMundo = AdelantoCabeza.Valor - RetrasoPecho;
	Salida.GiroCuello = GiroCabezaMundo * 0.4f;
	Salida.GiroCabeza = GiroCabezaMundo * 0.6f;
	Salida.PasoGiro = TiempoPasoGiro > 0.f ? 1.f - TiempoPasoGiro / FMath::Max(Ajuste.DuracionPasoGiro / Lentitud, UE_KINDA_SMALL_NUMBER) : 0.f;
	Salida.LadoPasoGiro = LadoPasoGiro;

	// --- Ladeo: hacia dentro de las curvas y hacia donde se corre de lado; el vaivén del peso, quieto. ---
	const float ObjetivoCurva = Entrada.bEnSuelo
		? FMath::Clamp(VelocidadGiro * Ajuste.LadeoCurva * FMath::Min(Avance, 1.2f), -Ajuste.MaxLadeoCurva, Ajuste.MaxLadeoCurva) : 0.f;
	LadeoCurva.Avanzar(ObjetivoCurva, FrecuenciaLadeo * Lentitud, AmortiguacionLadeo, Dt);
	const float Lateral = Entrada.bEnSuelo
		? Ajuste.LadeoLateral * FMath::Clamp(static_cast<float>(VelocidadCuerpo.X) / FMath::Max(Entrada.VelocidadReferencia, 1.f), -1.f, 1.f) : 0.f;
	Salida.VaivenPeso = Quietud * FMath::Sin(UE_TWO_PI * Ajuste.FrecuenciaVaivenPeso * Lentitud * Tiempo + Desfase);
	Salida.Ladeo = LadeoCurva.Valor + Lateral + Salida.VaivenPeso * Ajuste.LadeoPeso;

	// --- Inercia del tronco: frena el cuerpo y el tronco sigue; arranca y el tronco se queda atrás. ---
	const FVector2D CambioCuerpo = AceleracionCuerpo * Dt;
	Inercia.Impulsar(FVector(-CambioCuerpo.Y, -CambioCuerpo.X, 0.f) * Ajuste.EscalaInercia);
	Inercia.Avanzar(FVector::ZeroVector, Ajuste.FrecuenciaInercia * Lentitud, Ajuste.AmortiguacionInercia, Dt);
	Inercia.Valor = Inercia.Valor.GetClampedToMaxSize(Ajuste.MaxInercia);
	Salida.InerciaTronco = FVector2D(Inercia.Valor.X, Inercia.Valor.Y);
	Salida.InerciaCabeza = -Salida.InerciaTronco * Ajuste.EstabilizacionCabeza;

	// --- Rebote de la cadera. ---
	Rebote.Avanzar(0.f, Ajuste.FrecuenciaRebote * Lentitud, Ajuste.AmortiguacionRebote, Dt);
	Rebote.Valor = FMath::Clamp(Rebote.Valor, -Ajuste.MaxRebote * 0.5f, Ajuste.MaxRebote);
	Salida.Rebote = Rebote.Valor;

	// --- Respiración: el cansancio la acelera y la hace más honda. ---
	Cansancio = FMath::Clamp(Cansancio + (Entrada.bEsfuerzo ? Ajuste.SubidaCansancio : -Ajuste.BajadaCansancio) * Dt, 0.f, 1.f);
	FaseRespiracion += FMath::Lerp(Ajuste.FrecuenciaRespiracionCalma, Ajuste.FrecuenciaRespiracionAgitada, Cansancio) * Lentitud * Dt * UE_TWO_PI;
	const float Amplitud = FMath::Lerp(Ajuste.AmplitudRespiracionCalma, Ajuste.AmplitudRespiracionAgitada, Cansancio)
		* FMath::Lerp(1.f, RespiracionEnMovimiento, FMath::Min(Avance, 1.f));
	// Inspirar es más corto que soltar el aire: la onda se afila hacia arriba.
	const float Onda = FMath::Sin(FaseRespiracion + Desfase);
	Salida.Respiracion = Amplitud * (Onda > 0.f ? FMath::Pow(Onda, 0.7f) : Onda);
	Salida.Cansancio = Cansancio;

	// --- Mirada ociosa: ruido lento, solo quieto. ---
	const float Ruido = Tiempo * Ajuste.RitmoMiradaOciosa * Lentitud;
	Salida.MiradaOciosa = Quietud * FVector2D(FMath::PerlinNoise1D(Ruido + Desfase) * Ajuste.GiroMiradaOciosa,
		FMath::PerlinNoise1D(Ruido * 0.7f + Desfase + 17.3f) * Ajuste.CabeceoMiradaOciosa);

	// --- Gestos de reposo, por turno, tras un rato quieto. ---
	if (!Entrada.bReposoLibre || !bQuieto)
	{
		GestoReposo = ESAEVUSGestoReposo::Ninguno;
		TiempoGestoReposo = 0.f;
	}
	else if (GestoReposo == ESAEVUSGestoReposo::Ninguno && TiempoQuieto >= Ajuste.TiempoGestoReposo * FactorEsperaGesto)
	{
		// Al azar, pero nunca el mismo gesto dos veces seguidas.
		ESAEVUSGestoReposo Elegido = static_cast<ESAEVUSGestoReposo>(Azar.RandRange(1, 3));
		if (Elegido == UltimoGestoReposo)
		{
			Elegido = static_cast<ESAEVUSGestoReposo>(static_cast<uint8>(Elegido) % 3 + 1);
		}
		GestoReposo = Elegido;
		UltimoGestoReposo = Elegido;
		TiempoGestoReposo = 0.f;
	}
	if (GestoReposo != ESAEVUSGestoReposo::Ninguno)
	{
		TiempoGestoReposo += Dt;
		if (TiempoGestoReposo >= Ajuste.DuracionGestoReposo)
		{
			GestoReposo = ESAEVUSGestoReposo::Ninguno;
			// El siguiente gesto, otro rato después.
			TiempoQuieto = EsperaQuietud + RampaQuietud;
			FactorEsperaGesto = 1.f + Azar.FRandRange(-Ajuste.VariacionEsperaGesto, Ajuste.VariacionEsperaGesto);
		}
	}
	Salida.GestoReposo = GestoReposo;
	Salida.AvanceGestoReposo = GestoReposo != ESAEVUSGestoReposo::Ninguno ? FMath::Clamp(TiempoGestoReposo / Ajuste.DuracionGestoReposo, 0.f, 1.f) : 0.f;
}
