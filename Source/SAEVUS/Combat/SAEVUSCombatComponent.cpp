// Copyright SAEVUS. All Rights Reserved.

#include "Combat/SAEVUSCombatComponent.h"
#include "CollisionQueryParams.h"
#include "Debug/SAEVUSDebugCVars.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Movement/SAEVUSMovementComponent.h"

namespace SAEVUSCombat
{
	const FSAEVUSMeleeMoveset DefaultMoveset;

	// Debug drawing of the hit sphere (SAEVUS.Debug.Combat).
	constexpr int32 DebugSphereSegments = 12;
}

USAEVUSCombatComponent::USAEVUSCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

const FSAEVUSMeleeMoveset& USAEVUSCombatComponent::GetMoveset() const
{
	return MovesetAsset ? MovesetAsset->Moveset : SAEVUSCombat::DefaultMoveset;
}

void USAEVUSCombatComponent::SetMoveset(const USAEVUSMeleeMovesetAsset* NewMoveset)
{
	// CurrentAttack points into the old moveset.
	EndAttack();
	MovesetAsset = NewMoveset;
}

ACharacter* USAEVUSCombatComponent::GetCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

USAEVUSMovementComponent* USAEVUSCombatComponent::GetMovement() const
{
	const ACharacter* Character = GetCharacter();
	return Character ? Cast<USAEVUSMovementComponent>(Character->GetCharacterMovement()) : nullptr;
}

void USAEVUSCombatComponent::RequestAttack()
{
	BufferedAttackTimeRemaining = GetMoveset().InputBufferTime;
	// Start at once if possible, so a press doesn't wait a frame.
	ProcessBufferedAttack();
}

bool USAEVUSCombatComponent::TryCancelForAction(ESAEVUSCancelAction Action)
{
	if (!CurrentAttack)
	{
		return true;
	}
	const float CancelFrom = CurrentAttack->GetCancelFrom(Action);
	if (CancelFrom < 0.f || AttackTime < CancelFrom)
	{
		return false;
	}
	EndAttack();
	return true;
}

ESAEVUSAttackPhase USAEVUSCombatComponent::GetAttackPhase() const
{
	return CurrentAttack ? CurrentAttack->GetPhaseAt(AttackTime) : ESAEVUSAttackPhase::None;
}

void USAEVUSCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentAttack)
	{
		const USAEVUSMovementComponent* Movement = GetMovement();
		if (Movement && (Movement->IsWallRunning() || Movement->IsWallClimbing()))
		{
			// Wall movement takes over the body; the attack is dropped.
			EndAttack();
		}
	}

	if (CurrentAttack)
	{
		AttackTime += DeltaTime;
		if (CurrentAttack->GetPhaseAt(AttackTime) == ESAEVUSAttackPhase::Active)
		{
			DetectHits(*CurrentAttack);
		}
		if (AttackTime >= CurrentAttack->GetTotalTime())
		{
			EndAttack();
		}
	}

	ProcessBufferedAttack();
	BufferedAttackTimeRemaining = FMath::Max(0.f, BufferedAttackTimeRemaining - DeltaTime);
}

void USAEVUSCombatComponent::ProcessBufferedAttack()
{
	if (BufferedAttackTimeRemaining <= 0.f || !CanStartAttackNow())
	{
		return;
	}
	if (!CurrentAttack)
	{
		const USAEVUSMovementComponent* Movement = GetMovement();
		StartAttack(Movement && Movement->IsFalling(), 0);
		return;
	}
	// Chain: the next attack of the same chain, once the running one allows it.
	const TArray<FSAEVUSAttackDefinition>& Chain = bAirChain ? GetMoveset().AirCombo : GetMoveset().GroundCombo;
	if (AttackTime >= CurrentAttack->ChainFrom && Chain.IsValidIndex(ComboIndex + 1))
	{
		StartAttack(bAirChain, ComboIndex + 1);
	}
}

bool USAEVUSCombatComponent::CanStartAttackNow() const
{
	const USAEVUSMovementComponent* Movement = GetMovement();
	if (!Movement || !(Movement->IsMovingOnGround() || Movement->IsFalling()))
	{
		return false;
	}
	return !Movement->IsDashing() || GetMoveset().bAttackEndsDash;
}

void USAEVUSCombatComponent::StartAttack(bool bInAir, int32 Index)
{
	const TArray<FSAEVUSAttackDefinition>& Chain = bInAir ? GetMoveset().AirCombo : GetMoveset().GroundCombo;
	USAEVUSMovementComponent* Movement = GetMovement();
	if (!Chain.IsValidIndex(Index) || !Movement)
	{
		return;
	}
	const FSAEVUSAttackDefinition& Attack = Chain[Index];

	CurrentAttack = &Attack;
	ComboIndex = Index;
	bAirChain = bInAir;
	AttackTime = 0.f;
	BufferedAttackTimeRemaining = 0.f;
	HitActors.Reset();

	// Attack toward the movement input if there is any, otherwise straight ahead.
	const FVector InputDirection = Movement->GetCurrentAcceleration().GetSafeNormal2D();
	const FVector Direction = InputDirection.IsNearlyZero() ? GetOwner()->GetActorForwardVector().GetSafeNormal2D() : InputDirection;

	Movement->CancelDash();
	Movement->SnapFacing(Direction);
	Movement->StartLunge(Direction, Attack.LungeSpeed, Attack.LungeDuration);
	Movement->SetActionSpeedScale(Attack.GroundMoveSpeedScale);
}

void USAEVUSCombatComponent::EndAttack()
{
	CurrentAttack = nullptr;
	ComboIndex = INDEX_NONE;
	AttackTime = 0.f;
	HitActors.Reset();
	if (USAEVUSMovementComponent* Movement = GetMovement())
	{
		Movement->SetActionSpeedScale(1.f);
	}
}

void USAEVUSCombatComponent::DetectHits(const FSAEVUSAttackDefinition& Attack)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}
	const FVector Center = Owner->GetActorLocation() + Owner->GetActorForwardVector() * Attack.HitForwardOffset + FVector::UpVector * Attack.HitHeightOffset;

	FCollisionObjectQueryParams ObjectTypes;
	ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
	ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SAEVUSAttack), false, Owner);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, ObjectTypes, FCollisionShape::MakeSphere(Attack.HitRadius), QueryParams);

	if (SAEVUSDebug::IsDebugCombatEnabled())
	{
		DrawDebugSphere(World, Center, Attack.HitRadius, SAEVUSCombat::DebugSphereSegments, Overlaps.IsEmpty() ? FColor::Yellow : FColor::Red);
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target || Target == Owner || HitActors.Contains(Target))
		{
			continue;
		}
		HitActors.Add(Target);
		UGameplayStatics::ApplyDamage(Target, Attack.Damage, Owner->GetInstigatorController(), Owner, UDamageType::StaticClass());
		OnAttackHit.Broadcast(Target, Attack.Damage);
	}
}
