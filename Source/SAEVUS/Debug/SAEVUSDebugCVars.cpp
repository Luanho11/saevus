// Copyright SAEVUS. All Rights Reserved.

#include "Debug/SAEVUSDebugCVars.h"
#include "HAL/IConsoleManager.h"

namespace SAEVUSDebug
{
	static TAutoConsoleVariable<bool> CVarDebugHUD(
		TEXT("SAEVUS.Debug.HUD"),
		false,
		TEXT("Shows the SAEVUS movement debug HUD (gait, speed, movement mode)."),
		ECVF_Default);

	bool IsDebugHUDEnabled()
	{
		return CVarDebugHUD.GetValueOnGameThread();
	}

	void SetDebugHUDEnabled(bool bEnabled)
	{
		// Console priority so a key toggle and a typed command can override each other.
		CVarDebugHUD->Set(bEnabled, ECVF_SetByConsole);
	}
}
