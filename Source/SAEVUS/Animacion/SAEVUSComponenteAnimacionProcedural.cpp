// Copyright SAEVUS. Todos los derechos reservados.

#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Animacion/SAEVUSCinematicaInversa.h"
#include "Animacion/SAEVUSGestosAtaque.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteGuardia.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/SkinnedAsset.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Kismet/GameplayStatics.h"
#include "SAEVUS.h"

namespace SAEVUSAnimacion
{
	// Más allá de este giro (grados) el enemigo está detrás: la cabeza no lo sigue.
	constexpr float GiroMaximoParaMirar = 120.f;

	// Escala mínima de la zancada para el ciclo del paso (evita un ritmo desbocado a velocidades casi nulas).
	constexpr float EscalaZancadaMinima = 0.3f;

	// Margen sobre los límites de la malla poseable.
	constexpr float EscalaLimitesMalla = 1.6f;
	// Escala mínima de un arma "oculta": un cero exacto da una matriz no invertible (normales y sombras rotas).
	constexpr float EscalaArmaOculta = 0.001f;

	constexpr int32 NumHuesos = static_cast<int32>(ESAEVUSHueso::Total);
	static_assert(NumHuesos <= 32, "Subarbol usa una máscara de 32 bits");

	// Ejes del espacio de la malla: el personaje mira a +Y, su derecha es -X.
	const FVector Adelante(0.f, 1.f, 0.f);
	const FVector Derecha(-1.f, 0.f, 0.f);
	const FVector Arriba(0.f, 0.f, 1.f);

	// Por debajo de esta velocidad el personaje se considera quieto (respira en vez de andar).
	constexpr float VelocidadQuieto = 15.f;
	// Velocidad vertical a la que la pose aérea es "subiendo del todo" o "cayendo del todo".
	constexpr float VelocidadVerticalReferencia = 600.f;
	// Velocidades de referencia de la capa.
	constexpr float VelocidadCapaReferencia = 850.f;
	constexpr float VelocidadCaidaCapaReferencia = 1000.f;
	// Muerte: las rodillas ceden primero y luego cae el cuerpo.
	constexpr float DuracionRodillasMuerte = 0.35f;
	constexpr float InicioCaidaMuerte = 0.15f;
	constexpr float DuracionCaidaMuerte = 0.55f;
	// Por debajo de este producto con el frente, el paso va hacia atrás (la cadera gira al revés).
	constexpr float FrenteAtras = -0.2f;
	// Límites de la intensidad del aterrizaje (fracción de FlexionAterrizaje).
	constexpr float FuerzaAterrizajeMinima = 0.35f;
	constexpr float FuerzaAterrizajeMaxima = 1.3f;
	// Bajada del cuerpo durante la voltereta del dash.
	constexpr float BajadaVoltereta = 22.f;
	// Muelle de la mirada y del rebote del brazo.
	constexpr float FrecuenciaMirada = 2.6f;
	constexpr float AmortiguacionMirada = 0.85f;
	constexpr float AmortiguacionRetrocesoImpacto = 0.35f;
	// Largo aproximado de muslo y pierna frente al de la pierna entera (para doblar rodillas cuando baja la cadera).
	constexpr float FraccionMusloPierna = 0.5f;

	struct FDefinicionHueso
	{
		const TCHAR* Nombre;
		ESAEVUSHueso Padre;
		ESAEVUSHueso Hijo;          // Hijo que da su dirección de reposo; Total = hoja.
		FVector DireccionHoja;      // Dirección de reposo de las hojas (espacio de malla).
	};

	const FDefinicionHueso& Definicion(ESAEVUSHueso Hueso)
	{
		using E = ESAEVUSHueso;
		static const FDefinicionHueso Tabla[NumHuesos] = {
			{ TEXT("raiz"), E::Total, E::Pelvis, Arriba },
			{ TEXT("pelvis"), E::Raiz, E::Columna, Arriba },
			{ TEXT("columna"), E::Pelvis, E::Pecho, Arriba },
			{ TEXT("pecho"), E::Columna, E::Cuello, Arriba },
			{ TEXT("cuello"), E::Pecho, E::Cabeza, Arriba },
			{ TEXT("cabeza"), E::Cuello, E::Total, Arriba },
			{ TEXT("capa_1"), E::Pecho, E::Capa2, -Arriba },
			{ TEXT("capa_2"), E::Capa1, E::Capa3, -Arriba },
			{ TEXT("hombro_d"), E::Pecho, E::BrazoD, Derecha },
			{ TEXT("brazo_d"), E::HombroD, E::AntebrazoD, -Arriba },
			{ TEXT("antebrazo_d"), E::BrazoD, E::ManoD, -Arriba },
			{ TEXT("mano_d"), E::AntebrazoD, E::Total, -Arriba },
			{ TEXT("muslo_d"), E::Pelvis, E::PiernaD, -Arriba },
			{ TEXT("pierna_d"), E::MusloD, E::PieD, -Arriba },
			{ TEXT("pie_d"), E::PiernaD, E::Total, Adelante },
			{ TEXT("hombro_i"), E::Pecho, E::BrazoI, -Derecha },
			{ TEXT("brazo_i"), E::HombroI, E::AntebrazoI, -Arriba },
			{ TEXT("antebrazo_i"), E::BrazoI, E::ManoI, -Arriba },
			{ TEXT("mano_i"), E::AntebrazoI, E::Total, -Arriba },
			{ TEXT("muslo_i"), E::Pelvis, E::PiernaI, -Arriba },
			{ TEXT("pierna_i"), E::MusloI, E::PieI, -Arriba },
			{ TEXT("pie_i"), E::PiernaI, E::Total, Adelante },
			{ TEXT("arma"), E::ManoD, E::Total, -Arriba },
			{ TEXT("arma_fuego"), E::ManoD, E::Total, Adelante },
			{ TEXT("capa_3"), E::Capa2, E::Total, -Arriba },
		};
		return Tabla[static_cast<int32>(Hueso)];
	}

	/** Nombres de hueso como FName, creados una vez (no en cada fotograma). */
	const FName& Nombre(ESAEVUSHueso Hueso)
	{
		static const TArray<FName> Nombres = []
		{
			TArray<FName> Resultado;
			for (int32 Indice = 0; Indice < NumHuesos; ++Indice)
			{
				Resultado.Add(FName(Definicion(static_cast<ESAEVUSHueso>(Indice)).Nombre));
			}
			return Resultado;
		}();
		return Nombres[static_cast<int32>(Hueso)];
	}

	int32 Indice(ESAEVUSHueso Hueso) { return static_cast<int32>(Hueso); }

	/** Giro que lleva Desde hacia Hacia (perpendiculares) Grados grados; negativo gira al revés. */
	FQuat Girar(const FVector& Desde, const FVector& Hacia, float Grados)
	{
		const FVector Eje = FVector::CrossProduct(Desde, Hacia).GetSafeNormal();
		return Eje.IsNearlyZero() ? FQuat::Identity : FQuat(Eje, FMath::DegreesToRadians(Grados));
	}

	/** Desde girado hacia Hacia Grados grados. */
	FVector Dir(const FVector& Desde, const FVector& Hacia, float Grados)
	{
		return Girar(Desde, Hacia, Grados).RotateVector(Desde);
	}

	/** Interpolación esférica entre dos direcciones unitarias. */
	FVector Interpolar(const FVector& A, const FVector& B, float Alfa)
	{
		const FQuat Total = FQuat::FindBetweenNormals(A.GetSafeNormal(), B.GetSafeNormal());
		return FQuat::Slerp(FQuat::Identity, Total, FMath::Clamp(Alfa, 0.f, 1.f)).RotateVector(A.GetSafeNormal());
	}

	/** Dirección dada en los ejes del cuerpo (X = derecha, Y = adelante, Z = arriba) en el espacio de la malla. */
	FVector Cuerpo(const FVector& Ejes)
	{
		return (Derecha * Ejes.X + Adelante * Ejes.Y + Arriba * Ejes.Z).GetSafeNormal();
	}

	float SuavizarCurva(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return X * X * (3.f - 2.f * X);
	}

	/** Salida rápida y frenada al final: el golpe "corta" y se detiene. */
	float SalidaRapida(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return 1.f - (1.f - X) * (1.f - X) * (1.f - X);
	}

	float AlfaSuavizado(float Rapidez, float DeltaTime)
	{
		return Rapidez <= 0.f ? 1.f : 1.f - FMath::Exp(-Rapidez * DeltaTime);
	}

	/** Tramo del cuerpo de cada hueso, para el suavizado escalonado. */
	enum class ETramo : uint8 { Tronco, Cabeza, Brazo, Mano, Pierna, Capa };

	ETramo TramoDe(ESAEVUSHueso Hueso)
	{
		using E = ESAEVUSHueso;
		switch (Hueso)
		{
		case E::Cuello: case E::Cabeza:
			return ETramo::Cabeza;
		case E::HombroD: case E::BrazoD: case E::HombroI: case E::BrazoI:
			return ETramo::Brazo;
		case E::AntebrazoD: case E::ManoD: case E::AntebrazoI: case E::ManoI: case E::Arma: case E::ArmaFuego:
			return ETramo::Mano;
		case E::MusloD: case E::PiernaD: case E::PieD: case E::MusloI: case E::PiernaI: case E::PieI:
			return ETramo::Pierna;
		case E::Capa1: case E::Capa2: case E::Capa3:
			return ETramo::Capa;
		default:
			return ETramo::Tronco;
		}
	}
}

USAEVUSComponenteAnimacionProcedural::USAEVUSComponenteAnimacionProcedural()
{
	using namespace SAEVUSAnimacion;
	PrimaryComponentTick.bCanEverTick = true;
	// Después del movimiento, así anima el estado de este mismo fotograma.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		IndiceMalla[Hueso] = INDEX_NONE;
		Padre[Hueso] = INDEX_NONE;
		Subarbol[Hueso] = 0;
		RotacionActual[Hueso] = FQuat::Identity;
		PosicionActual[Hueso] = FVector::ZeroVector;
	}
}

FName USAEVUSComponenteAnimacionProcedural::NombreHueso(ESAEVUSHueso Hueso)
{
	return SAEVUSAnimacion::Nombre(Hueso);
}

void USAEVUSComponenteAnimacionProcedural::BeginPlay()
{
	using namespace SAEVUSAnimacion;
	Super::BeginPlay();
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	if (Personaje)
	{
		MovimientoBase = Personaje->GetCharacterMovement();
		Movimiento = Cast<USAEVUSComponenteMovimiento>(MovimientoBase);
	}
	IA = GetOwner()->FindComponentByClass<USAEVUSComponenteIAEnemigo>();
	Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>();
	Armas = GetOwner()->FindComponentByClass<USAEVUSComponenteArmas>();
	Vida = GetOwner()->FindComponentByClass<USAEVUSComponenteVida>();
	Alma = GetOwner()->FindComponentByClass<USAEVUSComponenteAlma>();
	Guardia = GetOwner()->FindComponentByClass<USAEVUSComponenteGuardia>();
	if (Armas)
	{
		Armas->AlDisparar.AddUniqueDynamic(this, &ThisClass::AlDisparar);
	}
	if (Movimiento)
	{
		ManejadorSaltoAereo = Movimiento->AlSaltoAereo.AddUObject(this, &ThisClass::AlSaltoAereo);
		ManejadorSaltoPared = Movimiento->AlSaltoPared.AddUObject(this, &ThisClass::AlSaltoPared);
		ManejadorDash = Movimiento->AlIniciarDash.AddUObject(this, &ThisClass::AlIniciarDash);
		ManejadorAterrizaje = Movimiento->AlAterrizar.AddUObject(this, &ThisClass::AlAterrizar);
	}
	if (Combate)
	{
		ManejadorImpacto = Combate->AlImpactar.AddUObject(this, &ThisClass::AlImpactar);
	}
	if (IA)
	{
		ManejadorFase = IA->AlCambiarFase.AddUObject(this, &ThisClass::AlCambiarFaseEnemigo);
	}
	if (Vida)
	{
		Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::AlCambiarVida);
	}
	// Cada cuerpo respira y mira a su ritmo: dos goblins iguales no van al unísono.
	CapaSecundaria.FijarDesfase(static_cast<float>(GetOwner()->GetUniqueID() % 97) * 0.731f);
	if (!Malla)
	{
		FijarMalla(GetOwner()->FindComponentByClass<UPoseableMeshComponent>());
	}
}

