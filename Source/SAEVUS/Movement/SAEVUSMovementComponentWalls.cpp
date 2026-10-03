// Copyright SAEVUS. All Rights Reserved.

// Wall movement of USAEVUSMovementComponent: wall detection, wall jump, wall run and wall climb.
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

	// On a wall the capsule is pulled to this gap from it each update, so it keeps contact
	// without grinding into it.
	constexpr float WallStickGap = 1.f;

	// A wall run whose normal is this close to the last one is the same wall.
	constexpr float SameWallNormalDot = 0.9f;

	// Moving less than this fraction of the expected distance along or up the wall means
	// something blocks the way.
	constexpr float BlockedProgressFraction = 0.5f;
}

bool USAEVUSMovementComponent::IsWallRunning() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ESAEVUSCustomMovementMode::WallRun);
}

bool USAEVUSMovementComponent::IsWallClimbing() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ESAEVUSCustomMovementMode::WallClimb);
}

int32 USAEVUSMovementComponent::GetWallJumpsRemaining() const
{
	return FMath::Max(0, GetWallJumpTuning().MaxWallJumps - WallJumpsUsed);
}

int32 USAEVUSMovementComponent::GetWallClimbsRemaining() const
{
	return FMath::Max(0, GetWallClimbTuning().MaxClimbs - WallClimbsUsed);
}

// --- Wall detection -----------------------------------------------------------

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

bool USAEVUSMovementComponent::ProbeCurrentWall(float DetectionDistance, float MaxWallNormalZ, FHitResult& OutWallHit)
{
	const FVector TowardWall[] = { -WallNormal };
	if (!FindNearestWall(TowardWall, DetectionDistance, MaxWallNormalZ, OutWallHit))
	{
		return false;
	}
	WallNormal = OutWallHit.ImpactNormal.GetSafeNormal2D();
	return true;
}

// --- Wall jump ----------------------------------------------------------------

bool USAEVUSMovementComponent::TryWallJump()
{
	// Only the first update of a jump press counts (JumpKeyHoldTime is reset by Jump() and grows
	// after each update), and not when that press already did a normal or coyote jump.
	const bool bJumpJustPressed = CharacterOwner && CharacterOwner->bPressedJump && CharacterOwner->JumpKeyHoldTime <= 0.f
		&& !CharacterOwner->bWasJumping;
	const bool bOnWall = IsWallRunning() || IsWallClimbing();
	if (!bJumpJustPressed || !(IsFalling() || bOnWall) || GetWallJumpsRemaining() <= 0)
	{
		return false;
	}
	// Jumping off a wall run or climb pushes off the wall the character is on.
	const FVector PushNormal = bOnWall ? WallNormal : FindWallJumpNormal();
	if (PushNormal.IsNearlyZero())
	{
		return false;
	}
	if (bOnWall)
	{
		SetMovementMode(MOVE_Falling);
	}
	LaunchOffWall(PushNormal);
	return true;
}

void USAEVUSMovementComponent::LaunchOffWall(const FVector& PushNormal)
{
	const FSAEVUSWallJumpTuning& Tuning = GetWallJumpTuning();
	if (IsDashing())
	{
		EndDash();
	}

	const FVector Horizontal(Velocity.X, Velocity.Y, 0.f);
	const FVector AlongWall = Horizontal - PushNormal * FVector::DotProduct(Horizontal, PushNormal);
	Velocity = PushNormal * Tuning.AwaySpeed + AlongWall * Tuning.TangentialSpeedKept;
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

// --- Shared wall-mode plumbing ---------------------------------------------------

void USAEVUSMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (IsWallRunning())
	{
		PhysWallRun(DeltaTime, Iterations);
		return;
	}
	if (IsWallClimbing())
	{
		PhysWallClimb(DeltaTime, Iterations);
		return;
	}
	Super::PhysCustom(DeltaTime, Iterations);
}

void USAEVUSMovementComponent::LeaveWall(float RemainingTime, int32 Iterations)
{
	WallTimeRemaining = 0.f;
	// Whatever velocity the caller set carries into the fall.
	SetMovementMode(MOVE_Falling);
	if (RemainingTime >= MIN_TICK_TIME)
	{
		StartNewPhysics(RemainingTime, Iterations);
	}
}

FVector USAEVUSMovementComponent::MoveAlongWall(float DeltaTime, int32 Iterations, const FHitResult& WallHit, const FQuat& Facing)
{
	using namespace SAEVUSWallMovement;
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Stick = -WallNormal * FMath::Max(0.f, static_cast<float>(WallHit.Distance) - WallProbeSkin - WallStickGap);
	const FVector Delta = Velocity * DeltaTime + Stick;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Delta, Facing, true, Hit);
	if (Hit.Time < 1.f)
	{
		if (Velocity.Z <= 0.f && IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
		{
			ProcessLanded(Hit, DeltaTime * (1.f - Hit.Time), Iterations);
			return UpdatedComponent->GetComponentLocation() - OldLocation;
		}
		HandleImpact(Hit, DeltaTime, Delta);
		SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}
	return UpdatedComponent->GetComponentLocation() - OldLocation;
}

// --- Wall run -------------------------------------------------------------------

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

	WallNormal = Normal;
	WallRunDirection = AlongWall.GetSafeNormal();
	WallTimeRemaining = Tuning.MaxDuration;
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

void USAEVUSMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	using namespace SAEVUSWallMovement;
	if (DeltaTime < MIN_TICK_TIME || !HasValidData())
	{
		return;
	}
	const FSAEVUSWallRunTuning& Tuning = GetWallRunTuning();

	WallTimeRemaining -= DeltaTime;
	if (WallTimeRemaining <= 0.f || FVector::DotProduct(Acceleration.GetSafeNormal2D(), WallNormal) > Tuning.DetachInputDot)
	{
		// Out of time, or steering away: fall, keeping the run's speed.
		LeaveWall(DeltaTime, Iterations);
		return;
	}

	// The wall must still be there; following its normal lets the run bend around curved walls.
	FHitResult WallHit;
	if (!ProbeCurrentWall(Tuning.DetectionDistance, Tuning.MaxWallNormalZ, WallHit))
	{
		LeaveWall(DeltaTime, Iterations);
		return;
	}
	WallRunDirection = (WallRunDirection - WallNormal * FVector::DotProduct(WallRunDirection, WallNormal)).GetSafeNormal();
	if (WallRunDirection.IsNearlyZero())
	{
		LeaveWall(DeltaTime, Iterations);
		return;
	}

	const float VerticalSpeed = Velocity.Z + GetGravityZ() * Tuning.GravityScale * DeltaTime;
	Velocity = WallRunDirection * Tuning.Speed;
	Velocity.Z = VerticalSpeed;

	const FVector Moved = MoveAlongWall(DeltaTime, Iterations, WallHit, WallRunDirection.ToOrientationQuat());
	if (!IsWallRunning())
	{
		return; // landed during the move
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
	if (FVector::DotProduct(Moved, WallRunDirection) < Tuning.Speed * DeltaTime * BlockedProgressFraction)
	{
		LeaveWall(0.f, Iterations);
	}
}

// --- Wall climb -----------------------------------------------------------------

bool USAEVUSMovementComponent::TryStartWallClimb()
{
	const FSAEVUSWallClimbTuning& Tuning = GetWallClimbTuning();
	if (Tuning.MaxDuration <= 0.f || !IsFalling() || IsDashing() || GetWallClimbsRemaining() <= 0
		|| Velocity.Z < -Tuning.MaxEntryFallSpeed)
	{
		return false;
	}
	const FVector InputDirection = Acceleration.GetSafeNormal2D();
	const FVector Horizontal(Velocity.X, Velocity.Y, 0.f);
	const FVector MoveDirection = Horizontal.GetSafeNormal();
	if (InputDirection.IsNearlyZero() || MoveDirection.IsNearlyZero())
	{
		return false;
	}

	// Only a wall straight ahead of the motion, approached head-on and steered into.
	const FVector Ahead[] = { MoveDirection };
	FHitResult WallHit;
	if (!FindNearestWall(Ahead, Tuning.DetectionDistance, Tuning.MaxWallNormalZ, WallHit))
	{
		return false;
	}
	const FVector Normal = WallHit.ImpactNormal.GetSafeNormal2D();
	const FVector IntoWall = -Normal;
	if (FVector::DotProduct(MoveDirection, IntoWall) < Tuning.MinHeadOnDot || FVector::DotProduct(InputDirection, IntoWall) < Tuning.MinHeadOnDot
		|| FVector::DotProduct(Horizontal, IntoWall) < Tuning.MinEntrySpeed)
	{
		return false;
	}

	WallNormal = Normal;
	WallTimeRemaining = Tuning.MaxDuration;
	++WallClimbsUsed;
	if (Tuning.bRefillsAirDashes)
	{
		AirDashesUsed = 0;
	}
	Velocity = FVector(0.f, 0.f, Tuning.Speed);
	SetMovementMode(MOVE_Custom, static_cast<uint8>(ESAEVUSCustomMovementMode::WallClimb));
	MoveUpdatedComponent(FVector::ZeroVector, IntoWall.ToOrientationQuat(), /*bSweep*/ false);
	return true;
}

void USAEVUSMovementComponent::PhysWallClimb(float DeltaTime, int32 Iterations)
{
	using namespace SAEVUSWallMovement;
	if (DeltaTime < MIN_TICK_TIME || !HasValidData())
	{
		return;
	}
	const FSAEVUSWallClimbTuning& Tuning = GetWallClimbTuning();

	WallTimeRemaining -= DeltaTime;
	if (WallTimeRemaining <= 0.f)
	{
		// Out of time: flip away from the wall.
		Velocity = WallNormal * Tuning.FlipAwaySpeed;
		Velocity.Z = Tuning.FlipUpSpeed;
		MoveUpdatedComponent(FVector::ZeroVector, WallNormal.ToOrientationQuat(), /*bSweep*/ false);
		LeaveWall(DeltaTime, Iterations);
		return;
	}
	if (FVector::DotProduct(Acceleration.GetSafeNormal2D(), WallNormal) > Tuning.DetachInputDot)
	{
		// Letting go: drop straight down.
		Velocity = FVector::ZeroVector;
		LeaveWall(DeltaTime, Iterations);
		return;
	}

	FHitResult WallHit;
	if (!ProbeCurrentWall(Tuning.DetectionDistance, Tuning.MaxWallNormalZ, WallHit))
	{
		// Past the top of the wall: keep rising and step forward onto it.
		Velocity = -WallNormal * Tuning.LedgeForwardSpeed;
		Velocity.Z = Tuning.Speed;
		LeaveWall(DeltaTime, Iterations);
		return;
	}

	Velocity = FVector(0.f, 0.f, Tuning.Speed);
	const FVector Moved = MoveAlongWall(DeltaTime, Iterations, WallHit, (-WallNormal).ToOrientationQuat());
	if (IsWallClimbing() && Moved.Z < Tuning.Speed * DeltaTime * BlockedProgressFraction)
	{
		// Something overhead stops the climb.
		Velocity = FVector::ZeroVector;
		LeaveWall(0.f, Iterations);
	}
}
