// Copyright SAEVUS. Todos los derechos reservados.

// Animación de los enemigos con esqueleto (goblins y Rey Goblin, REF-05/REF-06): la misma locomoción y capa
// secundaria que el protagonista, encorvada y nerviosa, y un gesto por ataque (aviso, golpe y recuperación).

#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/PoseableMeshComponent.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace SAEVUSAnimacionEnemigo
{
	const FVector Adelante(0.f, 1.f, 0.f);
	const FVector Derecha(-1.f, 0.f, 0.f);
	const FVector Arriba(0.f, 0.f, 1.f);

	FQuat Girar(const FVector& Desde, const FVector& Hacia, float Grados)
	{
		const FVector Eje = FVector::CrossProduct(Desde, Hacia).GetSafeNormal();
		return Eje.IsNearlyZero() ? FQuat::Identity : FQuat(Eje, FMath::DegreesToRadians(Grados));
	}

	FVector Dir(const FVector& Desde, const FVector& Hacia, float Grados)
	{
		return Girar(Desde, Hacia, Grados).RotateVector(Desde);
	}

	FVector Interpolar(const FVector& A, const FVector& B, float Alfa)
	{
		const FQuat Total = FQuat::FindBetweenNormals(A.GetSafeNormal(), B.GetSafeNormal());
		return FQuat::Slerp(FQuat::Identity, Total, FMath::Clamp(Alfa, 0.f, 1.f)).RotateVector(A.GetSafeNormal());
	}

	/** Ejes del cuerpo (X = derecha, Y = adelante, Z = arriba) al espacio de la malla. */
	FVector Cuerpo(float X, float Y, float Z)
	{
		return (Derecha * X + Adelante * Y + Arriba * Z).GetSafeNormal();
	}

	float Suavizar(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return X * X * (3.f - 2.f * X);
	}

	float SalidaRapida(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return 1.f - (1.f - X) * (1.f - X) * (1.f - X);
	}

	/** Una postura del brazo del arma, la hoja, el brazo libre y el tronco; se mezclan entre sí. */
	struct FPostura
	{
		FVector Brazo;
		FVector Arma;
		FVector Libre;
		float Codo = 30.f;
		float Giro = 0.f;
		float Inclinacion = 0.f;
		float Bajada = 0.f;
		/** Zancada (0–1): pie derecho delante, izquierdo atrás. */
		float Zancada = 0.f;
	};

	FPostura Mezclar(const FPostura& A, const FPostura& B, float Alfa)
	{
		return { Interpolar(A.Brazo, B.Brazo, Alfa), Interpolar(A.Arma, B.Arma, Alfa), Interpolar(A.Libre, B.Libre, Alfa),
			FMath::Lerp(A.Codo, B.Codo, Alfa), FMath::Lerp(A.Giro, B.Giro, Alfa), FMath::Lerp(A.Inclinacion, B.Inclinacion, Alfa),
			FMath::Lerp(A.Bajada, B.Bajada, Alfa), FMath::Lerp(A.Zancada, B.Zancada, Alfa) };
	}

	/** Lista: el arma baja y adelantada, el brazo libre colgando abierto como una garra. */
	const FPostura Lista = { Cuerpo(0.35f, 0.3f, -0.85f), Cuerpo(0.2f, 0.75f, -0.6f), Cuerpo(-0.35f, 0.2f, -0.9f), 30.f, 0.f, 0.f, 0.f, 0.f };

	/** Aviso (lo que lee el jugador) y golpe de cada gesto. */
	void Gesto(ESAEVUSGestoEnemigo Gesto, FPostura& OutAviso, FPostura& OutGolpe)
	{
		switch (Gesto)
		{
		case ESAEVUSGestoEnemigo::Mazazo:
			// A dos manos por encima de la cabeza, arqueado atrás; abajo con todo el cuerpo.
			OutAviso = { Cuerpo(0.1f, -0.3f, 0.95f), Cuerpo(0.f, -0.7f, 0.7f), Cuerpo(-0.1f, -0.3f, 0.95f), 70.f, 0.f, -18.f, -4.f, 0.5f };
			OutGolpe = { Cuerpo(0.05f, 0.6f, -0.8f), Cuerpo(0.f, 0.6f, -0.8f), Cuerpo(-0.05f, 0.6f, -0.8f), 10.f, 0.f, 38.f, 22.f, 1.f };
			break;
		case ESAEVUSGestoEnemigo::Manotazo:
			// La garra libre atrás, el tronco enroscado; revés que barre por delante.
			OutAviso = { Cuerpo(0.4f, 0.4f, -0.6f), Cuerpo(0.3f, 0.6f, -0.5f), Cuerpo(-0.8f, -0.45f, 0.2f), 40.f, -40.f, 5.f, 8.f, 0.4f };
			OutGolpe = { Cuerpo(0.5f, -0.2f, -0.6f), Cuerpo(0.4f, 0.2f, -0.6f), Cuerpo(0.7f, 0.65f, 0.f), 30.f, 45.f, 12.f, 12.f, 0.8f };
			break;
		case ESAEVUSGestoEnemigo::Estocada:
			// El arma recogida junto a la cadera, la punta al frente; sale recta con una zancada larga.
			OutAviso = { Cuerpo(0.4f, -0.7f, -0.1f), Cuerpo(0.f, 1.f, 0.f), Cuerpo(-0.3f, 0.8f, 0.1f), 95.f, 30.f, 10.f, 10.f, 0.5f };
			OutGolpe = { Cuerpo(0.05f, 1.f, 0.f), Cuerpo(0.f, 1.f, 0.f), Cuerpo(-0.6f, -0.6f, -0.3f), 0.f, -15.f, 28.f, 20.f, 1.f };
			break;
		case ESAEVUSGestoEnemigo::Lanzamiento:
			// Brazo atrás por encima del hombro, la garra apunta; lanza con un paso.
			OutAviso = { Cuerpo(0.4f, -0.6f, 0.65f), Cuerpo(0.2f, -0.8f, 0.5f), Cuerpo(-0.3f, 0.9f, 0.2f), 80.f, 35.f, -6.f, 6.f, 0.5f };
			OutGolpe = { Cuerpo(-0.3f, 0.7f, -0.5f), Cuerpo(-0.2f, 0.6f, -0.7f), Cuerpo(-0.5f, -0.5f, -0.4f), 10.f, -30.f, 20.f, 10.f, 1.f };
			break;
		case ESAEVUSGestoEnemigo::Embestida:
			// Muy agachado y echado hacia delante, el arma por delante: va a cargar.
			OutAviso = { Cuerpo(0.3f, 0.8f, -0.3f), Cuerpo(0.2f, 1.f, -0.15f), Cuerpo(-0.4f, -0.4f, -0.6f), 40.f, 10.f, 35.f, 25.f, 0.6f };
			OutGolpe = { Cuerpo(0.25f, 0.9f, -0.1f), Cuerpo(0.1f, 1.f, 0.f), Cuerpo(-0.5f, -0.6f, -0.4f), 20.f, 0.f, 40.f, 15.f, 0.f };
			break;
		case ESAEVUSGestoEnemigo::Salto:
			// Encogido con los brazos atrás para impulsarse; arriba con el arma por encima de la cabeza.
			OutAviso = { Cuerpo(0.3f, -0.6f, -0.6f), Cuerpo(0.2f, -0.8f, -0.4f), Cuerpo(-0.3f, -0.6f, -0.6f), 20.f, 0.f, 25.f, 30.f, 0.f };
			OutGolpe = { Cuerpo(0.1f, -0.2f, 0.95f), Cuerpo(0.f, -0.6f, 0.8f), Cuerpo(-0.4f, 0.3f, 0.8f), 60.f, 0.f, -10.f, 0.f, 0.f };
			break;
		case ESAEVUSGestoEnemigo::Tajo:
		case ESAEVUSGestoEnemigo::Automatico:
		default:
			// Arma alzada atrás por encima del hombro y tajo cruzado hacia delante y abajo (REF-05).
			OutAviso = { Cuerpo(0.45f, -0.35f, 0.75f), Cuerpo(0.3f, -0.6f, 0.6f), Cuerpo(-0.4f, 0.6f, -0.3f), 45.f, 30.f, -5.f, 10.f, 0.6f };
			OutGolpe = { Cuerpo(-0.35f, 0.75f, -0.45f), Cuerpo(-0.35f, 0.75f, -0.45f), Cuerpo(-0.5f, -0.4f, -0.5f), 5.f, -25.f, 15.f, 12.f, 1.f };
			break;
		}
	}

	// Ritmos del enemigo: tiembla al final del aviso y respira hondo al recuperarse.
	constexpr float FrecuenciaTemblorAviso = 38.f;
	constexpr float TemblorAviso = 4.f;
	constexpr float FrecuenciaTembleque = 11.f;
	constexpr float TemblequeAturdido = 10.f;
	constexpr float DuracionRugido = 1.2f;
	// Los goblins miran a su alrededor a tirones (ruido rápido); el jefe con calma.
	constexpr float RitmoTirones = 1.6f;
	constexpr float GiroTirones = 16.f;
	// Pasos del jefe: cuánto más hunde la cadera en cada pisada por unidad de peso de más.
	constexpr float PisadaPorPeso = 3.f;
}

