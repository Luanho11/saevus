// Copyright SAEVUS. Todos los derechos reservados.

#include "Enemigos/SAEVUSEnemigo.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Audio/SAEVUSComponenteSonidoEnemigo.h"
#include "Components/PoseableMeshComponent.h"
#include "Efectos/SAEVUSComponenteParticulas.h"
#include "Enemigos/SAEVUSComponenteEscudo.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

FSAEVUSEventoEnemigo ASAEVUSEnemigo::AlMorirEnemigo;

namespace SAEVUSEnemigo
{
	const FSAEVUSAjusteEnemigo AjusteBase;

	// Formas del motor: 100 cm, pivote en el centro.
	const TCHAR* const RutaEsfera = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	const TCHAR* const RutaCono = TEXT("/Engine/BasicShapes/Cone.Cone");
	constexpr float TamanoForma = 100.f;

	// Proporciones del boceto respecto a la cápsula (fracciones del radio y la media altura).
	constexpr float FraccionMascara = 0.7f;
	constexpr float AdelanteMascara = 0.55f;
	constexpr float AlturaMascara = 0.35f;
	constexpr float FraccionCuerno = 0.35f;
	constexpr float LadoCuerno = 0.35f;
	constexpr float AlturaCuerno = 0.95f;
	constexpr float InclinacionCuerno = 20.f;

	// Aspecto según el estado: encogido al avisar, estirado al atacar; brillo de la máscara.
	constexpr float AplastadoAviso = 0.25f;
	constexpr float EstiradoAtaque = 0.2f;
	constexpr float RapidezEscala = 18.f;
	constexpr float BrilloReposo = 0.25f;
	constexpr float BrilloAviso = 3.f;
	constexpr int32 ParticulasMuerte = 40;
	constexpr float VelocidadParticulasMuerte = 300.f;
	constexpr float DispersionParticulasMuerte = 250.f;

	// Destello al recibir un golpe: duración, mezcla hacia el blanco y brillo.
	constexpr float DuracionDestello = 0.12f;
	constexpr float MezclaDestello = 0.85f;
	constexpr float BrilloDestello = 1.5f;

	// Ranura del cuerpo con esqueleto que brilla al avisar (nombre de material de Blender).
	const TCHAR* const PrefijoRanuraArma = TEXT("Arma");
	// El cuerpo con esqueleto mira a +Y: se gira para mirar al frente del actor (+X), como el protagonista.
	constexpr float GiroCuerpoAnimado = -90.f;

	const FName ParametroColor(TEXT("Color"));
	const FName ParametroAutoIluminacion(TEXT("AutoIluminacion"));
}

ASAEVUSEnemigo::ASAEVUSEnemigo()
{
	using namespace SAEVUSEnemigo;
	PrimaryActorTick.bCanEverTick = true;

	// Sin controlador de IA: la IA es un componente y el movimiento simula igualmente.
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bCanWalkOffLedges = false;
	bUseControllerRotationYaw = false;
	AutoPossessAI = EAutoPossessAI::Disabled;

	Vida = CreateDefaultSubobject<USAEVUSComponenteVida>(TEXT("Vida"));
	IA = CreateDefaultSubobject<USAEVUSComponenteIAEnemigo>(TEXT("IA"));

	Piezas = CreateDefaultSubobject<USceneComponent>(TEXT("Piezas"));
	Piezas->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Esfera(RutaEsfera);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cono(RutaCono);
	auto CrearPieza = [this](const TCHAR* Nombre, UStaticMesh* Malla)
	{
		UStaticMeshComponent* Pieza = CreateDefaultSubobject<UStaticMeshComponent>(Nombre);
		Pieza->SetupAttachment(Piezas);
		Pieza->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Pieza->SetStaticMesh(Malla);
		return Pieza;
	};
	Cuerpo = CrearPieza(TEXT("Cuerpo"), Esfera.Object);
	Mascara = CrearPieza(TEXT("Mascara"), Esfera.Object);
	CuernoD = CrearPieza(TEXT("CuernoD"), Cono.Object);
	CuernoI = CrearPieza(TEXT("CuernoI"), Cono.Object);

	CuerpoAnimado = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("CuerpoAnimado"));
	CuerpoAnimado->SetupAttachment(Piezas);
	CuerpoAnimado->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CuerpoAnimado->SetVisibility(false);
	Animacion = CreateDefaultSubobject<USAEVUSComponenteAnimacionProcedural>(TEXT("Animacion"));

	Efectos = CreateDefaultSubobject<USAEVUSComponenteParticulas>(TEXT("Efectos"));
	Efectos->SetupAttachment(RootComponent);
	Sonido = CreateDefaultSubobject<USAEVUSComponenteSonidoEnemigo>(TEXT("Sonido"));
	Escudo = CreateDefaultSubobject<USAEVUSComponenteEscudo>(TEXT("Escudo"));
}

