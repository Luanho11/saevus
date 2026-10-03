// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSSmokeTestSubsystem.generated.h"

class ASAEVUSCharacter;

/**
 * Unattended in-game smoke test, active only with the -SAEVUSSmokeTest command-line switch
 * (never in Shipping). Drives the local player character through run, sprint and jump,
 * compares the results with its movement tuning, takes a screenshot with the debug HUD on,
 * logs "[SAEVUS smoke] PASSED/FAILED" plus a frame-time baseline, then quits.
 *
 * Usage: UnrealEditor.exe SAEVUS.uproject -game -windowed -ResX=1280 -ResY=720 -SAEVUSSmokeTest
 */
UCLASS()
class SAEVUS_API USAEVUSSmokeTestSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem interface

	//~ Begin FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject interface

private:
	enum class EStage : uint8
	{
		WaitForPawn,
		Settle,
		Run,
		Sprint,
		Jump,
		Dash,
		Screenshot,
		Report,
		Done
	};

	void EnterStage(EStage NewStage);
	void Check(const TCHAR* Description, float Measured, float Expected, float RelativeTolerance);
	void Finish();

	TWeakObjectPtr<ASAEVUSCharacter> Character;
	EStage Stage = EStage::WaitForPawn;
	float StageTime = 0.f;

	float MaxRunSpeed = 0.f;
	float MaxSprintSpeed = 0.f;
	float JumpStartZ = 0.f;
	float JumpApexZ = 0.f;
	bool bLeftGround = false;
	float MaxDashSpeed = 0.f;
	bool bProfiledGPU = false;

	/** Frame timing accumulated during the movement stages only (spawn and settle hitches excluded). */
	void SampleFrameTiming(float DeltaTime);

	double FrameTimeSum = 0.0;
	double GameThreadMsSum = 0.0;
	double RenderThreadMsSum = 0.0;
	double GPUMsSum = 0.0;
	float WorstFrameTime = 0.f;
	int32 FrameCount = 0;
	int32 FailureCount = 0;
};
