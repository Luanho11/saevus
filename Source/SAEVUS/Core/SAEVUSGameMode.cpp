// Copyright SAEVUS. All Rights Reserved.

#include "Core/SAEVUSGameMode.h"
#include "Character/SAEVUSCharacter.h"
#include "Core/SAEVUSPlayerController.h"
#include "Debug/SAEVUSDebugHUD.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

ASAEVUSGameMode::ASAEVUSGameMode()
{
	DefaultPawnClass = ASAEVUSCharacter::StaticClass();
	PlayerControllerClass = ASAEVUSPlayerController::StaticClass();
	HUDClass = ASAEVUSDebugHUD::StaticClass();
}

void ASAEVUSGameMode::HandlePlayerDeath(AController* Controller)
{
	if (!Controller)
	{
		return;
	}
	FTimerHandle Unused;
	GetWorldTimerManager().SetTimer(Unused, FTimerDelegate::CreateUObject(this, &ThisClass::RespawnPlayer, TWeakObjectPtr<AController>(Controller)),
		FMath::Max(RespawnDelay, UE_KINDA_SMALL_NUMBER), false);
}

void ASAEVUSGameMode::RespawnPlayer(TWeakObjectPtr<AController> Controller)
{
	if (!Controller.IsValid())
	{
		return;
	}
	if (APawn* DeadPawn = Controller->GetPawn())
	{
		Controller->UnPossess();
		DeadPawn->SetLifeSpan(FMath::Max(CorpseLifeSpan, UE_KINDA_SMALL_NUMBER));
	}
	RestartPlayer(Controller.Get());
}
