// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSBanco.h"
#include "Nucleo/SAEVUSFormasMotor.h"
#include "Nucleo/SAEVUSCanalesColision.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Audio/SAEVUSSubsistemaSonidoMundo.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Nucleo/SAEVUSModoJuego.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "SAEVUSBanco"

namespace SAEVUSBanco
{
	const TCHAR* const RutaCubo = SAEVUSFormas::Cubo;
	// Medidas del boceto (cm; el cubo del motor mide 100): asiento largo y bajo con respaldo.
	const FVector ExtensionZona(110.f, 110.f, 100.f);
	const FVector EscalaAsiento(0.6f, 2.f, 0.4f);
	const FVector PosicionAsiento(0.f, 0.f, -80.f);
	const FVector EscalaRespaldo(0.12f, 2.f, 0.7f);
	const FVector PosicionRespaldo(-26.f, 0.f, -45.f);
	// El origen del banco está a la altura del centro de un personaje de pie (100 cm sobre el suelo).
	constexpr float AlturaReaparicion = 0.f;
	constexpr float DuracionAviso = 2.5f;
}

ASAEVUSBanco::ASAEVUSBanco()
{
	using namespace SAEVUSBanco;
	Zona = CreateDefaultSubobject<UBoxComponent>(TEXT("Zona"));
	Zona->SetBoxExtent(ExtensionZona);
	SAEVUSColision::ConfigurarZonaJugador(Zona);
	RootComponent = Zona;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cubo(RutaCubo);
	UStaticMesh* MallaCubo = Cubo.Object;
	auto CrearPieza = [this, MallaCubo](const TCHAR* Nombre, const FVector& Posicion, const FVector& Escala)
	{
		UStaticMeshComponent* Pieza = CreateDefaultSubobject<UStaticMeshComponent>(Nombre);
		Pieza->SetupAttachment(Zona);
		Pieza->SetStaticMesh(MallaCubo);
		Pieza->SetRelativeLocation(Posicion);
		Pieza->SetRelativeScale3D(Escala);
		Pieza->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return Pieza;
	};
	Asiento = CrearPieza(TEXT("Asiento"), PosicionAsiento, EscalaAsiento);
	Respaldo = CrearPieza(TEXT("Respaldo"), PosicionRespaldo, EscalaRespaldo);
}

void ASAEVUSBanco::BeginPlay()
{
	Super::BeginPlay();
	Zona->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::AlEntrar);
	Zona->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::AlSalir);
}

void ASAEVUSBanco::AlEntrar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe)
{
	if (ASAEVUSPersonaje* Personaje = Cast<ASAEVUSPersonaje>(Otro))
	{
		Personaje->FijarInteractuable(this);
	}
}

void ASAEVUSBanco::AlSalir(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice)
{
	if (ASAEVUSPersonaje* Personaje = Cast<ASAEVUSPersonaje>(Otro))
	{
		Personaje->QuitarInteractuable(this);
	}
}

FText ASAEVUSBanco::ObtenerTextoInteraccion() const
{
	return LOCTEXT("Sentarse", "Sentarse");
}

FTransform ASAEVUSBanco::ObtenerPuntoReaparicion() const
{
	const FVector Delante = GetActorForwardVector().GetSafeNormal2D();
	return FTransform(Delante.ToOrientationRotator(), GetActorLocation() + Delante * DistanciaReaparicion + FVector::UpVector * SAEVUSBanco::AlturaReaparicion);
}

void ASAEVUSBanco::Interactuar(ASAEVUSPersonaje* Personaje)
{
	// De pie en el suelo y sin atacar: no se sienta en pleno salto ni a mitad de un tajo.
	if (!Personaje || !Personaje->PuedeSentarse())
	{
		return;
	}
	++Usos;
	USAEVUSComponenteVida* Vida = Personaje->ObtenerVida();
	Vida->Curar(Vida->ObtenerVidaMaxima());
	Personaje->ObtenerArmas()->RellenarMunicion();
	// Sentado en el banco, mirando hacia donde mira el banco.
	Personaje->Sentarse(FTransform(GetActorRotation(), GetActorLocation()));
	if (ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>())
	{
		ModoJuego->FijarPuntoControl(ObtenerPuntoReaparicion());
	}
	USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Banco, GetActorLocation());
	if (USAEVUSSubsistemaTutorial* Tutorial = GetWorld()->GetSubsystem<USAEVUSSubsistemaTutorial>())
	{
		Tutorial->MostrarMensaje(LOCTEXT("Descansas", "Descansas"), LOCTEXT("DescansasTexto", "Vida recuperada. Si caes, vuelves aquí; \"Continuar\" también."),
			SAEVUSBanco::DuracionAviso);
	}
}

#undef LOCTEXT_NAMESPACE
