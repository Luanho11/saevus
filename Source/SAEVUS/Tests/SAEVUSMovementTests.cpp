// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movement/SAEVUSMovementSettings.h"

namespace SAEVUSMovementTests
{
	// High above an empty world, so the character is airborne with nothing to land on.
	const FVector AirborneLocation(0.f, 0.f, 10000.f);

	// Integration error allowed against the closed-form expectation.
	constexpr float JumpApexTolerance = 0.02f;
	constexpr float AirControlTolerance = 0.1f;
	constexpr float AirControlDuration = 0.5f;
	constexpr float MaxJumpSimTime = 3.f;
}

using namespace SAEVUSTests;
using namespace SAEVUSMovementTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSGaitPriorityTest, "SAEVUS.Movement.Gait.Priority", SAEVUSTests::TestFlags)
bool FSAEVUSGaitPriorityTest::RunTest(const FString& Parameters)
{
	USAEVUSMovementComponent* Movement = NewObject<USAEVUSMovementComponent>(GetTransientPackage());

	TestEqual(TEXT("No request runs"), Movement->GetCurrentGait(), ESAEVUSGait::Run);

	Movement->SetWalkRequested(true);
	TestEqual(TEXT("Walk request walks"), Movement->GetCurrentGait(), ESAEVUSGait::Walk);

	Movement->SetSprintRequested(true);
	TestEqual(TEXT("Sprint wins over walk"), Movement->GetCurrentGait(), ESAEVUSGait::Sprint);

	Movement->SetWalkRequested(false);
	Movement->SetSprintRequested(false);
	TestEqual(TEXT("Releasing both runs"), Movement->GetCurrentGait(), ESAEVUSGait::Run);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSGaitSpeedsTest, "SAEVUS.Movement.Gait.Speeds", SAEVUSTests::TestFlags)
