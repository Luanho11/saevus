// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Debug switches backed by console variables, so they work from the console, ini files and key bindings alike. */
namespace SAEVUSDebug
{
	/** SAEVUS.Debug.HUD */
	SAEVUS_API bool IsDebugHUDEnabled();
	SAEVUS_API void SetDebugHUDEnabled(bool bEnabled);

	/** SAEVUS.Debug.Combat: draws attack hit spheres. */
	SAEVUS_API bool IsDebugCombatEnabled();
}
