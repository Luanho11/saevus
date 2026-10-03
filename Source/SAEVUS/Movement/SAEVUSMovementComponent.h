// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movement/SAEVUSMovementSettings.h"
#include "Movement/SAEVUSMovementTypes.h"
#include "SAEVUSMovementComponent.generated.h"

/**
 * Character locomotion for SAEVUS.
 *
 * Owns gait selection (walk / run / sprint) and applies the data-driven tuning
 * from USAEVUSMovementSettings to the base movement model. Gait requests travel
 * through saved moves so they stay client-predicted when networking is enabled.
 *
 * Dash is a timed state inside walking / falling; wall jump is an instant launch
 * while falling; wall run and wall climb are custom movement modes (ESAEVUSCustomMovementMode).
 * Wall movement lives in SAEVUSMovementComponentWalls.cpp. See Docs/movement.md.
 */
UCLASS(ClassGroup = (SAEVUS))
class SAEVUS_API USAEVUSMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USAEVUSMovementComponent();

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Movement")
	void SetSprintRequested(bool bRequested) { bWantsToSprint = bRequested; }

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Movement")
	void SetWalkRequested(bool bRequested) { bWantsToWalk = bRequested; }

	/** Sprint takes priority over walk; with neither requested the character runs. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	ESAEVUSGait GetCurrentGait() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	float GetGaitSpeed(ESAEVUSGait Gait) const;

	/** Gait whose speed caps air movement; the take-off gait while bLockGaitInAir is on. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	ESAEVUSGait GetAirGait() const;

	/** True while falling shortly after walking off a ledge, when the ground jump is still allowed. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	bool IsInCoyoteWindow() const;

	/** Remembers a jump pressed in the air so it fires on landing (see JumpBufferTime). */
	void BufferJumpRequest();

	/** Asks for a dash on the next movement update; dropped if a dash isn't allowed then. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Movement")
	void RequestDash() { bWantsToDash = true; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	bool IsDashing() const { return DashTimeRemaining > 0.f; }

	/** Ends the current dash early, with the normal dash exit (speed cap, cooldown). */
	void CancelDash();

	/**
	 * Action-driven motion such as a sword lunge: horizontal velocity held at Direction x Speed
	 * for Duration, on the ground or in the air. A dash takes priority over it.
	 */
	void StartLunge(const FVector& Direction, float Speed, float Duration);

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	bool IsLunging() const { return LungeTimeRemaining > 0.f; }

	/** Scales the ground speed cap while an action (e.g. an attack) commits the character. 1 = none. */
	void SetActionSpeedScale(float Scale) { ActionSpeedScale = Scale; }

	/** Turns the body to face Direction (horizontal) at once. */
	void SnapFacing(const FVector& Direction);

	/** Air dashes left before landing. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	int32 GetAirDashesRemaining() const;

	/** True if a dash requested now would start (ignores the pending request itself). */
	bool CanDash() const;

	/** Wall jumps left before landing. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	int32 GetWallJumpsRemaining() const;

	/**
	 * Looks for a near-vertical wall within DetectionDistance of the capsule.
	 * Returns its horizontal normal (pointing away from the wall), or zero if none.
	 */
	FVector FindWallJumpNormal() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	bool IsWallRunning() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movement")
	bool IsWallClimbing() const;

	/** Time left in the current wall run or wall climb; 0 when not on a wall. */
	float GetWallTimeRemaining() const { return (IsWallRunning() || IsWallClimbing()) ? WallTimeRemaining : 0.f; }

	/** Wall climbs left before landing. */
	int32 GetWallClimbsRemaining() const;

	const FSAEVUSGroundMovementTuning& GetGroundTuning() const;
	const FSAEVUSAirMovementTuning& GetAirTuning() const;
	const FSAEVUSDashTuning& GetDashTuning() const;
	const FSAEVUSWallJumpTuning& GetWallJumpTuning() const;
	const FSAEVUSWallRunTuning& GetWallRunTuning() const;
	const FSAEVUSWallClimbTuning& GetWallClimbTuning() const;

	/** Swaps the movement profile and re-applies it immediately. Null restores code defaults. */
	void SetMovementSettings(const USAEVUSMovementSettings* NewSettings);

	//~ Begin UCharacterMovementComponent interface
	using Super::DoJump;
	virtual void InitializeComponent() override;
	virtual float GetMaxSpeed() const override;
	virtual float GetGravityZ() const override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual bool DoJump(bool bReplayingMoves, float DeltaTime) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual FString GetMovementName() const override;
	//~ End UCharacterMovementComponent interface