const FSAEVUSAjusteEnemigo& ASAEVUSEnemigo::ObtenerAjuste() const
{
	return Datos ? Datos->Enemigo : SAEVUSEnemigo::AjusteBase;
}

void ASAEVUSEnemigo::FijarDatos(const USAEVUSDatosEnemigo* NuevosDatos)
{
	Datos = NuevosDatos;
	AplicarAjuste();
	CrearMateriales();
	if (HasActorBegunPlay())
	{
		Sonido->ActualizarTipo();
	}
}

bool ASAEVUSEnemigo::TieneEsqueleto() const
{
	return Datos && Datos->MallaEsqueletica;
}

UMaterialInterface* ASAEVUSEnemigo::ObtenerMaterialProyectil() const
{
	return MaterialMascara;
}

void ASAEVUSEnemigo::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	AplicarAjuste();
}

void ASAEVUSEnemigo::BeginPlay()
{
	Super::BeginPlay();
	AplicarAjuste();
	CrearMateriales();
	Vida->AlMorir.AddUniqueDynamic(this, &ThisClass::ManejarMuerte);
	Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::ManejarCambioVida);
}

void ASAEVUSEnemigo::ManejarCambioVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador)
{
	if (Delta < 0.f)
	{
		TiempoDestello = SAEVUSEnemigo::DuracionDestello;
	}
}

float ASAEVUSEnemigo::ObtenerDestello() const
{
	return FMath::Clamp(TiempoDestello / SAEVUSEnemigo::DuracionDestello, 0.f, 1.f);
}

void ASAEVUSEnemigo::AplicarDestello(float DeltaSeconds)
{
	using namespace SAEVUSEnemigo;
	TiempoDestello = FMath::Max(0.f, TiempoDestello - DeltaSeconds);
	const float Fraccion = ObtenerDestello();
	if (!MaterialCuerpo || (Fraccion <= 0.f && !bDestelloPintado))
	{
		return;
	}
	// Mientras dura se pinta cada fotograma; al acabar, una vez más con los colores del tipo.
	const FSAEVUSAjusteEnemigo& Ajuste = ObtenerAjuste();
	MaterialCuerpo->SetVectorParameterValue(ParametroColor, FMath::Lerp(Ajuste.ColorCuerpo, FLinearColor::White, Fraccion * MezclaDestello));
	MaterialCuerpo->SetScalarParameterValue(ParametroAutoIluminacion, Fraccion * BrilloDestello);
	if (MaterialMascara)
	{
		MaterialMascara->SetVectorParameterValue(ParametroColor, FMath::Lerp(Ajuste.ColorMascara, FLinearColor::White, Fraccion * MezclaDestello));
	}
	for (int32 Ranura = 0; Ranura < MaterialesPiezas.Num(); ++Ranura)
	{
		if (MaterialesPiezas[Ranura] != MaterialMascara)
		{
			MaterialesPiezas[Ranura]->SetVectorParameterValue(ParametroColor, FMath::Lerp(ColoresPiezasBase[Ranura], FLinearColor::White, Fraccion * MezclaDestello));
			MaterialesPiezas[Ranura]->SetScalarParameterValue(ParametroAutoIluminacion, Fraccion * BrilloDestello);
		}
	}
	bDestelloPintado = Fraccion > 0.f;
}

