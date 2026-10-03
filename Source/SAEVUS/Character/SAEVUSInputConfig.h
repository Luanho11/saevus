// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSInputConfig.generated.h"

class UInputAction;

/**
 * Input actions the character binds. Mapping contexts (which keys trigger each
 * action) are added by the player controller, so actions stay device-agnostic.
 */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Axis2D: X = right, Y = forward. */
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	TObjectPtr<UInputAction> MoveAction;

	/** Axis2D look from a gamepad stick. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TObjectPtr<UInputAction> LookAction;

	/** Axis2D look from the mouse. Kept separate so touch platforms can exclude it. */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	TObjectPtr<UInputAction> JumpAction;

	/** Bool, held. */
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	TObjectPtr<UInputAction> SprintAction;

	/** Bool, held. */
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	TObjectPtr<UInputAction> WalkAction;

	/** Bool, pressed. */
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	TObjectPtr<UInputAction> DashAction;
};
