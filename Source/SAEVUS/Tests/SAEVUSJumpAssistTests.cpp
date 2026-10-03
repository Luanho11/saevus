// Copyright SAEVUS. All Rights Reserved.

#include "Tests/SAEVUSMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/CapsuleComponent.h"
#include "Movement/SAEVUSMovementSettings.h"

namespace SAEVUSJumpAssistTests
{
	// 10 m x 10 m platform: top surface at Z = 0, +X edge at X = 500, empty space beyond.
	const FVector PlatformCenter(0.f, 0.f, -50.f);
	const FVector PlatformSize(1000.f, 1000.f, 100.f);

	// Dropped a little above the platform so the character lands and starts grounded.
	const FVector StandLocation(300.f, 0.f, 150.f);
	// Dropped from higher up, to press jump while falling.
	const FVector DropLocation(0.f, 0.f, 250.f);

	constexpr float MaxWalkOffTime = 2.f;
	constexpr float MaxFallTime = 2.f;
	constexpr float LandingCheckTime = 0.5f;

	// The buffered press happens this close above the floor: well inside the default buffer window.
	constexpr float BufferPressHeight = 15.f;

	// A real jump launches at JumpZVelocity; landing never produces more than a fraction of it.
	constexpr float JumpDetectionFraction = 0.5f;

	USAEVUSMovementSettings* MakeSettings()
	{
		return NewObject<USAEVUSMovementSettings>(GetTransientPackage());
	}

	/** Walks +X until the character leaves the platform edge. */
	bool WalkOffLedge(SAEVUSTests::FScopedTestWorld& TestWorld, ASAEVUSCharacter* Character)
	{
		const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
		for (int32 Step = 0; Step < SAEVUSTests::StepsFor(MaxWalkOffTime) && !Movement->IsFalling(); ++Step)
		{
			Character->AddMovementInput(FVector::ForwardVector, 1.f);
			TestWorld.Step();
		}
		return Movement->IsFalling();
	}

	bool HasJumped(const USAEVUSMovementComponent* Movement)
	{
		return Movement->Velocity.Z > Movement->JumpZVelocity * JumpDetectionFraction;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSCoyoteTimeTest, "SAEVUS.Movement.JumpAssist.CoyoteTime", SAEVUSTests::TestFlags)
bool FSAEVUSCoyoteTimeTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSJumpAssistTests;

	// Returns true if a jump pressed right after walking off the ledge launches the character.
	auto JumpAfterWalkingOff = [this](const USAEVUSMovementSettings* Settings) -> bool
	{
		SAEVUSTests::FScopedTestWorld TestWorld;
		TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
		ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation, Settings);
		if (!TestWorld.SettleOnGround())
		{
			AddError(TEXT("Character never landed on the platform"));
			return false;
		}
		if (!WalkOffLedge(TestWorld, Character))
		{
			const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
			const FFindFloorResult& Floor = Movement->CurrentFloor;
			AddError(FString::Printf(TEXT("Character never walked off the ledge: at %s, velocity %s, mode %s, floor hit=%d walkable=%d line=%d dist=%.2f impact=%s normal=%s comp=%s"),
				*Character->GetActorLocation().ToCompactString(), *Movement->Velocity.ToCompactString(), *Movement->GetMovementName(),
				Floor.bBlockingHit, Floor.bWalkableFloor, Floor.bLineTrace, Floor.FloorDist, *Floor.HitResult.ImpactPoint.ToCompactString(),
				*Floor.HitResult.ImpactNormal.ToCompactString(), *GetNameSafe(Floor.HitResult.GetComponent())));
			return false;
		}
		TestTrue(TEXT("Walking off a ledge opens the coyote window when enabled"),
			Character->GetSAEVUSMovement()->IsInCoyoteWindow() == (Character->GetSAEVUSMovement()->GetAirTuning().CoyoteTime > 0.f));

		Character->RequestJump();
		TestWorld.Step();
		return HasJumped(Character->GetSAEVUSMovement());
	};

	TestTrue(TEXT("Jump just after leaving the ledge is allowed"), JumpAfterWalkingOff(MakeSettings()));

	USAEVUSMovementSettings* NoCoyote = MakeSettings();
	NoCoyote->Air.CoyoteTime = 0.f;
	TestFalse(TEXT("Without coyote time the same jump is refused"), JumpAfterWalkingOff(NoCoyote));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSJumpBufferTest, "SAEVUS.Movement.JumpAssist.JumpBuffer", SAEVUSTests::TestFlags)
bool FSAEVUSJumpBufferTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSJumpAssistTests;

	// Returns true if a jump pressed just before touchdown fires after landing.
	auto JumpPressedBeforeLanding = [this](const USAEVUSMovementSettings* Settings) -> bool
	{
		SAEVUSTests::FScopedTestWorld TestWorld;
		TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
		ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(DropLocation, Settings);
		const USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
		const float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		for (int32 Step = 0; Step < SAEVUSTests::StepsFor(MaxFallTime); ++Step)
		{
			if (Character->GetActorLocation().Z - HalfHeight < BufferPressHeight)
			{
				break;
			}
			TestWorld.Step();
		}
		if (!Movement->IsFalling())
		{
			AddError(TEXT("Character was not falling when jump was pressed"));
			return false;
		}

		Character->RequestJump();
		Character->StopJumping();

		bool bLanded = false;
		for (int32 Step = 0; Step < SAEVUSTests::StepsFor(LandingCheckTime); ++Step)
		{
			TestWorld.Step();
			bLanded |= Movement->IsMovingOnGround();
			if (bLanded && HasJumped(Movement))
			{
				return true;
			}
		}
		TestTrue(TEXT("Character landed"), bLanded);
		return false;
	};

	TestTrue(TEXT("Jump pressed just before landing fires on touchdown"), JumpPressedBeforeLanding(MakeSettings()));

	USAEVUSMovementSettings* NoBuffer = MakeSettings();
	NoBuffer->Air.JumpBufferTime = 0.f;
	TestFalse(TEXT("Without a jump buffer the early press is dropped"), JumpPressedBeforeLanding(NoBuffer));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSAirGaitLockTest, "SAEVUS.Movement.JumpAssist.AirGaitLock", SAEVUSTests::TestFlags)
bool FSAEVUSAirGaitLockTest::RunTest(const FString& Parameters)
{
	using namespace SAEVUSJumpAssistTests;

	for (const bool bLockGait : { true, false })
	{
		USAEVUSMovementSettings* Settings = MakeSettings();
		Settings->Air.bLockGaitInAir = bLockGait;

		SAEVUSTests::FScopedTestWorld TestWorld;
		TestWorld.SpawnBlock(PlatformCenter, PlatformSize);
		ASAEVUSCharacter* Character = TestWorld.SpawnCharacter(StandLocation, Settings);
		USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
		if (!TestWorld.SettleOnGround())
		{
			AddError(TEXT("Character never landed on the platform"));
			continue;
		}

		Character->RequestJump();
		TestWorld.Step();
		TestTrue(TEXT("Jump from the ground"), Movement->IsFalling() && HasJumped(Movement));
		TestFalse(TEXT("A jump take-off does not open the coyote window"), Movement->IsInCoyoteWindow());

		Movement->SetSprintRequested(true);
		const ESAEVUSGait ExpectedGait = bLockGait ? ESAEVUSGait::Run : ESAEVUSGait::Sprint;
		TestEqual(bLockGait ? TEXT("Locked: sprint mid-air keeps the run cap") : TEXT("Unlocked: sprint mid-air raises the cap"),
			Movement->GetMaxSpeed(), Movement->GetGaitSpeed(ExpectedGait));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
