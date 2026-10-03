// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movement/SAEVUSMovementSettings.h"

namespace SAEVUSWallJumpTests
{
	// 40 m x 40 m floor, top surface at Z = 0.
	const FVector FloorCenter(0.f, 0.f, -50.f);
	const FVector FloorSize(4000.f, 4000.f, 100.f);

	// 6 m tall wall whose face is the plane X = 200, facing -X.
	const FVector WallCenter(250.f, 0.f, 300.f);
	const FVector WallSize(100.f, 2000.f, 600.f);
	const FVector WallNormal(-1.f, 0.f, 0.f);

	// Capsule radius 42: 10 cm from the wall face, inside the default detection distance.
	const FVector NearWallLocation(148.f, 0.f, 300.f);
	const FVector FarFromWallLocation(-1000.f, 0.f, 300.f);

	constexpr float SpeedTolerance = 1.f;
	constexpr float YawToleranceDeg = 1.f;
	// One step of gravity is applied after the launch.
	constexpr float UpSpeedMinFraction = 0.9f;
	// Speed along the wall before the jump, for the tangential test.
	constexpr float AlongWallSpeed = 400.f;

	USAEVUSMovementSettings* MakeSettings()
	{
		return NewObject<USAEVUSMovementSettings>(GetTransientPackage());
	}

	/** World with the floor and the wall, and a character falling at Location. */
	ASAEVUSCharacter* SetUp(SAEVUSTests::FScopedTestWorld& TestWorld, const FVector& Location, const USAEVUSMovementSettings* Settings = nullptr)
	{
		TestWorld.SpawnBlock(FloorCenter, FloorSize);
		TestWorld.SpawnBlock(WallCenter, WallSize);
		return TestWorld.SpawnCharacter(Location, Settings);
	}

	/** Presses jump in the air and runs one step; true if the character was launched upward. */
	bool JumpInAir(SAEVUSTests::FScopedTestWorld& TestWorld, ASAEVUSCharacter* Character)
	{
		Character->RequestJump();
		TestWorld.Step();
		return Character->GetSAEVUSMovement()->Velocity.Z > 0.f;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallJumpBasicTest, "SAEVUS.Movement.WallJump.Basic", SAEVUSTests::TestFlags)
bool FSAEVUSWallJumpBasicTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallJumpTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, NearWallLocation);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
	const FSAEVUSWallJumpTuning& Tuning = Movement->GetWallJumpTuning();

	TestTrue(TEXT("Wall found next to the character"), Movement->FindWallJumpNormal().Equals(WallNormal, 0.01f));
	if (!TestTrue(TEXT("Jump in the air next to a wall launches"), JumpInAir(TestWorld, Character)))
	{
		return false;
	}
	TestEqual(TEXT("Pushed away from the wall at AwaySpeed"), static_cast<float>(FVector::DotProduct(Movement->Velocity, WallNormal)), Tuning.AwaySpeed, SpeedTolerance);
	TestTrue(TEXT("Vertical speed from UpSpeed"),
		Movement->Velocity.Z > Tuning.UpSpeed * UpSpeedMinFraction && Movement->Velocity.Z <= Tuning.UpSpeed);
	TestEqual(TEXT("Body faces away from the wall"), FMath::Abs(static_cast<float>(Character->GetActorRotation().Yaw)), 180.f, YawToleranceDeg);
	TestEqual(TEXT("One wall jump spent"), Movement->GetWallJumpsRemaining(), Tuning.MaxWallJumps - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallJumpNoWallTest, "SAEVUS.Movement.WallJump.NoWall", SAEVUSTests::TestFlags)
bool FSAEVUSWallJumpNoWallTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallJumpTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, FarFromWallLocation);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	TestTrue(TEXT("No wall in range"), Movement->FindWallJumpNormal().IsNearlyZero());
	TestFalse(TEXT("Jump in open air does nothing"), JumpInAir(TestWorld, Character));
	TestEqual(TEXT("No wall jump spent"), Movement->GetWallJumpsRemaining(), Movement->GetWallJumpTuning().MaxWallJumps);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallJumpNotOffLedgeTest, "SAEVUS.Movement.WallJump.NotOffLedge", SAEVUSTests::TestFlags)
