// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAEVUSCharacter.generated.h"

class USpringArmComponent;
class USAEVUSCameraComponent;
class USAEVUSInputConfig;
class USAEVUSMovementComponent;
struct FInputActionValue;

/**
 * Player character for SAEVUS.
 *
 * Composition root: locomotion lives in USAEVUSMovementComponent and the follow
 * camera in USAEVUSCameraComponent. The character only routes input to them.
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

	USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	USAEVUSCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Jump press from any input source: jumps now, or buffers the press while airborne. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Character")
	void RequestJump();

	//~ Begin ACharacter interface
	virtual bool CanJumpWhileFalling() const override;
	//~ End ACharacter interface

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Input")
	TObjectPtr<const USAEVUSInputConfig> InputConfig;

private:
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleSprint(const FInputActionValue& Value);
	void HandleWalk(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSCameraComponent> FollowCamera;
};
