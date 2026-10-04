// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSPuertaArena.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/StaticMeshComponent.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSPuerta
{
	const TCHAR* const RutaCubo = TEXT("/Engine/BasicShapes/Cube.Cube");
	constexpr float DuracionAviso = 5.f;
}

ASAEVUSPuertaArena::ASAEVUSPuertaArena()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	Malla = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malla"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cubo(SAEVUSPuerta::RutaCubo);
	Malla->SetStaticMesh(Cubo.Object);
	Malla->SetMobility(EComponentMobility::Movable);
	RootComponent = Malla;
}

void ASAEVUSPuertaArena::BeginPlay()
{
	Super::BeginPlay();
	PosicionCerrada = GetActorLocation();
	Manejador = ASAEVUSEnemigo::AlMorirEnemigo.AddUObject(this, &ThisClass::AlMorirEnemigo);
}

void ASAEVUSPuertaArena::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ASAEVUSEnemigo::AlMorirEnemigo.Remove(Manejador);
	Super::EndPlay(EndPlayReason);
}

void ASAEVUSPuertaArena::FijarEnemigos(const TArray<ASAEVUSEnemigo*>& NuevosEnemigos)
{
	Enemigos.Reset();
	for (ASAEVUSEnemigo* Enemigo : NuevosEnemigos)
	{
		Enemigos.Add(Enemigo);
	}
}

bool ASAEVUSPuertaArena::QuedanEnemigos() const
{
	for (const TObjectPtr<ASAEVUSEnemigo>& Enemigo : Enemigos)
	{
		if (IsValid(Enemigo) && !Enemigo->ObtenerVida()->EstaMuerto())
		{
			return true;
		}
	}
	return false;
}

void ASAEVUSPuertaArena::AlMorirEnemigo(ASAEVUSEnemigo* Enemigo)
{
	if (!bAbierta && !Enemigos.IsEmpty() && Enemigos.Contains(Enemigo) && !QuedanEnemigos())
	{
		Abrir();
	}
}

void ASAEVUSPuertaArena::Abrir()
{
	if (bAbierta)
	{
		return;
	}
	bAbierta = true;
	TiempoAbriendo = 0.f;
	Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(true);
	if (!TituloAlAbrir.IsEmpty())
	{
		if (USAEVUSSubsistemaTutorial* Tutorial = GetWorld()->GetSubsystem<USAEVUSSubsistemaTutorial>())
		{
			Tutorial->MostrarMensaje(TituloAlAbrir, TextoAlAbrir, SAEVUSPuerta::DuracionAviso);
		}
	}
}

void ASAEVUSPuertaArena::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Se hunde toda su altura en el suelo.
	TiempoAbriendo += DeltaSeconds;
	const float Fraccion = DuracionApertura > 0.f ? FMath::Clamp(TiempoAbriendo / DuracionApertura, 0.f, 1.f) : 1.f;
	const float Altura = static_cast<float>(Malla->Bounds.BoxExtent.Z * 2.0);
	SetActorLocation(PosicionCerrada - FVector(0.f, 0.f, Altura * FMath::InterpEaseInOut(0.f, 1.f, Fraccion, 2.f)));
	if (Fraccion >= 1.f)
	{
		SetActorHiddenInGame(true);
		SetActorTickEnabled(false);
	}
}