void USAEVUSComponenteAnimacionProcedural::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Movimiento)
	{
		Movimiento->AlSaltoAereo.Remove(ManejadorSaltoAereo);
		Movimiento->AlSaltoPared.Remove(ManejadorSaltoPared);
		Movimiento->AlIniciarDash.Remove(ManejadorDash);
		Movimiento->AlAterrizar.Remove(ManejadorAterrizaje);
	}
	if (Combate)
	{
		Combate->AlImpactar.Remove(ManejadorImpacto);
	}
	if (IA)
	{
		IA->AlCambiarFase.Remove(ManejadorFase);
	}
	Super::EndPlay(EndPlayReason);
}

void USAEVUSComponenteAnimacionProcedural::FijarMalla(UPoseableMeshComponent* NuevaMalla)
{
	Malla = NuevaMalla;
	PrepararEsqueleto();
}

void USAEVUSComponenteAnimacionProcedural::PrepararEsqueleto()
{
	using namespace SAEVUSAnimacion;
	bEsqueletoValido = false;
	bPoseIniciada = false;
	CapaSecundaria.Reiniciar();
	if (!Malla || !Malla->GetSkinnedAsset())
	{
		return;
	}
	// Los límites de la malla son los de la pose de reposo: la espada extendida, el giratorio o la caída de la muerte
	// los superan y el cuerpo podía desaparecer en el borde de la pantalla (ANIMVFX-25).
	Malla->BoundsScale = FMath::Max(Malla->BoundsScale, SAEVUSAnimacion::EscalaLimitesMalla);
	Malla->UpdateBounds();
	const FReferenceSkeleton& Referencia = Malla->GetSkinnedAsset()->GetRefSkeleton();
	const TArray<FTransform>& PoseReferencia = Referencia.GetRefBonePose();

	// Pose de reposo de toda la malla en espacio de componente (los padres van antes que los hijos).
	const int32 NumHuesosMalla = Referencia.GetNum();
	ComponenteMalla.SetNum(NumHuesosMalla);
	PadreMalla.SetNum(NumHuesosMalla);
	for (int32 HuesoMalla = 0; HuesoMalla < NumHuesosMalla; ++HuesoMalla)
	{
		PadreMalla[HuesoMalla] = Referencia.GetParentIndex(HuesoMalla);
		ComponenteMalla[HuesoMalla] = PadreMalla[HuesoMalla] == INDEX_NONE ? PoseReferencia[HuesoMalla] : PoseReferencia[HuesoMalla] * ComponenteMalla[PadreMalla[HuesoMalla]];
	}

	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		const FDefinicionHueso& Def = Definicion(static_cast<ESAEVUSHueso>(Hueso));
		IndiceMalla[Hueso] = Referencia.FindBoneIndex(Def.Nombre);
		Padre[Hueso] = Def.Padre == ESAEVUSHueso::Total ? INDEX_NONE : Indice(Def.Padre);
		if (IndiceMalla[Hueso] == INDEX_NONE)
		{
			UE_LOG(LogSAEVUS, Warning, TEXT("%s: la malla %s no tiene el hueso '%s'; sin animación procedural."),
				*GetNameSafe(GetOwner()), *GetNameSafe(Malla->GetSkinnedAsset()), Def.Nombre);
			return;
		}
		ReposoComponente[Hueso] = ComponenteMalla[IndiceMalla[Hueso]];
		RotacionActual[Hueso] = ReposoComponente[Hueso].GetRotation();
	}
	// Qué cuelga de cada hueso: cada uno se marca en sí mismo y en todos sus antepasados.
	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		Subarbol[Hueso] = 0;
	}
	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		for (int32 Antepasado = Hueso; Antepasado != INDEX_NONE; Antepasado = Padre[Antepasado])
		{
			Subarbol[Antepasado] |= 1u << Hueso;
		}
	}
	bEsqueletoValido = true;
}

FVector USAEVUSComponenteAnimacionProcedural::DireccionReposo(int32 Hueso) const
{
	using namespace SAEVUSAnimacion;
	const FDefinicionHueso& Def = Definicion(static_cast<ESAEVUSHueso>(Hueso));
	if (Def.Hijo == ESAEVUSHueso::Total)
	{
		return Def.DireccionHoja;
	}
	const FVector Delta = ReposoComponente[Indice(Def.Hijo)].GetLocation() - ReposoComponente[Hueso].GetLocation();
	return Delta.IsNearlyZero() ? Def.DireccionHoja : Delta.GetSafeNormal();
}

void USAEVUSComponenteAnimacionProcedural::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Los enemigos que no se ven se animan a saltos (el tiempo se acumula, nada se pierde).
	TiempoPendiente += DeltaTime;
	if (IA && Malla && bPoseIniciada)
	{
		float Intervalo = 0.f;
		if (Ajuste.IntervaloFueraDePantalla > 0.f && !Malla->WasRecentlyRendered(Ajuste.IntervaloFueraDePantalla))
		{
			Intervalo = Ajuste.IntervaloFueraDePantalla;
		}
		else if (const APlayerCameraManager* Camara = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			// Visible pero lejos: menos fotogramas de pose (LOD por distancia, ANIMVFX-17).
			Intervalo = Ajuste.IntervaloPorDistancia(FVector::Dist(Camara->GetCameraLocation(), Malla->GetComponentLocation()));
		}
		if (TiempoPendiente < Intervalo)
		{
			return;
		}
	}
	Actualizar(TiempoPendiente);
	TiempoPendiente = 0.f;
}

void USAEVUSComponenteAnimacionProcedural::Actualizar(float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	if (!bEsqueletoValido || !Malla || DeltaTime <= 0.f)
	{
		return;
	}
	TiempoTotal += DeltaTime;

	FPose Pose;
	ConstruirPoseObjetivo(Pose, DeltaTime);
	FQuat Objetivo[NumHuesos];
	ResolverObjetivo(Pose, Objetivo);
	const ESAEVUSEstadoEnemigo EstadoEnemigo = IA ? IA->ObtenerEstado() : ESAEVUSEstadoEnemigo::Reposo;
	const bool bAccionEnemigo = EstadoEnemigo == ESAEVUSEstadoEnemigo::Telegrafiar || EstadoEnemigo == ESAEVUSEstadoEnemigo::Atacar
		|| EstadoEnemigo == ESAEVUSEstadoEnemigo::Aturdido;
	const bool bAccion = (Combate && Combate->EstaAtacando()) || (Movimiento && Movimiento->EstaEnDash()) || TiempoGiroSaltoPared > 0.f || bAccionEnemigo;
	SuavizarYAplicar(Pose, Objetivo, DeltaTime, bAccion);
}

void USAEVUSComponenteAnimacionProcedural::ConstruirPoseObjetivo(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	if (!Movimiento)
	{
		if (IA)
		{
			ConstruirPoseEnemigo(Pose, DeltaTime);
		}
		return;
	}
	const float Velocidad = static_cast<float>(Movimiento->Velocity.Size2D());
	const float VelocidadVertical = static_cast<float>(Movimiento->Velocity.Z);
	const ASAEVUSPersonaje* Personaje = Cast<ASAEVUSPersonaje>(GetOwner());

	AplicarParpadeo();
	if (Vida && Vida->EstaMuerto())
	{
		PoseMuerte(Pose, DeltaTime);
		return;
	}
	TiempoMuerto = 0.f;

	// El aterrizaje (flexión y rebote) llega por el evento AlAterrizar del movimiento.
	const bool bEnAire = Movimiento->IsFalling();
	TiempoAterrizaje = FMath::Max(0.f, TiempoAterrizaje - DeltaTime);
	TiempoGiroSaltoPared = FMath::Max(0.f, TiempoGiroSaltoPared - DeltaTime);
	TiempoRecogidaSaltoAereo = FMath::Max(0.f, TiempoRecogidaSaltoAereo - DeltaTime);
	TiempoReaccionGolpe = FMath::Max(0.f, TiempoReaccionGolpe - DeltaTime);

	// Capa base del cuerpo.
	const bool bSentado = Personaje && Personaje->EstaSentado();
	const bool bCurando = Alma && Alma->EstaCurando();
	if (Movimiento->EstaEscalando())
	{
		PoseEscalada(Pose, DeltaTime);
	}
	else if (Movimiento->EstaEnCarreraPared())
	{
		PoseCarreraPared(Pose, DeltaTime);
	}
	else if (Movimiento->EstaEnDash())
	{
		PoseDash(Pose);
	}
	else if (bEnAire)
	{
		PoseAire(Pose, VelocidadVertical);
	}
	else if (bSentado)
	{
		PoseSentado(Pose);
	}
	else if (bCurando)
	{
		PoseCuracion(Pose);
	}
	else
	{
		PoseLocomocion(Pose, Velocidad, DeltaTime);
	}

	// Parte superior: ataque, guardia o arma de fuego por encima de la base.
	const bool bArmaFuego = Armas && Armas->TieneArmaFuegoEquipada();
	const bool bAtacando = Combate && Combate->ObtenerAtaqueActual();
	if (bAtacando)
	{
		PoseAtaque(Pose, *Combate->ObtenerAtaqueActual(), Combate->ObtenerTiempoAtaque());
	}
	else if (Guardia && Guardia->EstaEnGuardia())
	{
		PoseGuardia(Pose);
	}
	else if (bArmaFuego && !Movimiento->EstaEscalando())
	{
		PoseDisparo(Pose);
	}
	Retroceso = FMath::Max(0.f, Retroceso - Ajuste.RetrocesoDisparo * Ajuste.RecuperacionRetroceso * DeltaTime);

	// Arma visible: aguijón con armas cuerpo a cuerpo (o sin arsenal), pistola con armas de fuego.
	Pose.EscalaArma = bArmaFuego ? 0.f : 1.f;
	Pose.EscalaArmaFuego = bArmaFuego ? 1.f : 0.f;

	PoseMirada(Pose, DeltaTime);
	PoseReaccionGolpe(Pose);
	const bool bPared = Movimiento->EstaEnCarreraPared() || Movimiento->EstaEscalando();
	const bool bAccion = bAtacando || Movimiento->EstaEnDash() || bPared || TiempoGiroSaltoPared > 0.f;
	const bool bEsfuerzo = bAtacando || Movimiento->EstaEnDash() || bPared || (Movimiento->ObtenerPasoActual() == ESAEVUSPaso::Esprint && Velocidad > VelocidadQuieto);
	const bool bReposoLibre = !bAccion && !bEnAire && !bSentado && !bCurando && !(Guardia && Guardia->EstaEnGuardia());
	AplicarSecundaria(Pose, DeltaTime, bAccion, bEsfuerzo, bReposoLibre);
	PoseCapa(Pose, DeltaTime);
}

