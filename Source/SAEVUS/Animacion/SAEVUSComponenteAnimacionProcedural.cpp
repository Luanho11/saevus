// Copyright SAEVUS. Todos los derechos reservados.

#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
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
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "SAEVUS.h"

namespace SAEVUSAnimacion
{
	// Más allá de este giro (grados) el enemigo está detrás: la cabeza no lo sigue.
	constexpr float GiroMaximoParaMirar = 120.f;

	constexpr int32 NumHuesos = static_cast<int32>(ESAEVUSHueso::Total);

	// Ejes del espacio de la malla: el personaje mira a +Y, su derecha es -X.
	const FVector Adelante(0.f, 1.f, 0.f);
	const FVector Derecha(-1.f, 0.f, 0.f);
	const FVector Arriba(0.f, 0.f, 1.f);

	// Por debajo de esta velocidad el personaje se considera quieto (respira en vez de andar).
	constexpr float VelocidadQuieto = 15.f;
	// Velocidad vertical a la que la pose aérea es "subiendo del todo" o "cayendo del todo".
	constexpr float VelocidadVerticalReferencia = 600.f;
	// Velocidad de esprint de referencia para la capa.
	constexpr float VelocidadCapaReferencia = 850.f;
	constexpr float VelocidadCaidaCapaReferencia = 1000.f;
	// Duración del desplome al morir.
	constexpr float DuracionDesplome = 0.5f;
	// Por debajo de este producto con el frente, el paso va hacia atrás (la cadera gira al revés).
	constexpr float FrenteAtras = -0.2f;
	// Límites de la intensidad del aterrizaje (fracción de FlexionAterrizaje).
	constexpr float FuerzaAterrizajeMinima = 0.35f;
	constexpr float FuerzaAterrizajeMaxima = 1.3f;
	// Bajada del cuerpo durante la voltereta del dash.
	constexpr float BajadaVoltereta = 22.f;

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
			{ TEXT("capa_2"), E::Capa1, E::Total, -Arriba },
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
		};
		return Tabla[static_cast<int32>(Hueso)];
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

	float SuavizarCurva(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return X * X * (3.f - 2.f * X);
	}

	/** Salida rápida y frenada al final: el tajo "corta" y se detiene. */
	float SalidaRapida(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return 1.f - (1.f - X) * (1.f - X) * (1.f - X);
	}

	float AlfaSuavizado(float Rapidez, float DeltaTime)
	{
		return Rapidez <= 0.f ? 1.f : 1.f - FMath::Exp(-Rapidez * DeltaTime);
	}

	/** Trayectoria de un tajo: dirección del brazo al empezar y al terminar, y giro del pecho. */
	struct FTrayectoria
	{
		FVector Inicio;
		FVector Fin;
		float GiroInicio;
		float GiroFin;
		float InclinacionInicio;
		float InclinacionFin;
	};

	FTrayectoria TrayectoriaAtaque(int32 IndiceCombo, bool bAereo, ESAEVUSDireccionAtaque Direccion)
	{
		if (Direccion == ESAEVUSDireccionAtaque::Arriba)
		{
			// De delante-abajo a por encima de la cabeza, con el cuerpo echado atrás.
			return { (Adelante * 0.8f - Arriba * 0.2f + Derecha * 0.2f).GetSafeNormal(), (Arriba * 0.95f - Adelante * 0.3f).GetSafeNormal(), 10.f, -5.f, -5.f, -18.f };
		}
		if (bAereo)
		{
			// Diagonal de arriba-derecha a abajo-izquierda.
			return { (Arriba * 0.7f + Derecha * 0.6f).GetSafeNormal(), (-Arriba * 0.6f - Derecha * 0.3f + Adelante * 0.6f).GetSafeNormal(), 20.f, -25.f, -5.f, 15.f };
		}
		switch (IndiceCombo)
		{
		case 0:
			// Horizontal de derecha a izquierda.
			return { (Derecha * 0.9f - Adelante * 0.25f + Arriba * 0.15f).GetSafeNormal(), (-Derecha * 0.8f + Adelante * 0.6f + Arriba * 0.1f).GetSafeNormal(), 30.f, -35.f, 0.f, 8.f };
		case 1:
			// Revés: de izquierda a derecha.
			return { (-Derecha * 0.7f + Adelante * 0.5f + Arriba * 0.3f).GetSafeNormal(), (Derecha * 0.9f + Adelante * 0.3f).GetSafeNormal(), -30.f, 30.f, 0.f, 8.f };
		default:
			// Remate vertical: por encima de la cabeza hasta el suelo delante.
			return { (Arriba * 0.9f - Adelante * 0.4f).GetSafeNormal(), (Adelante * 0.6f - Arriba * 0.8f).GetSafeNormal(), 0.f, 0.f, -12.f, 28.f };
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
		RotacionActual[Hueso] = FQuat::Identity;
	}
}

