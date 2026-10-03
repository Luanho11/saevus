// Copyright SAEVUS. All Rights Reserved.

#include "Core/SAEVUSPlayerController.h"
#include "Debug/SAEVUSDebugCVars.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void ASAEVUSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (const UInputMappingContext* Context : DefaultMappingContexts)
		{
			if (Context)
			{
				Subsystem->AddMappingContext(Context, 0);
			}
		}
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (Input && ToggleDebugHUDAction)
	{
		Input->BindAction(ToggleDebugHUDAction, ETriggerEvent::Started, this, &ThisClass::HandleToggleDebugHUD);
	}
}

void ASAEVUSPlayerController::HandleToggleDebugHUD()
{
	SAEVUSDebug::SetDebugHUDEnabled(!SAEVUSDebug::IsDebugHUDEnabled());
}
