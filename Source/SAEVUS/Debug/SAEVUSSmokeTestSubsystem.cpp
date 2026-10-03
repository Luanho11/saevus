// Copyright SAEVUS. All Rights Reserved.

#include "Debug/SAEVUSSmokeTestSubsystem.h"
#include "Character/SAEVUSCharacter.h"
#include "Debug/SAEVUSDebugCVars.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "DynamicRHI.h"
#include "Engine/Engine.h"
#include "Movement/SAEVUSMovementComponent.h"
#include "RenderTimer.h"
#include "SAEVUS.h"
#include "UnrealClient.h"

namespace SAEVUSSmokeTest
{
	const TCHAR* const CommandLineSwitch = TEXT("SAEVUSSmokeTest");
	// Optional: also capture one ProfileGPU frame mid-sprint (per-pass GPU timings in the log).
	const TCHAR* const ProfileGPUSwitch = TEXT("SAEVUSSmokeProfileGPU");
	const TCHAR* const ScreenshotName = TEXT("SAEVUS_SmokeTest.png");

	// Stage lengths in seconds. Settling covers spawn, landing and first-frame hitches.
	constexpr float SettleDuration = 2.f;
	constexpr float RunDuration = 2.f;
	constexpr float SprintDuration = 2.f;
	constexpr float MaxJumpDuration = 2.f;
	// Covers the dash, its exit and the cooldown.
	constexpr float DashStageDuration = 1.f;
	constexpr float ScreenshotDelay = 0.5f;
	constexpr float PawnTimeout = 120.f;

	// Measured values versus the character's own tuning.
	constexpr float SpeedTolerance = 0.05f;
	constexpr float JumpApexTolerance = 0.1f;
}

bool USAEVUSSmokeTestSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld() && FParse::Param(FCommandLine::Get(), SAEVUSSmokeTest::CommandLineSwitch);
#endif
}

TStatId USAEVUSSmokeTestSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USAEVUSSmokeTestSubsystem, STATGROUP_Tickables);
}

void USAEVUSSmokeTestSubsystem::EnterStage(EStage NewStage)
{
	Stage = NewStage;
	StageTime = 0.f;
}

void USAEVUSSmokeTestSubsystem::Check(const TCHAR* Description, float Measured, float Expected, float RelativeTolerance)
{
	const bool bPassed = FMath::IsNearlyEqual(Measured, Expected, FMath::Abs(Expected) * RelativeTolerance);
	FailureCount += bPassed ? 0 : 1;
	UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS smoke] %s %s: %.1f (expected %.1f +-%.0f%%)"),
		bPassed ? TEXT("ok  ") : TEXT("FAIL"), Description, Measured, Expected, RelativeTolerance * 100.f);
}

