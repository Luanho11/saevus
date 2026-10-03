// Copyright SAEVUS. All Rights Reserved.

// Wall movement of USAEVUSMovementComponent: wall detection, wall jump and wall run.
// Kept apart from SAEVUSMovementComponent.cpp (gait, dash, jump assists, saved moves).

#include "Movement/SAEVUSMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

namespace SAEVUSWallMovement
{
	// Wall jump probe: eight horizontal capsule sweeps.
	constexpr int32 WallJumpProbeDirections = 8;

	// Probe capsules are shrunk a little so a capsule already touching the wall doesn't start
	// the sweep in penetration.
	constexpr float WallProbeSkin = 2.f;

	// While wall running the capsule is pulled to this gap from the wall each update, so it
	// keeps contact without grinding into it.
	constexpr float WallRunStickGap = 1.f;

	// A wall run whose normal is this close to the last one is the same wall.
	constexpr float SameWallNormalDot = 0.9f;

	// Moving less than this fraction of the expected distance along the wall means something
	// blocks the run.
	constexpr float BlockedProgressFraction = 0.5f;
}

bool USAEVUSMovementComponent::IsWallRunning() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ESAEVUSCustomMovementMode::WallRun);
}

int32 USAEVUSMovementComponent::GetWallJumpsRemaining() const
{
	return FMath::Max(0, GetWallJumpTuning().MaxWallJumps - WallJumpsUsed);
}

bool USAEVUSMovementComponent::FindNearestWall(TConstArrayView<FVector> Directions, float Distance, float MaxWallNormalZ, FHitResult& OutHit) const
{
	using namespace SAEVUSWallMovement;
	if (!HasValidData())
	{
		return false;
	}
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FCollisionShape Probe = GetPawnCapsuleCollisionShape(SHRINK_RadiusCustom, WallProbeSkin);
	const float ProbeDistance = Distance + WallProbeSkin;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SAEVUSWallProbe), false, CharacterOwner);
	FCollisionResponseParams ResponseParams;
	InitCollisionParams(QueryParams, ResponseParams);
	const ECollisionChannel Channel = UpdatedComponent->GetCollisionObjectType();

	// Contacts on the lower hemisphere are ledges at foot level (e.g. the edge just walked off),
	// not walls: a wall has to touch the capsule's straight side.
	const float LowestWallContactZ = Start.Z - Probe.GetCapsuleHalfHeight() + Probe.GetCapsuleRadius();

	bool bFound = false;
	for (const FVector& Direction : Directions)
	{
		FHitResult Hit;
		if (!GetWorld()->SweepSingleByChannel(Hit, Start, Start + Direction * ProbeDistance, FQuat::Identity, Channel, Probe, QueryParams, ResponseParams)
			|| Hit.bStartPenetrating || FMath::Abs(Hit.ImpactNormal.Z) > MaxWallNormalZ || Hit.ImpactPoint.Z < LowestWallContactZ)
		{
			continue;
		}
		if (!bFound || Hit.Distance < OutHit.Distance)
		{
			OutHit = Hit;
			bFound = true;
		}
	}
	return bFound;
}

FVector USAEVUSMovementComponent::FindWallJumpNormal() const
{
	TArray<FVector, TInlineAllocator<SAEVUSWallMovement::WallJumpProbeDirections>> Directions;
	for (int32 Index = 0; Index < SAEVUSWallMovement::WallJumpProbeDirections; ++Index)
	{
		Directions.Add(FRotator(0.f, 360.f * Index / SAEVUSWallMovement::WallJumpProbeDirections, 0.f).Vector());
	}
	const FSAEVUSWallJumpTuning& Tuning = GetWallJumpTuning();
	FHitResult Hit;
	return FindNearestWall(Directions, Tuning.DetectionDistance, Tuning.MaxWallNormalZ, Hit) ? Hit.ImpactNormal.GetSafeNormal2D() : FVector::ZeroVector;
}

