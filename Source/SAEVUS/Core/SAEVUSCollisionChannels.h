// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "Engine/EngineTypes.h"

/** Project collision channels. Names and default responses live in Config/DefaultEngine.ini. */
namespace SAEVUSCollision
{
	/** "Weapon" trace channel: hitscan shots. Blocks by default, including pawns (unlike Visibility). */
	constexpr ECollisionChannel WeaponTrace = ECC_GameTraceChannel1;
}
