// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/SAEVUSCombatTypes.h"
#include "World/SAEVUSTrainingDummy.h"

namespace SAEVUSCombatTests
{
	// 40 m x 40 m floor, top surface at Z = 0.
	const FVector FloorCenter(0.f, 0.f, -50.f);
	const FVector FloorSize(4000.f, 4000.f, 100.f);
	const FVector StandLocation(0.f, 0.f, 150.f);

	// Training dummies stand on the floor in front of the character (+X).
	constexpr float DummyHalfHeight = 96.f;
	constexpr float InRangeDistance = 150.f;
	constexpr float OutOfRangeDistance = 400.f;

	constexpr float MaxAttackTime = 3.f;
	constexpr int32 TimingToleranceSteps = 1;
	constexpr float YawToleranceDeg = 1.f;
	constexpr float SpeedTolerance = 5.f;
	constexpr float LungeDistanceTolerance = 0.15f;
	// A buffer shorter than the first attack's chain point, so the press expires unused.
	constexpr float ShortInputBuffer = 0.05f;
	// Steps after a jump before attacking, so the character is airborne.
	constexpr int32 AirborneSteps = 6;
	// Recovery-time check of the slowed ground speed, after the lunge has ended.
	constexpr float SlowedSpeedCheckTime = 0.3f;

	/** Floor, a settled character and its components. */
	struct FCombatWorld
	{
		SAEVUSTests::FScopedTestWorld TestWorld;
		ASAEVUSCharacter* Character = nullptr;
		USAEVUSMovementComponent* Movement = nullptr;
		USAEVUSCombatComponent* Combat = nullptr;
		bool bReady = false;

		FCombatWorld()
		{
			TestWorld.SpawnBlock(FloorCenter, FloorSize);
			Character = TestWorld.SpawnCharacter(StandLocation);
			Movement = Character->GetSAEVUSMovement();
			Combat = Character->GetCombat();
			bReady = TestWorld.SettleOnGround();
		}

		ASAEVUSTrainingDummy* SpawnDummy(float Distance)
		{
			return TestWorld.SpawnActor<ASAEVUSTrainingDummy>(FVector(Distance, 0.f, DummyHalfHeight));
		}

		void Step(const FVector& Input = FVector::ZeroVector)
		{
			if (!Input.IsZero())
			{
				Character->AddMovementInput(Input, 1.f);
			}
			TestWorld.Step();
		}

		/** Steps until no attack is running; false if one is still running after MaxAttackTime. */
		bool StepUntilIdle()
		{
			for (int32 Index = 0; Index < SAEVUSTests::StepsFor(MaxAttackTime) && Combat->IsAttacking(); ++Index)
			{
				Step();
			}
			return !Combat->IsAttacking();
		}

		/** Steps until the running attack reaches Time; false if it ended first. */
		bool StepUntilAttackTime(float Time)
		{
			while (Combat->IsAttacking() && Combat->GetAttackTime() < Time)
			{
				Step();
			}
			return Combat->IsAttacking();
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSCombatTimelineTest, "SAEVUS.Combat.Timeline", SAEVUSTests::TestFlags)
bool FSAEVUSCombatTimelineTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSCombatTests;

	FCombatWorld World;
	if (!TestTrue(TEXT("Character settled"), World.bReady))
	{
		return false;
	}
	const FSAEVUSAttackDefinition& First = World.Combat->GetMoveset().GroundCombo[0];

	World.Character->RequestAttack();
	TestTrue(TEXT("Attack starts on the press"), World.Combat->IsAttacking());
	TestEqual(TEXT("First attack of the ground chain"), World.Combat->GetComboIndex(), 0);
	TestEqual(TEXT("Starts in startup"), World.Combat->GetAttackPhase(), ESAEVUSAttackPhase::Startup);

	int32 StartupSteps = 0;
	int32 ActiveSteps = 0;
	int32 TotalSteps = 0;
	for (int32 Index = 0; Index < SAEVUSTests::StepsFor(MaxAttackTime) && World.Combat->IsAttacking(); ++Index)
	{
		const ESAEVUSAttackPhase Phase = World.Combat->GetAttackPhase();
		StartupSteps += Phase == ESAEVUSAttackPhase::Startup ? 1 : 0;
		ActiveSteps += Phase == ESAEVUSAttackPhase::Active ? 1 : 0;
		++TotalSteps;
		World.Step();
	}
	TestFalse(TEXT("Attack ends"), World.Combat->IsAttacking());
	TestTrue(TEXT("Startup lasts StartupTime"), FMath::Abs(StartupSteps - SAEVUSTests::StepsFor(First.StartupTime)) <= TimingToleranceSteps);
	TestTrue(TEXT("Active lasts ActiveTime"), FMath::Abs(ActiveSteps - SAEVUSTests::StepsFor(First.ActiveTime)) <= TimingToleranceSteps);
	TestTrue(TEXT("Whole attack lasts its total time"), FMath::Abs(TotalSteps - SAEVUSTests::StepsFor(First.GetTotalTime())) <= TimingToleranceSteps);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSCombatHitTest, "SAEVUS.Combat.HitsDummy", SAEVUSTests::TestFlags)
bool FSAEVUSCombatHitTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSCombatTests;