protected:
	//~ Begin UCharacterMovementComponent interface
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	//~ End UCharacterMovementComponent interface

protected:
	/** Optional profile; when unset the defaults in the tuning structs are used. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Movement")
	TObjectPtr<const USAEVUSMovementSettings> MovementSettings;

private:
	/** Pushes the active tuning into the base movement properties. */
	void ApplyMovementSettings();

	/** Launches off the wall when the jump was just pressed in the air next to one. True if it did. */
	bool TryWallJump();
	void LaunchOffWall(const FVector& PushNormal);

	/** Nearest near-vertical wall hit by capsule sweeps along Directions (horizontal, unit). */
	bool FindNearestWall(TConstArrayView<FVector> Directions, float Distance, float MaxWallNormalZ, FHitResult& OutHit) const;

	/** Starts a wall run when falling along a wall with steering input. True if it did. */
	bool TryStartWallRun();
	void PhysWallRun(float DeltaTime, int32 Iterations);

	/** Starts a climb when jumping at a wall head-on with input into it. True if it did. */
	bool TryStartWallClimb();
	void PhysWallClimb(float DeltaTime, int32 Iterations);

	/** Re-finds the wall the character is on (behind -WallNormal) and follows its normal. */
	bool ProbeCurrentWall(float DetectionDistance, float MaxWallNormalZ, FHitResult& OutWallHit);
	/**
	 * Moves by Velocity, pulled to a small gap from the wall in WallHit; lands if it touches
	 * walkable floor while coming down. Returns the displacement.
	 */
	FVector MoveAlongWall(float DeltaTime, int32 Iterations, const FHitResult& WallHit, const FQuat& Facing);
	/** Leaves the wall into a fall and simulates the rest of the update there. */
	void LeaveWall(float RemainingTime, int32 Iterations);

	void StartDash();
	/** Ends the dash: caps horizontal speed to ExitSpeed and starts the cooldown. */
	void EndDash();

	friend class FSAEVUSSavedMove;

	bool bWantsToSprint = false;
	bool bWantsToWalk = false;
	bool bWantsToDash = false;

	// Action motion driven by combat. Local state, not in the saved move yet (combat is not
	// networked until the networking phase).
	FVector LungeDirection = FVector::ForwardVector;
	float LungeSpeed = 0.f;
	float LungeTimeRemaining = 0.f;
	float ActionSpeedScale = 1.f;

	// Dash state. Saved and restored by FSAEVUSSavedMove so replays after a correction match.
	FVector DashDirection = FVector::ForwardVector;
	float DashTimeRemaining = 0.f;
	float DashCooldownRemaining = 0.f;
	int32 AirDashesUsed = 0;
	int32 WallJumpsUsed = 0;

	// Wall run / wall climb state, also saved and restored by FSAEVUSSavedMove.
	FVector WallNormal = FVector::ZeroVector;
	FVector WallRunDirection = FVector::ZeroVector;
	float WallTimeRemaining = 0.f;
	/** Normal of the wall last run on; that wall can't be run again before landing. */
	FVector LastWallRunNormal = FVector::ZeroVector;
	int32 WallClimbsUsed = 0;

	// Local simulation state derived from movement-mode transitions. Not part of the saved move:
	// after a server correction it can be briefly off by one transition (acceptable for now).
	ESAEVUSGait AirGait = ESAEVUSGait::Run;
	float CoyoteTimeRemaining = 0.f;
	float JumpBufferTimeRemaining = 0.f;
};
