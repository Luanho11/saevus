// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSWeaponTypes.generated.h"

class USAEVUSMeleeMovesetAsset;

UENUM(BlueprintType)
enum class ESAEVUSWeaponKind : uint8
{
	/** Attacks come from USAEVUSCombatComponent using the weapon's moveset. */
	Melee,
	/** Hitscan shots from USAEVUSWeaponComponent. */
	Firearm
};

/** Hitscan firearm tuning. The member initializers describe a plain semi-automatic sidearm. */
USTRUCT(BlueprintType)
struct FSAEVUSFirearmTuning
{
	GENERATED_BODY()

	/** Damage per pellet. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firearm", meta = (ClampMin = "0"))
	float Damage = 20.f;

	/** Traces per shot; above 1 for spread weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firearm", meta = (ClampMin = "1"))
	int32 PelletsPerShot = 1;

	/** Shortest time between shots. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firearm", meta = (ClampMin = "0", ForceUnits = "s"))
	float FireInterval = 0.25f;

	/** Keeps firing while the trigger is held; otherwise one shot per press. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firearm")
	bool bAutomatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "1"))
	int32 MagazineSize = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0", ForceUnits = "s"))
	float ReloadTime = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (ClampMin = "0", ForceUnits = "cm"))
	float Range = 5000.f;

	/** Half-angle of the cone each pellet is randomly placed in. 0 = perfectly accurate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (ClampMin = "0", ClampMax = "45", ForceUnits = "deg"))
	float SpreadDegrees = 0.5f;
};

/** One weapon of a loadout: a melee moveset or a firearm, plus how long it takes to equip. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName DisplayName = TEXT("Weapon");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	ESAEVUSWeaponKind Kind = ESAEVUSWeaponKind::Firearm;

	/** Time from the switch request until this weapon can be used. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = "0", ForceUnits = "s"))
	float EquipTime = 0.25f;

	/** Melee only. Null uses the code default moveset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (EditCondition = "Kind == ESAEVUSWeaponKind::Melee"))
	TObjectPtr<const USAEVUSMeleeMovesetAsset> Moveset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Firearm", meta = (EditCondition = "Kind == ESAEVUSWeaponKind::Firearm"))
	FSAEVUSFirearmTuning Firearm;

	bool IsFirearm() const { return Kind == ESAEVUSWeaponKind::Firearm; }
};
