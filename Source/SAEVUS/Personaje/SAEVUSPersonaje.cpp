// Copyright SAEVUS. Todos los derechos reservados.

#include "Personaje/SAEVUSPersonaje.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Camara/SAEVUSComponenteCamara.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Nucleo/SAEVUSModoJuego.h"
#include "Personaje/SAEVUSConfigEntrada.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "SAEVUS.h"

namespace SAEVUSPersonaje
{
	// Cápsula a la medida del cuerpo provisional.
	constexpr float RadioCapsula = 42.f;
	constexpr float MediaAlturaCapsula = 96.f;

	// Las mallas esqueléticas miran a +Y; el actor mira a +X.
	const FRotator CorreccionOrientacionMalla(0.f, -90.f, 0.f);

	// Perfil de colisión del motor para ragdolls (BaseEngine.ini).
	const FName PerfilRagdoll(TEXT("Ragdoll"));
}

ASAEVUSPersonaje::ASAEVUSPersonaje(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USAEVUSComponenteMovimiento>(CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(SAEVUSPersonaje::RadioCapsula, SAEVUSPersonaje::MediaAlturaCapsula);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -SAEVUSPersonaje::MediaAlturaCapsula), SAEVUSPersonaje::CorreccionOrientacionMalla);

	// La rotación del controlador solo mueve la cámara; el cuerpo gira hacia donde se mueve.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	Combate = CreateDefaultSubobject<USAEVUSComponenteCombate>(TEXT("Combate"));
	Armas = CreateDefaultSubobject<USAEVUSComponenteArmas>(TEXT("Armas"));
	Vida = CreateDefaultSubobject<USAEVUSComponenteVida>(TEXT("Vida"));

	BrazoCamara = CreateDefaultSubobject<USpringArmComponent>(TEXT("BrazoCamara"));
	BrazoCamara->SetupAttachment(RootComponent);
	BrazoCamara->bUsePawnControlRotation = true;

	Camara = CreateDefaultSubobject<USAEVUSComponenteCamara>(TEXT("Camara"));
	Camara->SetupAttachment(BrazoCamara, USpringArmComponent::SocketName);
	Camara->bUsePawnControlRotation = false;
}

USAEVUSComponenteMovimiento* ASAEVUSPersonaje::ObtenerMovimiento() const
{
	return CastChecked<USAEVUSComponenteMovimiento>(GetCharacterMovement());
}

void ASAEVUSPersonaje::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// Armas y combate se actualizan primero, así una estocada, un cambio de velocidad o un giro
	// que fijen se aplica en el movimiento del mismo fotograma.
	GetCharacterMovement()->PrimaryComponentTick.AddPrerequisite(Combate, Combate->PrimaryComponentTick);
	GetCharacterMovement()->PrimaryComponentTick.AddPrerequisite(Armas, Armas->PrimaryComponentTick);

	Vida->AlMorir.AddUniqueDynamic(this, &ThisClass::ManejarMuerte);
}

void ASAEVUSPersonaje::ManejarMuerte(USAEVUSComponenteVida* ComponenteVida, AActor* Asesino)
{
	UE_LOG(LogSAEVUS, Log, TEXT("%s muerto por %s"), *GetName(), *GetNameSafe(Asesino));
	Armas->FijarGatillo(false);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GetMesh()->GetPhysicsAsset())
	{
		GetMesh()->SetCollisionProfileName(SAEVUSPersonaje::PerfilRagdoll);
		GetMesh()->SetSimulatePhysics(true);
	}

	AController* ControladorDuenio = GetController();
	if (APlayerController* ControladorJugador = Cast<APlayerController>(ControladorDuenio))
	{
		DisableInput(ControladorJugador);
	}
	if (ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>())
	{
		ModoJuego->NotificarMuerteJugador(ControladorDuenio);
	}
}

void ASAEVUSPersonaje::PedirSalto()
{
	if (!Combate->IntentarCancelarPor(ESAEVUSAccionCancelacion::Salto))
	{
		return;
	}
	USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	if (Movimiento->IsFalling() && !Movimiento->EnVentanaCoyote())
	{
		Movimiento->GuardarSaltoEnBuffer();
	}
	Jump();
}

bool ASAEVUSPersonaje::CanJumpWhileFalling() const
{
	// Tiempo coyote: el primer salto (de suelo) sigue disponible un momento tras salir caminando de un borde.
	return ObtenerMovimiento()->EnVentanaCoyote();
}

