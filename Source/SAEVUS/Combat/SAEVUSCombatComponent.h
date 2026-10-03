// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/SAEVUSCombatTypes.h"
#include "Components/ActorComponent.h"
#include "SAEVUSCombatComponent.generated.h"

class ACharacter;
class USAEVUSMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSAEVUSAttackHitSignature, AActor*, HitActor, float, Damage);

/**
 * Melee foundation: runs attack timelines from a moveset, buffers attack presses, chains
 * combos, applies hits, and decides when movement actions may cancel an attack.
 *
 * Movement effects (lunge, slowed ground speed, facing) go through USAEVUSMovementComponent,
 * which ticks after this component. See Docs/combat.md.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSCombatComponent();

	/** Attack press from any input source; starts or chains an attack as soon as allowed. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Combat")
	void RequestAttack();

	/**
	 * Asks whether a movement action may happen now. Not attacking: yes. Attacking: only inside
	 * the attack's cancel window for that action, and then the attack ends.
	 */
	bool TryCancelForAction(ESAEVUSCancelAction Action);

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Combat")
	bool IsAttacking() const { return CurrentAttack != nullptr; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Combat")
	ESAEVUSAttackPhase GetAttackPhase() const;

	/** The running attack, or null. */
	const FSAEVUSAttackDefinition* GetCurrentAttack() const { return CurrentAttack; }

	/** Position of the running attack in its chain (0 = first), or INDEX_NONE. */
	int32 GetComboIndex() const { return ComboIndex; }
	bool IsAirChain() const { return bAirChain; }
	float GetAttackTime() const { return AttackTime; }
	bool HasBufferedAttack() const { return BufferedAttackTimeRemaining > 0.f; }

	const FSAEVUSMeleeMoveset& GetMoveset() const;

	/** Swaps the moveset. Null restores the code defaults. Ends a running attack. */
	void SetMoveset(const USAEVUSMeleeMovesetAsset* NewMoveset);

	/** Fired once per actor hit by an attack. */
	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Combat")
	FSAEVUSAttackHitSignature OnAttackHit;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/** Optional moveset; when unset the defaults in FSAEVUSMeleeMoveset are used. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Combat")
	TObjectPtr<const USAEVUSMeleeMovesetAsset> MovesetAsset;

private:
	/** Starts or chains an attack if the buffered press can be used now. */
	void ProcessBufferedAttack();
	/** Attacks start on the ground or in the air, not on walls; a dash may be cut short for one. */
	bool CanStartAttackNow() const;
	void StartAttack(bool bInAir, int32 Index);
	void EndAttack();
	void DetectHits(const FSAEVUSAttackDefinition& Attack);

	ACharacter* GetCharacter() const;
	USAEVUSMovementComponent* GetMovement() const;

	const FSAEVUSAttackDefinition* CurrentAttack = nullptr;
	int32 ComboIndex = INDEX_NONE;
	bool bAirChain = false;
	float AttackTime = 0.f;
	float BufferedAttackTimeRemaining = 0.f;

	/** Actors already hit by the running attack; each is hit once per attack. */
	TArray<TWeakObjectPtr<AActor>> HitActors;
};