FName USAEVUSComponenteAnimacionProcedural::NombreHueso(ESAEVUSHueso Hueso)
{
	using namespace SAEVUSAnimacion;
	return FName(Definicion(Hueso).Nombre);
}

void USAEVUSComponenteAnimacionProcedural::BeginPlay()
{
	using namespace SAEVUSAnimacion;
	Super::BeginPlay();
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	if (Personaje)
	{
		Movimiento = Cast<USAEVUSComponenteMovimiento>(Personaje->GetCharacterMovement());
	}
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
		Movimiento->AlSaltoAereo.AddUObject(this, &ThisClass::AlSaltoAereo);
		Movimiento->AlSaltoPared.AddUObject(this, &ThisClass::AlSaltoPared);
	}
	if (Vida)
	{
		Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::AlCambiarVida);
	}
	if (!Malla)
	{
		FijarMalla(GetOwner()->FindComponentByClass<UPoseableMeshComponent>());
	}
}

void USAEVUSComponenteAnimacionProcedural::FijarMalla(UPoseableMeshComponent* NuevaMalla)
{
	using namespace SAEVUSAnimacion;
	Malla = NuevaMalla;
	PrepararEsqueleto();
}

void USAEVUSComponenteAnimacionProcedural::PrepararEsqueleto()
{
	using namespace SAEVUSAnimacion;
	bEsqueletoValido = false;
	bPoseIniciada = false;
	if (!Malla || !Malla->GetSkinnedAsset())
	{
		return;
	}
	const FReferenceSkeleton& Referencia = Malla->GetSkinnedAsset()->GetRefSkeleton();
	const TArray<FTransform>& PoseReferencia = Referencia.GetRefBonePose();
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
		// Transformación de reposo en espacio de malla: componer hacia arriba por toda la jerarquía.
		FTransform Componente = PoseReferencia[IndiceMalla[Hueso]];
		for (int32 Ancestro = Referencia.GetParentIndex(IndiceMalla[Hueso]); Ancestro != INDEX_NONE; Ancestro = Referencia.GetParentIndex(Ancestro))
		{
			Componente = Componente * PoseReferencia[Ancestro];
		}
		ReposoComponente[Hueso] = Componente;
		RotacionActual[Hueso] = Componente.GetRotation();
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
	using namespace SAEVUSAnimacion;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Actualizar(DeltaTime);
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
	const bool bAccion = (Combate && Combate->EstaAtacando()) || (Movimiento && Movimiento->EstaEnDash()) || TiempoGiroSaltoPared > 0.f;
	SuavizarYAplicar(Pose, Objetivo, DeltaTime, bAccion);
}