	for (const float Distance : { InRangeDistance, OutOfRangeDistance })
	{
		FCombatWorld World;
		if (!TestTrue(TEXT("Character settled"), World.bReady))
		{
			return false;
		}
		const ASAEVUSTrainingDummy* Dummy = World.SpawnDummy(Distance);
		World.Character->RequestAttack();
		World.StepUntilIdle();

		const bool bInRange = Distance == InRangeDistance;
		const FSAEVUSAttackDefinition& First = World.Combat->GetMoveset().GroundCombo[0];
		TestEqual(bInRange ? TEXT("Dummy in range is hit exactly once") : TEXT("Dummy out of range is not hit"), Dummy->GetHitCount(), bInRange ? 1 : 0);
		TestEqual(TEXT("Damage from the attack data"), Dummy->GetTotalDamage(), bInRange ? First.Damage : 0.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSCombatComboTest, "SAEVUS.Combat.Combo", SAEVUSTests::TestFlags)
bool FSAEVUSCombatComboTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSCombatTests;

	FCombatWorld World;
	if (!TestTrue(TEXT("Character settled"), World.bReady))
	{
		return false;
	}
	ASAEVUSTrainingDummy* Dummy = World.SpawnDummy(InRangeDistance);
	const TArray<FSAEVUSAttackDefinition>& Chain = World.Combat->GetMoveset().GroundCombo;

	// Press once per attack, right at its start: the press is buffered until the chain point.
	World.Character->RequestAttack();
	for (int32 Expected = 1; Expected < Chain.Num(); ++Expected)
	{
		World.Character->RequestAttack();
		const float ChainFrom = Chain[Expected - 1].ChainFrom;
		World.StepUntilAttackTime(ChainFrom);
		World.Step();
		TestEqual(FString::Printf(TEXT("Buffered press chains into attack %d"), Expected + 1), World.Combat->GetComboIndex(), Expected);
	}
	TestTrue(TEXT("Chain ends after its last attack"), World.StepUntilIdle());
	TestTrue(TEXT("Every attack of the chain hit the dummy"), Dummy->GetHitCount() >= 2);

	// Too-short buffer: a press at the start of the first attack has expired by its chain point.
	USAEVUSMeleeMovesetAsset* ShortBuffer = NewObject<USAEVUSMeleeMovesetAsset>(GetTransientPackage());
	ShortBuffer->Moveset.InputBufferTime = ShortInputBuffer;
	World.Combat->SetMoveset(ShortBuffer);
	World.Character->RequestAttack();
	World.Character->RequestAttack();
	World.StepUntilIdle();
	TestFalse(TEXT("An expired press does not chain"), World.Combat->IsAttacking());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSCombatCancelTest, "SAEVUS.Combat.Cancels", SAEVUSTests::TestFlags)
bool FSAEVUSCombatCancelTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSCombatTests;

	FCombatWorld World;
	if (!TestTrue(TEXT("Character settled"), World.bReady))
	{
		return false;
	}
	const TArray<FSAEVUSAttackDefinition>& Chain = World.Combat->GetMoveset().GroundCombo;

	// Dash during startup: refused, the attack goes on.
	World.Character->RequestAttack();
	World.Character->RequestDash();
	World.Step();
	TestFalse(TEXT("No dash before the attack's dash-cancel window"), World.Movement->IsDashing());
	TestTrue(TEXT("The attack goes on"), World.Combat->IsAttacking());

	// Dash inside the window: the attack ends and the dash starts.
	World.StepUntilAttackTime(Chain[0].DashCancelFrom);
	World.Character->RequestDash();
	World.Step();
	TestFalse(TEXT("Dash in the window ends the attack"), World.Combat->IsAttacking());
	TestTrue(TEXT("Dash in the window starts"), World.Movement->IsDashing());

