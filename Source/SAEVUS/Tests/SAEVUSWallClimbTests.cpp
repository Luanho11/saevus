// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movement/SAEVUSMovementSettings.h"

namespace SAEVUSWallClimbTests
{
	// 40 m x 40 m floor, top surface at Z = 0.
	const FVector FloorCenter(0.f, 0.f, -50.f);
	const FVector FloorSize(4000.f, 4000.f, 100.f);

	// Both walls have their face on the plane X = 200, facing -X.
	const FVector WallNormal(-1.f, 0.f, 0.f);
	// 10 m tall: never topped by one climb.
	const FVector TallWallCenter(250.f, 0.f, 500.f);
	const FVector TallWallSize(100.f, 2000.f, 1000.f);
	// 3.5 m tall, 4 m deep: a climb reaches its top.
	const FVector LedgeWallCenter(400.f, 0.f, 175.f);
	const FVector LedgeWallSize(400.f, 2000.f, 350.f);
	constexpr float LedgeTopZ = 350.f;
	constexpr float WallFaceX = 200.f;

	// 58 cm from the wall face, in the air, running at it.
	const FVector Start(100.f, 0.f, 150.f);
	// 20 cm from the wall face: inside the detection distance from the first step.
	const FVector NearStart(138.f, 0.f, 150.f);
	constexpr float ApproachSpeed = 500.f;

	constexpr float MaxApproachTime = 0.5f;
	constexpr float MaxClimbTime = 2.f;
	constexpr float MaxLedgeTime = 1.5f;
	constexpr float NoClimbCheckTime = 0.3f;
	constexpr int32 StepsBeforeJumpOff = 5;
	constexpr float SpeedTolerance = 1.f;
	constexpr float YawToleranceDeg = 1.f;
	constexpr float HeightTolerance = 0.1f;

	struct FClimbWorld
	{
		SAEVUSTests::FScopedTestWorld TestWorld;
		ASAEVUSCharacter* Character = nullptr;
		USAEVUSMovementComponent* Movement = nullptr;

		FClimbWorld(const FVector& WallCenter, const FVector& WallSize, const FVector& Velocity = FVector(ApproachSpeed, 0.f, 0.f),
			const FVector& Location = Start)
		{
			TestWorld.SpawnBlock(FloorCenter, FloorSize);
			TestWorld.SpawnBlock(WallCenter, WallSize);
			Character = TestWorld.SpawnCharacter(Location);
			Movement = Character->GetSAEVUSMovement();
			Movement->Velocity = Velocity;
		}

		void Step(const FVector& Input = FVector::ForwardVector)
		{
			if (!Input.IsZero())
			{
				Character->AddMovementInput(Input, 1.f);
			}
			TestWorld.Step();
		}

		/** Runs at the wall until a climb starts; false if none within MaxApproachTime. */
		bool ApproachUntilClimbing(const FVector& Input = FVector::ForwardVector)
		{
			for (int32 Index = 0; Index < SAEVUSTests::StepsFor(MaxApproachTime) && !Movement->IsWallClimbing(); ++Index)
			{
				Step(Input);
			}
			return Movement->IsWallClimbing();
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallClimbEnterTest, "SAEVUS.Movement.WallClimb.Enter", SAEVUSTests::TestFlags)
bool FSAEVUSWallClimbEnterTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallClimbTests;

	FClimbWorld World(TallWallCenter, TallWallSize);
	if (!TestTrue(TEXT("Jumping at a wall head-on with input into it starts a climb"), World.ApproachUntilClimbing()))
	{
		return false;
	}
	TestEqual(TEXT("Climbs at the climb speed"), static_cast<float>(World.Movement->Velocity.Z), World.Movement->GetWallClimbTuning().Speed, SpeedTolerance);
	TestEqual(TEXT("Faces the wall"), static_cast<float>(World.Character->GetActorRotation().Yaw), 0.f, YawToleranceDeg);
	TestEqual(TEXT("One climb spent"), World.Movement->GetWallClimbsRemaining(), World.Movement->GetWallClimbTuning().MaxClimbs - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallClimbEntryConditionsTest, "SAEVUS.Movement.WallClimb.EntryConditions", SAEVUSTests::TestFlags)
bool FSAEVUSWallClimbEntryConditionsTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallClimbTests;

