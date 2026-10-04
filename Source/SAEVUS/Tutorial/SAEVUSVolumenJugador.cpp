// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSVolumenJugador.h"
#include "Components/BoxComponent.h"
#include "Personaje/SAEVUSPersonaje.h"

ASAEVUSVolumenJugador::ASAEVUSVolumenJugador()
{
	PrimaryActorTick.bCanEverTick = false;
	Caja = CreateDefaultSubobject<UBoxComponent>(TEXT("Caja"));
	Caja->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
	Caja->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Caja->SetCollisionResponseToAllChannels(ECR_Ignore);
	Caja->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Caja->SetGenerateOverlapEvents(true);
	RootComponent = Caja;
}

void ASAEVUSVolumenJugador::BeginPlay()
{
	Super::BeginPlay();
	Caja->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::AlSolapar);
}

void ASAEVUSVolumenJugador::AlSolapar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe)
{
	if (ASAEVUSPersonaje* Personaje = Cast<ASAEVUSPersonaje>(Otro))
	{
		NotificarEntrada(Personaje);
	}
}

void ASAEVUSVolumenJugador::NotificarEntrada(ASAEVUSPersonaje* Personaje)
{
	if (!Personaje || (bSoloUnaVez && bUsado))
	{
		return;
	}
	bUsado = true;
	AlEntrarJugador(Personaje);
}