	// Jump during the finisher, which never allows a jump cancel.
	World.StepUntilIdle();
	World.TestWorld.SettleOnGround();
	for (int32 Index = 0; Index < SAEVUSTests::StepsFor(World.Movement->GetDashTuning().Cooldown + 0.5f); ++Index)
	{
		World.Step();
	}
	World.Character->RequestAttack();
	for (int32 Expected = 1; Expected < Chain.Num(); ++Expected)
	{
		World.Character->RequestAttack();
		World.StepUntilAttackTime(Chain[Expected - 1].ChainFrom);
		World.Step();
	}
	if (!TestEqual(TEXT("Reached the finisher"), World.Combat->GetComboIndex(), Chain.Num() - 1))
	{
		return false;
	}
	World.StepUntilAttackTime(Chain.Last().StartupTime + Chain.Last().ActiveTime);
	World.Character->RequestJump();
	World.Step();
	TestTrue(TEXT("Finisher can't be jump-cancelled"), World.Combat->IsAttacking() && World.Movement->IsMovingOnGround());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSCombatMovementTest, "SAEVUS.Combat.Movement", SAEVUSTests::TestFlags)
bool FSAEVUSCombatMovementTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSCombatTests;

	FCombatWorld World;
	if (!TestTrue(TEXT("Character settled"), World.bReady))
	{
		return false;
	}
	const FSAEVUSAttackDefinition& First = World.Combat->GetMoveset().GroundCombo[0];

	// Attack toward the input: steer +Y for one step, then attack while still steering.
	World.Step(FVector::RightVector);
	World.Character->AddMovementInput(FVector::RightVector, 1.f);
	World.Character->RequestAttack();
	TestEqual(TEXT("Attack faces the input direction"), static_cast<float>(World.Character->GetActorRotation().Yaw), 90.f, YawToleranceDeg);

	// Sword-driven movement: the lunge carries the character forward.
	const FVector Start = World.Character->GetActorLocation();
	for (int32 Index = 0; Index < SAEVUSTests::StepsFor(First.LungeDuration); ++Index)
	{
		World.Step(FVector::RightVector);
	}
	const float LungeDistance = static_cast<float>(World.Character->GetActorLocation().Y - Start.Y);
	const float ExpectedLunge = First.LungeSpeed * First.LungeDuration;
	AddInfo(FString::Printf(TEXT("Lunge distance %.1f cm, expected %.1f cm"), LungeDistance, ExpectedLunge));
	TestEqual(TEXT("Lunge distance ~ LungeSpeed x LungeDuration"), LungeDistance, ExpectedLunge, ExpectedLunge * LungeDistanceTolerance);

	// After the lunge, steering is slowed to the attack's ground speed scale.
	World.StepUntilAttackTime(SlowedSpeedCheckTime);
	const float SlowedCap = World.Movement->GetGaitSpeed(ESAEVUSGait::Run) * First.GroundMoveSpeedScale;
	TestTrue(TEXT("Ground speed slowed during the attack"), World.Movement->Velocity.Size2D() <= SlowedCap + SpeedTolerance);

	World.StepUntilIdle();
	TestEqual(TEXT("Full speed cap back after the attack"), World.Movement->GetMaxSpeed(), World.Movement->GetGaitSpeed(ESAEVUSGait::Run));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSCombatAirTest, "SAEVUS.Combat.AirAttack", SAEVUSTests::TestFlags)
bool FSAEVUSCombatAirTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSCombatTests;

	FCombatWorld World;
	if (!TestTrue(TEXT("Character settled"), World.bReady))
	{
		return false;
	}
	World.Character->RequestJump();
	for (int32 Index = 0; Index < AirborneSteps; ++Index)
	{
		World.Step();
	}
	if (!TestTrue(TEXT("Airborne"), World.Movement->IsFalling()))
	{
		return false;
	}
	World.Character->RequestAttack();
	TestTrue(TEXT("Attack in the air uses the air chain"), World.Combat->IsAirChain());
	TestEqual(TEXT("First air attack"), World.Combat->GetCurrentAttack()->Name, World.Combat->GetMoveset().AirCombo[0].Name);
	World.Step();
	TestTrue(TEXT("Still airborne: an air attack doesn't stop the jump"), World.Movement->IsFalling());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSCombatAttackEndsDashTest, "SAEVUS.Combat.AttackEndsDash", SAEVUSTests::TestFlags)
bool FSAEVUSCombatAttackEndsDashTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSCombatTests;

	FCombatWorld World;
	if (!TestTrue(TEXT("Character settled"), World.bReady))
	{
		return false;
	}
	World.Character->RequestDash();
	World.Step();
	if (!TestTrue(TEXT("Dashing"), World.Movement->IsDashing()))
	{
		return false;
	}
	World.Character->RequestAttack();
	TestTrue(TEXT("Attack starts during the dash"), World.Combat->IsAttacking());
	TestFalse(TEXT("The attack ends the dash"), World.Movement->IsDashing());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