void USAEVUSComponenteAnimacionProcedural::ConstruirPoseObjetivo(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	if (!Movimiento)
	{
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

	// Aterrizaje: flexión breve al pasar del aire al suelo, más honda cuanto más rápida la caída.
	const bool bEnAire = Movimiento->IsFalling();
	if (bEstabaEnAire && Movimiento->IsMovingOnGround())
	{
		TiempoAterrizaje = Ajuste.DuracionAterrizaje;
		FuerzaAterrizaje = FMath::Clamp(-VelocidadVerticalAire / Ajuste.VelocidadCaidaAterrizaje, FuerzaAterrizajeMinima, FuerzaAterrizajeMaxima);
		TiempoGiroSaltoPared = 0.f;
	}
	if (bEnAire)
	{
		VelocidadVerticalAire = VelocidadVertical;
	}
	bEstabaEnAire = bEnAire;
	TiempoAterrizaje = FMath::Max(0.f, TiempoAterrizaje - DeltaTime);
	TiempoGiroSaltoPared = FMath::Max(0.f, TiempoGiroSaltoPared - DeltaTime);
	TiempoRecogidaSaltoAereo = FMath::Max(0.f, TiempoRecogidaSaltoAereo - DeltaTime);
	TiempoReaccionGolpe = FMath::Max(0.f, TiempoReaccionGolpe - DeltaTime);

	// Capa base del cuerpo.
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
	else if (Personaje && Personaje->EstaSentado())
	{
		PoseSentado(Pose);
	}
	else if (Alma && Alma->EstaCurando())
	{
		PoseCuracion(Pose);
	}
	else
	{
		PoseLocomocion(Pose, Velocidad, DeltaTime);
	}

	// Parte superior: ataque o arma de fuego por encima de la base.
	const bool bArmaFuego = Armas && Armas->TieneArmaFuegoEquipada();
	if (Combate && Combate->ObtenerAtaqueActual())
	{
		PoseAtaque(Pose, *Combate->ObtenerAtaqueActual(), Combate->ObtenerIndiceCombo(), Combate->EsCadenaAerea(), Combate->ObtenerDireccionAtaque(),
			Combate->ObtenerTiempoAtaque());
		if (Combate->EsAtaqueCargado())
		{
			// Giro completo durante la fase activa, con el aguijón extendido de lado.
			const FSAEVUSDefinicionAtaque& Ataque = *Combate->ObtenerAtaqueActual();
			const float U = FMath::Clamp((Combate->ObtenerTiempoAtaque() - Ataque.TiempoPreparacion) / FMath::Max(Ataque.TiempoActivo, UE_KINDA_SMALL_NUMBER), 0.f, 1.f);
			Pose.Huesos[Indice(ESAEVUSHueso::Raiz)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, 360.f * U) };
			Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Derecha * 0.9f + Adelante * 0.3f).GetSafeNormal() };
			Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, (Derecha * 0.9f + Adelante * 0.3f).GetSafeNormal() };
		}
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

	PoseCapa(Pose, Velocidad, VelocidadVertical);
	PoseMirada(Pose);
	PoseReaccionGolpe(Pose);
}