void ASAEVUSPersonaje::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Entrada = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Entrada)
	{
		UE_LOG(LogSAEVUS, Error, TEXT("%s: el componente de entrada no es EnhancedInputComponent; revisa DefaultInput.ini."), *GetNameSafe(this));
		return;
	}
	if (!ConfigEntrada)
	{
		UE_LOG(LogSAEVUS, Error, TEXT("%s: sin ConfigEntrada asignada; el personaje no responderá a la entrada."), *GetNameSafe(this));
		return;
	}

	if (ConfigEntrada->AccionMover)
	{
		Entrada->BindAction(ConfigEntrada->AccionMover, ETriggerEvent::Triggered, this, &ThisClass::ManejarMover);
	}
	if (ConfigEntrada->AccionMirar)
	{
		Entrada->BindAction(ConfigEntrada->AccionMirar, ETriggerEvent::Triggered, this, &ThisClass::ManejarMirar);
	}
	if (ConfigEntrada->AccionMirarRaton)
	{
		Entrada->BindAction(ConfigEntrada->AccionMirarRaton, ETriggerEvent::Triggered, this, &ThisClass::ManejarMirar);
	}
	if (ConfigEntrada->AccionSaltar)
	{
		Entrada->BindAction(ConfigEntrada->AccionSaltar, ETriggerEvent::Started, this, &ThisClass::PedirSalto);
		Entrada->BindAction(ConfigEntrada->AccionSaltar, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	// Acciones mantenidas: Triggered cada fotograma mientras se mantienen (cierto), Completed al soltar (falso).
	if (ConfigEntrada->AccionEsprint)
	{
		Entrada->BindAction(ConfigEntrada->AccionEsprint, ETriggerEvent::Triggered, this, &ThisClass::ManejarEsprint);
		Entrada->BindAction(ConfigEntrada->AccionEsprint, ETriggerEvent::Completed, this, &ThisClass::ManejarEsprint);
	}
	if (ConfigEntrada->AccionCaminar)
	{
		Entrada->BindAction(ConfigEntrada->AccionCaminar, ETriggerEvent::Triggered, this, &ThisClass::ManejarCaminar);
		Entrada->BindAction(ConfigEntrada->AccionCaminar, ETriggerEvent::Completed, this, &ThisClass::ManejarCaminar);
	}
	if (ConfigEntrada->AccionDash)
	{
		Entrada->BindAction(ConfigEntrada->AccionDash, ETriggerEvent::Started, this, &ThisClass::PedirDash);
	}
	if (ConfigEntrada->AccionAtacar)
	{
		Entrada->BindAction(ConfigEntrada->AccionAtacar, ETriggerEvent::Started, this, &ThisClass::PedirAtaque);
		Entrada->BindAction(ConfigEntrada->AccionAtacar, ETriggerEvent::Completed, this, &ThisClass::SoltarAtaque);
	}
	if (ConfigEntrada->AccionRecargar)
	{
		Entrada->BindAction(ConfigEntrada->AccionRecargar, ETriggerEvent::Started, this, &ThisClass::PedirRecarga);
	}
	if (ConfigEntrada->AccionSiguienteArma)
	{
		Entrada->BindAction(ConfigEntrada->AccionSiguienteArma, ETriggerEvent::Started, this, &ThisClass::PedirSiguienteArma);
	}
	for (int32 Ranura = 0; Ranura < ConfigEntrada->AccionesRanuraArma.Num(); ++Ranura)
	{
		if (ConfigEntrada->AccionesRanuraArma[Ranura])
		{
			Entrada->BindAction(ConfigEntrada->AccionesRanuraArma[Ranura], ETriggerEvent::Started, this, &ThisClass::PedirRanuraArma, Ranura);
		}
	}
}

void ASAEVUSPersonaje::ManejarMover(const FInputActionValue& Valor)
{
	if (!Controller)
	{
		return;
	}
	const FVector2D Eje = Valor.Get<FVector2D>();
	const FRotationMatrix MatrizGuinada(FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f));
	AddMovementInput(MatrizGuinada.GetUnitAxis(EAxis::X), Eje.Y);
	AddMovementInput(MatrizGuinada.GetUnitAxis(EAxis::Y), Eje.X);
}

void ASAEVUSPersonaje::ManejarMirar(const FInputActionValue& Valor)
{
	const FVector2D Eje = Valor.Get<FVector2D>();
	AddControllerYawInput(Eje.X);
	AddControllerPitchInput(Eje.Y);
}

void ASAEVUSPersonaje::ManejarEsprint(const FInputActionValue& Valor)
{
	ObtenerMovimiento()->PedirEsprint(Valor.Get<bool>());
}

void ASAEVUSPersonaje::ManejarCaminar(const FInputActionValue& Valor)
{
	ObtenerMovimiento()->PedirCaminar(Valor.Get<bool>());
}

void ASAEVUSPersonaje::PedirDash()
{
	if (Combate->IntentarCancelarPor(ESAEVUSAccionCancelacion::Dash))
	{
		ObtenerMovimiento()->PedirDash();
	}
}

void ASAEVUSPersonaje::PedirAtaque()
{
	if (Armas->TieneArmaFuegoEquipada())
	{
		Armas->FijarGatillo(true);
	}
	else if (!Armas->EstaCambiando())
	{
		// Arma cuerpo a cuerpo, o sin arsenal (repertorio por defecto del combate).
		Combate->PedirAtaque();
	}
}

void ASAEVUSPersonaje::SoltarAtaque()
{
	Armas->FijarGatillo(false);
}

void ASAEVUSPersonaje::PedirRanuraArma(int32 Ranura)
{
	if (Armas->PuedeCambiarA(Ranura) && Combate->IntentarCancelarPor(ESAEVUSAccionCancelacion::CambioArma))
	{
		Armas->PedirCambio(Ranura);
	}
}

void ASAEVUSPersonaje::PedirSiguienteArma()
{
	PedirRanuraArma(Armas->ObtenerSiguienteRanura());
}

void ASAEVUSPersonaje::PedirRecarga()
{
	Armas->PedirRecarga();
}