void USAEVUSComponenteAnimacionProcedural::PoseLocomocion(FPose& Pose, float Velocidad, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	const float Amplitud = Velocidad < VelocidadQuieto ? 0.f : FMath::Clamp(Velocidad / Ajuste.VelocidadReferencia, 0.f, 1.5f);
	// Esprint: por encima de la velocidad de referencia el cuerpo se echa más y las rodillas suben.
	const float Esprint = FMath::Clamp((Amplitud - 1.f) / 0.5f, 0.f, 1.f);
	// La zancada crece con la velocidad (el muslo abre más cuanto más rápido), así que la longitud de un ciclo también:
	// el pie en apoyo recorre lo mismo que el cuerpo y no patina (auditoría ANIMVFX-3). Con una longitud fija, a media
	// velocidad el muslo abría la mitad pero el ciclo pedía el doble de camino.
	const float EscalaZancada = FMath::Max(Amplitud * (1.f + 0.25f * Esprint), SAEVUSAnimacion::EscalaZancadaMinima);
	FaseCiclo = FMath::Fmod(FaseCiclo + Velocidad * DeltaTime / (Ajuste.LongitudCiclo * EscalaZancada) * UE_TWO_PI, UE_TWO_PI);
	const float Seno = FMath::Sin(FaseCiclo);
	const float Coseno = FMath::Cos(FaseCiclo);
	const float Carrera = FMath::Min(1.f, Amplitud);

	// GunZ: el cuerpo mira a la cámara y se mueve en ocho direcciones. Las piernas oscilan en la
	// dirección del paso; de lado, la cadera gira hacia él (al revés si va hacia atrás) y la columna
	// lo compensa, así el pecho sigue de frente. Hacia atrás, el cuerpo se echa algo atrás.
	const FVector Paso = DireccionMalla(VelocidadDueno());
	const float Frente = static_cast<float>(FVector::DotProduct(Paso, Adelante));
	const float Lado = static_cast<float>(FVector::DotProduct(Paso, Derecha));
	const float GiroCadera = Ajuste.GiroCaderaLateral * Lado * (Frente < FrenteAtras ? -1.f : 1.f) * Carrera;
	const float FactorInclinacion = Frente < 0.f ? FMath::Lerp(1.f, Ajuste.FraccionInclinacionAtras, -Frente) : 1.f;
	const float Inclinacion = (Ajuste.InclinacionCarrera * Amplitud + Ajuste.InclinacionEsprint * Esprint) * FactorInclinacion;
	// La cadera gira con la zancada (la del pie que avanza va delante) y cae hacia la pierna que se levanta.
	const float GiroZancada = -Ajuste.GiroCaderaPaso * Carrera * Seno;
	const float CaidaCadera = Ajuste.CaidaCaderaPaso * Carrera * Coseno;
	Pose.Huesos[Indice(ESAEVUSHueso::Pelvis)] = { false, FVector::ZeroVector,
		Girar(Adelante, Derecha, GiroCadera + GiroZancada) * Girar(Arriba, Derecha, CaidaCadera) };

	auto Pierna = [&](ESAEVUSHueso Muslo, ESAEVUSHueso PiernaHueso, ESAEVUSHueso Pie, float Signo)
	{
		const float Zancada = Ajuste.GiroMuslo * (1.f + 0.25f * Esprint) * Amplitud * Signo * Seno;
		Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Paso, Zancada) };
		// La rodilla se dobla sobre todo cuando la pierna va hacia delante en el aire (más al esprintar);
		// en reposo, algo doblada (guardia baja).
		const float Vuelo = FMath::Max(0.f, Signo * Coseno);
		const float Flexion = (Ajuste.FlexionRodilla * Amplitud + Ajuste.RodillasEsprint * Esprint) * Vuelo + 8.f * Amplitud
			+ Ajuste.FlexionRodillaReposo * (1.f - Carrera);
		Pose.Huesos[Indice(PiernaHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Flexion) };
		// El pie rueda: la punta sube al apoyar el talón delante y baja al despegar detrás.
		Pose.Huesos[Indice(Pie)] = { false, FVector::ZeroVector, Girar(Adelante, Arriba, Ajuste.RodaduraPie * Carrera * Signo * Seno) };
	};
	Pierna(ESAEVUSHueso::MusloD, ESAEVUSHueso::PiernaD, ESAEVUSHueso::PieD, 1.f);
	Pierna(ESAEVUSHueso::MusloI, ESAEVUSHueso::PiernaI, ESAEVUSHueso::PieI, -1.f);

	// Brazos en contrafase, un poco separados del cuerpo; el codo se dobla más cuando el brazo va adelante.
	auto Brazo = [&](ESAEVUSHueso BrazoHueso, ESAEVUSHueso Antebrazo, const FVector& LadoBrazo, float Signo)
	{
		const FVector Colgado = Dir(-Arriba, Adelante, -Ajuste.GiroBrazo * (1.f + 0.3f * Esprint) * Amplitud * Signo * Seno);
		Pose.Huesos[Indice(BrazoHueso)] = { true, (Colgado + LadoBrazo * 0.15f).GetSafeNormal() };
		const float Bombeo = Ajuste.BombeoCodo * Amplitud * FMath::Max(0.f, -Signo * Seno);
		Pose.Huesos[Indice(Antebrazo)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 20.f + 25.f * Amplitud + Bombeo) };
	};
	Brazo(ESAEVUSHueso::BrazoD, ESAEVUSHueso::AntebrazoD, Derecha, 1.f);
	Brazo(ESAEVUSHueso::BrazoI, ESAEVUSHueso::AntebrazoI, -Derecha, -1.f);

	// Espada (REF-04): en reposo baja en diagonal, adelantada y abierta, con la punta lejos del suelo;
	// al correr, el brazo se echa atrás y la hoja apunta atrás casi horizontal, y el brazo libre se adelanta.
	const FVector HojaReposo = Dir(Dir(-Arriba, Adelante, Ajuste.GradosHojaReposoAdelante), Derecha, Ajuste.GradosHojaReposoAfuera);
	const FVector HojaCarrera = (Dir(-Arriba, -Paso, Ajuste.GradosHojaCarrera) + Derecha * 0.25f).GetSafeNormal();
	Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, FMath::Lerp(HojaReposo, HojaCarrera, Carrera).GetSafeNormal() };
	if (Carrera > 0.f)
	{
		const FVector BrazoArma = (Dir(-Arriba, -Paso, Ajuste.GradosBrazoArmaCarrera) + Derecha * 0.2f).GetSafeNormal();
		const FObjetivo& Oscilante = Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)];
		Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, FMath::Lerp(Oscilante.Direccion, BrazoArma, Carrera).GetSafeNormal() };
		const FVector BrazoLibre = (Dir(-Arriba, Paso, Ajuste.GradosBrazoLibreCarrera) - Derecha * 0.25f).GetSafeNormal();
		const FObjetivo& Libre = Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)];
		Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, FMath::Lerp(Libre.Direccion, BrazoLibre, Carrera).GetSafeNormal() };
		Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, FMath::Lerp(20.f, Ajuste.FlexionCodoLibreCarrera, Carrera)) };
	}

	// Tronco: inclinación con la velocidad y el pecho girando al revés que la cadera; la cabeza compensa
	// casi todo el giro y el bote del paso, así la mirada va estable.
	const float GiroPechoPaso = 6.f * Amplitud * Seno - GiroZancada;
	Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, -GiroCadera) * Girar(Arriba, Paso, Inclinacion) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector,
		Girar(Adelante, Derecha, GiroPechoPaso * 0.6f) * Girar(Arriba, Paso, Inclinacion) * Girar(Arriba, Derecha, -CaidaCadera * 0.6f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector,
		Girar(Adelante, Derecha, -GiroPechoPaso * 0.6f * Ajuste.EstabilizacionCabezaPaso) * Girar(Arriba, Adelante, -Inclinacion * 1.4f * Ajuste.EstabilizacionCabezaPaso) };
	Pose.DesplazamientoRaiz.Z = Ajuste.BotePaso * Amplitud * FMath::Abs(Seno) - Ajuste.BajadaCarrera * Amplitud;

	// Flexión de aterrizaje.
	if (TiempoAterrizaje > 0.f)
	{
		const float Fraccion = FuerzaAterrizaje * TiempoAterrizaje / FMath::Max(Ajuste.DuracionAterrizaje, UE_KINDA_SMALL_NUMBER);
		for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
		{
			Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Adelante, Ajuste.FlexionAterrizaje * 0.6f * Fraccion) };
		}
		for (const ESAEVUSHueso PiernaHueso : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
		{
			Pose.Huesos[Indice(PiernaHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Ajuste.FlexionAterrizaje * Fraccion) };
		}
		Pose.DesplazamientoRaiz.Z -= 10.f * Fraccion;
		if (FuerzaAterrizaje >= Ajuste.FuerzaAterrizajeFuerte)
		{
			// REF-04 n.º 11: rodillas abiertas, la mano libre al suelo y la espada afuera, atrás.
			Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, (Dir(-Arriba, Adelante, Ajuste.FlexionAterrizaje * 0.5f * Fraccion) + Derecha * 0.5f * Fraccion).GetSafeNormal() };
			Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, (Dir(-Arriba, Adelante, Ajuste.FlexionAterrizaje * 0.5f * Fraccion) - Derecha * 0.5f * Fraccion).GetSafeNormal() };
			Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (-Arriba * 0.9f + Adelante * 0.35f - Derecha * 0.15f).GetSafeNormal() };
			Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Derecha * 0.8f - Arriba * 0.35f - Adelante * 0.3f).GetSafeNormal() };
			Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, (Derecha * 0.6f - Arriba * 0.3f - Adelante * 0.7f).GetSafeNormal() };
			Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 25.f * Fraccion) };
			Pose.DesplazamientoRaiz.Z -= 18.f * Fraccion;
		}
	}
}