bool FSAEVUSWallJumpNotOffLedgeTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallJumpTests;

	// 10 m platform, top at Z = 0, +X edge at X = 500, nothing below. Coyote time off, so only
	// a wall jump could launch the character after it walks off.
	const FVector PlatformCenter(0.f, 0.f, -50.f);
	const FVector PlatformSize(1000.f, 1000.f, 100.f);
	const FVector StandLocation(300.f, 0.f, 150.f);
	constexpr float MaxWalkOffTime = 2.f;

	USAEVUSMovementSettings* Settings = MakeSettings();
	Settings->Air.CoyoteTime = 0.f;

	SAEVUSTests::FScopedTestWorld TestWorld;
	TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation, Settings);
	const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
	if (!TestWorld.SettleOnGround())
	{
		AddError(TEXT("Character never landed on the platform"));
		return false;
	}
	for (int32 Step = 0; Step < SAEVUSTests::StepsFor(MaxWalkOffTime) && !Movement->IsFalling(); ++Step)
	{
		Character->AddMovementInput(FVector::ForwardVector, 1.f);
		TestWorld.Step();
	}
	if (!TestTrue(TEXT("Walked off the ledge"), Movement->IsFalling()))
	{
		return false;
	}
	TestTrue(TEXT("The ledge just walked off is not a wall"), Movement->FindWallJumpNormal().IsNearlyZero());
	TestFalse(TEXT("No wall jump off the ledge"), JumpInAir(TestWorld, Character));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallJumpTangentialTest, "SAEVUS.Movement.WallJump.Tangential", SAEVUSTests::TestFlags)
bool FSAEVUSWallJumpTangentialTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallJumpTests;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, NearWallLocation);
	USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	Movement->Velocity = FVector(0.f, AlongWallSpeed, 0.f);
	if (!TestTrue(TEXT("Wall jump"), JumpInAir(TestWorld, Character)))
	{
		return false;
	}
	TestEqual(TEXT("Keeps TangentialSpeedKept of the speed along the wall"), static_cast<float>(Movement->Velocity.Y),
		AlongWallSpeed * Movement->GetWallJumpTuning().TangentialSpeedKept, SpeedTolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallJumpBudgetTest, "SAEVUS.Movement.WallJump.Budget", SAEVUSTests::TestFlags)
bool FSAEVUSWallJumpBudgetTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallJumpTests;

	USAEVUSMovementSettings* Settings = MakeSettings();
	Settings->WallJump.MaxWallJumps = 1;

	SAEVUSTests::FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = SetUp(TestWorld, NearWallLocation, Settings);
	USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	TestTrue(TEXT("First wall jump"), JumpInAir(TestWorld, Character));

	// Put the character back against the wall, still airborne.
	Character->SetActorLocation(NearWallLocation);
	Movement->Velocity = FVector::ZeroVector;
	TestFalse(TEXT("Second wall jump refused (budget 1)"), JumpInAir(TestWorld, Character));

	if (!TestTrue(TEXT("Lands"), TestWorld.SettleOnGround()))
	{
		return false;
	}
	TestEqual(TEXT("Landing refills the wall jumps"), Movement->GetWallJumpsRemaining(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSWallJumpRefillsAirDashTest, "SAEVUS.Movement.WallJump.RefillsAirDash", SAEVUSTests::TestFlags)
bool FSAEVUSWallJumpRefillsAirDashTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSWallJumpTests;

	for (const bool bRefill : { true, false })
	{
		USAEVUSMovementSettings* Settings = MakeSettings();
		Settings->WallJump.bRefillsAirDashes = bRefill;

		SAEVUSTests::FScopedTestWorld TestWorld;
		ASAEVUSCharacter* Character = SetUp(TestWorld, NearWallLocation, Settings);
		USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

		Movement->RequestDash();
		TestWorld.Step();
		TestEqual(TEXT("Air dash spent"), Movement->GetAirDashesRemaining(), 0);

		TestTrue(TEXT("Wall jump during the air dash"), JumpInAir(TestWorld, Character));
		TestFalse(TEXT("Wall jump ends the dash"), Movement->IsDashing());
		TestEqual(bRefill ? TEXT("Refill on: air dashes back") : TEXT("Refill off: air dashes still spent"),
			Movement->GetAirDashesRemaining(), bRefill ? Settings->Dash.MaxAirDashes : 0);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
