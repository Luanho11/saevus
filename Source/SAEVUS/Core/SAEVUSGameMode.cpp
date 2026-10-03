// Copyright SAEVUS. All Rights Reserved.

#include "Core/SAEVUSGameMode.h"
#include "Character/SAEVUSCharacter.h"
#include "Core/SAEVUSPlayerController.h"
#include "Debug/SAEVUSDebugHUD.h"

ASAEVUSGameMode::ASAEVUSGameMode()
{
	DefaultPawnClass = ASAEVUSCharacter::StaticClass();
	PlayerControllerClass = ASAEVUSPlayerController::StaticClass();
	HUDClass = ASAEVUSDebugHUD::StaticClass();
}
