// Copyright SAEVUS. All Rights Reserved.

#include "Camera/SAEVUSCameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Movement/SAEVUSMovementComponent.h"

namespace SAEVUSCamera
{
	const FSAEVUSCameraTuning DefaultTuning;
}

const FSAEVUSCameraTuning& USAEVUSCameraComponent::GetTuning() const
{
	return CameraSettings ? CameraSettings->Camera : SAEVUSCamera::DefaultTuning;
}

void USAEVUSCameraComponent::SetCameraSettings(const USAEVUSCameraSettings* NewSettings)
{
	CameraSettings = NewSettings;
	ApplyCameraSettings();
}

void USAEVUSCameraComponent::OnRegister()
{
	Super::OnRegister();
	ApplyCameraSettings();
}

void USAEVUSCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	FieldOfView = FMath::FInterpTo(FieldOfView, GetTargetFieldOfView(), DeltaTime, GetTuning().FieldOfViewInterpSpeed);
	Super::GetCameraView(DeltaTime, DesiredView);
}

void USAEVUSCameraComponent::ApplyCameraSettings()
{
	const FSAEVUSCameraTuning& Tuning = GetTuning();
	FieldOfView = Tuning.BaseFieldOfView;

	USpringArmComponent* Arm = Cast<USpringArmComponent>(GetAttachParent());
	if (!Arm)
	{
		return;
	}
	Arm->TargetArmLength = Tuning.ArmLength;
	Arm->SocketOffset = Tuning.SocketOffset;
	Arm->ProbeSize = Tuning.ProbeSize;
	Arm->bEnableCameraLag = Tuning.bEnableLocationLag;
	Arm->CameraLagSpeed = Tuning.LocationLagSpeed;
	Arm->CameraLagMaxDistance = Tuning.LocationLagMaxDistance;
}

float USAEVUSCameraComponent::GetTargetFieldOfView() const
{
	const FSAEVUSCameraTuning& Tuning = GetTuning();

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const USAEVUSMovementComponent* Movement = Character ? Cast<USAEVUSMovementComponent>(Character->GetCharacterMovement()) : nullptr;
	if (!Movement)
	{
		return Tuning.BaseFieldOfView;
	}

	// Driven by actual speed, not the sprint request, so the FOV never widens while blocked by a wall.
	const float RunSpeed = Movement->GetGaitSpeed(ESAEVUSGait::Run);
	const float SprintSpeed = Movement->GetGaitSpeed(ESAEVUSGait::Sprint);
	const float SprintAlpha = FMath::Clamp(FMath::GetRangePct(RunSpeed, SprintSpeed, Movement->Velocity.Size2D()), 0.f, 1.f);
	return FMath::Lerp(Tuning.BaseFieldOfView, Tuning.SprintFieldOfView, SprintAlpha);
}
