// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SAEVUSGameMode.generated.h"

/**
 * Default game mode. The native classes are fallbacks; the Blueprint subclass
 * swaps in the Blueprint pawn and controller that carry asset references.
 */
UCLASS()
class SAEVUS_API ASAEVUSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASAEVUSGameMode();
};