void USAEVUSSmokeTestSubsystem::Tick(float DeltaTime)
{
	using namespace SAEVUSSmokeTest;
	StageTime += DeltaTime;

	if (Stage == EStage::Run || Stage == EStage::Sprint || Stage == EStage::Jump || Stage == EStage::Dash)
	{
		SampleFrameTiming(DeltaTime);
	}

	ASAEVUSCharacter* Pawn = Character.Get();
	USAEVUSMovementComponent* Movement = Pawn ? Pawn->GetSAEVUSMovement() : nullptr;
	if (Stage != EStage::WaitForPawn && Stage != EStage::Done && !Movement)
	{
		UE_LOG(LogSAEVUS, Error, TEXT("[SAEVUS smoke] FAILED: player character disappeared"));
		++FailureCount;
		Finish();
		return;
	}

	switch (Stage)
	{
	case EStage::WaitForPawn:
		if (const APlayerController* Controller = GetWorld()->GetFirstPlayerController())
		{
			Character = Cast<ASAEVUSCharacter>(Controller->GetPawn());
		}
		if (Character.IsValid())
		{
			UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS smoke] pawn %s spawned at %s"), *Character->GetName(), *Character->GetActorLocation().ToCompactString());
			SAEVUSDebug::SetDebugHUDEnabled(true);
			EnterStage(EStage::Settle);
		}
		else if (StageTime > PawnTimeout)
		{
			UE_LOG(LogSAEVUS, Error, TEXT("[SAEVUS smoke] FAILED: no ASAEVUSCharacter possessed after %.0f s"), PawnTimeout);
			++FailureCount;
			Finish();
		}
		break;

	case EStage::Settle:
		if (StageTime >= SettleDuration)
		{
			UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS smoke] settled, mode %s"), *Movement->GetMovementName());
			EnterStage(EStage::Run);
		}
		break;

	case EStage::Run:
		Pawn->AddMovementInput(FVector::ForwardVector, 1.f);
		MaxRunSpeed = FMath::Max(MaxRunSpeed, static_cast<float>(Movement->Velocity.Size2D()));
		if (StageTime >= RunDuration)
		{
			Movement->SetSprintRequested(true);
			EnterStage(EStage::Sprint);
		}
		break;

	case EStage::Sprint:
		Pawn->AddMovementInput(FVector::ForwardVector, 1.f);
		MaxSprintSpeed = FMath::Max(MaxSprintSpeed, static_cast<float>(Movement->Velocity.Size2D()));
		if (!bProfiledGPU && StageTime >= SprintDuration * 0.5f && FParse::Param(FCommandLine::Get(), ProfileGPUSwitch))
		{
			bProfiledGPU = true;
			GEngine->Exec(GetWorld(), TEXT("ProfileGPU"));
		}
		if (StageTime >= SprintDuration)
		{
			Movement->SetSprintRequested(false);
			JumpStartZ = JumpApexZ = Pawn->GetActorLocation().Z;
			Pawn->Jump();
			EnterStage(EStage::Jump);
		}
		break;

	case EStage::Jump:
		Pawn->StopJumping();
		JumpApexZ = FMath::Max(JumpApexZ, static_cast<float>(Pawn->GetActorLocation().Z));
		bLeftGround |= Movement->IsFalling();
		if ((bLeftGround && Movement->IsMovingOnGround()) || StageTime >= MaxJumpDuration)
		{
			Movement->RequestDash();
			EnterStage(EStage::Dash);
		}
		break;

	case EStage::Dash:
		MaxDashSpeed = FMath::Max(MaxDashSpeed, static_cast<float>(Movement->Velocity.Size2D()));
		if (StageTime >= DashStageDuration)
		{
			FScreenshotRequest::RequestScreenshot(ScreenshotName, true, false);
			EnterStage(EStage::Screenshot);
		}
		break;

	case EStage::Screenshot:
		if (StageTime >= ScreenshotDelay)
		{
			EnterStage(EStage::Report);
		}
		break;

	case EStage::Report:
	{
		Check(TEXT("run speed"), MaxRunSpeed, Movement->GetGaitSpeed(ESAEVUSGait::Run), SpeedTolerance);
		Check(TEXT("sprint speed"), MaxSprintSpeed, Movement->GetGaitSpeed(ESAEVUSGait::Sprint), SpeedTolerance);
		const float ExpectedApex = FMath::Square(Movement->JumpZVelocity) / (2.f * FMath::Abs(Movement->GetGravityZ()));
		Check(TEXT("jump apex"), JumpApexZ - JumpStartZ, ExpectedApex, JumpApexTolerance);
		Check(TEXT("dash speed"), MaxDashSpeed, Movement->GetDashTuning().Speed, SpeedTolerance);
		Finish();
		break;
	}

	case EStage::Done:
	default:
		break;
	}
}

void USAEVUSSmokeTestSubsystem::SampleFrameTiming(float DeltaTime)
{
	FrameTimeSum += DeltaTime;
	WorstFrameTime = FMath::Max(WorstFrameTime, DeltaTime);
	GameThreadMsSum += FPlatformTime::ToMilliseconds(GGameThreadTime);
	RenderThreadMsSum += FPlatformTime::ToMilliseconds(GRenderThreadTime);
	GPUMsSum += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	++FrameCount;
}

void USAEVUSSmokeTestSubsystem::Finish()
{
	if (FrameCount > 0)
	{
		const double AverageFrameTime = FrameTimeSum / FrameCount;
		UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS smoke] frames %d, average %.2f ms (%.1f fps), worst %.2f ms | game %.2f ms, render %.2f ms, GPU %.2f ms"),
			FrameCount, AverageFrameTime * 1000.0, AverageFrameTime > 0.0 ? 1.0 / AverageFrameTime : 0.0, WorstFrameTime * 1000.f,
			GameThreadMsSum / FrameCount, RenderThreadMsSum / FrameCount, GPUMsSum / FrameCount);
	}
	UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS smoke] %s"), FailureCount == 0 ? TEXT("PASSED") : TEXT("FAILED"));
	EnterStage(EStage::Done);
	FPlatformMisc::RequestExit(false, TEXT("SAEVUS smoke test"));
}
