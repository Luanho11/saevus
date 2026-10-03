// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SAEVUSHealthComponent.generated.h"

class USAEVUSHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSAEVUSHealthChangedSignature, USAEVUSHealthComponent*, HealthComponent, float, Delta, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSAEVUSDeathSignature, USAEVUSHealthComponent*, HealthComponent, AActor*, Killer);

/**
 * Health shared by the player and (later) enemies. Listens to the owner's engine damage
 * (OnTakeAnyDamage), so melee hits and shots reach it without knowing about it.
 * Dies once at zero; damage and healing are ignored while dead until ResetHealth().
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSHealthComponent();

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Health")
	bool IsDead() const { return bDead; }

	/** Removes health; returns how much was actually removed (0 when dead or for non-positive amounts). */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Health")
	float ApplyDamage(float Amount, AActor* DamageCauser);

	/** Adds health up to MaxHealth; returns how much was actually added. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Health")
	float Heal(float Amount);

	/** Full health and alive again. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Health")
	void ResetHealth();

	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Health")
	FSAEVUSHealthChangedSignature OnHealthChanged;

	/** Fired once, when health reaches zero. */
	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Health")
	FSAEVUSDeathSignature OnDeath;

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Health", meta = (ClampMin = "1"))
	float MaxHealth = 100.f;

	/** Ignore damage whose causer is the owner itself (own shots, own explosions later). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Health")
	bool bIgnoreSelfDamage = true;

private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	float Health = 0.f;
	bool bDead = false;
};
