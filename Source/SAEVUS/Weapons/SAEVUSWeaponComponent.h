// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/RandomStream.h"
#include "Weapons/SAEVUSWeaponTypes.h"
#include "SAEVUSWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSAEVUSWeaponFiredSignature, FVector, Muzzle, FVector, Impact, AActor*, HitActor);

/**
 * Loadout, switching and firearms. Melee weapons hand their moveset to
 * USAEVUSCombatComponent; firearms fire hitscan shots here. Each slot keeps its own
 * magazine. See Docs/combat.md.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSWeaponComponent();

	/** Replaces the loadout and equips StartSlot at once (no equip time). Resets magazines. */
	void SetLoadout(const TArray<const USAEVUSWeaponDefinition*>& NewLoadout, int32 StartSlot = 0);

	int32 GetSlotCount() const { return Loadout.Num(); }
	int32 GetCurrentSlot() const { return CurrentSlot; }

	/** Equipped weapon; null with an empty loadout or while the first weapon is still being equipped. */
	const USAEVUSWeaponDefinition* GetCurrentWeapon() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Weapons")
	bool IsFirearmEquipped() const;

	/** False if Slot is invalid, already equipped (and no switch is running) or already being switched to. */
	bool CanSwitchTo(int32 Slot) const;

	/** Starts switching to Slot; false if CanSwitchTo says no. */
	bool RequestSwitch(int32 Slot);

	/** Slot after the equipped (or pending) one, wrapping around; INDEX_NONE with fewer than two weapons. */
	int32 GetNextSlot() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Weapons")
	bool IsSwitching() const { return SwitchTimeRemaining > 0.f; }

	/** Slot being switched to, or INDEX_NONE. */
	int32 GetPendingSlot() const { return PendingSlot; }

	/** Trigger press (true) and release (false). A press always asks for one shot. */
	void SetTriggerHeld(bool bHeld);

	/** Reloads the equipped firearm if its magazine isn't full. */
	void RequestReload();

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Weapons")
	bool IsReloading() const { return ReloadTimeRemaining > 0.f; }

	/** Rounds left in Slot's magazine (0 for melee or invalid slots). */
	int32 GetAmmo(int32 Slot) const;
	int32 GetCurrentAmmo() const { return GetAmmo(CurrentSlot); }

	/** Shots fired since spawn; for debugging and tests. */
	int32 GetShotsFired() const { return ShotsFired; }

	/** Fired once per pellet trace. */
	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Weapons")
	FSAEVUSWeaponFiredSignature OnWeaponFired;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	/** Weapons by slot (keys 1, 2, 3...). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Weapons")
	TArray<TObjectPtr<const USAEVUSWeaponDefinition>> Loadout;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Weapons", meta = (ClampMin = "0"))
	int32 StartingSlot = 0;

	/** Where shots leave from, relative to the owner (actor space). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Weapons")
	FVector MuzzleOffset = FVector(0.f, 0.f, 50.f);

private:
	void ResetMagazines();
	/** Makes Slot the equipped weapon; melee weapons hand their moveset to combat. */
	void Equip(int32 Slot);
	void TryFire(float DeltaTime);
	void FireShot(const FSAEVUSFirearmTuning& Tuning);
	/** Camera view for players, eyes otherwise (also in tests, with no controller). */
	void GetAimViewPoint(FVector& OutLocation, FRotator& OutRotation) const;

	TArray<int32> Magazines;
	int32 CurrentSlot = INDEX_NONE;
	int32 PendingSlot = INDEX_NONE;
	float SwitchTimeRemaining = 0.f;
	float ReloadTimeRemaining = 0.f;
	/** Time until the next shot; may dip below zero by up to one frame to keep automatic fire on rate. */
	float FireCooldown = 0.f;
	bool bTriggerHeld = false;
	bool bShotRequested = false;
	int32 ShotsFired = 0;
	FRandomStream SpreadStream;
};
