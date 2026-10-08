// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSArenaJefe.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/BoxComponent.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "Tutorial/SAEVUSPuertaArena.h"

namespace SAEVUSArenaJefe
{
	// Cada cuánto se mira si el combate sigue (s).
	constexpr float IntervaloComprobacion = 0.25f;
	// Zona por defecto (cm): la sala entera; se ajusta en el mapa con la escala.
	const FVector ExtensionZona(400.f, 400.f, 200.f);
}

ASAEVUSArenaJefe::ASAEVUSArenaJefe()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	Zona = CreateDefaultSubobject<UBoxComponent>(TEXT("Zona"));
	Zona->SetBoxExtent(SAEVUSArenaJefe::ExtensionZona);
	Zona->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Zona->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Zona->SetCollisionResponseToAllChannels(ECR_Ignore);
	Zona->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Zona->SetGenerateOverlapEvents(true);
	RootComponent = Zona;
}

void ASAEVUSArenaJefe::Configurar(ASAEVUSEnemigo* NuevoJefe, const TArray<ASAEVUSPuertaArena*>& NuevasPuertas)
{
	Jefe = NuevoJefe;
	PuertasEntrada.Reset();
	for (ASAEVUSPuertaArena* Puerta : NuevasPuertas)
	{
		PuertasEntrada.Add(Puerta);
	}
}

void ASAEVUSArenaJefe::BeginPlay()
{
	Super::BeginPlay();
	Zona->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::AlSolapar);
}

void ASAEVUSArenaJefe::AlSolapar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe)
{
	if (Cast<ASAEVUSPersonaje>(Otro))
	{
		Activar();
	}
}

void ASAEVUSArenaJefe::Activar()
{
	// Sin jefe (ya vencido en esta partida) o con la pelea en marcha no hay nada que empezar.
	if (bActiva || !IsValid(Jefe) || Jefe->ObtenerVida()->EstaMuerto())
	{
		return;
	}
	bActiva = true;
	TiempoSinComprobar = 0.f;
	SetActorTickEnabled(true);
	for (ASAEVUSPuertaArena* Puerta : PuertasEntrada)
	{
		if (Puerta)
		{
			Puerta->Cerrar();
		}
	}
	Jefe->ObtenerIA()->Despertar();
}

void ASAEVUSArenaJefe::Terminar(bool bJefeVencido)
{
	bActiva = false;
	SetActorTickEnabled(!bJefeVencido);
	for (ASAEVUSPuertaArena* Puerta : PuertasEntrada)
	{
		if (Puerta)
		{
			Puerta->Abrir();
		}
	}
	if (bJefeVencido)
	{
		SetActorTickEnabled(false);
	}
}

void ASAEVUSArenaJefe::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TiempoSinComprobar += DeltaSeconds;
	if (!bActiva || TiempoSinComprobar < SAEVUSArenaJefe::IntervaloComprobacion)
	{
		return;
	}
	TiempoSinComprobar = 0.f;
	if (!IsValid(Jefe) || Jefe->ObtenerVida()->EstaMuerto())
	{
		Terminar(/*bJefeVencido*/ true);
		return;
	}
	// El jefe vuelve a reposo cuando el jugador muere o reinicia (ReiniciarCombate): la arena se rearma.
	if (Jefe->ObtenerIA()->ObtenerEstado() == ESAEVUSEstadoEnemigo::Reposo)
	{
		Terminar(/*bJefeVencido*/ false);
	}
}
