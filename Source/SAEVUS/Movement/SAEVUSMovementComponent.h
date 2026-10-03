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
 * while falling. Wall run and wall climb will be custom movement modes; see
 * Docs/movement.md.
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

	const FSAEVUSGroundMovementTuning& GetGroundTuning() const;
	const FSAEVUSAirMovementTuning& GetAirTuning() const;
	const FSAEVUSDashTuning& GetDashTuning() const;
	const FSAEVUSWallJumpTuning& GetWallJumpTuning() const;

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
	//~ End UCharacterMovementComponent interface

protected:
	//~ Begin UCharacterMovementComponent interface
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
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
	void LaunchOffWall(const FVector& WallNormal);

	void StartDash();
	/** Ends the dash: caps horizontal speed to ExitSpeed and starts the cooldown. */
	void EndDash();

	friend class FSAEVUSSavedMove;

	bool bWantsToSprint = false;
	bool bWantsToWalk = false;
	bool bWantsToDash = false;

	// Dash state. Saved and restored by FSAEVUSSavedMove so replays after a correction match.
	FVector DashDirection = FVector::ForwardVector;
	float DashTimeRemaining = 0.f;
	float DashCooldownRemaining = 0.f;
	int32 AirDashesUsed = 0;
	int32 WallJumpsUsed = 0;

	// Local simulation state derived from movement-mode transitions. Not part of the saved move:
	// after a server correction it can be briefly off by one transition (acceptable for now).
	ESAEVUSGait AirGait = ESAEVUSGait::Run;
	float CoyoteTimeRemaining = 0.f;
	float JumpBufferTimeRemaining = 0.f;
};