void USAEVUSComponenteAnimacionProcedural::PoseLocomocion(FPose& Pose, float Velocidad, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	const float Amplitud = Velocidad < VelocidadQuieto ? 0.f : FMath::Clamp(Velocidad / Ajuste.VelocidadReferencia, 0.f, 1.5f);
	FaseCiclo = FMath::Fmod(FaseCiclo + Velocidad * DeltaTime / Ajuste.LongitudCiclo * UE_TWO_PI, UE_TWO_PI);
	const float Seno = FMath::Sin(FaseCiclo);
	const float Coseno = FMath::Cos(FaseCiclo);

	// GunZ: el cuerpo mira a la cámara y se mueve en ocho direcciones. Las piernas oscilan en la
	// dirección del paso; de lado, la cadera gira hacia él (al revés si va hacia atrás) y la columna
	// lo compensa, así el pecho sigue de frente. Hacia atrás, el cuerpo se echa algo atrás.
	const FVector Paso = DireccionMalla(Movimiento->Velocity);
	const float Frente = static_cast<float>(FVector::DotProduct(Paso, Adelante));
	const float Lado = static_cast<float>(FVector::DotProduct(Paso, Derecha));
	const float GiroCadera = Ajuste.GiroCaderaLateral * Lado * (Frente < FrenteAtras ? -1.f : 1.f) * FMath::Min(1.f, Amplitud);
	const float FactorInclinacion = Frente < 0.f ? FMath::Lerp(1.f, Ajuste.FraccionInclinacionAtras, -Frente) : 1.f;
	const float Inclinacion = Ajuste.InclinacionCarrera * Amplitud * 0.5f * FactorInclinacion;
	Pose.Huesos[Indice(ESAEVUSHueso::Pelvis)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, GiroCadera) };

	auto Pierna = [&](ESAEVUSHueso Muslo, ESAEVUSHueso PiernaHueso, float Signo)
	{
		Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Paso, Ajuste.GiroMuslo * Amplitud * Signo * Seno) };
		// La rodilla se dobla sobre todo cuando la pierna va hacia delante en el aire.
		const float Flexion = Ajuste.FlexionRodilla * Amplitud * FMath::Max(0.f, Signo * Coseno) + 8.f * Amplitud;
		Pose.Huesos[Indice(PiernaHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Flexion) };
	};
	Pierna(ESAEVUSHueso::MusloD, ESAEVUSHueso::PiernaD, 1.f);
	Pierna(ESAEVUSHueso::MusloI, ESAEVUSHueso::PiernaI, -1.f);

	// Brazos en contrafase, un poco separados del cuerpo.
	auto Brazo = [&](ESAEVUSHueso BrazoHueso, ESAEVUSHueso Antebrazo, const FVector& Lado, float Signo)
	{
		const FVector Colgado = Dir(-Arriba, Adelante, -Ajuste.GiroBrazo * Amplitud * Signo * Seno);
		Pose.Huesos[Indice(BrazoHueso)] = { true, (Colgado + Lado * 0.15f).GetSafeNormal() };
		Pose.Huesos[Indice(Antebrazo)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 20.f + 25.f * Amplitud) };
	};
	Brazo(ESAEVUSHueso::BrazoD, ESAEVUSHueso::AntebrazoD, Derecha, 1.f);
	Brazo(ESAEVUSHueso::BrazoI, ESAEVUSHueso::AntebrazoI, -Derecha, -1.f);

	// Tronco: inclinación con la velocidad, giro de hombros y respiración en reposo.
	const float Respiracion = Ajuste.AmplitudRespiracion * FMath::Sin(TiempoTotal * Ajuste.FrecuenciaRespiracion * UE_TWO_PI) * (1.f - FMath::Min(1.f, Amplitud));
	Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, -GiroCadera) * Girar(Arriba, Paso, Inclinacion) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector,
		Girar(Adelante, Derecha, 6.f * Amplitud * Seno) * Girar(Arriba, Paso, Inclinacion) * Girar(Arriba, Adelante, Respiracion) };
	Pose.DesplazamientoRaiz.Z = Ajuste.BotePaso * Amplitud * FMath::Abs(Seno);

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
	Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, Dir(-Arriba, Adelante, GiroMuslo) };
	Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, Dir(-Arriba, Adelante, GiroMuslo * 0.5f) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Flexion) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Flexion * 0.8f) };

	const float AlturaBrazos = FMath::Lerp(0.6f, 0.2f, Subida);
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Derecha * 0.8f + Arriba * AlturaBrazos + Adelante * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (-Derecha * 0.8f + Arriba * AlturaBrazos + Adelante * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, FMath::Lerp(-6.f, 8.f, Subida)) };

	// Salto de pared: una vuelta sobre sí mismo que acaba otra vez de frente a la cámara.
	if (TiempoGiroSaltoPared > 0.f && Ajuste.GradosGiroSaltoPared > 0.f)
	{
		const float U = 1.f - TiempoGiroSaltoPared / FMath::Max(Ajuste.DuracionGiroSaltoPared, UE_KINDA_SMALL_NUMBER);
		Pose.Huesos[Indice(ESAEVUSHueso::Raiz)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, Ajuste.GradosGiroSaltoPared * SuavizarCurva(U)) };
	}
}