void USAEVUSComponenteAnimacionProcedural::PoseAire(FPose& Pose, float VelocidadVertical)
{
	using namespace SAEVUSAnimacion;
	// Mezcla entre "subiendo" (piernas recogidas) y "cayendo" (piernas estiradas, brazos arriba).
	const float Subida = FMath::Clamp(0.5f + 0.5f * VelocidadVertical / VelocidadVerticalReferencia, 0.f, 1.f);
	// El doble salto recoge las piernas de golpe y las suelta: seno sobre la duración de la recogida.
	const float Recogida = TiempoRecogidaSaltoAereo > 0.f
		? Ajuste.RecogidaSaltoAereo * FMath::Sin(PI * TiempoRecogidaSaltoAereo / FMath::Max(Ajuste.DuracionRecogidaSaltoAereo, UE_KINDA_SMALL_NUMBER))
		: 0.f;
	const float GiroMuslo = FMath::Lerp(12.f, 40.f, Subida) + Recogida;
	const float Flexion = FMath::Lerp(20.f, 70.f, Subida) + Recogida;
	// Las piernas pedalean un poco en el aire, más al caer: no cuelgan muertas.
	const float Pedaleo = 8.f * (1.f - Subida) * FMath::Sin(TiempoTotal * 7.f);
	Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, Dir(-Arriba, Adelante, GiroMuslo + Pedaleo) };
	Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, Dir(-Arriba, Adelante, GiroMuslo * 0.5f - Pedaleo) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Flexion) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Flexion * 0.8f) };
	// Puntas de los pies hacia abajo al caer.
	for (const ESAEVUSHueso Pie : { ESAEVUSHueso::PieD, ESAEVUSHueso::PieI })
	{
		Pose.Huesos[Indice(Pie)] = { false, FVector::ZeroVector, Girar(Adelante, -Arriba, 20.f * (1.f - Subida)) };
	}

	// REF-04 n.º 4–5: la espada abajo y atrás, el brazo libre adelante y doblado; al caer los brazos se abren algo.
	const float Apertura = FMath::Lerp(0.45f, 0.15f, Subida);
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (-Adelante * 0.55f - Arriba * 0.55f + Derecha * (0.35f + Apertura)).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, (-Adelante * 0.7f - Arriba * 0.6f + Derecha * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Adelante * 0.6f - Arriba * 0.45f - Derecha * (0.3f + Apertura)).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 60.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, FMath::Lerp(4.f, 14.f, Subida)) };

	// Salto de pared: una vuelta sobre sí mismo que acaba otra vez de frente a la cámara.
	if (TiempoGiroSaltoPared > 0.f && Ajuste.GradosGiroSaltoPared > 0.f)
	{
		const float U = 1.f - TiempoGiroSaltoPared / FMath::Max(Ajuste.DuracionGiroSaltoPared, UE_KINDA_SMALL_NUMBER);
		Pose.Huesos[Indice(ESAEVUSHueso::Raiz)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, Ajuste.GradosGiroSaltoPared * SuavizarCurva(U)) };
	}
	else if (TiempoGiroSaltoPared > 0.f && !NormalSaltoPared.IsNearlyZero())
	{
		const FVector HaciaPared = -Malla->GetComponentTransform().InverseTransformVectorNoScale(NormalSaltoPared).GetSafeNormal2D();
		const float P = TiempoGiroSaltoPared / FMath::Max(Ajuste.DuracionGiroSaltoPared, UE_KINDA_SMALL_NUMBER);
		// La mano más cercana a la pared empuja; el cuerpo se abre hacia fuera.
		const bool bDerecha = FVector::DotProduct(HaciaPared, Derecha) > 0.f;
		const ESAEVUSHueso BrazoEmpuje = bDerecha ? ESAEVUSHueso::BrazoD : ESAEVUSHueso::BrazoI;
		const ESAEVUSHueso BrazoLibre = bDerecha ? ESAEVUSHueso::BrazoI : ESAEVUSHueso::BrazoD;
		Pose.Huesos[Indice(BrazoEmpuje)] = { true, (HaciaPared + Arriba * 0.2f).GetSafeNormal() };
		Pose.Huesos[Indice(BrazoLibre)] = { true, (-HaciaPared * 0.7f - Arriba * 0.4f - Adelante * 0.3f).GetSafeNormal() };
		Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Arriba, -HaciaPared, 22.f * P) };
		Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, Dir(-Arriba, -HaciaPared, 25.f * P) };
		Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, Dir(-Arriba, HaciaPared, 15.f * P) };
	}
}

void USAEVUSComponenteAnimacionProcedural::PoseDash(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	const FVector Direccion = DireccionMalla(Movimiento->ObtenerDireccionDash());
	const float Fraccion = Movimiento->ObtenerFraccionDash();
	if (Ajuste.GradosVolteretaDash > 0.f && FVector::DotProduct(Direccion, Adelante) < Ajuste.UmbralDashFrontal)
	{
		// A los lados o hacia atrás (GunZ): voltereta recogida sobre la pelvis en la dirección del dash.
		// Los miembros van en giros locales para rodar con el cuerpo.
		const float Vuelta = Ajuste.GradosVolteretaDash * SuavizarCurva(Fraccion);
		Pose.Huesos[Indice(ESAEVUSHueso::Pelvis)] = { false, FVector::ZeroVector, Girar(Arriba, Direccion, Vuelta) };
		Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 25.f) };
		Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 20.f) };
		for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
		{
			Pose.Huesos[Indice(Muslo)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 80.f) };
		}
		for (const ESAEVUSHueso PiernaHueso : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
		{
			Pose.Huesos[Indice(PiernaHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 110.f) };
		}
		for (const ESAEVUSHueso BrazoHueso : { ESAEVUSHueso::BrazoD, ESAEVUSHueso::BrazoI })
		{
			Pose.Huesos[Indice(BrazoHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 55.f) };
		}
		for (const ESAEVUSHueso Antebrazo : { ESAEVUSHueso::AntebrazoD, ESAEVUSHueso::AntebrazoI })
		{
			Pose.Huesos[Indice(Antebrazo)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 70.f) };
		}
		Pose.DesplazamientoRaiz.Z = -BajadaVoltereta;
		return;
	}

	// REF-04 n.º 6–8 (y REF-08 D): estocada baja inclinada hacia donde va, en tres tiempos. Arranque: el cuerpo
	// se encoge y sale disparado. Vuelo: estirado, la pierna de atrás tendida. Frenada: la pierna de delante
	// se adelanta a frenar y el tronco se endereza (la capa secundaria lo balancea al acabar).
	const FVector Va = Direccion.IsNearlyZero() ? Adelante : Direccion;
	const float Frente = static_cast<float>(FVector::DotProduct(Va, Adelante));
	const float Lado = static_cast<float>(FVector::DotProduct(Va, Derecha));
	const float PesoFrente = FMath::Max(0.f, Frente);
	const float PesoAtras = FMath::Max(0.f, -Frente);
	const float PesoLado = FMath::Abs(Lado);
	const float Arranque = 1.f - SuavizarCurva(Fraccion / Ajuste.FraccionArranqueDash);
	const float Frenada = SuavizarCurva((Fraccion - Ajuste.FraccionFrenadaDash) / FMath::Max(1.f - Ajuste.FraccionFrenadaDash, UE_KINDA_SMALL_NUMBER));

	// Tronco inclinado hacia donde va: mucho de frente, ladeado de lado, echado atrás hacia atrás.
	const float Inclinacion = (Ajuste.InclinacionDash * PesoFrente + Ajuste.LadeoDashLateral * PesoLado + Ajuste.InclinacionDashAtras * PesoAtras)
		* (1.f - 0.5f * Frenada) + 8.f * Arranque;
	Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Arriba, Va, Inclinacion * 0.5f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Va, Inclinacion * 0.5f) };
	// La cabeza compensa: sigue mirando al frente.
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, -Va, Inclinacion * 0.6f) };

	// Piernas: la de delante (del lado al que va) tira y la otra queda tendida atrás; al frenar se cambian las tornas.
	const bool bDelanteDerecha = PesoLado > PesoFrente + PesoAtras ? Lado > 0.f : false;
	const ESAEVUSHueso MusloDelante = bDelanteDerecha ? ESAEVUSHueso::MusloD : ESAEVUSHueso::MusloI;
	const ESAEVUSHueso MusloDetras = bDelanteDerecha ? ESAEVUSHueso::MusloI : ESAEVUSHueso::MusloD;
	const ESAEVUSHueso PiernaDelante = bDelanteDerecha ? ESAEVUSHueso::PiernaD : ESAEVUSHueso::PiernaI;
	const ESAEVUSHueso PiernaDetras = bDelanteDerecha ? ESAEVUSHueso::PiernaI : ESAEVUSHueso::PiernaD;
	Pose.Huesos[Indice(MusloDelante)] = { true, Dir(-Arriba, Va, FMath::Lerp(35.f, 50.f, Frenada) + 10.f * Arranque) };
	Pose.Huesos[Indice(MusloDetras)] = { true, Dir(-Arriba, -Va, FMath::Lerp(32.f + 12.f * (1.f - Arranque), 15.f, Frenada)) };
	Pose.Huesos[Indice(PiernaDelante)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, FMath::Lerp(30.f + 30.f * Arranque, 15.f, Frenada)) };
	Pose.Huesos[Indice(PiernaDetras)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, FMath::Lerp(45.f, 70.f, Frenada)) };

	// Brazos: de frente la espada va atrás casi horizontal y el brazo libre adelante; hacia atrás la espada se
	// adelanta en guardia; de lado el brazo libre se abre al lado contrario de contrapeso.
	const FVector BrazoFrente = (-Va * 0.7f - Arriba * 0.45f + Derecha * 0.3f).GetSafeNormal();
	const FVector HojaFrente = (-Va * 0.9f - Arriba * 0.15f + Derecha * 0.2f).GetSafeNormal();
	const FVector LibreFrente = (Va * 0.7f - Arriba * 0.35f - Derecha * 0.25f).GetSafeNormal();
	const FVector BrazoAtras = (Adelante * 0.7f - Arriba * 0.2f + Derecha * 0.2f).GetSafeNormal();
	const FVector HojaAtras = (Adelante * 0.5f + Arriba * 0.6f - Derecha * 0.4f).GetSafeNormal();
	const FVector LibreAtras = (-Adelante * 0.3f - Arriba * 0.7f - Derecha * 0.6f).GetSafeNormal();
	const FVector BrazoLado = (-Va * 0.6f - Arriba * 0.55f + Derecha * 0.1f).GetSafeNormal();
	const FVector HojaLado = (-Va * 0.8f - Arriba * 0.3f).GetSafeNormal();
	const FVector LibreLado = (-Va * 0.85f + Arriba * 0.15f + Adelante * 0.2f).GetSafeNormal();
	auto Mezcla = [&](const FVector& F, const FVector& A, const FVector& L)
	{
		return (F * PesoFrente + A * PesoAtras + L * PesoLado).GetSafeNormal();
	};
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, Mezcla(BrazoFrente, BrazoAtras, BrazoLado) };
	Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, Mezcla(HojaFrente, HojaAtras, HojaLado) };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, Mezcla(LibreFrente, LibreAtras, LibreLado) };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 35.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 15.f + 25.f * PesoAtras) };
	Pose.DesplazamientoRaiz.Z = -(14.f + 10.f * Arranque - 4.f * Frenada);
}

void USAEVUSComponenteAnimacionProcedural::PoseCarreraPared(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	PoseLocomocion(Pose, Movimiento->ObtenerAjusteCarreraPared().Velocidad, DeltaTime);
	// Los pies hacia la pared: el cuerpo se inclina alejándose de ella.
	const FVector NormalMalla = Malla->GetComponentTransform().InverseTransformVectorNoScale(Movimiento->ObtenerNormalPared()).GetSafeNormal2D();
	if (!NormalMalla.IsNearlyZero())
	{
		Pose.Huesos[Indice(ESAEVUSHueso::Raiz)] = { false, FVector::ZeroVector, Girar(Arriba, NormalMalla, Ajuste.InclinacionCarreraPared) };
		// REF-04 n.º 9: la mano del lado de la pared la toca; si es la de la espada, la hoja va atrás, a lo largo de la carrera.
		const bool bParedDerecha = FVector::DotProduct(-NormalMalla, Derecha) > 0.f;
		const FVector Toque = (-NormalMalla + Arriba * 0.35f).GetSafeNormal();
		Pose.Huesos[Indice(bParedDerecha ? ESAEVUSHueso::BrazoD : ESAEVUSHueso::BrazoI)] = { true, Toque };
		Pose.Huesos[Indice(bParedDerecha ? ESAEVUSHueso::AntebrazoD : ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, FQuat::Identity };
		Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, (-DireccionMalla(VelocidadDueno()) * 0.8f - Arriba * 0.5f + NormalMalla * 0.2f).GetSafeNormal() };
	}
}

void USAEVUSComponenteAnimacionProcedural::PoseEscalada(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	FaseCiclo = FMath::Fmod(FaseCiclo + FMath::Abs(static_cast<float>(Movimiento->Velocity.Z)) * DeltaTime / Ajuste.LongitudCiclo * UE_TWO_PI, UE_TWO_PI);
	const float Seno = FMath::Sin(FaseCiclo);
	// Brazos alternando hacia arriba contra la pared, piernas empujando; el tronco se tuerce con cada brazada.
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Dir(Arriba, Adelante, 30.f + 25.f * Seno) + Derecha * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Dir(Arriba, Adelante, 30.f - 25.f * Seno) - Derecha * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, Dir(-Arriba, Adelante, 25.f + 30.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, Dir(-Arriba, Adelante, 25.f - 30.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 50.f + 30.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 50.f - 30.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 10.f) * Girar(Arriba, Derecha, 6.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Adelante, Arriba, 18.f) };
}

