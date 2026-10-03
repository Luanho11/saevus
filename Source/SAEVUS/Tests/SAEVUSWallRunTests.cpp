// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movement/SAEVUSMovementSettings.h"

namespace SAEVUSWallRunTests
{
	// 40 m x 40 m floor, top surface at Z = 0.
	const FVector FloorCenter(0.f, 0.f, -50.f);
	const FVector FloorSize(4000.f, 4000.f, 100.f);

	// 40 m long, 10 m tall wall whose face is the plane Y = 200, facing -Y. It ends at X = 2000.
	const FVector WallCenter(0.f, 250.f, 500.f);
	const FVector WallSize(4000.f, 100.f, 1000.f);
	const FVector WallNormal(0.f, -1.f, 0.f);
	constexpr float WallEndX = 2000.f;

	// Capsule radius 42: 10 cm from the wall face.
	constexpr float NearWallY = 148.f;
	// High enough that a full-length wall run never reaches the floor.
	const FVector HighStart(-1000.f, NearWallY, 800.f);
	// Feet about 45 cm above the floor.
	const FVector LowStart(-1000.f, NearWallY, 140.f);
	// 2 m before the end of the wall.
	const FVector NearWallEndStart(WallEndX - 200.f, NearWallY, 600.f);

	// Horizontal speed along the wall when the character reaches it.
	constexpr float ApproachSpeed = 600.f;

	constexpr float SpeedTolerance = 1.f;
	constexpr float YawToleranceDeg = 1.f;
	constexpr int32 DurationToleranceSteps = 2;
	constexpr float NoReattachCheckTime = 0.3f;
	constexpr float MaxLandingTime = 1.f;
	constexpr float WallEndCheckTime = 0.6f;
	constexpr int32 StepsBeforeJumpOff = 5;

	USAEVUSMovementSettings* MakeSettings()
	{
		return NewObject<USAEVUSMovementSettings>(GetTransientPackage());
	}

	/** Floor, wall, and a character falling at Location while moving along the wall. */
	ASAEVUSCharacter* SetUp(SAEVUSTests::FScopedTestWorld& TestWorld, const FVector& Location, const FVector& Velocity = FVector(ApproachSpeed, 0.f, 0.f))
	{
		TestWorld.SpawnBlock(FloorCenter, FloorSize);
		TestWorld.SpawnBlock(WallCenter, WallSize);
		ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(Location);
		Character->GetSAEVUSMovement()->Velocity = Velocity;
		return Character;
	}

	/** One step with movement input along Direction. */
	void StepWithInput(SAEVUSTests::FScopedTestWorld& TestWorld, ASAEVUSCharacter* Character, const FVector& Direction = FVector::ForwardVector)
	{
		Character->AddMovementInput(Direction, 1.f);
		TestWorld.Step();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallRunEnterTest, "SAEVUS.Movement.WallRun.Enter", SAEVUSTests::TestFlags)
bool FSAEVUSWallRunEnterTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallRunTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, HighStart);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	StepWithInput(TestWorld, Character);
	if (!TestTrue(TEXT("Falling along a wall with input starts a wall run"), Movement->IsWallRunning()))
	{
		return false;
	}
	TestEqual(TEXT("Runs along the wall at the wall-run speed"), static_cast<float>(Movement->Velocity.X), Movement->GetWallRunTuning().Speed, SpeedTolerance);
	TestEqual(TEXT("Body faces along the wall"), static_cast<float>(Character->GetActorRotation().Yaw), 0.f, YawToleranceDeg);
	TestTrue(TEXT("Stays next to the wall"), FMath::Abs(Character->GetActorLocation().Y - NearWallY) < Movement->GetWallRunTuning().DetectionDistance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallRunEntryConditionsTest, "SAEVUS.Movement.WallRun.EntryConditions", SAEVUSTests::TestFlags)
bool FSAEVUSWallRunEntryConditionsTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallRunTests;

	// Each case: initial velocity, input direction (zero = no input), expected result.
	struct FCase
	{
		const TCHAR* Description;
		FVector Velocity;
		FVector Input;
	};
	const FCase Refused[] = {
		{ TEXT("No input: no wall run"), FVector(ApproachSpeed, 0.f, 0.f), FVector::ZeroVector },
		{ TEXT("Too slow along the wall: no wall run"), FVector(100.f, 0.f, 0.f), FVector::ForwardVector },
		{ TEXT("Input away from the wall: no wall run"), FVector(ApproachSpeed, 0.f, 0.f), WallNormal },
		{ TEXT("Falling too fast: no wall run"), FVector(ApproachSpeed, 0.f, -1000.f), FVector::ForwardVector },
	};
	for (const FCase& Case : Refused)
	{
		SAEVUSTests::FScopedTestWorld TestWorld;
		ASAEVUSCharacter* Character = SetUp(TestWorld, HighStart, Case.Velocity);
		if (!Case.Input.IsZero())
		{
			Character->AddMovementInput(Case.Input, 1.f);
		}
		TestWorld.Step();
		TestFalse(Case.Description, Character->GetSAEVUSMovement()->IsWallRunning());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallRunDurationTest, "SAEVUS.Movement.WallRun.Duration", SAEVUSTests::TestFlags)
bool FSAEVUSWallRunDurationTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallRunTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, HighStart);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
	const float MaxDuration = Movement->GetWallRunTuning().MaxDuration;

	int32 WallRunSteps = 0;
	for (int32 Step = 0; Step < SAEVUSTests::StepsFor(MaxDuration * 2.f); ++Step)
	{
		StepWithInput(TestWorld, Character);
		if (!Movement->IsWallRunning())
		{
			break;
		}
		++WallRunSteps;
	}
	AddInfo(FString::Printf(TEXT("Wall run lasted %d steps, expected %d"), WallRunSteps, SAEVUSTests::StepsFor(MaxDuration)));
	TestTrue(TEXT("Wall run lasts MaxDuration"), FMath::Abs(WallRunSteps - SAEVUSTests::StepsFor(MaxDuration)) <= DurationToleranceSteps);
	TestTrue(TEXT("Falls when the time is up"), Movement->IsFalling());

	// Still next to the same wall and still steering along it: no second run on it.
	bool bReattached = false;
	for (int32 Step = 0; Step < SAEVUSTests::StepsFor(NoReattachCheckTime); ++Step)
	{
		StepWithInput(TestWorld, Character);
		bReattached |= Movement->IsWallRunning();
	}
	TestFalse(TEXT("The same wall can't be run again before landing"), bReattached);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallRunLandingTest, "SAEVUS.Movement.WallRun.Landing", SAEVUSTests::TestFlags)
bool FSAEVUSWallRunLandingTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallRunTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, LowStart);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	StepWithInput(TestWorld, Character);
	if (!TestTrue(TEXT("Wall run starts low on the wall"), Movement->IsWallRunning()))
	{
		return false;
	}
	for (int32 Step = 0; Step < SAEVUSTests::StepsFor(MaxLandingTime) && !Movement->IsMovingOnGround(); ++Step)
	{
		StepWithInput(TestWorld, Character);
	}
	TestTrue(TEXT("Sliding down onto the floor lands"), Movement->IsMovingOnGround());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallRunWallEndsTest, "SAEVUS.Movement.WallRun.WallEnds", SAEVUSTests::TestFlags)
bool FSAEVUSWallRunWallEndsTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallRunTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, NearWallEndStart);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	StepWithInput(TestWorld, Character);
	if (!TestTrue(TEXT("Wall run starts"), Movement->IsWallRunning()))
	{
		return false;
	}
	for (int32 Step = 0; Step < SAEVUSTests::StepsFor(WallEndCheckTime); ++Step)
	{
		StepWithInput(TestWorld, Character);
	}
	TestTrue(TEXT("Past the end of the wall"), Character->GetActorLocation().X > WallEndX);
	TestTrue(TEXT("Running off the end of the wall becomes a fall"), Movement->IsFalling());
	TestTrue(TEXT("The run's speed carries into the fall"), Movement->Velocity.X > Movement->GetGaitSpeed(ESAEVUSGait::Run));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallRunJumpOffTest, "SAEVUS.Movement.WallRun.JumpOff", SAEVUSTests::TestFlags)
bool FSAEVUSWallRunJumpOffTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallRunTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, HighStart);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	for (int32 Step = 0; Step < StepsBeforeJumpOff; ++Step)
	{
		StepWithInput(TestWorld, Character);
	}
	if (!TestTrue(TEXT("Wall running"), Movement->IsWallRunning()))
	{
		return false;
	}
	Character->RequestJump();
	TestWorld.Step();

	const FSAEVUSWallJumpTuning& WallJump = Movement->GetWallJumpTuning();
	TestTrue(TEXT("Jump leaves the wall"), Movement->IsFalling());
	TestEqual(TEXT("Pushed off the wall at the wall-jump AwaySpeed"), static_cast<float>(FVector::DotProduct(Movement->Velocity, WallNormal)), WallJump.AwaySpeed, SpeedTolerance);
	TestEqual(TEXT("Keeps part of the run's speed along the wall"), static_cast<float>(Movement->Velocity.X),
		Movement->GetWallRunTuning().Speed * WallJump.TangentialSpeedKept, SpeedTolerance);
	TestTrue(TEXT("Goes up"), Movement->Velocity.Z > 0.f);
	TestEqual(TEXT("Counts as a wall jump"), Movement->GetWallJumpsRemaining(), WallJump.MaxWallJumps - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallRunDetachTest, "SAEVUS.Movement.WallRun.Detach", SAEVUSTests::TestFlags)
bool FSAEVUSWallRunDetachTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallRunTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, HighStart);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	StepWithInput(TestWorld, Character);
	if (!TestTrue(TEXT("Wall running"), Movement->IsWallRunning()))
	{
		return false;
	}
	StepWithInput(TestWorld, Character, WallNormal);
	TestTrue(TEXT("Steering away from the wall drops off it"), Movement->IsFalling());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
