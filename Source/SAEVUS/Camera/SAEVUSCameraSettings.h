// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSCameraSettings.generated.h"

/**
 * Third-person follow camera tuning. The member initializers are the single
 * source of default values; a USAEVUSCameraSettings asset overrides them.
 */
USTRUCT(BlueprintType)
struct FSAEVUSCameraTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Framing", meta = (ClampMin = "0", ForceUnits = "cm"))
	float ArmLength = 400.f;

	/** Offset at the camera end of the arm. Positive Y frames the character off-centre (over the shoulder). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Framing")
	FVector SocketOffset = FVector(0.f, 50.f, 60.f);

	/** Radius of the collision probe that pulls the camera in front of walls. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Framing", meta = (ClampMin = "0", ForceUnits = "cm"))
	float ProbeSize = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lag")
	bool bEnableLocationLag = true;

	/** Higher values follow the character more tightly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lag", meta = (ClampMin = "0", EditCondition = "bEnableLocationLag"))
	float LocationLagSpeed = 12.f;

	/** Hard cap on how far the camera may trail behind. 0 = unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lag", meta = (ClampMin = "0", ForceUnits = "cm", EditCondition = "bEnableLocationLag"))
	float LocationLagMaxDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Field Of View", meta = (ClampMin = "5", ClampMax = "170", ForceUnits = "deg"))
	float BaseFieldOfView = 90.f;

	/** FOV reached at full sprint speed; blends from the base FOV above run speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Field Of View", meta = (ClampMin = "5", ClampMax = "170", ForceUnits = "deg"))
	float SprintFieldOfView = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Field Of View", meta = (ClampMin = "0"))
	float FieldOfViewInterpSpeed = 6.f;
};

/** Designer-editable camera profile assigned to USAEVUSCameraComponent. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSCameraSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FSAEVUSCameraTuning Camera;
};