void USAEVUSComponenteAnimacionProcedural::PoseAtaque(FPose& Pose, const FSAEVUSDefinicionAtaque& Ataque, float Tiempo)
{
	using namespace SAEVUSAnimacion;
	const float TiempoFases = SAEVUSGestos::TiempoEnFases(Tiempo, Ataque.TiempoPreparacion, Ataque.TiempoActivo, Ataque.TiempoRecuperacion);
	const FSAEVUSPoseGesto Gesto = SAEVUSGestos::Evaluar(Ataque.Gesto, TiempoFases);

	// Al acertar, la hoja rebota hacia arriba un instante (muelle RetrocesoImpacto).
	const FQuat Rebote = Girar(Adelante, Arriba, RetrocesoImpacto.Valor);
	const FVector DireccionBrazo = Rebote.RotateVector(Cuerpo(Gesto.Brazo));
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, DireccionBrazo };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, Gesto.Codo) };
	Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, Rebote.RotateVector(Cuerpo(Gesto.Hoja)) };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, Cuerpo(Gesto.BrazoLibre) };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, Gesto.CodoLibre) };

	// Todo el cuerpo (giro completo, vuelco del remate aéreo), la cadera y el tronco por tramos; la cabeza
	// compensa casi todo, así la mirada sigue al frente mientras el cuerpo se tuerce.
	Pose.Huesos[Indice(ESAEVUSHueso::Raiz)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, Gesto.GiroRaiz) * Girar(Arriba, Adelante, Gesto.Volteo) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pelvis)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, Gesto.GiroCadera) };
	const FQuat InclinaMitad = Girar(Arriba, Adelante, Gesto.Inclinacion * 0.5f) * Girar(Arriba, Derecha, Gesto.Ladeo * 0.5f);
	Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, Gesto.GiroPecho * 0.45f - Gesto.GiroCadera * 0.5f) * InclinaMitad };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, Gesto.GiroPecho * 0.55f) * InclinaMitad };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector,
		Girar(Adelante, Derecha, -Gesto.GiroPecho * 0.75f) * Girar(Arriba, Adelante, -(Gesto.Inclinacion + Gesto.Volteo) * 0.55f) };

	const bool bEnAire = Movimiento && Movimiento->IsFalling();
	if (bEnAire)
	{
		// En el aire (REF-04 n.º 15): rodillas recogidas bajo el cuerpo, tanto como pida el gesto.
		const float Recogida = Ajuste.RecogidaPiernasAtaqueAereo * Gesto.Postura;
		for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
		{
			Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Adelante, Recogida) };
		}
		for (const ESAEVUSHueso PiernaHueso : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
		{
			Pose.Huesos[Indice(PiernaHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Recogida * 1.6f) };
		}
		return;
	}
	// Zancada (REF-04 n.º 12–14): un pie delante y el otro atrás, muslos abiertos, cadera baja; el revés pisa
	// con el izquierdo. Un gesto aéreo que acaba en el suelo pisa a medias.
	const FSAEVUSPisadaGesto Pisada = SAEVUSGestos::Pisada(Ataque.Gesto);
	const float Postura = Gesto.Postura * (SAEVUSGestos::EsAereo(Ataque.Gesto) ? 0.5f : 1.f);
	const float Escala = Pisada.EscalaZancada * Postura;
	const FVector Fuera = Derecha * FMath::Sin(FMath::DegreesToRadians(Ajuste.AperturaAtaque)) * Postura;
	const bool bIzquierdo = Pisada.bPieIzquierdoDelante;
	const ESAEVUSHueso MusloDelante = bIzquierdo ? ESAEVUSHueso::MusloI : ESAEVUSHueso::MusloD;
	const ESAEVUSHueso MusloDetras = bIzquierdo ? ESAEVUSHueso::MusloD : ESAEVUSHueso::MusloI;
	const FVector FueraDelante = bIzquierdo ? -Fuera : Fuera;
	Pose.Huesos[Indice(MusloDelante)] = { true, (Dir(-Arriba, Adelante, Ajuste.ZancadaAtaqueDelante * Escala) + FueraDelante).GetSafeNormal() };
	Pose.Huesos[Indice(MusloDetras)] = { true, (Dir(-Arriba, -Adelante, Ajuste.ZancadaAtaqueAtras * Escala) - FueraDelante).GetSafeNormal() };
	Pose.Huesos[Indice(bIzquierdo ? ESAEVUSHueso::PiernaI : ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Ajuste.FlexionRodillaAtaqueDelante * Escala) };
	Pose.Huesos[Indice(bIzquierdo ? ESAEVUSHueso::PiernaD : ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Ajuste.FlexionRodillaAtaqueAtras * Escala) };
	// El pie de atrás se apoya en la punta.
	Pose.Huesos[Indice(bIzquierdo ? ESAEVUSHueso::PieD : ESAEVUSHueso::PieI)] = { false, FVector::ZeroVector, Girar(Adelante, -Arriba, 25.f * Postura) };
	Pose.DesplazamientoRaiz.Z = -FMath::Max(Gesto.Bajada, -Ajuste.BajadaAtaque) * FMath::Max(Escala, 0.6f);
}

void USAEVUSComponenteAnimacionProcedural::PoseDisparo(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	// Brazo derecho al frente a la altura del hombro; el retroceso lo levanta y vuelve.
	const FVector Apunte = Dir(Adelante, Arriba, Retroceso);
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, Apunte };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false };
	Pose.Huesos[Indice(ESAEVUSHueso::ArmaFuego)] = { true, Apunte };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Adelante * 0.8f - Derecha * 0.25f - Arriba * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 30.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, 10.f) };
}

void USAEVUSComponenteAnimacionProcedural::PoseMuerte(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	TiempoMuerto += DeltaTime;
	// Primero ceden las rodillas, luego el cuerpo cae lejos del golpe que lo mató (de frente si no se sabe).
	const float Rodillas = SuavizarCurva(TiempoMuerto / (DuracionRodillasMuerte * Peso));
	const float Caida = SuavizarCurva((TiempoMuerto - InicioCaidaMuerte * Peso) / (DuracionCaidaMuerte * Peso));
	const FVector HaciaDonde = DireccionUltimoGolpe.IsNearlyZero() ? Adelante : -DireccionUltimoGolpe;
	const bool bAtras = FVector::DotProduct(HaciaDonde, Adelante) < 0.f;
	Pose.Huesos[Indice(ESAEVUSHueso::Raiz)] = { false, FVector::ZeroVector, Girar(Arriba, HaciaDonde, 80.f * Caida) };
	Pose.DesplazamientoRaiz.Z = -40.f * Rodillas - 15.f * Caida;
	for (const ESAEVUSHueso PiernaHueso : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
	{
		Pose.Huesos[Indice(PiernaHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 85.f * Rodillas * (1.f - 0.4f * Caida)) };
	}
	for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
	{
		Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Adelante, 50.f * Rodillas) };
	}
	// Los brazos se abren al caer y quedan sueltos; la cabeza cae la última.
	const float Apertura = FMath::Sin(PI * FMath::Clamp(Caida, 0.f, 1.f));
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (HaciaDonde * 0.4f - Arriba * 0.8f + Derecha * (0.3f + 0.5f * Apertura)).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (HaciaDonde * 0.4f - Arriba * 0.8f - Derecha * (0.3f + 0.5f * Apertura)).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, (Derecha * 0.7f - Arriba * 0.6f + HaciaDonde * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, bAtras ? -Adelante : Adelante, 30.f * Caida) };
	PoseCapa(Pose, DeltaTime);
}

void USAEVUSComponenteAnimacionProcedural::PoseCuracion(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	// Agachado y recogido, las manos juntas delante del pecho, la cabeza baja: concentra el alma. El cuerpo
	// tiembla un poco con el esfuerzo.
	const float Temblor = 1.5f * FMath::Sin(TiempoTotal * 31.f);
	Pose.DesplazamientoRaiz.Z = -Ajuste.BajadaCuracion;
	for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
	{
		Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Adelante, Ajuste.FlexionCuracion * 0.6f) };
	}
	for (const ESAEVUSHueso Pierna : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
	{
		Pose.Huesos[Indice(Pierna)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Ajuste.FlexionCuracion) };
	}
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Adelante * 0.7f - Arriba * 0.4f - Derecha * 0.15f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Adelante * 0.7f - Arriba * 0.4f + Derecha * 0.15f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 60.f + Temblor) };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 60.f - Temblor) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 12.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 20.f) };
}

void USAEVUSComponenteAnimacionProcedural::PoseSentado(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	// Sentado en el banco: muslos hacia delante, piernas colgando, manos en las rodillas, cabeza algo baja.
	Pose.DesplazamientoRaiz.Z = -Ajuste.BajadaSentado;
	for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
	{
		Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Adelante, Ajuste.FlexionSentado) };
	}
	for (const ESAEVUSHueso Pierna : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
	{
		Pose.Huesos[Indice(Pierna)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Ajuste.FlexionSentado) };
	}
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Adelante * 0.6f - Arriba * 0.7f - Derecha * 0.1f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Adelante * 0.6f - Arriba * 0.7f + Derecha * 0.1f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 12.f) };
}

void USAEVUSComponenteAnimacionProcedural::PoseGuardia(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	// Brazos al frente a la altura del pecho y el aguijón cruzado en diagonal: cubre el frente; las rodillas se doblan.
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Adelante * 0.8f - Arriba * 0.25f + Derecha * 0.3f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 35.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, (-Derecha * 0.8f + Arriba * 0.5f + Adelante * 0.3f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Adelante * 0.8f - Arriba * 0.3f - Derecha * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 35.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 8.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 6.f) };
	Pose.DesplazamientoRaiz.Z -= 6.f;
}

void USAEVUSComponenteAnimacionProcedural::AplicarParpadeo()
{
	if (!Malla)
	{
		return;
	}
	const bool bParpadeando = bParpadeoJugador && Vida && !Vida->EstaMuerto() && Vida->EstaEnInvulnerabilidadTrasGolpe();
	const bool bVisible = !bParpadeando || FMath::Frac(TiempoTotal * Ajuste.FrecuenciaParpadeo) < 0.5f;
	if (Malla->IsVisible() != bVisible)
	{
		Malla->SetVisibility(bVisible);
	}
}

