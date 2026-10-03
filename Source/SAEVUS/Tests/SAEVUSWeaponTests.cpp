// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/SAEVUSCombatTypes.h"
#include "Weapons/SAEVUSWeaponTypes.h"
#include "World/SAEVUSTrainingDummy.h"

namespace SAEVUSWeaponTests
{
	// 40 m x 40 m floor, top surface at Z = 0.
	const FVector FloorCenter(0.f, 0.f, -50.f);
	const FVector FloorSize(4000.f, 4000.f, 100.f);
	const FVector StandLocation(0.f, 0.f, 150.f);

	// Dummy straight ahead (+X), standing on the floor.
	constexpr float DummyHalfHeight = 96.f;
	constexpr float DummyDistance = 500.f;
	// Cover between the character and the dummy, taller than both.
	const FVector CoverCenter(250.f, 0.f, 200.f);
	const FVector CoverSize(50.f, 400.f, 400.f);

	constexpr float ShortRange = 300.f;
	constexpr float HoldTime = 1.f;
	constexpr int32 RateToleranceShots = 1;
	constexpr int32 Pellets = 5;
	constexpr float SettleMargin = 0.1f;

	/** Test firearm: accurate (no spread) so every shot goes where the character faces. */
	USAEVUSWeaponDefinition* MakeFirearm(FName Name, bool bAutomatic, int32 MagazineSize, float FireInterval)
	{
		USAEVUSWeaponDefinition* Weapon = NewObject<USAEVUSWeaponDefinition>(GetTransientPackage());
		Weapon->DisplayName = Name;
		Weapon->Kind = ESAEVUSWeaponKind::Firearm;
		Weapon->Firearm.bAutomatic = bAutomatic;
		Weapon->Firearm.MagazineSize = MagazineSize;
		Weapon->Firearm.FireInterval = FireInterval;
		Weapon->Firearm.SpreadDegrees = 0.f;
		return Weapon;
	}

	USAEVUSWeaponDefinition* MakePistol() { return MakeFirearm(TEXT("Pistol"), false, 12, 0.25f); }
	USAEVUSWeaponDefinition* MakeRifle(int32 MagazineSize = 30) { return MakeFirearm(TEXT("Rifle"), true, MagazineSize, 0.1f); }

	USAEVUSWeaponDefinition* MakeMelee(const USAEVUSMeleeMovesetAsset* Moveset = nullptr)
	{
		USAEVUSWeaponDefinition* Weapon = NewObject<USAEVUSWeaponDefinition>(GetTransientPackage());
		Weapon->DisplayName = TEXT("Sword");
		Weapon->Kind = ESAEVUSWeaponKind::Melee;
		Weapon->Moveset = Moveset;
		return Weapon;
	}

	/** Floor, a settled character, an optional dummy, and a loadout. */
	struct FWeaponWorld
	{
		SAEVUSTests::FScopedTestWorld TestWorld;
		ASAEVUSCharacter* Character = nullptr;
		USAEVUSWeaponComponent* Weapons = nullptr;
		USAEVUSCombatComponent* Combat = nullptr;
		ASAEVUSTrainingDummy* Dummy = nullptr;
		bool bReady = false;

		explicit FWeaponWorld(const TArray<const USAEVUSWeaponDefinition*>& Loadout, bool bWithDummy = true)
		{
			TestWorld.SpawnBlock(FloorCenter, FloorSize);
			if (bWithDummy)
			{
				Dummy = TestWorld.SpawnActor<ASAEVUSTrainingDummy>(FVector(DummyDistance, 0.f, DummyHalfHeight));
			}
			Character = TestWorld.SpawnCharacter(StandLocation);
			Weapons = Character->GetWeapons();
			Combat = Character->GetCombat();
			Weapons->SetLoadout(Loadout);
			bReady = TestWorld.SettleOnGround();
		}

		void StepFor(float Seconds)
		{
			for (int32 Index = 0; Index < SAEVUSTests::StepsFor(Seconds); ++Index)
			{
				TestWorld.Step();
			}
		}

		/** Presses and releases the trigger with one step in between. */
		void Tap()
		{
			Character->RequestAttack();
			TestWorld.Step();
			Character->ReleaseAttack();
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWeaponHitscanTest, "SAEVUS.Weapons.Hitscan", SAEVUSTests::TestFlags)
bool FSAEVUSWeaponHitscanTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWeaponTests;