	struct FCase
	{
		const TCHAR* Description;
		FVector Velocity;
		FVector Input;
	};
	// 60 degrees off head-on: a glancing approach.
	const FVector Glancing = FRotator(0.f, 60.f, 0.f).Vector();
	const FCase Refused[] = {
		{ TEXT("No input: no climb"), FVector(ApproachSpeed, 0.f, 0.f), FVector::ZeroVector },
		{ TEXT("Input along the wall: no climb"), FVector(ApproachSpeed, 0.f, 0.f), FVector::RightVector },
		{ TEXT("Glancing approach: no climb"), Glancing * ApproachSpeed, Glancing },
		{ TEXT("Too slow toward the wall: no climb"), FVector(100.f, 0.f, 0.f), FVector::ForwardVector },
	};
	// One step from within detection range: later steps would let air control change the approach.
	for (const FCase& Case : Refused)
	{
		FClimbWorld World(TallWallCenter, TallWallSize, Case.Velocity, NearStart);
		World.Step(Case.Input);
		TestFalse(Case.Description, World.Movement->IsWallClimbing());
	}
	FClimbWorld Accepted(TallWallCenter, TallWallSize, FVector(ApproachSpeed, 0.f, 0.f), NearStart);
	Accepted.Step();
	TestTrue(TEXT("Same setup, head-on with input: climbs"), Accepted.Movement->IsWallClimbing());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallClimbFlipTest, "SAEVUS.Movement.WallClimb.Flip", SAEVUSTests::TestFlags)
bool FSAEVUSWallClimbFlipTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallClimbTests;

	FClimbWorld World(TallWallCenter, TallWallSize);
	if (!TestTrue(TEXT("Climb starts"), World.ApproachUntilClimbing()))
	{
		return false;
	}
	const FSAEVUSWallClimbTuning& Tuning = World.Movement->GetWallClimbTuning();
	const float StartZ = static_cast<float>(World.Character->GetActorLocation().Z);
	// No input once climbing: the climb doesn't need it, and steering would shape the flip.
	for (int32 Index = 0; Index < SAEVUSTests::StepsFor(MaxClimbTime) && World.Movement->IsWallClimbing(); ++Index)
	{
		World.Step(FVector::ZeroVector);
	}
	const float Height = static_cast<float>(World.Character->GetActorLocation().Z) - StartZ;
	const float ExpectedHeight = Tuning.Speed * Tuning.MaxDuration;
	AddInfo(FString::Printf(TEXT("Climb height %.1f cm, expected %.1f cm"), Height, ExpectedHeight));
	TestEqual(TEXT("Climbs about Speed x MaxDuration"), Height, ExpectedHeight, ExpectedHeight * HeightTolerance);
	TestTrue(TEXT("Falls after the climb"), World.Movement->IsFalling());
	TestEqual(TEXT("Flips away from the wall"), static_cast<float>(FVector::DotProduct(World.Movement->Velocity, WallNormal)), Tuning.FlipAwaySpeed, SpeedTolerance);
	TestEqual(TEXT("Faces away from the wall after the flip"), FMath::Abs(static_cast<float>(World.Character->GetActorRotation().Yaw)), 180.f, YawToleranceDeg);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallClimbLedgeTest, "SAEVUS.Movement.WallClimb.Ledge", SAEVUSTests::TestFlags)
bool FSAEVUSWallClimbLedgeTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallClimbTests;

	FClimbWorld World(LedgeWallCenter, LedgeWallSize);
	if (!TestTrue(TEXT("Climb starts"), World.ApproachUntilClimbing()))
	{
		return false;
	}
	for (int32 Index = 0; Index < SAEVUSTests::StepsFor(MaxLedgeTime) && !World.Movement->IsMovingOnGround(); ++Index)
	{
		World.Step();
	}
	const FVector Location = World.Character->GetActorLocation();
	AddInfo(FString::Printf(TEXT("After the climb: %s, mode %s"), *Location.ToCompactString(), *World.Movement->GetMovementName()));
	TestTrue(TEXT("Lands after topping the wall"), World.Movement->IsMovingOnGround());
	TestTrue(TEXT("Stands on top of the wall"), Location.X > WallFaceX && Location.Z > LedgeTopZ);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallClimbOncePerAirTest, "SAEVUS.Movement.WallClimb.OncePerAir", SAEVUSTests::TestFlags)
bool FSAEVUSWallClimbOncePerAirTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallClimbTests;

	FClimbWorld World(TallWallCenter, TallWallSize);
	if (!TestTrue(TEXT("First climb"), World.ApproachUntilClimbing()))
	{
		return false;
	}
	for (int32 Index = 0; Index < SAEVUSTests::StepsFor(MaxClimbTime) && World.Movement->IsWallClimbing(); ++Index)
	{
		World.Step();
	}

	// Back in front of the wall, still airborne, running at it again.
	World.Character->SetActorLocation(Start);
	World.Movement->Velocity = FVector(ApproachSpeed, 0.f, 0.f);
	bool bClimbed = false;
	for (int32 Index = 0; Index < SAEVUSTests::StepsFor(NoClimbCheckTime); ++Index)
	{
		World.Step();
		bClimbed |= World.Movement->IsWallClimbing();
	}
	TestFalse(TEXT("No second climb in the same airborne period"), bClimbed);

	World.Character->SetActorLocation(FVector(-500.f, 0.f, 150.f));
	if (!TestTrue(TEXT("Lands"), World.TestWorld.SettleOnGround()))
	{
		return false;
	}
	TestEqual(TEXT("Landing refills the climbs"), World.Movement->GetWallClimbsRemaining(), World.Movement->GetWallClimbTuning().MaxClimbs);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallClimbJumpOffTest, "SAEVUS.Movement.WallClimb.JumpOff", SAEVUSTests::TestFlags)
bool FSAEVUSWallClimbJumpOffTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallClimbTests;

	FClimbWorld World(TallWallCenter, TallWallSize);
	if (!TestTrue(TEXT("Climb starts"), World.ApproachUntilClimbing()))
	{
		return false;
	}
	for (int32 Index = 0; Index < StepsBeforeJumpOff; ++Index)
	{
		World.Step();
	}
	World.Character->RequestJump();
	World.Step(FVector::ZeroVector);

	const FSAEVUSWallJumpTuning& WallJump = World.Movement->GetWallJumpTuning();
	TestTrue(TEXT("Jump leaves the wall"), World.Movement->IsFalling());
	TestEqual(TEXT("Pushed off at the wall-jump AwaySpeed"), static_cast<float>(FVector::DotProduct(World.Movement->Velocity, WallNormal)), WallJump.AwaySpeed, SpeedTolerance);
	TestTrue(TEXT("Goes up"), World.Movement->Velocity.Z > 0.f);
	TestEqual(TEXT("Counts as a wall jump"), World.Movement->GetWallJumpsRemaining(), WallJump.MaxWallJumps - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallClimbLetGoTest, "SAEVUS.Movement.WallClimb.LetGo", SAEVUSTests::TestFlags)
bool FSAEVUSWallClimbLetGoTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallClimbTests;

	FClimbWorld World(TallWallCenter, TallWallSize);
	if (!TestTrue(TEXT("Climb starts"), World.ApproachUntilClimbing()))
	{
		return false;
	}
	World.Step(WallNormal);
	TestTrue(TEXT("Steering away from the wall lets go"), World.Movement->IsFalling());
	TestTrue(TEXT("Drops instead of rising"), World.Movement->Velocity.Z <= 0.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