bool USAEVUSMovementComponent::TryWallJump()
{
	// Only the first update of a jump press counts (JumpKeyHoldTime is reset by Jump() and grows
	// after each update), and not when that press already did a normal or coyote jump.
	const bool bJumpJustPressed = CharacterOwner && CharacterOwner->bPressedJump && CharacterOwner->JumpKeyHoldTime <= 0.f
		&& !CharacterOwner->bWasJumping;
	const bool bWallRunning = IsWallRunning();
	if (!bJumpJustPressed || !(IsFalling() || bWallRunning) || GetWallJumpsRemaining() <= 0)
	{
		return false;
	}
	// Jumping off a wall run pushes off the wall being run on.
	const FVector WallNormal = bWallRunning ? WallRunNormal : FindWallJumpNormal();
	if (WallNormal.IsNearlyZero())
	{
		return false;
	}
	if (bWallRunning)
	{
		SetMovementMode(MOVE_Falling);
	}
	LaunchOffWall(WallNormal);
	return true;
}

void USAEVUSMovementComponent::LaunchOffWall(const FVector& WallNormal)
{
	const FSAEVUSWallJumpTuning& Tuning = GetWallJumpTuning();
	if (IsDashing())
	{
		EndDash();
	}

	const FVector Horizontal(Velocity.X, Velocity.Y, 0.f);
	const FVector AlongWall = Horizontal - WallNormal * FVector::DotProduct(Horizontal, WallNormal);
	Velocity = WallNormal * Tuning.AwaySpeed + AlongWall * Tuning.TangentialSpeedKept;
	Velocity.Z = Tuning.UpSpeed;

	++WallJumpsUsed;
	if (Tuning.bRefillsAirDashes)
	{
		AirDashesUsed = 0;
	}
	// The press was used here; it must not also fire as a buffered jump on landing.
	JumpBufferTimeRemaining = 0.f;
	CoyoteTimeRemaining = 0.f;

	if (Tuning.bFaceJumpDirection)
	{
		MoveUpdatedComponent(FVector::ZeroVector, Velocity.GetSafeNormal2D().ToOrientationQuat(), /*bSweep*/ false);
	}
}

bool USAEVUSMovementComponent::TryStartWallRun()
{
	const FSAEVUSWallRunTuning& Tuning = GetWallRunTuning();
	if (Tuning.MaxDuration <= 0.f || !IsFalling() || IsDashing() || Velocity.Z < -Tuning.MaxEntryFallSpeed)
	{
		return false;
	}
	// The player has to steer: no input, no wall run.
	const FVector InputDirection = Acceleration.GetSafeNormal2D();
	const FVector MoveDirection = FVector(Velocity.X, Velocity.Y, 0.f).GetSafeNormal();
	if (InputDirection.IsNearlyZero() || MoveDirection.IsNearlyZero())
	{
		return false;
	}

	// Walls beside the motion, or diagonally ahead of it.
	const FVector Side = FVector::CrossProduct(FVector::UpVector, MoveDirection);
	const FVector Directions[] = { Side, -Side, (MoveDirection + Side).GetSafeNormal(), (MoveDirection - Side).GetSafeNormal() };
	FHitResult WallHit;
	if (!FindNearestWall(Directions, Tuning.DetectionDistance, Tuning.MaxWallNormalZ, WallHit))
	{
		return false;
	}
	const FVector Normal = WallHit.ImpactNormal.GetSafeNormal2D();
	// No running the same wall again before landing or running another one.
	if (!LastWallRunNormal.IsNearlyZero() && FVector::DotProduct(Normal, LastWallRunNormal) > SAEVUSWallMovement::SameWallNormalDot)
	{
		return false;
	}
	if (FVector::DotProduct(InputDirection, Normal) > Tuning.DetachInputDot)
	{
		return false;
	}
	const FVector Horizontal(Velocity.X, Velocity.Y, 0.f);
	const FVector AlongWall = Horizontal - Normal * FVector::DotProduct(Horizontal, Normal);
	if (AlongWall.Size() < Tuning.MinEntrySpeed)
	{
		return false;
	}

	WallRunNormal = Normal;
	WallRunDirection = AlongWall.GetSafeNormal();
	WallRunTimeRemaining = Tuning.MaxDuration;
	const float EntryVerticalSpeed = FMath::Clamp(static_cast<float>(Velocity.Z), 0.f, Tuning.MaxEntryUpSpeed);
	Velocity = WallRunDirection * Tuning.Speed;
	Velocity.Z = EntryVerticalSpeed;
	if (Tuning.bRefillsAirDashes)
	{
		AirDashesUsed = 0;
	}
	SetMovementMode(MOVE_Custom, static_cast<uint8>(ESAEVUSCustomMovementMode::WallRun));
	MoveUpdatedComponent(FVector::ZeroVector, WallRunDirection.ToOrientationQuat(), /*bSweep*/ false);
	return true;
}