void USAEVUSComponenteAnimacionProcedural::PoseCapa(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	// Objetivo de la capa en los ejes del cuerpo: atrás con la velocidad hacia delante y al caer; de lado al
	// correr de lado y hacia fuera de las curvas. Cada tramo es un muelle más lento que el de arriba: la onda
	// baja por la capa hasta las tiras y al frenar la capa se pasa hacia delante y vuelve.
	const FVector Velocidad = VelocidadDueno();
	const FVector Local = Malla ? Malla->GetComponentTransform().InverseTransformVectorNoScale(Velocidad) : FVector::ZeroVector;
	const float HaciaDelante = static_cast<float>(FVector::DotProduct(Local, Adelante));
	const float HaciaDerecha = static_cast<float>(FVector::DotProduct(Local, Derecha));
	const float Atras = FMath::Clamp(HaciaDelante / VelocidadCapaReferencia, -0.3f, 1.5f) * Ajuste.AtrasCapa
		+ FMath::Clamp(static_cast<float>(-Velocidad.Z) / VelocidadCaidaCapaReferencia, 0.f, 1.f) * 35.f + 4.f;
	const float LadoCapa = -FMath::Clamp(HaciaDerecha / VelocidadCapaReferencia, -1.f, 1.f) * Ajuste.LadoCapa
		- CapaSecundaria.ObtenerSalida().Ladeo * 1.2f;
	const float Fracciones[3] = { 0.7f, 0.5f, 0.35f };
	const ESAEVUSHueso Tramos[3] = { ESAEVUSHueso::Capa1, ESAEVUSHueso::Capa2, ESAEVUSHueso::Capa3 };
	float Frecuencia = Ajuste.FrecuenciaCapa / FMath::Sqrt(Peso);
	for (int32 Tramo = 0; Tramo < 3; ++Tramo)
	{
		// Ondeo propio de cada tramo, desfasado: la tela no está nunca quieta del todo.
		const float Ondeo = (4.f + 4.f * Tramo) * FMath::Sin(TiempoTotal * 3.1f - 0.7f * Tramo) + 3.f * FMath::PerlinNoise1D(TiempoTotal * 0.9f + 5.f * Tramo);
		const FVector Objetivo(Atras * Fracciones[Tramo] + Ondeo, LadoCapa * Fracciones[Tramo], 0.f);
		MuellesCapa[Tramo].Avanzar(Objetivo, Frecuencia, Ajuste.AmortiguacionCapa, DeltaTime);
		Frecuencia *= Ajuste.RetrasoCadenaCapa;
		const FVector& Valor = MuellesCapa[Tramo].Valor;
		Pose.Huesos[Indice(Tramos[Tramo])] = { false, FVector::ZeroVector,
			Girar(-Arriba, -Adelante, static_cast<float>(Valor.X)) * Girar(-Arriba, Derecha, static_cast<float>(Valor.Y)) };
	}
}

void USAEVUSComponenteAnimacionProcedural::GirarTramo(FPose& Pose, ESAEVUSHueso Hueso, const FQuat& Giro) const
{
	using namespace SAEVUSAnimacion;
	const int32 Propio = Indice(Hueso);
	for (int32 Otro = 0; Otro < NumHuesos; ++Otro)
	{
		if ((Subarbol[Propio] & (1u << Otro)) == 0)
		{
			continue;
		}
		FObjetivo& Objetivo = Pose.Huesos[Otro];
		if (Objetivo.bApuntar)
		{
			// Las direcciones absolutas no heredan el giro del padre: se giran igual que el tramo.
			Objetivo.Direccion = Giro.RotateVector(Objetivo.Direccion);
		}
		else if (Otro == Propio)
		{
			Objetivo.Giro = Giro * Objetivo.Giro;
		}
	}
}

void USAEVUSComponenteAnimacionProcedural::AplicarSecundaria(FPose& Pose, float DeltaTime, bool bAccion, bool bEsfuerzo, bool bReposoLibre)
{
	using namespace SAEVUSAnimacion;
	const AActor* Duenio = GetOwner();
	FSAEVUSEntradaSecundaria Entrada;
	Entrada.DeltaTime = DeltaTime;
	Entrada.Guinada = static_cast<float>(Duenio->GetActorRotation().Yaw);
	const FVector Velocidad = VelocidadDueno();
	Entrada.Velocidad = FVector2D(Velocidad.X, Velocidad.Y);
	Entrada.VelocidadReferencia = Ajuste.VelocidadReferencia;
	Entrada.bEnSuelo = !MovimientoBase || !MovimientoBase->IsFalling();
	Entrada.bAccion = bAccion;
	Entrada.bEsfuerzo = bEsfuerzo;
	Entrada.bReposoLibre = bReposoLibre;
	Entrada.Peso = Peso;
	CapaSecundaria.Actualizar(Ajuste.Secundario, Entrada);
	const FSAEVUSSalidaSecundaria& Salida = CapaSecundaria.ObtenerSalida();

	// Ladeo de todo el cuerpo sobre los pies y vaivén del peso (la cadera va hacia el pie que carga).
	GirarTramo(Pose, ESAEVUSHueso::Raiz, Girar(Arriba, Derecha, Salida.Ladeo));
	Pose.DesplazamientoRaiz += Derecha * Salida.VaivenPeso * Ajuste.Secundario.DesplazamientoPeso;
	const ESAEVUSHueso RodillaLibre = Salida.VaivenPeso > 0.f ? ESAEVUSHueso::PiernaI : ESAEVUSHueso::PiernaD;
	FObjetivo& Rodilla = Pose.Huesos[Indice(RodillaLibre)];
	if (!Rodilla.bApuntar)
	{
		Rodilla.Giro = Girar(-Arriba, -Adelante, Ajuste.Secundario.FlexionRodillaSinPeso * FMath::Abs(Salida.VaivenPeso)) * Rodilla.Giro;
	}

	// Giro por capas: la cadera atrás, el pecho a medias, la cabeza por delante.
	GirarTramo(Pose, ESAEVUSHueso::Pelvis, Girar(Adelante, Derecha, Salida.GiroCadera));
	// Inercia del tronco (delante/detrás y a los lados), repartida en columna y pecho; respiración en el pecho.
	const FQuat InerciaMitad = Girar(Arriba, Adelante, static_cast<float>(Salida.InerciaTronco.X) * 0.5f) * Girar(Arriba, Derecha, static_cast<float>(Salida.InerciaTronco.Y) * 0.5f);
	GirarTramo(Pose, ESAEVUSHueso::Columna, Girar(Adelante, Derecha, Salida.GiroColumna) * InerciaMitad);
	GirarTramo(Pose, ESAEVUSHueso::Pecho, Girar(Adelante, Derecha, Salida.GiroPecho) * InerciaMitad * Girar(Arriba, -Adelante, Salida.Respiracion));
	// Los hombros suben al inspirar.
	GirarTramo(Pose, ESAEVUSHueso::HombroD, Girar(Derecha, Arriba, Salida.Respiracion * 0.8f));
	GirarTramo(Pose, ESAEVUSHueso::HombroI, Girar(-Derecha, Arriba, Salida.Respiracion * 0.8f));
	GirarTramo(Pose, ESAEVUSHueso::Cuello, Girar(Adelante, Derecha, Salida.GiroCuello));
	GirarTramo(Pose, ESAEVUSHueso::Cabeza, Girar(Adelante, Derecha, Salida.GiroCabeza)
		* Girar(Arriba, Adelante, static_cast<float>(Salida.InerciaCabeza.X) - Salida.Respiracion * 0.5f) * Girar(Arriba, Derecha, static_cast<float>(Salida.InerciaCabeza.Y)));

	// Rebote: la cadera baja y las rodillas se doblan lo justo para que los pies no se hundan en el suelo.
	if (Salida.Rebote > 0.f)
	{
		Pose.DesplazamientoRaiz.Z -= Salida.Rebote;
		const float LargoPierna = FMath::Max(1.f, static_cast<float>(ReposoComponente[Indice(ESAEVUSHueso::MusloD)].GetLocation().Z - ReposoComponente[Indice(ESAEVUSHueso::PieD)].GetLocation().Z));
		const float Angulo = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(1.f - Salida.Rebote / (2.f * LargoPierna * FraccionMusloPierna), -1.f, 1.f)));
		for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
		{
			FObjetivo& Objetivo = Pose.Huesos[Indice(Muslo)];
			Objetivo.Direccion = Objetivo.bApuntar ? Dir(Objetivo.Direccion.GetSafeNormal(), Adelante, Angulo) : Objetivo.Direccion;
			Objetivo.Giro = Objetivo.bApuntar ? Objetivo.Giro : Girar(-Arriba, Adelante, Angulo) * Objetivo.Giro;
		}
		for (const ESAEVUSHueso PiernaHueso : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
		{
			FObjetivo& Objetivo = Pose.Huesos[Indice(PiernaHueso)];
			Objetivo.Giro = Objetivo.bApuntar ? Objetivo.Giro : Girar(-Arriba, -Adelante, Angulo * 2.f) * Objetivo.Giro;
		}
	}

	// Paso de giro: quieto y muy torcido, la pierna del lado al que gira da un paso corto.
	if (Salida.PasoGiro > 0.f)
	{
		const float Alzada = FMath::Sin(PI * Salida.PasoGiro);
		const bool bDerecha = Salida.LadoPasoGiro > 0.f;
		FObjetivo& Muslo = Pose.Huesos[Indice(bDerecha ? ESAEVUSHueso::MusloD : ESAEVUSHueso::MusloI)];
		if (Muslo.bApuntar)
		{
			Muslo.Direccion = Dir(Muslo.Direccion.GetSafeNormal(), Adelante, 25.f * Alzada);
		}
		FObjetivo& Pierna = Pose.Huesos[Indice(bDerecha ? ESAEVUSHueso::PiernaD : ESAEVUSHueso::PiernaI)];
		Pierna.Giro = Girar(-Arriba, -Adelante, 45.f * Alzada) * Pierna.Giro;
		Pose.DesplazamientoRaiz.Z -= 2.f * Alzada;
	}
	PoseGestoReposo(Pose, Salida);
}

void USAEVUSComponenteAnimacionProcedural::PoseGestoReposo(FPose& Pose, const FSAEVUSSalidaSecundaria& Salida)
{
	using namespace SAEVUSAnimacion;
	if (Salida.GestoReposo == ESAEVUSGestoReposo::Ninguno)
	{
		return;
	}
	// Entra y sale suave: una campana sobre su duración.
	const float U = Salida.AvanceGestoReposo;
	const float Campana = SuavizarCurva(U / 0.25f) * SuavizarCurva((1.f - U) / 0.25f);
	switch (Salida.GestoReposo)
	{
	case ESAEVUSGestoReposo::MirarAtras:
	{
		// Mira por encima del hombro izquierdo y luego del derecho.
		const float Lado = FMath::Sin(UE_TWO_PI * U);
		GirarTramo(Pose, ESAEVUSHueso::Cabeza, Girar(Adelante, Derecha, -45.f * Lado * Campana));
		GirarTramo(Pose, ESAEVUSHueso::Pecho, Girar(Adelante, Derecha, -15.f * Lado * Campana));
		break;
	}
	case ESAEVUSGestoReposo::GirarHoja:
	{
		// Sube la hoja delante, la mira y la vuelve a bajar.
		Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, Interpolar(Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)].Direccion,
			(Adelante * 0.8f - Arriba * 0.1f + Derecha * 0.2f).GetSafeNormal(), Campana) };
		Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, Interpolar(Pose.Huesos[Indice(ESAEVUSHueso::Arma)].Direccion,
			Dir((Arriba * 0.9f + Adelante * 0.4f).GetSafeNormal(), -Derecha, 25.f * FMath::Sin(UE_TWO_PI * U)), Campana) };
		GirarTramo(Pose, ESAEVUSHueso::Cabeza, Girar(Arriba, Adelante, 10.f * Campana) * Girar(Adelante, Derecha, 12.f * Campana));
		break;
	}
	case ESAEVUSGestoReposo::EstirarHombros:
	default:
	{
		// Echa el pecho atrás, sube los hombros y rueda el cuello.
		GirarTramo(Pose, ESAEVUSHueso::Pecho, Girar(Arriba, -Adelante, 10.f * Campana));
		GirarTramo(Pose, ESAEVUSHueso::HombroD, Girar(Derecha, Arriba, 12.f * Campana));
		GirarTramo(Pose, ESAEVUSHueso::HombroI, Girar(-Derecha, Arriba, 12.f * Campana));
		GirarTramo(Pose, ESAEVUSHueso::Cabeza, Girar(Arriba, Derecha, 12.f * FMath::Sin(UE_TWO_PI * U) * Campana));
		break;
	}
	}
}

