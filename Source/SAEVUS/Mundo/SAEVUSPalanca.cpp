// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSPalanca.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Tutorial/SAEVUSPuertaArena.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSPalanca
{
	const TCHAR* const RutaCubo = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* const RutaCilindro = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	// Caja golpeable y piezas boceto (cm; las formas del motor miden 100).
	const FVector ExtensionCaja(40.f, 40.f, 80.f);
	const FVector EscalaBase(0.6f, 0.6f, 0.3f);
	const FVector PosicionBase(0.f, 0.f, -65.f);
	const FVector PosicionPivote(0.f, 0.f, -50.f);
	const FVector EscalaBrazo(0.12f, 0.12f, 1.1f);
	const FVector PosicionBrazo(0.f, 0.f, 55.f);
	constexpr float DuracionAviso = 4.f;
}

ASAEVUSPalanca::ASAEVUSPalanca()
{
	using namespace SAEVUSPalanca;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Bloquea como el escenario y la encuentran las esferas de golpe (WorldDynamic).
	Caja = CreateDefaultSubobject<UBoxComponent>(TEXT("Caja"));
	Caja->SetBoxExtent(ExtensionCaja);
	Caja->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	RootComponent = Caja;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cubo(RutaCubo);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cilindro(RutaCilindro);
	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	Base->SetupAttachment(Caja);
	Base->SetStaticMesh(Cubo.Object);
	Base->SetRelativeLocation(PosicionBase);
	Base->SetRelativeScale3D(EscalaBase);
	Base->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Pivote = CreateDefaultSubobject<USceneComponent>(TEXT("Pivote"));
	Pivote->SetupAttachment(Caja);
	Pivote->SetRelativeLocation(PosicionPivote);
	Brazo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Brazo"));
	Brazo->SetupAttachment(Pivote);
	Brazo->SetStaticMesh(Cilindro.Object);
	Brazo->SetRelativeLocation(PosicionBrazo);
	Brazo->SetRelativeScale3D(EscalaBrazo);
	Brazo->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Pivote->SetRelativeRotation(FRotator(0.f, 0.f, AnguloReposo));
}

float ASAEVUSPalanca::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Accionar();
	// No es daño hecho: ni alma, ni munición, ni parada de golpe.
	return 0.f;
}

void ASAEVUSPalanca::Accionar()
{
	if (bAccionada)
	{
		return;
	}
	bAccionada = true;
	TiempoGiro = 0.f;
	SetActorTickEnabled(true);
	for (ASAEVUSPuertaArena* Puerta : Puertas)
	{
		if (Puerta)
		{
			Puerta->Abrir();
		}
	}
	if (!TituloAlAccionar.IsEmpty())
	{
		if (USAEVUSSubsistemaTutorial* Tutorial = GetWorld()->GetSubsystem<USAEVUSSubsistemaTutorial>())
		{
			Tutorial->MostrarMensaje(TituloAlAccionar, TextoAlAccionar, SAEVUSPalanca::DuracionAviso);
		}
	}
}

void ASAEVUSPalanca::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TiempoGiro += DeltaSeconds;
	const float Fraccion = DuracionGiro > 0.f ? FMath::Clamp(TiempoGiro / DuracionGiro, 0.f, 1.f) : 1.f;
	Pivote->SetRelativeRotation(FRotator(0.f, 0.f, FMath::InterpEaseOut(AnguloReposo, AnguloAccionada, Fraccion, 2.f)));
	if (Fraccion >= 1.f)
	{
		SetActorTickEnabled(false);
	}
}