	{
		const USAEVUSWeaponDefinition* Pistol = MakePistol();
		FWeaponWorld World({ Pistol });
		if (!TestTrue(TEXT("Character settled"), World.bReady))
		{
			return false;
		}
		TestTrue(TEXT("Firearm equipped from the loadout"), World.Weapons->IsFirearmEquipped());
		World.Tap();
		TestEqual(TEXT("One shot"), World.Weapons->GetShotsFired(), 1);
		TestEqual(TEXT("Shot hits the dummy once"), World.Dummy->GetHitCount(), 1);
		TestEqual(TEXT("Damage from the firearm data"), World.Dummy->GetTotalDamage(), Pistol->Firearm.Damage);
		TestEqual(TEXT("One round used"), World.Weapons->GetCurrentAmmo(), Pistol->Firearm.MagazineSize - 1);
	}
	{
		USAEVUSWeaponDefinition* ShortPistol = MakePistol();
		ShortPistol->Firearm.Range = ShortRange;
		FWeaponWorld World({ ShortPistol });
		World.Tap();
		TestEqual(TEXT("Out of range: fired but no hit"), World.Dummy->GetHitCount(), 0);
	}
	{
		FWeaponWorld World({ MakePistol() });
		World.TestWorld.SpawnBlock(CoverCenter, CoverSize);
		World.Tap();
		TestEqual(TEXT("Cover in front of the muzzle stops the shot"), World.Dummy->GetHitCount(), 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWeaponFireModesTest, "SAEVUS.Weapons.FireModes", SAEVUSTests::TestFlags)
bool FSAEVUSWeaponFireModesTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWeaponTests;

	{
		FWeaponWorld World({ MakePistol() }, false);
		World.Character->RequestAttack();
		World.StepFor(HoldTime);
		World.Character->ReleaseAttack();
		TestEqual(TEXT("Semi-automatic: one shot per press, however long it is held"), World.Weapons->GetShotsFired(), 1);
	}
	{
		const USAEVUSWeaponDefinition* Rifle = MakeRifle();
		FWeaponWorld World({ Rifle }, false);
		World.Character->RequestAttack();
		World.StepFor(HoldTime);
		World.Character->ReleaseAttack();
		const int32 Expected = FMath::RoundToInt32(HoldTime / Rifle->Firearm.FireInterval);
		AddInfo(FString::Printf(TEXT("Automatic: %d shots in %.1f s, expected %d"), World.Weapons->GetShotsFired(), HoldTime, Expected));
		TestTrue(TEXT("Automatic: fires at its rate while held"), FMath::Abs(World.Weapons->GetShotsFired() - Expected) <= RateToleranceShots);
	}
	{
		USAEVUSWeaponDefinition* Shotgun = MakeFirearm(TEXT("Shotgun"), false, 6, 0.8f);
		Shotgun->Firearm.PelletsPerShot = Pellets;
		FWeaponWorld World({ Shotgun });
		World.Tap();
		TestEqual(TEXT("Every pellet is its own hit"), World.Dummy->GetHitCount(), Pellets);
		TestEqual(TEXT("Damage per pellet"), World.Dummy->GetTotalDamage(), Shotgun->Firearm.Damage * Pellets);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWeaponReloadTest, "SAEVUS.Weapons.Reload", SAEVUSTests::TestFlags)
bool FSAEVUSWeaponReloadTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWeaponTests;

	constexpr int32 SmallMagazine = 3;
	const USAEVUSWeaponDefinition* Rifle = MakeRifle(SmallMagazine);
	FWeaponWorld World({ Rifle }, false);

	// Hold until the magazine runs dry: the dry trigger starts a reload.
	World.Character->RequestAttack();
	World.StepFor(Rifle->Firearm.FireInterval * (SmallMagazine + 1));
	TestEqual(TEXT("Empties the magazine"), World.Weapons->GetShotsFired(), SmallMagazine);
	TestTrue(TEXT("Dry trigger reloads"), World.Weapons->IsReloading());
	World.StepFor(Rifle->Firearm.ReloadTime * 0.5f);
	TestEqual(TEXT("No shots while reloading"), World.Weapons->GetShotsFired(), SmallMagazine);
	World.Character->ReleaseAttack();
	World.StepFor(Rifle->Firearm.ReloadTime * 0.5f + SettleMargin);
	TestFalse(TEXT("Reload finishes"), World.Weapons->IsReloading());
	TestEqual(TEXT("Magazine full after the reload"), World.Weapons->GetCurrentAmmo(), SmallMagazine);

	// Manual reload: refused with a full magazine, accepted after a shot.
	World.Character->RequestReload();
	TestFalse(TEXT("No reload with a full magazine"), World.Weapons->IsReloading());
	World.Tap();
	World.StepFor(Rifle->Firearm.FireInterval);
	World.Character->RequestReload();
	TestTrue(TEXT("Reload after a shot"), World.Weapons->IsReloading());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWeaponSwitchTest, "SAEVUS.Weapons.Switch", SAEVUSTests::TestFlags)
bool FSAEVUSWeaponSwitchTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWeaponTests;