void USAEVUSComponenteAnimacionProcedural::AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador)
{
	using namespace SAEVUSAnimacion;
	if (Delta >= 0.f)
	{
		return;
	}
	// El tronco sale despedido lejos de quien golpea; la cadera cede.
	FVector Desde = Adelante;
	if (Instigador && Instigador != GetOwner() && Malla)
	{
		Desde = DireccionMalla(Instigador->GetActorLocation() - GetOwner()->GetActorLocation());
	}
	DireccionUltimoGolpe = Desde;
	// Un golpe fuerte mueve más el cuerpo que un roce (ANIMVFX-10): proporcional al daño, entre media y doble fuerza.
	const float Fuerza = FMath::Clamp(-Delta / FMath::Max(Ajuste.DanioReferenciaReaccion, UE_KINDA_SMALL_NUMBER), 0.5f, 2.f);
	const FVector2D Empuje(-FVector::DotProduct(Desde, Adelante) * Ajuste.ImpulsoGolpeRecibido * Fuerza, -FVector::DotProduct(Desde, Derecha) * Ajuste.ImpulsoGolpeRecibido * Fuerza);
	CapaSecundaria.Empujar(Empuje, Ajuste.HundimientoGolpeRecibido * Fuerza);
	if (!ComponenteVida->EstaMuerto())
	{
		TiempoReaccionGolpe = Ajuste.DuracionReaccionGolpe;
	}
}

void USAEVUSComponenteAnimacionProcedural::PoseReaccionGolpe(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	if (TiempoReaccionGolpe <= 0.f)
	{
		return;
	}
	// Los brazos se sueltan un instante (el tronco y la cabeza los mueve la capa secundaria).
	const float P = TiempoReaccionGolpe / FMath::Max(Ajuste.DuracionReaccionGolpe, UE_KINDA_SMALL_NUMBER);
	const float Grados = Ajuste.InclinacionReaccionGolpe * P * P;
	FObjetivo& Libre = Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)];
	if (Libre.bApuntar)
	{
		Libre.Direccion = Dir(Libre.Direccion.GetSafeNormal(), -Derecha, Grados);
	}
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)].Giro = Girar(Arriba, -Adelante, Grados * 0.6f) * Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)].Giro;
}

void USAEVUSComponenteAnimacionProcedural::PoseMirada(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	if (Ajuste.RadioMirada <= 0.f || !Malla)
	{
		return;
	}
	const FVector Ojos = Malla->GetComponentLocation() + Malla->GetComponentQuat().RotateVector(ReposoComponente[Indice(ESAEVUSHueso::Cabeza)].GetLocation());
	// El enemigo vivo más cercano se busca cada poco, no cada fotograma.
	TiempoBusquedaMirada -= DeltaTime;
	if (TiempoBusquedaMirada <= 0.f)
	{
		TiempoBusquedaMirada = Ajuste.IntervaloBusquedaMirada;
		ObjetivoMirada.Reset();
		double MejorDistancia = FMath::Square(static_cast<double>(Ajuste.RadioMirada));
		for (TActorIterator<ASAEVUSEnemigo> Enemigo(GetWorld()); Enemigo; ++Enemigo)
		{
			const double Distancia = FVector::DistSquared(Enemigo->GetActorLocation(), Ojos);
			if (*Enemigo != GetOwner() && Distancia < MejorDistancia && Enemigo->ObtenerIA()->ObtenerEstado() != ESAEVUSEstadoEnemigo::Muerto)
			{
				MejorDistancia = Distancia;
				ObjetivoMirada = *Enemigo;
			}
		}
	}

	// Hacia dónde: el enemigo (en giro e inclinación) o, sin él, la mirada ociosa de la capa secundaria.
	FVector Objetivo(CapaSecundaria.ObtenerSalida().MiradaOciosa.X, CapaSecundaria.ObtenerSalida().MiradaOciosa.Y, 0.f);
	if (const AActor* Mas = ObjetivoMirada.Get())
	{
		const FVector Hacia = Malla->GetComponentQuat().UnrotateVector(Mas->GetActorLocation() - Ojos).GetSafeNormal();
		const float Giro = FMath::RadiansToDegrees(FMath::Atan2(FVector::DotProduct(Hacia, Derecha), FVector::DotProduct(Hacia, Adelante)));
		const float Inclinacion = FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(static_cast<float>(FVector::DotProduct(Hacia, Arriba)), -1.f, 1.f)));
		// Detrás del todo no se gira: quedaría forzado.
		if (FMath::Abs(Giro) <= GiroMaximoParaMirar)
		{
			Objetivo = FVector(FMath::Clamp(Giro, -Ajuste.GiroMaximoCabeza, Ajuste.GiroMaximoCabeza),
				FMath::Clamp(Inclinacion, -Ajuste.InclinacionMaximaCabeza, Ajuste.InclinacionMaximaCabeza), 0.f);
		}
	}
	GiroMirada.Avanzar(Objetivo, FrecuenciaMirada, AmortiguacionMirada, DeltaTime);
	// Solo si ninguna pose de acción ocupa la cabeza (la locomoción solo la estabiliza: se suma).
	const bool bOcupada = (Combate && Combate->EstaAtacando()) || (Movimiento && (Movimiento->EstaEnDash() || Movimiento->EstaEscalando()));
	if (bOcupada)
	{
		return;
	}
	FObjetivo& Cabeza = Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)];
	Cabeza.Giro = Girar(Adelante, Derecha, static_cast<float>(GiroMirada.Valor.X)) * Girar(Adelante, Arriba, static_cast<float>(GiroMirada.Valor.Y)) * Cabeza.Giro;
}

bool USAEVUSComponenteAnimacionProcedural::ObtenerHoja(FVector& OutBase, FVector& OutPunta) const
{
	using namespace SAEVUSAnimacion;
	if (!bEsqueletoValido || !Malla || !bPoseIniciada || EscalaArmaActual < 0.5f)
	{
		return false;
	}
	const int32 Hueso = Indice(ESAEVUSHueso::Arma);
	const FTransform& Componente = Malla->GetComponentTransform();
	const FVector DireccionHoja = (RotacionActual[Hueso] * ReposoComponente[Hueso].GetRotation().Inverse()).RotateVector(DireccionReposo(Hueso));
	OutBase = Componente.TransformPosition(PosicionActual[Hueso]);
	OutPunta = OutBase + Componente.TransformVectorNoScale(DireccionHoja) * Ajuste.LongitudHoja;
	return true;
}

void USAEVUSComponenteAnimacionProcedural::AlAterrizar(float VelocidadCaida)
{
	using namespace SAEVUSAnimacion;
	// Flexión breve, más honda cuanto más rápida la caída, y la cadera rebota.
	TiempoAterrizaje = Ajuste.DuracionAterrizaje;
	FuerzaAterrizaje = FMath::Clamp(VelocidadCaida / Ajuste.VelocidadCaidaAterrizaje, FuerzaAterrizajeMinima, FuerzaAterrizajeMaxima);
	TiempoGiroSaltoPared = 0.f;
	CapaSecundaria.Aterrizar(Ajuste.Secundario, FuerzaAterrizaje);
}

void USAEVUSComponenteAnimacionProcedural::AlSaltoAereo()
{
	TiempoRecogidaSaltoAereo = Ajuste.DuracionRecogidaSaltoAereo;
}

void USAEVUSComponenteAnimacionProcedural::AlSaltoPared(const FVector& NormalPared)
{
	NormalSaltoPared = NormalPared;
	TiempoGiroSaltoPared = Ajuste.DuracionGiroSaltoPared;
}

void USAEVUSComponenteAnimacionProcedural::AlIniciarDash(const FVector& Direccion)
{
	using namespace SAEVUSAnimacion;
	// Arranque explosivo: la cadera se hunde para impulsarse (el tronco que se queda atrás lo pone la inercia).
	const FVector Va = DireccionMalla(Direccion);
	CapaSecundaria.Empujar(FVector2D::ZeroVector, Ajuste.ImpulsoDash * 0.5f);
	// La capa sale despedida hacia atrás del dash.
	for (FSAEVUSMuelleVector& Tramo : MuellesCapa)
	{
		Tramo.Impulsar(FVector(FVector::DotProduct(Va, Adelante), -FVector::DotProduct(Va, Derecha), 0.f) * Ajuste.ImpulsoDash * 2.f);
	}
}

void USAEVUSComponenteAnimacionProcedural::AlImpactar(const FSAEVUSDefinicionAtaque& Ataque)
{
	// La hoja rebota en lo que golpea: un golpe seco del muelle hacia arriba.
	RetrocesoImpacto.Impulsar(Ajuste.RetrocesoImpacto * UE_TWO_PI * Ajuste.FrecuenciaRetrocesoImpacto);
}

FVector USAEVUSComponenteAnimacionProcedural::DireccionMalla(const FVector& DireccionMundo) const
{
	using namespace SAEVUSAnimacion;
	if (!Malla)
	{
		return Adelante;
	}
	const FVector Local = Malla->GetComponentTransform().InverseTransformVectorNoScale(DireccionMundo);
	const FVector Horizontal = FVector(Local.X, Local.Y, 0.f).GetSafeNormal();
	return Horizontal.IsNearlyZero() ? Adelante : Horizontal;
}

void USAEVUSComponenteAnimacionProcedural::AlDisparar(FVector Boca, FVector Impacto, AActor* ActorGolpeado)
{
	Retroceso = Ajuste.RetrocesoDisparo;
}

void USAEVUSComponenteAnimacionProcedural::ResolverObjetivo(const FPose& Pose, FQuat (&OutRotaciones)[SAEVUSAnimacion::NumHuesos]) const
{
	using namespace SAEVUSAnimacion;
	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		const FQuat Reposo = ReposoComponente[Hueso].GetRotation();
		const FObjetivo& Objetivo = Pose.Huesos[Hueso];
		if (Padre[Hueso] == INDEX_NONE)
		{
			OutRotaciones[Hueso] = Objetivo.Giro * Reposo;
			continue;
		}
		// Giro acumulado del padre respecto a su reposo: el hijo lo hereda.
		const FQuat GiroPadre = OutRotaciones[Padre[Hueso]] * ReposoComponente[Padre[Hueso]].GetRotation().Inverse();
		if (Objetivo.bApuntar && !Objetivo.Direccion.IsNearlyZero())
		{
			const FQuat Base = GiroPadre * Reposo;
			const FVector DireccionActual = GiroPadre.RotateVector(DireccionReposo(Hueso));
			OutRotaciones[Hueso] = FQuat::FindBetweenNormals(DireccionActual.GetSafeNormal(), Objetivo.Direccion.GetSafeNormal()) * Base;
		}
		else
		{
			OutRotaciones[Hueso] = GiroPadre * Objetivo.Giro * Reposo;
		}
	}
}

