// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSCombatTypes.generated.h"

/** Where an attack is in its timeline. */
UENUM(BlueprintType)
enum class ESAEVUSAttackPhase : uint8
{
	None,
	/** Wind-up: no hits yet. */
	Startup,
	/** The hit volume is live. */
	Active,
	/** Follow-through: no hits; the next attack can chain from ChainFrom. */
	Recovery
};

/** Player actions that may cut an attack short, each with its own window in the attack data. */
UENUM(BlueprintType)
enum class ESAEVUSCancelAction : uint8
{
	Dash,
	Jump
};

/**
 * One melee attack, timed in seconds from its start. Without attack animations yet, the
 * timeline itself is the attack: phases, hit volume, lunge and cancel windows.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAttackDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	FName Name = TEXT("Attack");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0", ForceUnits = "s"))
	float StartupTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0", ForceUnits = "s"))
	float ActiveTime = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0", ForceUnits = "s"))
	float RecoveryTime = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit", meta = (ClampMin = "0"))
	float Damage = 10.f;

	/** Hit sphere, placed in front of the character while the attack is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit", meta = (ClampMin = "0", ForceUnits = "cm"))
	float HitRadius = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit", meta = (ForceUnits = "cm"))
	float HitForwardOffset = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit", meta = (ForceUnits = "cm"))
	float HitHeightOffset = 20.f;

	/** Sword-driven movement: forward speed held from the start of the attack. 0 = none. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float LungeSpeed = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0", ForceUnits = "s"))
	float LungeDuration = 0.12f;

	/** Ground speed cap multiplier while this attack runs (the lunge is not limited by it). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0", ClampMax = "1"))
	float GroundMoveSpeedScale = 0.25f;

	/** From this time, a buffered attack press starts the next attack of the chain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancels", meta = (ForceUnits = "s"))
	float ChainFrom = 0.18f;

	/** From this time a dash may cut the attack short. Negative: never. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancels", meta = (ForceUnits = "s"))
	float DashCancelFrom = 0.18f;

	/** From this time a jump may cut the attack short. Negative: never. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancels", meta = (ForceUnits = "s"))
	float JumpCancelFrom = 0.18f;

	float GetTotalTime() const { return StartupTime + ActiveTime + RecoveryTime; }

	ESAEVUSAttackPhase GetPhaseAt(float Time) const
	{
		if (Time < StartupTime)
		{
			return ESAEVUSAttackPhase::Startup;
		}
		if (Time < StartupTime + ActiveTime)
		{
			return ESAEVUSAttackPhase::Active;
		}
		return Time < GetTotalTime() ? ESAEVUSAttackPhase::Recovery : ESAEVUSAttackPhase::None;
	}

	float GetCancelFrom(ESAEVUSCancelAction Action) const
	{
		return Action == ESAEVUSCancelAction::Dash ? DashCancelFrom : JumpCancelFrom;
	}
};

/**
 * A character's melee moves: one chain on the ground, one in the air, plus input handling.
 * The constructor holds the default moveset; a USAEVUSMeleeMovesetAsset overrides it.
 */
USTRUCT(BlueprintType)
struct SAEVUS_API FSAEVUSMeleeMoveset
{
	GENERATED_BODY()

	FSAEVUSMeleeMoveset();

	/** Attacks chained in order on the ground; the chain restarts after the last one. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moveset")
	TArray<FSAEVUSAttackDefinition> GroundCombo;

	/** Attacks chained in order when the first one starts in the air. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Moveset")
	TArray<FSAEVUSAttackDefinition> AirCombo;

	/** An attack press is remembered this long until it can start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (ClampMin = "0", ForceUnits = "s"))
	float InputBufferTime = 0.2f;

	/** An attack pressed during a dash ends the dash and starts at once (otherwise it waits). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancels")
	bool bAttackEndsDash = true;
};

/** Designer-editable melee moveset assigned to USAEVUSCombatComponent. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSMeleeMovesetAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Moveset")
	FSAEVUSMeleeMoveset Moveset;
};
