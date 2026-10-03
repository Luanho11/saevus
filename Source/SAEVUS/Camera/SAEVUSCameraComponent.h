// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Camera/SAEVUSCameraSettings.h"
#include "SAEVUSCameraComponent.generated.h"

/**
 * Third-person follow camera for SAEVUS.
 *
 * Attach it to a USpringArmComponent: it applies the framing and lag tuning to
 * that arm and widens the field of view as the owner moves above run speed.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	const FSAEVUSCameraTuning& GetTuning() const;

	/** Swaps the camera profile and re-applies it immediately. Null restores code defaults. */
	void SetCameraSettings(const USAEVUSCameraSettings* NewSettings);

	//~ Begin UCameraComponent interface
	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;
	//~ End UCameraComponent interface

protected:
	/** Optional profile; when unset the defaults in FSAEVUSCameraTuning are used. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Camera")
	TObjectPtr<const USAEVUSCameraSettings> CameraSettings;

private:
	/** Pushes the active tuning into this camera and its parent spring arm. */
	void ApplyCameraSettings();

	/** Base FOV up to run speed, blending to the sprint FOV at sprint speed. */
	float GetTargetFieldOfView() const;
};