void USAEVUSComponenteAnimacionProcedural::SuavizarYAplicar(const FPose& Pose, const FQuat (&Objetivo)[SAEVUSAnimacion::NumHuesos], float DeltaTime, bool bAccion)
{
	using namespace SAEVUSAnimacion;
	// Rebote del brazo al acertar: vuelve a cero con un muelle vivo.
	RetrocesoImpacto.Avanzar(0.f, Ajuste.FrecuenciaRetrocesoImpacto, AmortiguacionRetrocesoImpacto, DeltaTime);

	// Suavizado escalonado: el tronco llega primero y las puntas después (acción superpuesta). En una acción
	// todo va más seco, pero la hoja y las manos siguen arrastrando un poco.
	const float Base = bAccion ? Ajuste.SuavizadoAccion : Ajuste.Suavizado;
	auto Alfa = [&](float Factor) { return bPoseIniciada ? AlfaSuavizado(Base * Factor, DeltaTime) : 1.f; };
	const float AlfaTramo[] = {
		Alfa(1.f),
		Alfa(Ajuste.FactorSuavizadoCabeza),
		Alfa(bAccion ? 1.f : Ajuste.FactorSuavizadoBrazos),
		Alfa(bAccion ? FMath::Max(Ajuste.FactorSuavizadoManos, 0.85f) : Ajuste.FactorSuavizadoManos),
		Alfa(Ajuste.FactorSuavizadoPiernas),
		bPoseIniciada ? AlfaSuavizado(Ajuste.SuavizadoCapa, DeltaTime) : 1.f,
	};
	const float AlfaCuerpo = AlfaTramo[0];
	bPoseIniciada = true;

	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		const float AlfaHueso = AlfaTramo[static_cast<int32>(TramoDe(static_cast<ESAEVUSHueso>(Hueso)))];
		// Un NaN en la pose no se recupera solo (el Slerp lo arrastra para siempre): vuelve al reposo (ANIMVFX-1).
		if (RotacionActual[Hueso].ContainsNaN())
		{
			RotacionActual[Hueso] = ReposoComponente[Hueso].GetRotation();
		}
		const FQuat ObjetivoHueso = Objetivo[Hueso].ContainsNaN() ? ReposoComponente[Hueso].GetRotation() : Objetivo[Hueso];
		RotacionActual[Hueso] = FQuat::Slerp(RotacionActual[Hueso], ObjetivoHueso, AlfaHueso).GetNormalized();
	}
	DesplazamientoRaizActual = FMath::Lerp(DesplazamientoRaizActual, Pose.DesplazamientoRaiz, AlfaCuerpo);
	EscalaArmaActual = FMath::Lerp(EscalaArmaActual, Pose.EscalaArma, AlfaCuerpo);
	EscalaArmaFuegoActual = FMath::Lerp(EscalaArmaFuegoActual, Pose.EscalaArmaFuego, AlfaCuerpo);

	// Posiciones desde las rotaciones suavizadas: los huesos conservan su longitud.
	FTransform Componente[NumHuesos];
	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		const int32 IndicePadre = Padre[Hueso];
		if (IndicePadre == INDEX_NONE)
		{
			PosicionActual[Hueso] = ReposoComponente[Hueso].GetLocation() + DesplazamientoRaizActual + FVector(0.f, 0.f, AjusteCaderaSuelo);
		}
		else
		{
			const FQuat GiroPadre = RotacionActual[IndicePadre] * ReposoComponente[IndicePadre].GetRotation().Inverse();
			PosicionActual[Hueso] = PosicionActual[IndicePadre] + GiroPadre.RotateVector(ReposoComponente[Hueso].GetLocation() - ReposoComponente[IndicePadre].GetLocation());
		}
		float Escala = 1.f;
		if (Hueso == Indice(ESAEVUSHueso::Arma))
		{
			Escala = FMath::Max(EscalaArmaActual, SAEVUSAnimacion::EscalaArmaOculta);
		}
		else if (Hueso == Indice(ESAEVUSHueso::ArmaFuego))
		{
			Escala = FMath::Max(EscalaArmaFuegoActual, SAEVUSAnimacion::EscalaArmaOculta);
		}
		// La escala de reposo se conserva: el FBX de Blender puede traerla distinta de 1 (unidades del armazón).
		Componente[Hueso] = FTransform(RotacionActual[Hueso], PosicionActual[Hueso], ReposoComponente[Hueso].GetScale3D() * Escala);
	}
	AplicarPiesAlSuelo(Componente, DeltaTime);
	EscribirEnMalla(Componente);
}

bool USAEVUSComponenteAnimacionProcedural::AlturaSueloBajo(const FVector& PuntoMundo, float& OutAltura) const
{
	const UWorld* Mundo = GetWorld();
	if (!Mundo || !Malla)
	{
		return false;
	}
	const FTransform& Transformacion = Malla->GetComponentTransform();
	const float EscalaMalla = FMath::Max(static_cast<float>(Transformacion.GetScale3D().Z), UE_KINDA_SMALL_NUMBER);
	const float OrigenZ = static_cast<float>(Transformacion.GetLocation().Z);
	const float Alcance = Ajuste.AlcanceTrazaPies * EscalaMalla;
	const FVector Inicio(PuntoMundo.X, PuntoMundo.Y, OrigenZ + Alcance);
	const FVector Fin(PuntoMundo.X, PuntoMundo.Y, OrigenZ - Alcance);
	FCollisionQueryParams Parametros(SCENE_QUERY_STAT(PiesAlSuelo), false, GetOwner());
	FHitResult Golpe;
	const FCollisionObjectQueryParams Objetos(FCollisionObjectQueryParams::AllStaticObjects);
	if (!Mundo->LineTraceSingleByObjectType(Golpe, Inicio, Fin, Objetos, Parametros) || Golpe.bStartPenetrating)
	{
		return false;
	}
	OutAltura = static_cast<float>(Golpe.ImpactPoint.Z - OrigenZ) / EscalaMalla;
	return true;
}

void USAEVUSComponenteAnimacionProcedural::AplicarPiesAlSuelo(FTransform (&Componente)[SAEVUSAnimacion::NumHuesos], float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	if (!Ajuste.bPiesAlSuelo || !Movimiento || !MovimientoBase || !Malla)
	{
		return;
	}
	const bool bApoyado = !MovimientoBase->IsFalling() && !Movimiento->EstaEnDash() && !Movimiento->EstaEscalando()
		&& !Movimiento->EstaEnCarreraPared() && !(Vida && Vida->EstaMuerto());

	struct FPierna { ESAEVUSHueso Muslo, Pierna, Pie; };
	const FPierna Piernas[2] = {
		{ ESAEVUSHueso::MusloD, ESAEVUSHueso::PiernaD, ESAEVUSHueso::PieD },
		{ ESAEVUSHueso::MusloI, ESAEVUSHueso::PiernaI, ESAEVUSHueso::PieI } };

	// Ajuste de cada pie hacia el suelo que tiene debajo (0 si no está apoyado o no hay suelo cerca).
	for (int32 Lado = 0; Lado < 2; ++Lado)
	{
		float Objetivo = 0.f;
		float AlturaSuelo = 0.f;
		if (bApoyado && AlturaSueloBajo(Malla->GetComponentTransform().TransformPosition(Componente[Indice(Piernas[Lado].Pie)].GetLocation()), AlturaSuelo))
		{
			Objetivo = FMath::Clamp(AlturaSuelo, -Ajuste.AjusteMaximoPies, Ajuste.AjusteMaximoPies);
		}
		AjustePieSuelo[Lado] = FMath::FInterpTo(AjustePieSuelo[Lado], Objetivo, DeltaTime, Ajuste.VelocidadAjustePies);
	}

	// La cadera baja hasta el pie más bajo (nunca sube: el motor ya pone la cápsula sobre el suelo más alto).
	const float CaderaAplicada = AjusteCaderaSuelo;
	const float ObjetivoCadera = FMath::Min3(0.f, AjustePieSuelo[0], AjustePieSuelo[1]);
	AjusteCaderaSuelo = FMath::FInterpTo(AjusteCaderaSuelo, ObjetivoCadera, DeltaTime, Ajuste.VelocidadAjustePies);

	for (int32 Lado = 0; Lado < 2; ++Lado)
	{
		const FPierna& Hueso = Piernas[Lado];
		const FVector Cadera = Componente[Indice(Hueso.Muslo)].GetLocation();
		const FVector Rodilla = Componente[Indice(Hueso.Pierna)].GetLocation();
		const FVector Pie = Componente[Indice(Hueso.Pie)].GetLocation();
		// El cuerpo ya está bajado CaderaAplicada: el pie sube lo que le falta para llegar a su suelo, solo si está apoyado en la zancada.
		const float Elevacion = static_cast<float>(Pie.Z - ReposoComponente[Indice(Hueso.Pie)].GetLocation().Z);
		const float Contacto = 1.f - FMath::SmoothStep(0.f, Ajuste.AlturaContactoPies, Elevacion);
		const float Desplazamiento = Contacto * (AjustePieSuelo[Lado] - CaderaAplicada);
		if (FMath::Abs(Desplazamiento) < 0.05f)
		{
			continue;
		}
		const float LargoMuslo = static_cast<float>(FVector::Dist(Cadera, Rodilla));
		const float LargoPierna = static_cast<float>(FVector::Dist(Rodilla, Pie));
		const FVector PuntoMedio = (Cadera + Pie) * 0.5f;
		const FVector Polo = (Rodilla - PuntoMedio).IsNearlyZero() ? Adelante : (Rodilla - PuntoMedio);
		const FSAEVUSResultadoPierna Resuelta = SAEVUSCinematicaInversa::ResolverPierna(Cadera, LargoMuslo, LargoPierna, Pie + FVector(0.f, 0.f, Desplazamiento), Polo);
		if (Resuelta.Rodilla.ContainsNaN() || Resuelta.Pie.ContainsNaN())
		{
			continue;
		}
		const FQuat GiroMuslo = FQuat::FindBetweenNormals((Rodilla - Cadera).GetSafeNormal(), (Resuelta.Rodilla - Cadera).GetSafeNormal());
		const FQuat GiroPierna = FQuat::FindBetweenNormals(GiroMuslo.RotateVector((Pie - Rodilla).GetSafeNormal()), (Resuelta.Pie - Resuelta.Rodilla).GetSafeNormal()) * GiroMuslo;
		FTransform& TMuslo = Componente[Indice(Hueso.Muslo)];
		FTransform& TPierna = Componente[Indice(Hueso.Pierna)];
		FTransform& TPie = Componente[Indice(Hueso.Pie)];
		TMuslo.SetRotation((GiroMuslo * TMuslo.GetRotation()).GetNormalized());
		TPierna.SetRotation((GiroPierna * TPierna.GetRotation()).GetNormalized());
		TPierna.SetLocation(Resuelta.Rodilla);
		TPie.SetRotation((GiroPierna * TPie.GetRotation()).GetNormalized());
		TPie.SetLocation(Resuelta.Pie);
	}
}

void USAEVUSComponenteAnimacionProcedural::EscribirEnMalla(const FTransform (&Componente)[SAEVUSAnimacion::NumHuesos])
{
	using namespace SAEVUSAnimacion;
	TArray<FTransform>& Locales = Malla->BoneSpaceTransforms;
	if (Locales.Num() != ComponenteMalla.Num())
	{
		return;
	}
	// Los huesos propios toman su transformación nueva; el resto de la malla sigue en reposo bajo ellos. Los
	// padres van antes que los hijos, así cada local se calcula con el padre ya colocado.
	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		ComponenteMalla[IndiceMalla[Hueso]] = Componente[Hueso];
	}
	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		const int32 HuesoMalla = IndiceMalla[Hueso];
		const int32 PadreHuesoMalla = PadreMalla[HuesoMalla];
		Locales[HuesoMalla] = PadreHuesoMalla == INDEX_NONE ? Componente[Hueso] : Componente[Hueso].GetRelativeTransform(ComponenteMalla[PadreHuesoMalla]);
	}
	Malla->MarkRefreshTransformDirty();
}

FVector USAEVUSComponenteAnimacionProcedural::VelocidadDueno() const
{
	return MovimientoBase ? MovimientoBase->Velocity : FVector::ZeroVector;
}
