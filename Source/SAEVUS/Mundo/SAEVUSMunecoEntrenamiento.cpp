// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSMunecoEntrenamiento.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "Depuracion/SAEVUSSubsistemaNumerosDanio.h"
#include "Engine/StaticMesh.h"
#include "SAEVUS.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSMuneco
{
	// Mismo tamaño que la cápsula del jugador, así los alcances se leen igual que contra un personaje.
	constexpr float RadioCapsula = 42.f;
	constexpr float MediaAlturaCapsula = 96.f;

	// Cilindro del motor (100 cm de ancho y alto, pivote en el centro) estirado a la cápsula.
	const TCHAR* const RutaMallaCuerpo = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	constexpr float TamanoCilindro = 100.f;

	constexpr float AlturaLecturaSobreCapsula = 30.f;
	constexpr float TamanoMundoLectura = 24.f;
}

ASAEVUSMunecoEntrenamiento::ASAEVUSMunecoEntrenamiento()
{
	using namespace SAEVUSMuneco;
	PrimaryActorTick.bCanEverTick = false;

	Capsula = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsula"));
	Capsula->InitCapsuleSize(RadioCapsula, MediaAlturaCapsula);
	Capsula->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = Capsula;

	Cuerpo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cuerpo"));
	Cuerpo->SetupAttachment(Capsula);
	Cuerpo->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Cuerpo->SetRelativeScale3D(FVector(2.f * RadioCapsula / TamanoCilindro, 2.f * RadioCapsula / TamanoCilindro, 2.f * MediaAlturaCapsula / TamanoCilindro));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MallaCilindro(RutaMallaCuerpo);
	if (MallaCilindro.Succeeded())
	{
		Cuerpo->SetStaticMesh(MallaCilindro.Object);
	}

	Lectura = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Lectura"));
	Lectura->SetupAttachment(Capsula);
	Lectura->SetRelativeLocation(FVector(0.f, 0.f, MediaAlturaCapsula + AlturaLecturaSobreCapsula));
	Lectura->SetHorizontalAlignment(EHTA_Center);
	Lectura->SetWorldSize(TamanoMundoLectura);
	Lectura->SetTextRenderColor(FColor::White);
}

void ASAEVUSMunecoEntrenamiento::BeginPlay()
{
	Super::BeginPlay();
	ActualizarLectura();
}

float ASAEVUSMunecoEntrenamiento::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float Aplicado = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	++Golpes;
	UltimoDanio = DamageAmount;
	DanioTotal += DamageAmount;
	USAEVUSSubsistemaNumerosDanio::Registrar(this, GetActorLocation() + FVector::UpVector * GetSimpleCollisionHalfHeight(), DamageAmount, false);
	UE_LOG(LogSAEVUS, Verbose, TEXT("%s golpeado por %s con %.1f (golpes %d, total %.1f)"), *GetName(), *GetNameSafe(DamageCauser), DamageAmount, Golpes, DanioTotal);
	ActualizarLectura();
	return Aplicado;
}

void ASAEVUSMunecoEntrenamiento::ReiniciarContadores()
{
	Golpes = 0;
	DanioTotal = 0.f;
	UltimoDanio = 0.f;
	ActualizarLectura();
}

void ASAEVUSMunecoEntrenamiento::ActualizarLectura()
{
	// Es un dato de depuración: solo se ve con el HUD de depuración (F6).
	Lectura->SetVisibility(SAEVUSDepuracion::HUDActivo());
	Lectura->SetText(FText::FromString(FString::Printf(TEXT("Golpes %d | Último %.0f | Total %.0f"), Golpes, UltimoDanio, DanioTotal)));
}
