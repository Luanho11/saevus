// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSFragmentoMascara.h"
#include "Audio/SAEVUSSubsistemaSonidoMundo.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSFragmento
{
	const TCHAR* const RutaEsfera = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	constexpr float RadioZona = 60.f;
	// Media máscara: una esfera aplastada.
	const FVector EscalaMalla(0.25f, 0.12f, 0.35f);
}

ASAEVUSFragmentoMascara::ASAEVUSFragmentoMascara()
{
	PrimaryActorTick.bCanEverTick = true;
	Zona = CreateDefaultSubobject<USphereComponent>(TEXT("Zona"));
	Zona->InitSphereRadius(SAEVUSFragmento::RadioZona);
	Zona->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
	Zona->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Zona->SetCollisionResponseToAllChannels(ECR_Ignore);
	Zona->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Zona->SetGenerateOverlapEvents(true);
	RootComponent = Zona;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Esfera(SAEVUSFragmento::RutaEsfera);
	Malla = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malla"));
	Malla->SetupAttachment(Zona);
	Malla->SetStaticMesh(Esfera.Object);
	Malla->SetRelativeScale3D(SAEVUSFragmento::EscalaMalla);
	Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASAEVUSFragmentoMascara::BeginPlay()
{
	Super::BeginPlay();
	// Ya recogido en otra partida: no aparece.
	const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this);
	if (Guardado && Guardado->FragmentoRecogido(Identificador))
	{
		Destroy();
		return;
	}
	Zona->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::AlEntrar);
}

void ASAEVUSFragmentoMascara::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Malla->AddLocalRotation(FRotator(0.f, VelocidadGiro * DeltaSeconds, 0.f));
}

void ASAEVUSFragmentoMascara::AlEntrar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe)
{
	if (ASAEVUSPersonaje* Personaje = Cast<ASAEVUSPersonaje>(Otro))
	{
		Recoger(Personaje);
	}
}

void ASAEVUSFragmentoMascara::Recoger(ASAEVUSPersonaje* Personaje)
{
	if (!Personaje || IsActorBeingDestroyed())
	{
		return;
	}
	Personaje->RecogerFragmentoMascara(Identificador);
	USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Banco, GetActorLocation());
	Destroy();
}
