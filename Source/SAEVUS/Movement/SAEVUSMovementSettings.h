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

/**
 * Dash: a short burst at constant speed in the input direction (or facing, with no input),
 * on the ground or in the air. See Docs/movement.md.
 */
USTRUCT(BlueprintType)
struct FSAEVUSDashTuning
{
	GENERATED_BODY()

	/** Horizontal speed held for the whole dash. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Speed = 1600.f;

	/** Dash length in time; distance is roughly Speed x Duration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "s"))
	float Duration = 0.18f;

	/** Horizontal speed is capped to this when the dash ends (or is cancelled by a jump). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float ExitSpeed = 700.f;

	/** Time after a dash ends before the next one can start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "s"))
	float Cooldown = 0.3f;

	/** Snap the body to the dash direction at the start, so the move reads clearly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	bool bFaceDashDirection = true;

	/** Dashes allowed per airborne period; refilled on landing. 0 disables air dashes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Dash", meta = (ClampMin = "0"))
	int32 MaxAirDashes = 1;

	/** Vertical speed set at the start of an air dash (0 = flat). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Dash", meta = (ForceUnits = "cm/s"))
	float AirDashVerticalSpeed = 0.f;

	/** Gravity is off while an air dash lasts, so the vertical speed above is held. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Air Dash")
	bool bSuspendGravityDuringDash = true;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	FSAEVUSDashTuning Dash;
};