void USAEVUSComponenteAnimacionProcedural::PoseDash(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	const FVector Direccion = DireccionMalla(Movimiento->ObtenerDireccionDash());
	if (Ajuste.GradosVolteretaDash > 0.f && FVector::DotProduct(Direccion, Adelante) < Ajuste.UmbralDashFrontal)
	{
		// A los lados o hacia atrás (GunZ): voltereta recogida sobre la pelvis en la dirección del dash.
		// Los miembros van en giros locales para rodar con el cuerpo.
		const float Vuelta = Ajuste.GradosVolteretaDash * SuavizarCurva(Movimiento->ObtenerFraccionDash());
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
	// Hacia delante: cuerpo lanzado, piernas en zancada, brazos atrás; la cabeza mira al frente.
	Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, Ajuste.InclinacionDash * 0.5f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, Ajuste.InclinacionDash * 0.5f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, -Adelante, Ajuste.InclinacionDash * 0.6f) };
	Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, Dir(-Arriba, -Adelante, 30.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, Dir(-Arriba, Adelante, 35.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 45.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 30.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (-Adelante * 0.7f - Arriba * 0.6f + Derecha * 0.25f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (-Adelante * 0.7f - Arriba * 0.6f - Derecha * 0.25f).GetSafeNormal() };
	Pose.DesplazamientoRaiz.Z = -10.f;
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
	}
}

void USAEVUSComponenteAnimacionProcedural::PoseEscalada(FPose& Pose, float DeltaTime)
{
	using namespace SAEVUSAnimacion;
	FaseCiclo = FMath::Fmod(FaseCiclo + FMath::Abs(static_cast<float>(Movimiento->Velocity.Z)) * DeltaTime / Ajuste.LongitudCiclo * UE_TWO_PI, UE_TWO_PI);
	const float Seno = FMath::Sin(FaseCiclo);
	// Brazos alternando hacia arriba contra la pared, piernas empujando.
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Dir(Arriba, Adelante, 30.f + 25.f * Seno) + Derecha * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Dir(Arriba, Adelante, 30.f - 25.f * Seno) - Derecha * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, Dir(-Arriba, Adelante, 25.f + 30.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, Dir(-Arriba, Adelante, 25.f - 30.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 50.f + 30.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 50.f - 30.f * Seno) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 10.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Adelante, Arriba, 18.f) };
}

