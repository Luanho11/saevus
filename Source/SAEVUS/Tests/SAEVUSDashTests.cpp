// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movement/SAEVUSMovementSettings.h"

namespace SAEVUSDashTests
{
	// 40 m x 40 m platform, top surface at Z = 0: room to dash in any direction.
	const FVector PlatformCenter(0.f, 0.f, -50.f);
	const FVector PlatformSize(4000.f, 4000.f, 100.f);
	const FVector StandLocation(0.f, 0.f, 150.f);

	// Upper bound for any single dash; the default lasts well under this.
	constexpr float MaxDashTime = 1.f;

	// Fixed-step simulation rounds the dash up to whole steps, so distance gets a tolerance.
	constexpr float DistanceTolerance = 0.1f;
	constexpr float SpeedTolerance = 1.f;
	constexpr float DirectionMinDot = 0.99f;
	constexpr float YawToleranceDeg = 1.f;

	// Gravity is suspended during an air dash, so the height barely changes.
	constexpr float AirDashMaxHeightChange = 2.f;
	// Steps after take-off before the air dash, so the character is clearly airborne.
	constexpr int32 AirborneSteps = 10;

	USAEVUSMovementSettings* MakeSettings()
	{
		return NewObject<USAEVUSMovementSettings>(GetTransientPackage());
	}

	/** Steps until the current dash ends; false if it is still running after MaxDashTime. */
	bool StepUntilDashEnds(SAEVUSTests::FScopedTestWorld& TestWorld, const USAEVUSMovementComponent* Movement)
	{
		for (int32 Step = 0; Step < SAEVUSTests::StepsFor(MaxDashTime) && Movement->IsDashing(); ++Step)
		{
			TestWorld.Step();
		}
		return !Movement->IsDashing();
	}

	/** Presses dash and runs one step; true if a dash started. */
	bool DashNow(SAEVUSTests::FScopedTestWorld& TestWorld, USAEVUSMovementComponent* Movement)
	{
		Movement->RequestDash();
		TestWorld.Step();
		return Movement->IsDashing();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSDashGroundTest, "SAEVUS.Movement.Dash.Ground", SAEVUSTests::TestFlags)
bool FSAEVUSDashGroundTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSDashTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation);
	USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
	if (!TestWorld.SettleOnGround())
	{
		AddError(TEXT("Character never landed on the platform"));
		return false;
	}
	const FSAEVUSDashTuning& Tuning = Movement->GetDashTuning();
	const FVector Start = Character->GetActorLocation();

	// No input: the dash goes where the character faces (+X at spawn).
	if (!TestTrue(TEXT("Dash starts from standing"), DashNow(TestWorld, Movement)))
	{
		return false;
	}
	TestEqual(TEXT("Dash speed comes from tuning"), static_cast<float>(Movement->Velocity.Size2D()), Tuning.Speed, SpeedTolerance);
	TestTrue(TEXT("No input: dash follows facing"), Movement->Velocity.GetSafeNormal2D().X > DirectionMinDot);
	TestTrue(TEXT("Stays on the ground"), Movement->IsMovingOnGround());

	if (!TestTrue(TEXT("Dash ends"), StepUntilDashEnds(TestWorld, Movement)))
	{
		return false;
	}
	const float Distance = static_cast<float>(FVector::Dist2D(Start, Character->GetActorLocation()));
	const float ExpectedDistance = Tuning.Speed * Tuning.Duration;
	AddInfo(FString::Printf(TEXT("Dash distance %.1f cm, expected %.1f cm"), Distance, ExpectedDistance));
	TestEqual(TEXT("Distance ~ Speed x Duration"), Distance, ExpectedDistance, ExpectedDistance * DistanceTolerance);
	TestTrue(TEXT("Speed capped to ExitSpeed when the dash ends"), Movement->Velocity.Size2D() <= Tuning.ExitSpeed + SpeedTolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSDashDirectionTest, "SAEVUS.Movement.Dash.Direction", SAEVUSTests::TestFlags)
bool FSAEVUSDashDirectionTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSDashTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation);
	USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
	if (!TestWorld.SettleOnGround())
	{
		AddError(TEXT("Character never landed on the platform"));
		return false;
	}

	// Input to the side while facing +X: the dash follows the input, not the facing.
	Character->AddMovementInput(FVector::RightVector, 1.f);
	if (!TestTrue(TEXT("Dash starts"), DashNow(TestWorld, Movement)))
	{
		return false;
	}
	TestTrue(TEXT("Dash follows the input direction"), Movement->Velocity.GetSafeNormal2D().Y > DirectionMinDot);
	TestEqual(TEXT("Body snaps to the dash direction"), static_cast<float>(Character->GetActorRotation().Yaw), 90.f, YawToleranceDeg);

	// Steering mid-dash does nothing: the direction is locked for the whole dash.
	Character->AddMovementInput(FVector::ForwardVector, 1.f);
	TestWorld.Step();
	TestTrue(TEXT("Direction is locked during the dash"), Movement->Velocity.GetSafeNormal2D().Y > DirectionMinDot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSDashCooldownTest, "SAEVUS.Movement.Dash.Cooldown", SAEVUSTests::TestFlags)
bool FSAEVUSDashCooldownTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSDashTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation);
	USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
	if (!TestWorld.SettleOnGround())
	{
		AddError(TEXT("Character never landed on the platform"));
		return false;
	}

	TestTrue(TEXT("First dash starts"), DashNow(TestWorld, Movement));
	TestFalse(TEXT("No new dash while dashing"), Movement->CanDash());
	StepUntilDashEnds(TestWorld, Movement);
	TestFalse(TEXT("Dash refused right after the previous one (cooldown)"), DashNow(TestWorld, Movement));

	for (int32 Step = 0; Step < SAEVUSTests::StepsFor(Movement->GetDashTuning().Cooldown); ++Step)
	{
		TestWorld.Step();
	}
	TestTrue(TEXT("Dash allowed again after the cooldown"), DashNow(TestWorld, Movement));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSDashAirBudgetTest, "SAEVUS.Movement.Dash.AirBudget", SAEVUSTests::TestFlags)
bool FSAEVUSDashAirBudgetTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSDashTests;

	// No cooldown, so only the air budget can refuse the second air dash.
	USAEVUSMovementSettings* Settings = MakeSettings();
	Settings->Dash.Cooldown = 0.f;

	SAEVUSTests::FScopedTestWorld TestWorld;
	TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation, Settings);
	USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
	if (!TestWorld.SettleOnGround())
	{
		AddError(TEXT("Character never landed on the platform"));
		return false;
	}

	Character->RequestJump();
	for (int32 Step = 0; Step < AirborneSteps; ++Step)
	{
		TestWorld.Step();
	}
	if (!TestTrue(TEXT("Airborne after the jump"), Movement->IsFalling()))
	{
		return false;
	}

	const float HeightBefore = static_cast<float>(Character->GetActorLocation().Z);
	TestTrue(TEXT("Air dash starts"), DashNow(TestWorld, Movement));
	TestEqual(TEXT("Air dash budget spent"), Movement->GetAirDashesRemaining(), Settings->Dash.MaxAirDashes - 1);
	StepUntilDashEnds(TestWorld, Movement);
	const float HeightChange = FMath::Abs(static_cast<float>(Character->GetActorLocation().Z) - HeightBefore);
	AddInfo(FString::Printf(TEXT("Air dash height change %.2f cm"), HeightChange));
	TestTrue(TEXT("Gravity suspended during the air dash"), HeightChange < AirDashMaxHeightChange);

	TestFalse(TEXT("Second air dash refused (budget 1)"), DashNow(TestWorld, Movement));

	if (!TestTrue(TEXT("Lands again"), TestWorld.SettleOnGround()))
	{
		return false;
	}
	TestEqual(TEXT("Landing refills the air dashes"), Movement->GetAirDashesRemaining(), Settings->Dash.MaxAirDashes);
	TestTrue(TEXT("Ground dash works after landing"), DashNow(TestWorld, Movement));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSDashJumpCancelTest, "SAEVUS.Movement.Dash.JumpCancel", SAEVUSTests::TestFlags)
bool FSAEVUSDashJumpCancelTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSDashTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation);
	USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
	if (!TestWorld.SettleOnGround())
	{
		AddError(TEXT("Character never landed on the platform"));
		return false;
	}

	if (!TestTrue(TEXT("Dash starts"), DashNow(TestWorld, Movement)))
	{
		return false;
	}
	Character->RequestJump();
	TestWorld.Step();
	TestFalse(TEXT("Jump cancels the dash"), Movement->IsDashing());
	TestTrue(TEXT("The jump happened"), Movement->IsFalling() && Movement->Velocity.Z > 0.f);
	TestTrue(TEXT("Jump keeps the dash exit speed, not the dash speed"),
		Movement->Velocity.Size2D() <= Movement->GetDashTuning().ExitSpeed + SpeedTolerance);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
