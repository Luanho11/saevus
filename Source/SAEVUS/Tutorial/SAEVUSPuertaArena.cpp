// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSPuertaArena.h"
#include "Nucleo/SAEVUSFormasMotor.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Audio/SAEVUSSubsistemaSonidoMundo.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/StaticMeshComponent.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSPuerta
{
	const TCHAR* const RutaCubo = SAEVUSFormas::Cubo;
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
	// Abierta en esta partida: sigue abierta al volver (si no, el que descansó al otro lado quedaba encerrado, GAMEPLAY-2).
	const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this);
	if (bAbiertaAlInicio || (Guardado && Guardado->TieneEstadoMundo(this)))
	{
		Abrir(true);
	}
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
		// Uno destruido sin morir (caído fuera del mundo) ya no cuenta: la puerta no puede quedar cerrada para siempre.
		if (IsValid(Enemigo) && !Enemigo->IsActorBeingDestroyed() && !Enemigo->ObtenerVida()->EstaMuerto())
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

void ASAEVUSPuertaArena::Abrir(bool bSilenciosa)
{
	if (bAbierta)
	{
		return;
	}
	bAbierta = true;
	Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (bSilenciosa)
	{
		// Restaurada de la partida guardada: ya hundida y oculta, sin sonido, aviso ni nueva escritura.
		FraccionApertura = 1.f;
		SetActorLocation(PosicionCerrada - FVector(0.f, 0.f, Malla->Bounds.BoxExtent.Z * 2.0));
		SetActorHiddenInGame(true);
		return;
	}
	SetActorHiddenInGame(false);
	if (USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this))
	{
		Guardado->GuardarEstadoMundo(this);
	}
	USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Puerta, GetActorLocation());
	SetActorTickEnabled(true);
	if (!TituloAlAbrir.IsEmpty())
	{
		if (USAEVUSSubsistemaTutorial* Tutorial = GetWorld()->GetSubsystem<USAEVUSSubsistemaTutorial>())
		{
			Tutorial->MostrarMensaje(TituloAlAbrir, TextoAlAbrir, SAEVUSPuerta::DuracionAviso);
		}
	}
}

void ASAEVUSPuertaArena::Cerrar()
{
	if (!bAbierta)
	{
		return;
	}
	bAbierta = false;
	Malla->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetActorHiddenInGame(false);
	USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Puerta, GetActorLocation());
	SetActorTickEnabled(true);
}

void ASAEVUSPuertaArena::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Se hunde toda su altura en el suelo al abrirse y vuelve a subir al cerrarse.
	const float Rapidez = DuracionApertura > 0.f ? 1.f / DuracionApertura : TNumericLimits<float>::Max();
	FraccionApertura = FMath::FInterpConstantTo(FraccionApertura, bAbierta ? 1.f : 0.f, DeltaSeconds, Rapidez);
	const float Altura = static_cast<float>(Malla->Bounds.BoxExtent.Z * 2.0);
	SetActorLocation(PosicionCerrada - FVector(0.f, 0.f, Altura * FMath::InterpEaseInOut(0.f, 1.f, FraccionApertura, 2.f)));
	if (bAbierta && FraccionApertura >= 1.f)
	{
		SetActorHiddenInGame(true);
		SetActorTickEnabled(false);
	}
	else if (!bAbierta && FraccionApertura <= 0.f)
	{
		SetActorTickEnabled(false);
	}
}
