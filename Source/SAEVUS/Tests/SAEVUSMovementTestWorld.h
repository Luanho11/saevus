// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Character/SAEVUSCharacter.h"
#include "Combat/SAEVUSCombatComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Movement/SAEVUSMovementComponent.h"

/** Shared fixture for movement tests that simulate a real ASAEVUSCharacter. */
namespace SAEVUSTests
{
	constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;

	constexpr float SimStep = 1.f / 60.f;

	// In editor builds CMC refuses to start falling during the first second of world time
	// (UCharacterMovementComponent::StartFalling), so test worlds start past it.
	constexpr double WorldStartTime = 10.0;

	// Engine cube: 100 cm, pivot at the centre.
	const TCHAR* const CubeMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
	constexpr float CubeSize = 100.f;

	inline int32 StepsFor(float Seconds)
	{
		return FMath::CeilToInt32(Seconds / SimStep);
	}

	/** Throwaway game world (no map, no game mode, no controller) for ticking one character. */
	class FScopedTestWorld
	{
	public:
		FScopedTestWorld()
		{
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			// Unique name: a previous test world may not be garbage-collected yet.
			const FName WorldName = MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("SAEVUSMovementTestWorld"));
			World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
			// Without a game mode nothing dispatches BeginPlay, so spawned actors would not begin play.
			if (!World->GetBegunPlay())
			{
				World->GetWorldSettings()->NotifyBeginPlay();
			}
			World->TimeSeconds = World->UnpausedTimeSeconds = World->RealTimeSeconds = WorldStartTime;
		}

		~FScopedTestWorld()
		{
			for (AActor* Actor : FActorRange(World))
			{
				Actor->RouteEndPlay(EEndPlayReason::LevelTransition);
			}
			GEngine->ShutdownWorldNetDriver(World);
			World->DestroyWorld(true);
			World->SetPhysicsScene(nullptr);
			GEngine->DestroyWorldContext(World);
			World->RemoveFromRoot();
		}

		FScopedTestWorld(const FScopedTestWorld&) = delete;
		FScopedTestWorld& operator=(const FScopedTestWorld&) = delete;

		/** Solid box of the given size (cm), centred on Center. */
		AStaticMeshActor* SpawnBlock(const FVector& Center, const FVector& Size)
		{
			AStaticMeshActor* Block = World->SpawnActor<AStaticMeshActor>(Center, FRotator::ZeroRotator);
			UStaticMeshComponent* Mesh = Block->GetStaticMeshComponent();
			// Static mobility refuses mesh changes after BeginPlay.
			Mesh->SetMobility(EComponentMobility::Movable);
			Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, CubeMeshPath));
			Block->SetActorScale3D(Size / CubeSize);
			return Block;
		}

		/**
		 * Spawns an uncontrolled character that still simulates, falling from Location.
		 * Use SettleOnGround() when it should start standing.
		 */
		ASAEVUSCharacter* SpawnCharacter(const FVector& Location, const USAEVUSMovementSettings* Settings = nullptr)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ASAEVUSCharacter* Character = World->SpawnActor<ASAEVUSCharacter>(Location, FRotator::ZeroRotator, Params);

			USAEVUSMovementComponent* Movement = Character->GetSAEVUSMovement();
			Movement->bRunPhysicsWithNoController = true;
			Movement->SetMovementSettings(Settings);
			// The engine only picks a default mode at spawn when the flag above was already set.
			Movement->SetMovementMode(MOVE_Falling);
			Movement->Velocity = FVector::ZeroVector;
			SimulatedMovement = Movement;
			SimulatedCombat = Character->GetCombat();
			return Character;
		}

		/** Steps until the character stands on something; false if it never lands within MaxSeconds. */
		bool SettleOnGround(float MaxSeconds = 2.f)
		{
			for (int32 Step = 0; Step < StepsFor(MaxSeconds); ++Step)
			{
				if (SimulatedMovement->IsMovingOnGround())
				{
					return true;
				}
				this->Step();
			}
			return SimulatedMovement->IsMovingOnGround();
		}

		/** Spawns any actor (targets, props) at Location. */
		template <typename TActor>
		TActor* SpawnActor(const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			return World->SpawnActor<TActor>(Location, Rotation, Params);
		}

		/**
		 * Advances the spawned character by one fixed step: combat, then movement, as the tick
		 * prerequisite orders them in game. Components are ticked directly: a world without a game
		 * mode or viewport does not run its tick groups reliably, and a direct tick keeps the
		 * simulation deterministic.
		 */
		void Step()
		{
			check(SimulatedMovement);
			World->TimeSeconds += SimStep;
			World->UnpausedTimeSeconds += SimStep;
			World->RealTimeSeconds += SimStep;
			if (SimulatedCombat)
			{
				SimulatedCombat->TickComponent(SimStep, LEVELTICK_All, &SimulatedCombat->PrimaryComponentTick);
			}
			SimulatedMovement->TickComponent(SimStep, LEVELTICK_All, &SimulatedMovement->PrimaryComponentTick);
		}

	private:
		UWorld* World = nullptr;
		USAEVUSMovementComponent* SimulatedMovement = nullptr;
		USAEVUSCombatComponent* SimulatedCombat = nullptr;
	};
}

#endif // WITH_DEV_AUTOMATION_TESTS
