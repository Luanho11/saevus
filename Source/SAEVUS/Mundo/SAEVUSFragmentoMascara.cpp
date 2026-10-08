// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSFragmentoMascara.h"
#include "Nucleo/SAEVUSFormasMotor.h"
#include "Nucleo/SAEVUSCanalesColision.h"
#include "Audio/SAEVUSSubsistemaSonidoMundo.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "SAEVUS.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSFragmento
{
	const TCHAR* const RutaEsfera = SAEVUSFormas::Esfera;
	constexpr float RadioZona = 60.f;
	// Media máscara: una esfera aplastada.
	const FVector EscalaMalla(0.25f, 0.12f, 0.35f);
}

ASAEVUSFragmentoMascara::ASAEVUSFragmentoMascara()
{
	PrimaryActorTick.bCanEverTick = true;
	Zona = CreateDefaultSubobject<USphereComponent>(TEXT("Zona"));
	Zona->InitSphereRadius(SAEVUSFragmento::RadioZona);
	SAEVUSColision::ConfigurarZonaJugador(Zona);
	RootComponent = Zona;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Esfera(SAEVUSFragmento::RutaEsfera);
	Malla = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malla"));
	Malla->SetupAttachment(Zona);
	Malla->SetStaticMesh(Esfera.Object);
	Malla->SetRelativeScale3D(SAEVUSFragmento::EscalaMalla);
	Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FName ASAEVUSFragmentoMascara::ObtenerIdentificador() const
{
	// Sin identificador puesto a mano: el nombre del actor en su mapa, estable entre sesiones y distinto para cada
	// fragmento (si no, recoger uno sin identificador haría desaparecer a todos los demás que tampoco lo tienen).
	return Identificador.IsNone() ? FName(*FString::Printf(TEXT("%s.%s"), *SAEVUSGuardado::NombreMapa(GetWorld()).ToString(), *GetName())) : Identificador;
}

void ASAEVUSFragmentoMascara::BeginPlay()
{
	Super::BeginPlay();
	if (Identificador.IsNone())
	{
		UE_LOG(LogSAEVUS, Warning, TEXT("%s no tiene Identificador; se guarda como '%s'."), *GetName(), *ObtenerIdentificador().ToString());
	}
	// Ya recogido en otra partida: no aparece.
	const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this);
	if (Guardado && Guardado->FragmentoRecogido(ObtenerIdentificador()))
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
	Personaje->RecogerFragmentoMascara(ObtenerIdentificador());
	USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Banco, GetActorLocation());
	Destroy();
}
