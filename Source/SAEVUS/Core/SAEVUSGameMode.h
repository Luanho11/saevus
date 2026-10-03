// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SAEVUSGameMode.generated.h"

/**
 * Default game mode. The native classes are fallbacks; the Blueprint subclass
 * swaps in the Blueprint pawn and controller that carry asset references.
 * Also owns the respawn rule: a dead player comes back after RespawnDelay.
 */
UCLASS()
class SAEVUS_API ASAEVUSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASAEVUSGameMode();

	/** Called by a character that died. Schedules a respawn for its controller (if any). */
	void HandlePlayerDeath(AController* Controller);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Respawn", meta = (ClampMin = "0", ForceUnits = "s"))
	float RespawnDelay = 3.f;

	/** How long the dead body stays after the respawn. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Respawn", meta = (ClampMin = "0", ForceUnits = "s"))
	float CorpseLifeSpan = 10.f;

private:
	void RespawnPlayer(TWeakObjectPtr<AController> Controller);
};
