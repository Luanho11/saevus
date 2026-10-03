// Copyright SAEVUS. All Rights Reserved.

#include "Combat/SAEVUSCombatTypes.h"

FSAEVUSMeleeMoveset::FSAEVUSMeleeMoveset()
{
	// Default moveset: two quick slashes and a heavier finisher on the ground, one air slash.
	// First-pass values, tuned on the Data Asset later.

	// Quick slashes use the FSAEVUSAttackDefinition defaults.
	FSAEVUSAttackDefinition Slash1;
	Slash1.Name = TEXT("Slash1");

	FSAEVUSAttackDefinition Slash2;
	Slash2.Name = TEXT("Slash2");
	Slash2.Damage = 12.f;

	// Finisher: slower, wider, longer lunge, and it commits: no jump out, dash only late.
	FSAEVUSAttackDefinition Finisher;
	Finisher.Name = TEXT("Finisher");
	Finisher.StartupTime = 0.18f;
	Finisher.ActiveTime = 0.12f;
	Finisher.RecoveryTime = 0.45f;
	Finisher.Damage = 25.f;
	Finisher.HitRadius = 75.f;
	Finisher.HitForwardOffset = 100.f;
	Finisher.LungeSpeed = 650.f;
	Finisher.LungeDuration = 0.18f;
	Finisher.GroundMoveSpeedScale = 0.f;
	Finisher.DashCancelFrom = 0.4f;
	Finisher.JumpCancelFrom = -1.f;
	Finisher.SwitchCancelFrom = 0.4f;

	GroundCombo = { Slash1, Slash2, Finisher };

	// Air slash: no lunge, so the jump's momentum carries it.
	FSAEVUSAttackDefinition AirSlash;
	AirSlash.Name = TEXT("AirSlash");
	AirSlash.StartupTime = 0.08f;
	AirSlash.ActiveTime = 0.1f;
	AirSlash.RecoveryTime = 0.25f;
	AirSlash.Damage = 12.f;
	AirSlash.HitHeightOffset = 0.f;
	AirSlash.LungeSpeed = 0.f;
	AirSlash.ChainFrom = AirSlash.StartupTime + AirSlash.ActiveTime;
	AirSlash.DashCancelFrom = AirSlash.ChainFrom;
	AirSlash.SwitchCancelFrom = AirSlash.ChainFrom;

	AirCombo = { AirSlash };
}