void ASAEVUSEnemigo::AplicarAjuste()
{
	using namespace SAEVUSEnemigo;
	const FSAEVUSAjusteEnemigo& Ajuste = ObtenerAjuste();
	const float Radio = Ajuste.RadioCapsula;
	const float MediaAltura = FMath::Max(Ajuste.MediaAlturaCapsula, Radio);
	GetCapsuleComponent()->SetCapsuleSize(Radio, MediaAltura);
	UCharacterMovementComponent* Movimiento = GetCharacterMovement();
	Movimiento->MaxWalkSpeed = Movimiento->MaxFlySpeed = Ajuste.VelocidadMovimiento;
	// Los voladores "aterrizan" volando: el modo por defecto también es el vuelo.
	Movimiento->DefaultLandMovementMode = Ajuste.bVolador ? MOVE_Flying : MOVE_Walking;
	Movimiento->BrakingDecelerationFlying = Ajuste.FrenadaVuelo;
	if (Ajuste.bVolador && HasActorBegunPlay() && Movimiento->MovementMode != MOVE_None)
	{
		Movimiento->SetMovementMode(MOVE_Flying);
	}
	Vida->FijarVidaMaxima(Ajuste.VidaMaxima);

	// Cuerpo con esqueleto: de pie en la base de la cápsula, animado desde la IA; oculta todo lo demás.
	const bool bEsqueleto = TieneEsqueleto();
	CuerpoAnimado->SetVisibility(bEsqueleto);
	Cuerpo->SetVisibility(!bEsqueleto);
	if (bEsqueleto)
	{
		Mascara->SetVisibility(false);
		CuernoD->SetVisibility(false);
		CuernoI->SetVisibility(false);
		if (CuerpoAnimado->GetSkinnedAsset() != Datos->MallaEsqueletica)
		{
			CuerpoAnimado->SetSkinnedAssetAndUpdate(Datos->MallaEsqueletica);
			Animacion->FijarMalla(CuerpoAnimado);
		}
		CuerpoAnimado->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -MediaAltura), FRotator(0.f, GiroCuerpoAnimado, 0.f));
		return;
	}

	// Con cuerpo generado en Blender, ese cuerpo (ya a la medida de la cápsula) sustituye a las formas.
	const bool bMallaPropia = Datos && Datos->MallaCuerpo;
	Mascara->SetVisibility(!bMallaPropia);
	CuernoD->SetVisibility(!bMallaPropia);
	CuernoI->SetVisibility(!bMallaPropia);
	if (bMallaPropia)
	{
		Cuerpo->SetStaticMesh(Datos->MallaCuerpo);
		Cuerpo->SetRelativeScale3D(FVector::OneVector);
		return;
	}
	if (Cuerpo->GetStaticMesh() != Mascara->GetStaticMesh())
	{
		// Vuelta a las formas del motor (la máscara siempre es la esfera).
		Cuerpo->SetStaticMesh(Mascara->GetStaticMesh());
	}

	// Cuerpo: elipsoide que llena la cápsula; máscara delante y arriba; cuernos encima.
	Cuerpo->SetRelativeScale3D(FVector(2.f * Radio, 2.f * Radio, 2.f * MediaAltura) / TamanoForma);
	const float TamanoMascara = 2.f * Radio * FraccionMascara;
	Mascara->SetRelativeLocation(FVector(Radio * AdelanteMascara, 0.f, MediaAltura * AlturaMascara));
	Mascara->SetRelativeScale3D(FVector(TamanoMascara * 0.6f, TamanoMascara, TamanoMascara) / TamanoForma);
	const float TamanoCuerno = 2.f * Radio * FraccionCuerno;
	for (const float Lado : { 1.f, -1.f })
	{
		UStaticMeshComponent* Cuerno = Lado > 0.f ? CuernoD : CuernoI;
		Cuerno->SetRelativeLocation(FVector(Radio * 0.2f, Lado * Radio * LadoCuerno, MediaAltura * AlturaCuerno));
		Cuerno->SetRelativeRotation(FRotator(0.f, 0.f, Lado * InclinacionCuerno));
		Cuerno->SetRelativeScale3D(FVector(TamanoCuerno, TamanoCuerno, TamanoCuerno * 1.6f) / TamanoForma);
	}
}

void ASAEVUSEnemigo::CrearMateriales()
{
	using namespace SAEVUSEnemigo;
	if (!Datos || !Datos->MaterialBase)
	{
		return;
	}
	const FSAEVUSAjusteEnemigo& Ajuste = ObtenerAjuste();
	MaterialCuerpo = UMaterialInstanceDynamic::Create(Datos->MaterialBase, this);
	MaterialCuerpo->SetVectorParameterValue(ParametroColor, Ajuste.ColorCuerpo);
	MaterialMascara = UMaterialInstanceDynamic::Create(Datos->MaterialBase, this);
	MaterialMascara->SetVectorParameterValue(ParametroColor, Ajuste.ColorMascara);
	MaterialMascara->SetScalarParameterValue(ParametroAutoIluminacion, BrilloReposo);
	Cuerpo->SetMaterial(0, MaterialCuerpo);
	if (Datos->MallaCuerpo)
	{
		// Ranura 1 del cuerpo generado: la máscara (brilla con el aviso).
		Cuerpo->SetMaterial(1, MaterialMascara);
	}
	CuernoD->SetMaterial(0, MaterialCuerpo);
	CuernoI->SetMaterial(0, MaterialCuerpo);
	Mascara->SetMaterial(0, MaterialMascara);

	// Cuerpo con esqueleto: una instancia por ranura con el color de sus datos; la del arma es la "máscara" que brilla.
	MaterialesPiezas.Reset();
	ColoresPiezasBase.Reset();
	if (TieneEsqueleto())
	{
		const TArray<FName> Ranuras = CuerpoAnimado->GetMaterialSlotNames();
		for (int32 Ranura = 0; Ranura < Ranuras.Num(); ++Ranura)
		{
			const bool bArma = Ranuras[Ranura].ToString().StartsWith(PrefijoRanuraArma);
			const FLinearColor* Color = Datos->ColoresPiezas.Find(Ranuras[Ranura]);
			const FLinearColor ColorBase = bArma ? Ajuste.ColorMascara : (Color ? *Color : Ajuste.ColorCuerpo);
			UMaterialInstanceDynamic* Material = bArma ? MaterialMascara.Get() : UMaterialInstanceDynamic::Create(Datos->MaterialBase, this);
			Material->SetVectorParameterValue(ParametroColor, ColorBase);
			CuerpoAnimado->SetMaterial(Ranura, Material);
			MaterialesPiezas.Add(Material);
			ColoresPiezasBase.Add(ColorBase);
		}
	}
	if (Datos->MaterialParticulas)
	{
		Efectos->SetMaterial(0, Datos->MaterialParticulas);
	}
}