	const USAEVUSWeaponDefinition* Sword = MakeMelee();
	const USAEVUSWeaponDefinition* Pistol = MakePistol();
	FWeaponWorld World({ Sword, Pistol });
	if (!TestTrue(TEXT("Character settled"), World.bReady))
	{
		return false;
	}
	TestFalse(TEXT("Starts on the melee weapon"), World.Weapons->IsFirearmEquipped());

	World.Character->RequestWeaponSlot(1);
	TestTrue(TEXT("Switching to the pistol"), World.Weapons->IsSwitching());
	World.Character->RequestAttack();
	World.TestWorld.Step();
	World.Character->ReleaseAttack();
	TestTrue(TEXT("No attack and no shot while switching"), !World.Combat->IsAttacking() && World.Weapons->GetShotsFired() == 0);

	World.StepFor(Pistol->EquipTime + SettleMargin);
	TestEqual(TEXT("Pistol equipped after its equip time"), World.Weapons->GetCurrentSlot(), 1);
	World.Tap();
	TestEqual(TEXT("Primary action fires the pistol"), World.Dummy->GetHitCount(), 1);

	World.Character->RequestWeaponSlot(1);
	TestFalse(TEXT("Selecting the equipped weapon does nothing"), World.Weapons->IsSwitching());

	World.Character->RequestNextWeapon();
	World.StepFor(Sword->EquipTime + SettleMargin);
	TestEqual(TEXT("Next weapon wraps back to the sword"), World.Weapons->GetCurrentSlot(), 0);
	World.Character->RequestAttack();
	TestTrue(TEXT("Primary action attacks with the sword"), World.Combat->IsAttacking());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWeaponSwitchCancelTest, "SAEVUS.Weapons.SwitchCancels", SAEVUSTests::TestFlags)
bool FSAEVUSWeaponSwitchCancelTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWeaponTests;

	{
		FWeaponWorld World({ MakeMelee(), MakePistol() }, false);
		World.Character->RequestAttack();
		World.Character->RequestWeaponSlot(1);
		TestFalse(TEXT("No switch before the attack's switch-cancel window"), World.Weapons->IsSwitching());
		TestTrue(TEXT("The attack goes on"), World.Combat->IsAttacking());

		const float SwitchCancelFrom = World.Combat->GetCurrentAttack()->SwitchCancelFrom;
		while (World.Combat->IsAttacking() && World.Combat->GetAttackTime() < SwitchCancelFrom)
		{
			World.TestWorld.Step();
		}
		World.Character->RequestWeaponSlot(1);
		TestTrue(TEXT("Switch in the window starts"), World.Weapons->IsSwitching());
		TestFalse(TEXT("Switch in the window ends the attack"), World.Combat->IsAttacking());
	}
	{
		const USAEVUSWeaponDefinition* Pistol = MakePistol();
		FWeaponWorld World({ Pistol, MakePistol() }, false);
		World.Tap();
		World.StepFor(Pistol->Firearm.FireInterval);
		World.Character->RequestReload();
		World.Character->RequestWeaponSlot(1);
		TestFalse(TEXT("Switching drops the reload"), World.Weapons->IsReloading());
		World.StepFor(Pistol->EquipTime + SettleMargin);
		World.Character->RequestWeaponSlot(0);
		World.StepFor(Pistol->EquipTime + Pistol->Firearm.ReloadTime);
		TestEqual(TEXT("The dropped reload never refilled the magazine"), World.Weapons->GetCurrentAmmo(), Pistol->Firearm.MagazineSize - 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWeaponMeleeMovesetTest, "SAEVUS.Weapons.MeleeMoveset", SAEVUSTests::TestFlags)
bool FSAEVUSWeaponMeleeMovesetTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWeaponTests;

	USAEVUSMeleeMovesetAsset* Moveset = NewObject<USAEVUSMeleeMovesetAsset>(GetTransientPackage());
	Moveset->Moveset.GroundCombo[0].Name = TEXT("TestOpener");
	const USAEVUSWeaponDefinition* Sword = MakeMelee(Moveset);
	FWeaponWorld World({ MakePistol(), Sword }, false);

	World.Character->RequestWeaponSlot(1);
	World.StepFor(Sword->EquipTime + SettleMargin);
	World.Character->RequestAttack();
	if (!TestTrue(TEXT("Melee attack after switching to the melee weapon"), World.Combat->IsAttacking()))
	{
		return false;
	}
	TestEqual(TEXT("Combat uses the melee weapon's moveset"), World.Combat->GetCurrentAttack()->Name, FName(TEXT("TestOpener")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