void USAEVUSComponenteAnimacionProcedural::ConstruirPoseEnemigo(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacionEnemigo;
	auto Indice = [](ESAEVUSHueso Hueso) { return static_cast<int32>(Hueso); };
	TiempoReaccionGolpe = FMath::Max(0.f, TiempoReaccionGolpe - DeltaTime);
	const FVector Velocidad = VelocidadDueno();
	const ESAEVUSEstadoEnemigo Estado = IA->ObtenerEstado();
	if (Estado == ESAEVUSEstadoEnemigo::Muerto || (Vida && Vida->EstaMuerto()))
	{
		PoseMuerte(Pose, DeltaTime);
		return;
	}
	TiempoMuerto = 0.f;

	// Aterrizaje (saltos de ataque): la cadera cede, más cuanto más pesa.
	const bool bEnAire = MovimientoBase && MovimientoBase->IsFalling();
	if (bEstabaEnAire && !bEnAire)
	{
		CapaSecundaria.Aterrizar(Ajuste.Secundario, Peso);
	}
	bEstabaEnAire = bEnAire;

	// Base: la locomoción de siempre con el encorvado de la especie; la cabeza lo compensa para seguir
	// mirando al frente (REF-05). El jefe pisa fuerte: la cadera se hunde más en cada paso.
	const float Rapidez = static_cast<float>(Velocidad.Size2D());
	PoseLocomocion(Pose, Rapidez, DeltaTime);
	const float Paso = FMath::Abs(FMath::Sin(FaseCiclo));
	Pose.DesplazamientoRaiz.Z -= PisadaPorPeso * FMath::Max(0.f, Peso - 1.f) * (1.f - Paso) * FMath::Min(1.f, Rapidez / Ajuste.VelocidadReferencia);
	FQuat& GiroColumna = Pose.Huesos[Indice(ESAEVUSHueso::Columna)].Giro;
	GiroColumna = GiroColumna * Girar(Arriba, Adelante, Ajuste.EncorvadoEnemigo);
	FQuat& GiroCabeza = Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)].Giro;
	GiroCabeza = Girar(Arriba, -Adelante, Ajuste.EncorvadoEnemigo * 0.8f) * GiroCabeza;
	// Nervioso: mira a su alrededor a tirones cuando no ataca (el jefe, más despacio).
	if (Estado == ESAEVUSEstadoEnemigo::Reposo || Estado == ESAEVUSEstadoEnemigo::Perseguir)
	{
		const float Tiron = FMath::PerlinNoise1D(TiempoTotal * RitmoTirones / Peso + static_cast<float>(GetOwner()->GetUniqueID() % 13));
		GiroCabeza = Girar(Adelante, Derecha, GiroTirones * Tiron) * GiroCabeza;
	}

	const FSAEVUSAtaqueEnemigo* Ataque = IA->ObtenerAtaqueActual();
	FPostura Aviso;
	FPostura Golpe;
	Gesto(Ataque ? Ataque->ObtenerGesto() : ESAEVUSGestoEnemigo::Tajo, Aviso, Golpe);
	const float Tiempo = IA->ObtenerTiempoEstado();
	auto Aplicar = [&](const FPostura& Postura, float Fuerza)
	{
		auto Mezcla = [&](ESAEVUSHueso Hueso, const FVector& Hacia)
		{
			FObjetivo& Objetivo = Pose.Huesos[Indice(Hueso)];
			Objetivo = { true, Interpolar(Objetivo.bApuntar ? Objetivo.Direccion : Hacia, Hacia, Fuerza) };
		};
		Mezcla(ESAEVUSHueso::BrazoD, Postura.Brazo);
		Mezcla(ESAEVUSHueso::Arma, Postura.Arma);
		Mezcla(ESAEVUSHueso::BrazoI, Postura.Libre);
		Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, Postura.Codo * Fuerza) };
		GiroColumna = GiroColumna * Girar(Adelante, Derecha, Postura.Giro * Fuerza) * Girar(Arriba, Adelante, Postura.Inclinacion * Fuerza);
		// La cabeza no pierde de vista al objetivo aunque el tronco se tuerza.
		GiroCabeza = Girar(Adelante, Derecha, -Postura.Giro * 0.7f * Fuerza) * Girar(Arriba, Adelante, -Postura.Inclinacion * 0.6f * Fuerza) * GiroCabeza;
		Pose.DesplazamientoRaiz.Z -= Postura.Bajada * Fuerza;
		const float Zancada = Postura.Zancada * Fuerza;
		if (Zancada > 0.f && !bEnAire)
		{
			Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, Dir(-Arriba, Adelante, 32.f * Zancada) };
			Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, Dir(-Arriba, -Adelante, 22.f * Zancada) };
			Pose.Huesos[Indice(ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 45.f * Zancada) };
			Pose.Huesos[Indice(ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 25.f * Zancada) };
		}
	};

	switch (Estado)
	{
	case ESAEVUSEstadoEnemigo::Telegrafiar:
	{
		// Anticipación que se lee de lejos; al final tiembla de tensión, justo antes de soltar el golpe.
		const float P = Suavizar(IA->ObtenerProgresoAviso());
		Aplicar(Mezclar(Lista, Aviso, P), 1.f);
		const float Tension = FMath::Pow(P, 4.f) * TemblorAviso;
		GiroColumna = GiroColumna * Girar(Adelante, Derecha, Tension * FMath::Sin(TiempoTotal * FrecuenciaTemblorAviso));
		break;
	}
	case ESAEVUSEstadoEnemigo::Atacar:
	{
		// El golpe sale disparado. El salto: arriba con el arma alzada y, al caer, el golpe abajo.
		float U = SalidaRapida(Tiempo / FMath::Max(Ataque ? Ataque->Duracion * 0.5f : 0.f, UE_KINDA_SMALL_NUMBER));
		const ESAEVUSGestoEnemigo GestoActual = Ataque ? Ataque->ObtenerGesto() : ESAEVUSGestoEnemigo::Tajo;
		if (GestoActual == ESAEVUSGestoEnemigo::Salto && !bEnAire && Tiempo > 0.1f)
		{
			// Ya en el suelo: el mazazo final.
			FPostura Mazazo;
			FPostura Abajo;
			SAEVUSAnimacionEnemigo::Gesto(ESAEVUSGestoEnemigo::Mazazo, Mazazo, Abajo);
			Aplicar(Abajo, 1.f);
			break;
		}
		Aplicar(Mezclar(Aviso, Golpe, U), 1.f);
		if (bEnAire)
		{
			for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
			{
				Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Adelante, 55.f) };
			}
			for (const ESAEVUSHueso PiernaHueso : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
			{
				Pose.Huesos[Indice(PiernaHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 90.f) };
			}
		}
		break;
	}
	case ESAEVUSEstadoEnemigo::Recuperarse:
	{
		// Ventana de castigo: el golpe se queda abajo, jadea y vuelve despacio a la guardia.
		const float U = Suavizar(Tiempo / FMath::Max(Ataque ? Ataque->TiempoRecuperacion : 0.f, UE_KINDA_SMALL_NUMBER));
		const ESAEVUSGestoEnemigo GestoActual = Ataque ? Ataque->ObtenerGesto() : ESAEVUSGestoEnemigo::Tajo;
		FPostura Final = Golpe;
		if (GestoActual == ESAEVUSGestoEnemigo::Salto)
		{
			FPostura Mazazo;
			SAEVUSAnimacionEnemigo::Gesto(ESAEVUSGestoEnemigo::Mazazo, Mazazo, Final);
		}
		Aplicar(Mezclar(Final, Lista, U), 1.f);
		break;
	}
	case ESAEVUSEstadoEnemigo::Aturdido:
	{
		// Retroceso tambaleante: tronco y cabeza atrás, bamboleo que se apaga, brazos abiertos, rodillas que ceden.
		const float Apagado = FMath::Exp(-Tiempo * 4.f);
		const float Bamboleo = TemblequeAturdido * Apagado * FMath::Sin(Tiempo * FrecuenciaTembleque);
		GiroColumna = GiroColumna * Girar(Arriba, -Adelante, Ajuste.InclinacionAturdidoEnemigo) * Girar(Arriba, Derecha, Bamboleo);
		GiroCabeza = Girar(Arriba, -Adelante, Ajuste.InclinacionAturdidoEnemigo) * Girar(Adelante, Derecha, -Bamboleo * 1.5f) * GiroCabeza;
		Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Derecha * 0.7f - Arriba * 0.5f - Adelante * 0.2f).GetSafeNormal() };
		Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (-Derecha * 0.7f - Arriba * 0.5f - Adelante * 0.2f).GetSafeNormal() };
		Pose.Huesos[Indice(ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 30.f) };
		Pose.Huesos[Indice(ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 30.f) };
		Pose.DesplazamientoRaiz.Z -= Ajuste.BajadaAvisoEnemigo * 0.5f;
		break;
	}
	default:
		// Sin atacar: el arma se mece al paso y el brazo libre cuelga como una garra.
		Aplicar(Lista, 0.35f);
		break;
	}

	// Rugido al cambiar de fase (jefes): brazos abiertos, pecho arriba, cabeza atrás.
	TiempoRugido = FMath::Max(0.f, TiempoRugido - DeltaTime);
	if (TiempoRugido > 0.f)
	{
		const float U = 1.f - TiempoRugido / DuracionRugido;
		const float Campana = Suavizar(U / 0.2f) * Suavizar((1.f - U) / 0.3f);
		Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, Interpolar(Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)].Direccion, Cuerpo(0.9f, 0.1f, 0.4f), Campana) };
		Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, Interpolar(Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)].Direccion, Cuerpo(-0.9f, 0.1f, 0.4f), Campana) };
		GiroColumna = GiroColumna * Girar(Arriba, -Adelante, 25.f * Campana);
		GiroCabeza = Girar(Arriba, -Adelante, 30.f * Campana) * Girar(Adelante, Derecha, 6.f * FMath::Sin(TiempoTotal * 25.f) * Campana) * GiroCabeza;
	}

	PoseReaccionGolpe(Pose);
	const bool bAccion = Estado == ESAEVUSEstadoEnemigo::Telegrafiar || Estado == ESAEVUSEstadoEnemigo::Atacar || Estado == ESAEVUSEstadoEnemigo::Aturdido;
	const bool bEsfuerzo = bAccion || Estado == ESAEVUSEstadoEnemigo::Recuperarse || TiempoRugido > 0.f;
	AplicarSecundaria(Pose, DeltaTime, bAccion, bEsfuerzo, Estado == ESAEVUSEstadoEnemigo::Reposo && !bEnAire);
	PoseCapa(Pose, DeltaTime);
}

void USAEVUSComponenteAnimacionProcedural::AlCambiarFaseEnemigo(int32 Fase)
{
	// Una fase nueva (no la primera): ruge.
	if (Fase > 0)
	{
		TiempoRugido = SAEVUSAnimacionEnemigo::DuracionRugido;
	}
}