bool ASAEVUSEnemigo::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	return GetLocalRole() == ROLE_Authority && CanBeDamaged() && Damage != 0.f;
}

void ASAEVUSEnemigo::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	// Los saltos propios de la IA no se escalan; los empujes recibidos sí (los jefes apenas se mueven).
	const bool bPropio = IA && IA->ObtenerEstado() == ESAEVUSEstadoEnemigo::Atacar;
	Super::LaunchCharacter(bPropio ? LaunchVelocity : LaunchVelocity * ObtenerAjuste().EscalaEmpujeRecibido, bXYOverride, bZOverride);
}

void ASAEVUSEnemigo::ManejarMuerte(USAEVUSComponenteVida* ComponenteVida, AActor* Asesino)
{
	using namespace SAEVUSEnemigo;
	IA->Morir();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Efectos->EmitirRafaga(GetActorLocation(), FVector::UpVector * VelocidadParticulasMuerte, ParticulasMuerte, DispersionParticulasMuerte);
	SetLifeSpan(FMath::Max(ObtenerAjuste().DuracionMuerte, KINDA_SMALL_NUMBER));
	AlMorirEnemigo.Broadcast(this);
}

void ASAEVUSEnemigo::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ActualizarAspecto(DeltaSeconds);
	AplicarDestello(DeltaSeconds);
}

void ASAEVUSEnemigo::ActualizarAspecto(float DeltaSeconds)
{
	using namespace SAEVUSEnemigo;
	FVector Objetivo = FVector::OneVector;
	float Brillo = BrilloReposo;
	switch (IA->ObtenerEstado())
	{
	case ESAEVUSEstadoEnemigo::Telegrafiar:
	{
		// Se agacha y se enciende: el aviso tiene que leerse de lejos.
		const float Progreso = IA->ObtenerProgresoAviso();
		Objetivo = FVector(1.f + AplastadoAviso * Progreso, 1.f + AplastadoAviso * Progreso, 1.f - AplastadoAviso * Progreso);
		Brillo = FMath::Lerp(BrilloReposo, BrilloAviso, Progreso);
		break;
	}
	case ESAEVUSEstadoEnemigo::Atacar:
		Objetivo = FVector(1.f + EstiradoAtaque, 1.f - EstiradoAtaque * 0.5f, 1.f - EstiradoAtaque * 0.5f);
		Brillo = BrilloAviso;
		break;
	case ESAEVUSEstadoEnemigo::Muerto:
	{
		TiempoMuerto += DeltaSeconds;
		const float Fraccion = 1.f - FMath::Clamp(TiempoMuerto / FMath::Max(ObtenerAjuste().DuracionMuerte, KINDA_SMALL_NUMBER), 0.f, 1.f);
		Objetivo = FVector(Fraccion);
		Brillo = 0.f;
		break;
	}
	default:
		break;
	}
	if (TieneEsqueleto() && IA->ObtenerEstado() != ESAEVUSEstadoEnemigo::Muerto)
	{
		// El cuerpo con esqueleto avisa y ataca con su pose; sin aplastarse.
		Objetivo = FVector::OneVector;
	}
	EscalaPiezas = FMath::VInterpTo(EscalaPiezas, Objetivo, DeltaSeconds, RapidezEscala);
	Piezas->SetRelativeScale3D(EscalaPiezas);
	const FSAEVUSAjusteEnemigo& Ajuste = ObtenerAjuste();
	if (Ajuste.bVolador && IA->ObtenerEstado() != ESAEVUSEstadoEnemigo::Muerto)
	{
		// Aleteo: el cuerpo sube y baja; cada enemigo con su fase para que no vayan al unísono.
		TiempoAleteo += DeltaSeconds;
		const float Fase = static_cast<float>(GetUniqueID() % 7) * 0.9f;
		Piezas->SetRelativeLocation(FVector(0.f, 0.f, Ajuste.AmplitudAleteo * FMath::Sin(UE_TWO_PI * Ajuste.FrecuenciaAleteo * TiempoAleteo + Fase)));
	}
	if (MaterialMascara)
	{
		MaterialMascara->SetScalarParameterValue(ParametroAutoIluminacion, Brillo);
	}
}
