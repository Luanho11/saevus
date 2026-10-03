// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSMovementSettings.generated.h"

/**
 * Ground locomotion tuning. The member initializers are the single source of
 * default values; a USAEVUSMovementSettings asset overrides them per character.
 */
USTRUCT(BlueprintType)
struct FSAEVUSGroundMovementTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float WalkSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float RunSpeed = 550.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gait", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float SprintSpeed = 850.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Response", meta = (ClampMin = "0", ForceUnits = "cm/s^2"))
	float MaxAcceleration = 2400.f;

	/** Deceleration applied when there is no movement input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Response", meta = (ClampMin = "0", ForceUnits = "cm/s^2"))
	float BrakingDeceleration = 2000.f;

	/** Higher values make direction changes snappier and reduce sliding. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Response", meta = (ClampMin = "0"))
	float GroundFriction = 8.f;

	/** How fast the character turns to face its movement direction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Response", meta = (ClampMin = "0", ForceUnits = "deg/s"))
	float TurnRate = 720.f;
};

/** Jump and airborne tuning. */
USTRUCT(BlueprintType)
struct FSAEVUSAirMovementTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float JumpZVelocity = 700.f;

	/** Time the jump button keeps adding JumpZVelocity while held. 0 = fixed-height jump. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump", meta = (ClampMin = "0", ForceUnits = "s"))
	float JumpMaxHoldTime = 0.f;

	/** Multiplier on world gravity. Values above 1 give a heavier, snappier arc. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gravity", meta = (ClampMin = "0"))
	float GravityScale = 1.75f;

	/** Fraction of ground acceleration available while airborne (0 = none, 1 = full). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Control", meta = (ClampMin = "0", ClampMax = "1"))
	float AirControl = 0.35f;

	/** Air control multiplier applied while horizontal speed is below the boost threshold. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Control", meta = (ClampMin = "0"))
	float AirControlBoostMultiplier = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Control", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float AirControlBoostVelocityThreshold = 25.f;

	/** Lateral friction while falling. 0 keeps take-off momentum. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Control", meta = (ClampMin = "0"))
	float FallingLateralFriction = 0.f;

	/** Lateral deceleration while falling with no input. 0 keeps take-off momentum. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Control", meta = (ClampMin = "0", ForceUnits = "cm/s^2"))
	float BrakingDecelerationFalling = 0.f;

	/**
	 * Keep the speed cap of the gait used at take-off while airborne, so sprint can't be
	 * engaged (or dropped) mid-air. When off, the cap follows the current gait request.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Control")
	bool bLockGaitInAir = true;

	/** Grace period after walking off a ledge during which the ground jump is still allowed. 0 disables. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump Assist", meta = (ClampMin = "0", ForceUnits = "s"))
	float CoyoteTime = 0.12f;

	/** A jump pressed up to this long before landing fires on touchdown. 0 disables. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump Assist", meta = (ClampMin = "0", ForceUnits = "s"))
	float JumpBufferTime = 0.12f;
};

/** Designer-editable movement profile assigned to USAEVUSMovementComponent. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSMovementSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground")
	FSAEVUSGroundMovementTuning Ground;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Air")
	FSAEVUSAirMovementTuning Air;
};