void USAEVUSComponenteAnimacionProcedural::PoseAtaque(FPose& Pose, const FSAEVUSDefinicionAtaque& Ataque, int32 IndiceCombo, bool bAereo,
	ESAEVUSDireccionAtaque Direccion, float Tiempo)
{
	using namespace SAEVUSAnimacion;
	const FTrayectoria Trayectoria = TrayectoriaAtaque(IndiceCombo, bAereo, Direccion);
	const FVector PosturaReposo = (Adelante * 0.5f - Arriba * 0.7f + Derecha * 0.25f).GetSafeNormal();

	// Dirección del brazo y del aguijón a lo largo de la línea de tiempo del ataque.
	FVector DireccionBrazo;
	float GiroPecho;
	float Inclinacion;
	float FlexionAntebrazo = 0.f;
	const float FinActivo = Ataque.TiempoPreparacion + Ataque.TiempoActivo;
	if (Tiempo < Ataque.TiempoPreparacion)
	{
		const float U = SuavizarCurva(Tiempo / FMath::Max(Ataque.TiempoPreparacion, UE_KINDA_SMALL_NUMBER));
		DireccionBrazo = Interpolar(PosturaReposo, Trayectoria.Inicio, U);
		GiroPecho = FMath::Lerp(0.f, Trayectoria.GiroInicio, U);
		Inclinacion = FMath::Lerp(0.f, Trayectoria.InclinacionInicio, U);
		FlexionAntebrazo = 35.f * U;
	}
	else if (Tiempo < FinActivo)
	{
		const float U = SalidaRapida((Tiempo - Ataque.TiempoPreparacion) / FMath::Max(Ataque.TiempoActivo, UE_KINDA_SMALL_NUMBER));
		DireccionBrazo = Interpolar(Trayectoria.Inicio, Trayectoria.Fin, U);
		GiroPecho = FMath::Lerp(Trayectoria.GiroInicio, Trayectoria.GiroFin, U);
		Inclinacion = FMath::Lerp(Trayectoria.InclinacionInicio, Trayectoria.InclinacionFin, U);
	}
	else
	{
		const float U = SuavizarCurva((Tiempo - FinActivo) / FMath::Max(Ataque.TiempoRecuperacion, UE_KINDA_SMALL_NUMBER));
		DireccionBrazo = Interpolar(Trayectoria.Fin, PosturaReposo, U);
		GiroPecho = FMath::Lerp(Trayectoria.GiroFin, 0.f, U);
		Inclinacion = FMath::Lerp(Trayectoria.InclinacionFin, 0.f, U);
	}

	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, DireccionBrazo };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, FlexionAntebrazo) };
	// El aguijón prolonga el brazo: así la hoja barre el arco del golpe.
	Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, DireccionBrazo };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (-Derecha * 0.6f - Adelante * 0.3f - Arriba * 0.6f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, GiroPecho * 0.4f) * Girar(Arriba, Adelante, Inclinacion * 0.5f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Adelante, Derecha, GiroPecho * 0.6f) * Girar(Arriba, Adelante, Inclinacion * 0.5f) };

	if (!bAereo)
	{
		// Postura de estocada en el suelo.
		Pose.Huesos[Indice(ESAEVUSHueso::MusloD)] = { true, Dir(-Arriba, Adelante, 25.f) };
		Pose.Huesos[Indice(ESAEVUSHueso::MusloI)] = { true, Dir(-Arriba, -Adelante, 20.f) };
		Pose.Huesos[Indice(ESAEVUSHueso::PiernaD)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 30.f) };
		Pose.Huesos[Indice(ESAEVUSHueso::PiernaI)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 15.f) };
		Pose.DesplazamientoRaiz.Z = -6.f;
	}
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
	const float P = SuavizarCurva(TiempoMuerto / DuracionDesplome);
	// Rodillas que ceden y caída hacia delante.
	Pose.Huesos[Indice(ESAEVUSHueso::Raiz)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 75.f * P) };
	Pose.DesplazamientoRaiz.Z = -50.f * P;
	for (const ESAEVUSHueso PiernaHueso : { ESAEVUSHueso::PiernaD, ESAEVUSHueso::PiernaI })
	{
		Pose.Huesos[Indice(PiernaHueso)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, 80.f * P) };
	}
	for (const ESAEVUSHueso Muslo : { ESAEVUSHueso::MusloD, ESAEVUSHueso::MusloI })
	{
		Pose.Huesos[Indice(Muslo)] = { true, Dir(-Arriba, Adelante, 50.f * P) };
	}
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Adelante * 0.5f - Arriba * 0.8f + Derecha * 0.3f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Adelante * 0.5f - Arriba * 0.8f - Derecha * 0.3f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 25.f * P) };
	PoseCapa(Pose, 0.f, 0.f);
}

void USAEVUSComponenteAnimacionProcedural::PoseCuracion(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	// Agachado y recogido, las manos juntas delante del pecho, la cabeza baja: concentra el alma.
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
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 60.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 60.f) };
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
	// Brazos al frente a la altura del pecho y el aguijón cruzado en diagonal: cubre el frente.
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoD)] = { true, (Adelante * 0.8f - Arriba * 0.25f + Derecha * 0.3f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoD)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 35.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Arma)] = { true, (-Derecha * 0.8f + Arriba * 0.5f + Adelante * 0.3f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::BrazoI)] = { true, (Adelante * 0.8f - Arriba * 0.3f - Derecha * 0.2f).GetSafeNormal() };
	Pose.Huesos[Indice(ESAEVUSHueso::AntebrazoI)] = { false, FVector::ZeroVector, Girar(-Arriba, Adelante, 35.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Pecho)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 8.f) };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, Adelante, 6.f) };
}

void USAEVUSComponenteAnimacionProcedural::AplicarParpadeo()
{
	if (!Malla)
	{
		return;
	}
	const bool bParpadeando = Vida && !Vida->EstaMuerto() && Vida->EstaEnInvulnerabilidadTrasGolpe();
	const bool bVisible = !bParpadeando || FMath::Frac(TiempoTotal * Ajuste.FrecuenciaParpadeo) < 0.5f;
	if (Malla->IsVisible() != bVisible)
	{
		Malla->SetVisibility(bVisible);
	}
}

