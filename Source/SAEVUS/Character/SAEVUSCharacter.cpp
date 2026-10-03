// Copyright SAEVUS. All Rights Reserved.

#include "Character/SAEVUSCharacter.h"
#include "Camera/SAEVUSCameraComponent.h"
#include "Character/SAEVUSInputConfig.h"
#include "Combat/SAEVUSCombatComponent.h"
#include "Combat/SAEVUSHealthComponent.h"
#include "Core/SAEVUSGameMode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Movement/SAEVUSMovementComponent.h"
#include "SAEVUS.h"
#include "Weapons/SAEVUSWeaponComponent.h"

namespace SAEVUSCharacter
{
	// Capsule that fits the engine mannequin used as the placeholder body.
	constexpr float CapsuleRadius = 42.f;
	constexpr float CapsuleHalfHeight = 96.f;

	// Skeletal meshes face +Y; the actor faces +X.
	const FRotator MeshFacingCorrection(0.f, -90.f, 0.f);

	// Engine collision profile for ragdolls (BaseEngine.ini).
	const FName RagdollProfile(TEXT("Ragdoll"));
}

ASAEVUSCharacter::ASAEVUSCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USAEVUSMovementComponent>(CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(SAEVUSCharacter::CapsuleRadius, SAEVUSCharacter::CapsuleHalfHeight);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -SAEVUSCharacter::CapsuleHalfHeight), SAEVUSCharacter::MeshFacingCorrection);

	// The controller rotation drives the camera only; the body turns toward its movement.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	Combat = CreateDefaultSubobject<USAEVUSCombatComponent>(TEXT("Combat"));
	Weapons = CreateDefaultSubobject<USAEVUSWeaponComponent>(TEXT("Weapons"));
	Health = CreateDefaultSubobject<USAEVUSHealthComponent>(TEXT("Health"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<USAEVUSCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

USAEVUSMovementComponent* ASAEVUSCharacter::GetSAEVUSMovement() const
{
	return CastChecked<USAEVUSMovementComponent>(GetCharacterMovement());
}

void ASAEVUSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// Weapons and combat tick first, so a lunge, speed change or facing they set applies in the
	// same frame's movement.
	GetCharacterMovement()->PrimaryComponentTick.AddPrerequisite(Combat, Combat->PrimaryComponentTick);
	GetCharacterMovement()->PrimaryComponentTick.AddPrerequisite(Weapons, Weapons->PrimaryComponentTick);

	Health->OnDeath.AddUniqueDynamic(this, &ThisClass::HandleDeath);
}

void ASAEVUSCharacter::HandleDeath(USAEVUSHealthComponent* HealthComponent, AActor* Killer)
{
	UE_LOG(LogSAEVUS, Log, TEXT("%s killed by %s"), *GetName(), *GetNameSafe(Killer));
	Weapons->SetTriggerHeld(false);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GetMesh()->GetPhysicsAsset())
	{
		GetMesh()->SetCollisionProfileName(SAEVUSCharacter::RagdollProfile);
		GetMesh()->SetSimulatePhysics(true);
	}

	AController* OwningController = GetController();
	if (APlayerController* PlayerController = Cast<APlayerController>(OwningController))
	{
		DisableInput(PlayerController);
	}
	if (ASAEVUSGameMode* GameMode = GetWorld()->GetAuthGameMode<ASAEVUSGameMode>())
	{
		GameMode->HandlePlayerDeath(OwningController);
	}
}

void ASAEVUSCharacter::RequestJump()
{
	if (!Combat->TryCancelForAction(ESAEVUSCancelAction::Jump))
	{
		return;
	}
	USAEVUSMovementComponent* Movement = GetSAEVUSMovement();
	if (Movement->IsFalling() && !Movement->IsInCoyoteWindow())
	{
		Movement->BufferJumpRequest();
	}
	Jump();
}

bool ASAEVUSCharacter::CanJumpWhileFalling() const
{
	// Coyote time: the first (ground) jump stays available briefly after walking off a ledge.
	return GetSAEVUSMovement()->IsInCoyoteWindow();
}

void ASAEVUSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		UE_LOG(LogSAEVUS, Error, TEXT("%s: input component is not an EnhancedInputComponent; check DefaultInput.ini."), *GetNameSafe(this));
		return;
	}
	if (!InputConfig)
	{
		UE_LOG(LogSAEVUS, Error, TEXT("%s: no InputConfig assigned; the character will not respond to input."), *GetNameSafe(this));
		return;
	}

	if (InputConfig->MoveAction)
	{
		Input->BindAction(InputConfig->MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMove);
	}
	if (InputConfig->LookAction)
	{
		Input->BindAction(InputConfig->LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLook);
	}
	if (InputConfig->MouseLookAction)
	{
		Input->BindAction(InputConfig->MouseLookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLook);
	}
	if (InputConfig->JumpAction)
	{
		Input->BindAction(InputConfig->JumpAction, ETriggerEvent::Started, this, &ThisClass::RequestJump);
		Input->BindAction(InputConfig->JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	// Held actions: Triggered every frame while held (true), Completed on release (false).
	if (InputConfig->SprintAction)
	{
		Input->BindAction(InputConfig->SprintAction, ETriggerEvent::Triggered, this, &ThisClass::HandleSprint);
		Input->BindAction(InputConfig->SprintAction, ETriggerEvent::Completed, this, &ThisClass::HandleSprint);
	}
	if (InputConfig->WalkAction)
	{
		Input->BindAction(InputConfig->WalkAction, ETriggerEvent::Triggered, this, &ThisClass::HandleWalk);
		Input->BindAction(InputConfig->WalkAction, ETriggerEvent::Completed, this, &ThisClass::HandleWalk);
	}
	if (InputConfig->DashAction)
	{
		Input->BindAction(InputConfig->DashAction, ETriggerEvent::Started, this, &ThisClass::RequestDash);
	}
	if (InputConfig->AttackAction)
	{
		Input->BindAction(InputConfig->AttackAction, ETriggerEvent::Started, this, &ThisClass::RequestAttack);
		Input->BindAction(InputConfig->AttackAction, ETriggerEvent::Completed, this, &ThisClass::ReleaseAttack);
	}
	if (InputConfig->ReloadAction)
	{
		Input->BindAction(InputConfig->ReloadAction, ETriggerEvent::Started, this, &ThisClass::RequestReload);
	}
	if (InputConfig->NextWeaponAction)
	{
		Input->BindAction(InputConfig->NextWeaponAction, ETriggerEvent::Started, this, &ThisClass::RequestNextWeapon);
	}
	for (int32 Slot = 0; Slot < InputConfig->WeaponSlotActions.Num(); ++Slot)
	{
		if (InputConfig->WeaponSlotActions[Slot])
		{
			Input->BindAction(InputConfig->WeaponSlotActions[Slot], ETriggerEvent::Started, this, &ThisClass::RequestWeaponSlot, Slot);
		}
	}
}

void ASAEVUSCharacter::HandleMove(const FInputActionValue& Value)
{
	if (!Controller)
	{
		return;
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	const FRotationMatrix YawMatrix(FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f));
	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::X), Axis.Y);
	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::Y), Axis.X);
}

void ASAEVUSCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void ASAEVUSCharacter::HandleSprint(const FInputActionValue& Value)
{
	GetSAEVUSMovement()->SetSprintRequested(Value.Get<bool>());
}

void ASAEVUSCharacter::HandleWalk(const FInputActionValue& Value)
{
	GetSAEVUSMovement()->SetWalkRequested(Value.Get<bool>());
}

void ASAEVUSCharacter::RequestDash()
{
	if (Combat->TryCancelForAction(ESAEVUSCancelAction::Dash))
	{
		GetSAEVUSMovement()->RequestDash();
	}
}

void ASAEVUSCharacter::RequestAttack()
{
	if (Weapons->IsFirearmEquipped())
	{
		Weapons->SetTriggerHeld(true);
	}
	else if (!Weapons->IsSwitching())
	{
		// Melee weapon, or no loadout at all (combat's default moveset).
		Combat->RequestAttack();
	}
}

void ASAEVUSCharacter::ReleaseAttack()
{
	Weapons->SetTriggerHeld(false);
}

void ASAEVUSCharacter::RequestWeaponSlot(int32 Slot)
{
	if (Weapons->CanSwitchTo(Slot) && Combat->TryCancelForAction(ESAEVUSCancelAction::WeaponSwitch))
	{
		Weapons->RequestSwitch(Slot);
	}
}

void ASAEVUSCharacter::RequestNextWeapon()
{
	RequestWeaponSlot(Weapons->GetNextSlot());
}

void ASAEVUSCharacter::RequestReload()
{
	Weapons->RequestReload();
}