void USAEVUSMovementComponent::EndWallRun(float RemainingTime, int32 Iterations)
{
	WallRunTimeRemaining = 0.f;
	// The velocity along the wall carries into the fall.
	SetMovementMode(MOVE_Falling);
	if (RemainingTime >= MIN_TICK_TIME)
	{
		StartNewPhysics(RemainingTime, Iterations);
	}
}

void USAEVUSMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (IsWallRunning())
	{
		PhysWallRun(DeltaTime, Iterations);
		return;
	}
	Super::PhysCustom(DeltaTime, Iterations);
}

void USAEVUSMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	using namespace SAEVUSWallMovement;
	if (DeltaTime < MIN_TICK_TIME || !HasValidData())
	{
		return;
	}
	const FSAEVUSWallRunTuning& Tuning = GetWallRunTuning();

	WallRunTimeRemaining -= DeltaTime;
	if (WallRunTimeRemaining <= 0.f)
	{
		EndWallRun(DeltaTime, Iterations);
		return;
	}
	if (FVector::DotProduct(Acceleration.GetSafeNormal2D(), WallRunNormal) > Tuning.DetachInputDot)
	{
		EndWallRun(DeltaTime, Iterations);
		return;
	}

	// The wall must still be there; following its normal lets the run bend around curved walls.
	FHitResult WallHit;
	const FVector TowardWall[] = { -WallRunNormal };
	if (!FindNearestWall(TowardWall, Tuning.DetectionDistance, Tuning.MaxWallNormalZ, WallHit))
	{
		EndWallRun(DeltaTime, Iterations);
		return;
	}
	WallRunNormal = WallHit.ImpactNormal.GetSafeNormal2D();
	WallRunDirection = (WallRunDirection - WallRunNormal * FVector::DotProduct(WallRunDirection, WallRunNormal)).GetSafeNormal();
	if (WallRunDirection.IsNearlyZero())
	{
		EndWallRun(DeltaTime, Iterations);
		return;
	}

	const float VerticalSpeed = Velocity.Z + GetGravityZ() * Tuning.GravityScale * DeltaTime;
	Velocity = WallRunDirection * Tuning.Speed;
	Velocity.Z = VerticalSpeed;

	// Move along the wall and pull back to a small gap from it.
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Stick = -WallRunNormal * FMath::Max(0.f, static_cast<float>(WallHit.Distance) - WallProbeSkin - WallRunStickGap);
	const FVector Delta = Velocity * DeltaTime + Stick;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Delta, WallRunDirection.ToOrientationQuat(), true, Hit);
	if (Hit.Time < 1.f)
	{
		if (Velocity.Z <= 0.f && IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
		{
			ProcessLanded(Hit, DeltaTime * (1.f - Hit.Time), Iterations);
			return;
		}
		HandleImpact(Hit, DeltaTime, Delta);
		SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}

	// Coming down onto a floor ends the run in a landing.
	if (Velocity.Z <= 0.f)
	{
		FFindFloorResult Floor;
		FindFloor(UpdatedComponent->GetComponentLocation(), Floor, false);
		if (Floor.IsWalkableFloor() && Floor.GetDistanceToFloor() <= MAX_FLOOR_DIST)
		{
			ProcessLanded(Floor.HitResult, 0.f, Iterations);
			return;
		}
	}

	// Something ahead on the wall stopped the run.
	const float Progress = FVector::DotProduct(UpdatedComponent->GetComponentLocation() - OldLocation, WallRunDirection);
	if (Progress < Tuning.Speed * DeltaTime * BlockedProgressFraction)
	{
		EndWallRun(0.f, Iterations);
	}
}
