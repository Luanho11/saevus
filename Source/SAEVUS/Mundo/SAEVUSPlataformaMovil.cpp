// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSPlataformaMovil.h"
#include "Nucleo/SAEVUSFormasMotor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSPlataforma
{
	const TCHAR* const RutaCubo = SAEVUSFormas::Cubo;
}

ASAEVUSPlataformaMovil::ASAEVUSPlataformaMovil()
{
	PrimaryActorTick.bCanEverTick = true;
	// Antes que los personajes: quien está encima se mueve con la posición ya actualizada.
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cubo(SAEVUSPlataforma::RutaCubo);
	Malla = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malla"));
	Malla->SetStaticMesh(Cubo.Object);
	Malla->SetMobility(EComponentMobility::Movable);
	Malla->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Malla;
}

void ASAEVUSPlataformaMovil::BeginPlay()
{
	Super::BeginPlay();
	Inicio = GetActorLocation();
	Tiempo = 0.f;
}

FVector ASAEVUSPlataformaMovil::PosicionEn(float Instante) const
{
	// Ciclo: pausa, ida, pausa, vuelta. El viaje acelera y frena (suavizado).
	const float Viaje = static_cast<float>(Recorrido.Size()) / FMath::Max(Velocidad, 1.f);
	const float Ciclo = 2.f * (PausaExtremo + Viaje);
	const float T = FMath::Fmod(FMath::Max(Instante, 0.f), FMath::Max(Ciclo, UE_KINDA_SMALL_NUMBER));
	float Fraccion = 0.f;
	if (T < PausaExtremo)
	{
		Fraccion = 0.f;
	}
	else if (T < PausaExtremo + Viaje)
	{
		Fraccion = FMath::InterpEaseInOut(0.f, 1.f, (T - PausaExtremo) / Viaje, 2.f);
	}
	else if (T < 2.f * PausaExtremo + Viaje)
	{
		Fraccion = 1.f;
	}
	else
	{
		Fraccion = 1.f - FMath::InterpEaseInOut(0.f, 1.f, (T - 2.f * PausaExtremo - Viaje) / Viaje, 2.f);
	}
	return Inicio + Recorrido * Fraccion;
}

void ASAEVUSPlataformaMovil::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// El tiempo vuelve a empezar cada ciclo: un float que crece sin límite perdería precisión en sesiones largas (GAMEPLAY-27).
	const float Viaje = static_cast<float>(Recorrido.Size()) / FMath::Max(Velocidad, 1.f);
	const float Ciclo = FMath::Max(2.f * (PausaExtremo + Viaje), UE_KINDA_SMALL_NUMBER);
	Tiempo = FMath::Fmod(Tiempo + DeltaSeconds, Ciclo);
	SetActorLocation(PosicionEn(Tiempo));
}
