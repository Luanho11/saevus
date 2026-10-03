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
 * Future movement abilities (wall run, wall jump, dash) belong here as custom
 * movement modes; see Docs/movement.md.
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

	const FSAEVUSGroundMovementTuning& GetGroundTuning() const;
	const FSAEVUSAirMovementTuning& GetAirTuning() const;

	/** Swaps the movement profile and re-applies it immediately. Null restores code defaults. */
	void SetMovementSettings(const USAEVUSMovementSettings* NewSettings);

	//~ Begin UCharacterMovementComponent interface
	virtual void InitializeComponent() override;
	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	//~ End UCharacterMovementComponent interface

protected:
	/** Optional profile; when unset the defaults in the tuning structs are used. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Movement")
	TObjectPtr<const USAEVUSMovementSettings> MovementSettings;

private:
	/** Pushes the active tuning into the base movement properties. */
	void ApplyMovementSettings();

	friend class FSAEVUSSavedMove;

	bool bWantsToSprint = false;
	bool bWantsToWalk = false;
};