bool FSAEVUSGaitSpeedsTest::RunTest(const FString& Parameters)
{
	USAEVUSMovementComponent* Movement = NewObject<USAEVUSMovementComponent>(GetTransientPackage());
	const FSAEVUSGroundMovementTuning& Tuning = Movement->GetGroundTuning();

	TestEqual(TEXT("Walk speed"), Movement->GetGaitSpeed(ESAEVUSGait::Walk), Tuning.WalkSpeed);
	TestEqual(TEXT("Run speed"), Movement->GetGaitSpeed(ESAEVUSGait::Run), Tuning.RunSpeed);
	TestEqual(TEXT("Sprint speed"), Movement->GetGaitSpeed(ESAEVUSGait::Sprint), Tuning.SprintSpeed);
	TestTrue(TEXT("Default gaits are ordered walk < run < sprint"), Tuning.WalkSpeed < Tuning.RunSpeed && Tuning.RunSpeed < Tuning.SprintSpeed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSSettingsApplyTest, "SAEVUS.Movement.Settings.Apply", SAEVUSTests::TestFlags)
bool FSAEVUSSettingsApplyTest::RunTest(const FString& Parameters)
{
	USAEVUSMovementSettings* Settings = NewObject<USAEVUSMovementSettings>(GetTransientPackage());
	Settings->Ground.RunSpeed = Settings->Ground.RunSpeed + 1.f;
	Settings->Ground.MaxAcceleration = Settings->Ground.MaxAcceleration + 1.f;
	Settings->Air.JumpZVelocity = Settings->Air.JumpZVelocity + 1.f;
	Settings->Air.GravityScale = Settings->Air.GravityScale + 1.f;
	Settings->Air.AirControl = Settings->Air.AirControl * 0.5f;

	USAEVUSMovementComponent* Movement = NewObject<USAEVUSMovementComponent>(GetTransientPackage());
	Movement->SetMovementSettings(Settings);

	TestEqual(TEXT("MaxWalkSpeed mirrors run speed"), Movement->MaxWalkSpeed, Settings->Ground.RunSpeed);
	TestEqual(TEXT("MaxAcceleration"), Movement->MaxAcceleration, Settings->Ground.MaxAcceleration);
	TestEqual(TEXT("JumpZVelocity"), Movement->JumpZVelocity, Settings->Air.JumpZVelocity);
	TestEqual(TEXT("GravityScale"), Movement->GravityScale, Settings->Air.GravityScale);
	TestEqual(TEXT("AirControl"), Movement->AirControl, Settings->Air.AirControl);

	Movement->SetMovementSettings(nullptr);
	TestEqual(TEXT("Null settings restore the code defaults"), Movement->JumpZVelocity, FSAEVUSAirMovementTuning().JumpZVelocity);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSJumpApexTest, "SAEVUS.Movement.Air.JumpApex", SAEVUSTests::TestFlags)
bool FSAEVUSJumpApexTest::RunTest(const FString& Parameters)
{
	FScopedTestWorld TestWorld;
	ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(AirborneLocation);
	USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

	// Same launch the character applies on jump (fixed-height jump: no hold time).
	Movement->Velocity.Z = Movement->JumpZVelocity;
	const float StartZ = Character->GetActorLocation().Z;
	float ApexZ = StartZ;

	for (int32 Step = 0; Step < StepsFor(MaxJumpSimTime) && Movement->Velocity.Z >= 0.f; ++Step)
	{
		TestWorld.Step();
		ApexZ = FMath::Max(ApexZ, Character->GetActorLocation().Z);
	}

	TestTrue(TEXT("Character is falling"), Movement->IsFalling());
	const float Gravity = FMath::Abs(Movement->GetGravityZ());
	const float ExpectedHeight = FMath::Square(Movement->JumpZVelocity) / (2.f * Gravity);
	const float Height = ApexZ - StartZ;
	AddInfo(FString::Printf(TEXT("Jump apex %.1f cm, expected %.1f cm"), Height, ExpectedHeight));
	TestNearlyEqual(TEXT("Jump apex height matches v^2 / 2g"), Height, ExpectedHeight, ExpectedHeight * JumpApexTolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSAirControlTest, "SAEVUS.Movement.Air.AirControl", SAEVUSTests::TestFlags)
bool FSAEVUSAirControlTest::RunTest(const FString& Parameters)
{
	auto MeasureAirSpeed = [this](const USAEVUSMovementSettings* Settings, float& OutExpected) -> float
	{
		FScopedTestWorld TestWorld;
		ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(AirborneLocation, Settings);
		USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();

		const int32 Steps = StepsFor(AirControlDuration);
		for (int32 Step = 0; Step < Steps; ++Step)
		{
			Character->AddMovementInput(FVector::ForwardVector, 1.f);
			TestWorld.Step();
		}

		const FSAEVUSAirMovementTuning& Air = Movement->GetAirTuning();
		const float Duration = Steps * SimStep;
		OutExpected = FMath::Min(Movement->GetGroundTuning().MaxAcceleration * Air.AirControl * Duration, Movement->GetMaxSpeed());
		return Movement->Velocity.Size2D();
	};

	float Expected = 0.f;
	const float Speed = MeasureAirSpeed(nullptr, Expected);
	AddInfo(FString::Printf(TEXT("Air speed after %.2fs of input: %.1f cm/s, expected %.1f cm/s"), AirControlDuration, Speed, Expected));
	TestNearlyEqual(TEXT("Air control accelerates at MaxAcceleration * AirControl"), Speed, Expected, Expected * AirControlTolerance);

	USAEVUSMovementSettings* NoAirControl = NewObject<USAEVUSMovementSettings>(GetTransientPackage());
	NoAirControl->Air.AirControl = 0.f;
	const float LockedSpeed = MeasureAirSpeed(NoAirControl, Expected);
	TestNearlyEqual(TEXT("Zero air control keeps the take-off velocity"), LockedSpeed, 0.f, KINDA_SMALL_NUMBER);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
