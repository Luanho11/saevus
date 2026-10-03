// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SAEVUSPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

/** Adds the input mapping contexts for the local player and handles controller-level (non-pawn) input. */
UCLASS()
class SAEVUS_API ASAEVUSPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;

	/** Added in order, with priority 0, for the local player. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	/** Toggles the movement debug HUD (SAEVUS.Debug.HUD). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Input|Debug")
	TObjectPtr<UInputAction> ToggleDebugHUDAction;

private:
	void HandleToggleDebugHUD();
};
