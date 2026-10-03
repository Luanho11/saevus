// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SAEVUSMovementTypes.generated.h"

/** Ground locomotion speed tier requested by the player. */
UENUM(BlueprintType)
enum class ESAEVUSGait : uint8
{
	Walk,
	Run,
	Sprint
};

/** SAEVUS sub-modes of MOVE_Custom (UCharacterMovementComponent::CustomMovementMode). */
UENUM(BlueprintType)
enum class ESAEVUSCustomMovementMode : uint8
{
	None = 0 UMETA(Hidden),
	WallRun
};