void USAEVUSComponenteAnimacionProcedural::PoseCapa(FPose& Pose, float Velocidad, float VelocidadVertical)
{
	using namespace SAEVUSAnimacion;
	// La capa se va hacia atrás con la velocidad y se levanta al caer; su suavizado bajo la hace ondear.
	const float Atras = FMath::Clamp(Velocidad / VelocidadCapaReferencia, 0.f, 1.5f) * 40.f
		+ FMath::Clamp(-VelocidadVertical / VelocidadCaidaCapaReferencia, 0.f, 1.f) * 35.f + 4.f;
	const float Ondeo = 4.f * FMath::Sin(TiempoTotal * 3.1f);
	Pose.Huesos[Indice(ESAEVUSHueso::Capa1)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Atras * 0.7f + Ondeo) };
	Pose.Huesos[Indice(ESAEVUSHueso::Capa2)] = { false, FVector::ZeroVector, Girar(-Arriba, -Adelante, Atras * 0.5f + Ondeo * 1.5f) };
}

void USAEVUSComponenteAnimacionProcedural::AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador)
{
	if (Delta < 0.f && !ComponenteVida->EstaMuerto())
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
	// Sacudida: máximo al principio, vuelve suave.
	const float P = TiempoReaccionGolpe / FMath::Max(Ajuste.DuracionReaccionGolpe, UE_KINDA_SMALL_NUMBER);
	const float Grados = Ajuste.InclinacionReaccionGolpe * P * P;
	Pose.Huesos[Indice(ESAEVUSHueso::Columna)] = { false, FVector::ZeroVector, Girar(Arriba, -Adelante, Grados) };
	Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)] = { false, FVector::ZeroVector, Girar(Arriba, -Adelante, Grados * 1.2f) };
}

void USAEVUSComponenteAnimacionProcedural::PoseMirada(FPose& Pose)
{
	using namespace SAEVUSAnimacion;
	FObjetivo& Cabeza = Pose.Huesos[Indice(ESAEVUSHueso::Cabeza)];
	if (Ajuste.RadioMirada <= 0.f || !Malla || Cabeza.bApuntar || !Cabeza.Giro.Equals(FQuat::Identity))
	{
		return;
	}
	const FVector Ojos = Malla->GetComponentLocation() + Malla->GetComponentQuat().RotateVector(ReposoComponente[Indice(ESAEVUSHueso::Cabeza)].GetLocation());
	const AActor* Mas = nullptr;
	double MejorDistancia = FMath::Square(static_cast<double>(Ajuste.RadioMirada));
	for (TActorIterator<ASAEVUSEnemigo> Enemigo(GetWorld()); Enemigo; ++Enemigo)
	{
		const double Distancia = FVector::DistSquared(Enemigo->GetActorLocation(), Ojos);
		if (Distancia < MejorDistancia && Enemigo->ObtenerIA()->ObtenerEstado() != ESAEVUSEstadoEnemigo::Muerto)
		{
			MejorDistancia = Distancia;
			Mas = *Enemigo;
		}
	}
	if (!Mas)
	{
		return;
	}
	// Dirección al enemigo en el espacio de la malla, en giro (lados) e inclinación (arriba/abajo).
	const FVector Hacia = Malla->GetComponentQuat().UnrotateVector(Mas->GetActorLocation() - Ojos).GetSafeNormal();
	const float Giro = FMath::RadiansToDegrees(FMath::Atan2(FVector::DotProduct(Hacia, Derecha), FVector::DotProduct(Hacia, Adelante)));
	const float Inclinacion = FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(static_cast<float>(FVector::DotProduct(Hacia, Arriba)), -1.f, 1.f)));
	// Detrás del todo no se gira: quedaría forzado.
	if (FMath::Abs(Giro) > GiroMaximoParaMirar)
	{
		return;
	}
	Cabeza.Giro = Girar(Adelante, Derecha, FMath::Clamp(Giro, -Ajuste.GiroMaximoCabeza, Ajuste.GiroMaximoCabeza))
		* Girar(Adelante, Arriba, FMath::Clamp(Inclinacion, -Ajuste.InclinacionMaximaCabeza, Ajuste.InclinacionMaximaCabeza));
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
	const FVector DireccionMalla = (RotacionActual[Hueso] * ReposoComponente[Hueso].GetRotation().Inverse()).RotateVector(DireccionReposo(Hueso));
	OutBase = Componente.TransformPosition(PosicionActual[Hueso]);
	OutPunta = OutBase + Componente.TransformVectorNoScale(DireccionMalla) * Ajuste.LongitudHoja;
	return true;
}

