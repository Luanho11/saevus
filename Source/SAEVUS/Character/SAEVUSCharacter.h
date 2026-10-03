// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAEVUSCharacter.generated.h"

class USpringArmComponent;
class USAEVUSCameraComponent;
class USAEVUSCombatComponent;
class USAEVUSInputConfig;
class USAEVUSMovementComponent;
struct FInputActionValue;

/**
 * Player character for SAEVUS.
 *
 * Composition root: locomotion lives in USAEVUSMovementComponent, melee in
 * USAEVUSCombatComponent and the follow camera in USAEVUSCameraComponent. The
 * character routes input to them and lets combat veto movement actions mid-attack.
 * Mesh and animation are assigned in the Blueprint subclass, not in C++.
 */
UCLASS()
class SAEVUS_API ASAEVUSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	explicit ASAEVUSCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Character")
	USAEVUSMovementComponent* GetSAEVUSMovement() const;

	USAEVUSCombatComponent* GetCombat() const { return Combat; }
	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	USAEVUSCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/**
	 * Jump press from any input source: jumps now, or buffers the press while airborne.
	 * Dropped during an attack that can't be cancelled by a jump yet.
	 */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Character")
	void RequestJump();

	/** Dash press from any input source. Dropped during an attack that can't be cancelled by a dash yet. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Character")
	void RequestDash();

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Character")
	void RequestAttack();

	//~ Begin ACharacter interface
	virtual bool CanJumpWhileFalling() const override;
	//~ End ACharacter interface

protected:
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Input")
	TObjectPtr<const USAEVUSInputConfig> InputConfig;

private:
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleSprint(const FInputActionValue& Value);
	void HandleWalk(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSCombatComponent> Combat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSCameraComponent> FollowCamera;
};
