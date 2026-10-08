// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSParedRompible.h"
#include "Nucleo/SAEVUSFormasMotor.h"
#include "Audio/SAEVUSSubsistemaSonidoMundo.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSParedRompible
{
	const TCHAR* const RutaCubo = SAEVUSFormas::Cubo;
	// Frecuencia del temblor (oscilaciones por segundo).
	constexpr float FrecuenciaTemblor = 40.f;
}

ASAEVUSParedRompible::ASAEVUSParedRompible()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cubo(SAEVUSParedRompible::RutaCubo);
	Malla = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malla"));
	Malla->SetStaticMesh(Cubo.Object);
	// Bloquea como el escenario, pero como objeto dinámico: lo encuentran las esferas de golpe.
	Malla->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	RootComponent = Malla;
}

float ASAEVUSParedRompible::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bRota || DamageAmount <= 0.f)
	{
		return 0.f;
	}
	++Golpes;
	if (Golpes >= GolpesParaRomper)
	{
		Romper();
	}
	else
	{
		if (TiempoTemblor <= 0.f)
		{
			PosicionReposo = GetActorLocation();
		}
		TiempoTemblor = DuracionTemblor;
		SetActorTickEnabled(true);
		USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Escudo, GetActorLocation());
	}
	// No es daño hecho: ni alma ni munición.
	return 0.f;
}

void ASAEVUSParedRompible::Romper()
{
	bRota = true;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Puerta, GetActorLocation());
}

void ASAEVUSParedRompible::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TiempoTemblor = FMath::Max(0.f, TiempoTemblor - DeltaSeconds);
	const float Desplazamiento = TiempoTemblor > 0.f ? AmplitudTemblor * FMath::Sin(TiempoTemblor * SAEVUSParedRompible::FrecuenciaTemblor * UE_TWO_PI) : 0.f;
	SetActorLocation(PosicionReposo + GetActorRightVector() * Desplazamiento);
	if (TiempoTemblor <= 0.f)
	{
		SetActorTickEnabled(false);
	}
}
