// Copyright SAEVUS. All Rights Reserved.

#include "Character/SAEVUSCharacter.h"
#include "Camera/SAEVUSCameraComponent.h"
#include "Character/SAEVUSInputConfig.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Movement/SAEVUSMovementComponent.h"
#include "SAEVUS.h"

namespace SAEVUSCharacter
{
	// Capsule that fits the engine mannequin used as the placeholder body.
	constexpr float CapsuleRadius = 42.f;
	constexpr float CapsuleHalfHeight = 96.f;

	// Skeletal meshes face +Y; the actor faces +X.
	const FRotator MeshFacingCorrection(0.f, -90.f, 0.f);
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
		Input->BindAction(InputConfig->JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
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