void USAEVUSComponenteAnimacionProcedural::AlSaltoAereo()
{
	TiempoRecogidaSaltoAereo = Ajuste.DuracionRecogidaSaltoAereo;
}

void USAEVUSComponenteAnimacionProcedural::AlSaltoPared()
{
	TiempoGiroSaltoPared = Ajuste.DuracionGiroSaltoPared;
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
	using namespace SAEVUSAnimacion;
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
	const float AlfaCuerpo = bPoseIniciada ? AlfaSuavizado(bAccion ? Ajuste.SuavizadoAccion : Ajuste.Suavizado, DeltaTime) : 1.f;
	const float AlfaCapa = bPoseIniciada ? AlfaSuavizado(Ajuste.SuavizadoCapa, DeltaTime) : 1.f;
	bPoseIniciada = true;

	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		const bool bCapa = Hueso == Indice(ESAEVUSHueso::Capa1) || Hueso == Indice(ESAEVUSHueso::Capa2);
		RotacionActual[Hueso] = FQuat::Slerp(RotacionActual[Hueso], Objetivo[Hueso], bCapa ? AlfaCapa : AlfaCuerpo).GetNormalized();
	}
	DesplazamientoRaizActual = FMath::Lerp(DesplazamientoRaizActual, Pose.DesplazamientoRaiz, AlfaCuerpo);
	EscalaArmaActual = FMath::Lerp(EscalaArmaActual, Pose.EscalaArma, AlfaCuerpo);
	EscalaArmaFuegoActual = FMath::Lerp(EscalaArmaFuegoActual, Pose.EscalaArmaFuego, AlfaCuerpo);

	// Posiciones desde las rotaciones suavizadas: los huesos conservan su longitud.
	FVector Posicion[NumHuesos];
	for (int32 Hueso = 0; Hueso < NumHuesos; ++Hueso)
	{
		const int32 IndicePadre = Padre[Hueso];
		if (IndicePadre == INDEX_NONE)
		{
			Posicion[Hueso] = ReposoComponente[Hueso].GetLocation() + DesplazamientoRaizActual;
		}
		else
		{
			const FQuat GiroPadre = RotacionActual[IndicePadre] * ReposoComponente[IndicePadre].GetRotation().Inverse();
			Posicion[Hueso] = Posicion[IndicePadre] + GiroPadre.RotateVector(ReposoComponente[Hueso].GetLocation() - ReposoComponente[IndicePadre].GetLocation());
		}
		float Escala = 1.f;
		if (Hueso == Indice(ESAEVUSHueso::Arma))
		{
			Escala = EscalaArmaActual;
		}
		else if (Hueso == Indice(ESAEVUSHueso::ArmaFuego))
		{
			Escala = EscalaArmaFuegoActual;
		}
		// La escala de reposo se conserva: el FBX de Blender puede traerla distinta de 1 (unidades del armazón).
		const FVector EscalaFinal = ReposoComponente[Hueso].GetScale3D() * Escala;
		Malla->SetBoneTransformByName(NombreHueso(static_cast<ESAEVUSHueso>(Hueso)), FTransform(RotacionActual[Hueso], Posicion[Hueso], EscalaFinal), EBoneSpaces::ComponentSpace);
		PosicionActual[Hueso] = Posicion[Hueso];
	}
}
