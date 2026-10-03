// Copyright SAEVUS. All Rights Reserved.

#include "Movement/SAEVUSMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

namespace SAEVUSMovement
{
	// Gait requests ride in the custom compressed-flag bits reserved for games.
	constexpr uint8 FlagSprint = FSavedMove_Character::FLAG_Custom_0;
	constexpr uint8 FlagWalk = FSavedMove_Character::FLAG_Custom_1;
	constexpr uint8 FlagDash = FSavedMove_Character::FLAG_Custom_2;

	const FSAEVUSGroundMovementTuning DefaultGroundTuning;
	const FSAEVUSAirMovementTuning DefaultAirTuning;
	const FSAEVUSDashTuning DefaultDashTuning;
	const FSAEVUSWallJumpTuning DefaultWallJumpTuning;

	// Wall probe: eight horizontal capsule sweeps. The probe capsule is shrunk a little so a
	// capsule already touching the wall doesn't start the sweep in penetration.
	constexpr int32 WallProbeDirections = 8;
	constexpr float WallProbeSkin = 2.f;
}

/**
 * Saved move carrying the gait and dash requests, plus the dash state at the start of the
 * move, so they replay correctly during client correction.
 */
class FSAEVUSSavedMove : public FSavedMove_Character
{
public:
	using Super = FSavedMove_Character;

	virtual void Clear() override
	{
		Super::Clear();
		bSavedWantsToSprint = false;
		bSavedWantsToWalk = false;
		bSavedWantsToDash = false;
		SavedDashDirection = FVector::ForwardVector;
		SavedDashTimeRemaining = 0.f;
		SavedDashCooldownRemaining = 0.f;
		SavedAirDashesUsed = 0;
		SavedWallJumpsUsed = 0;
	}

	virtual uint8 GetCompressedFlags() const override
	{
		uint8 Flags = Super::GetCompressedFlags();
		if (bSavedWantsToSprint)
		{
			Flags |= SAEVUSMovement::FlagSprint;
		}
		if (bSavedWantsToWalk)
		{
			Flags |= SAEVUSMovement::FlagWalk;
		}
		if (bSavedWantsToDash)
		{
			Flags |= SAEVUSMovement::FlagDash;
		}
		return Flags;
	}

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override
	{
		const FSAEVUSSavedMove* Other = static_cast<const FSAEVUSSavedMove*>(NewMove.Get());
		if (bSavedWantsToSprint != Other->bSavedWantsToSprint || bSavedWantsToWalk != Other->bSavedWantsToWalk)
		{
			return false;
		}
		// A dash press, or a move that starts or ends a dash, is never merged.
		if (bSavedWantsToDash || Other->bSavedWantsToDash || SavedAirDashesUsed != Other->SavedAirDashesUsed
			|| SavedWallJumpsUsed != Other->SavedWallJumpsUsed
			|| (SavedDashTimeRemaining > 0.f) != (Other->SavedDashTimeRemaining > 0.f))
		{
			return false;
		}
		return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
	}

	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override
	{
		Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
		if (const USAEVUSMovementComponent* Movement = Cast<USAEVUSMovementComponent>(Character->GetCharacterMovement()))
		{
			bSavedWantsToSprint = Movement->bWantsToSprint;
			bSavedWantsToWalk = Movement->bWantsToWalk;
			bSavedWantsToDash = Movement->bWantsToDash;
			SavedDashDirection = Movement->DashDirection;
			SavedDashTimeRemaining = Movement->DashTimeRemaining;
			SavedDashCooldownRemaining = Movement->DashCooldownRemaining;
			SavedAirDashesUsed = Movement->AirDashesUsed;
			SavedWallJumpsUsed = Movement->WallJumpsUsed;
		}
	}

	virtual void PrepMoveFor(ACharacter* Character) override
	{
		Super::PrepMoveFor(Character);
		if (USAEVUSMovementComponent* Movement = Cast<USAEVUSMovementComponent>(Character->GetCharacterMovement()))
		{
			Movement->bWantsToSprint = bSavedWantsToSprint;
			Movement->bWantsToWalk = bSavedWantsToWalk;
			Movement->bWantsToDash = bSavedWantsToDash;
			Movement->DashDirection = SavedDashDirection;
			Movement->DashTimeRemaining = SavedDashTimeRemaining;
			Movement->DashCooldownRemaining = SavedDashCooldownRemaining;
			Movement->AirDashesUsed = SavedAirDashesUsed;
			Movement->WallJumpsUsed = SavedWallJumpsUsed;
		}
	}

private:
	bool bSavedWantsToSprint = false;
	bool bSavedWantsToWalk = false;
	bool bSavedWantsToDash = false;
	FVector SavedDashDirection = FVector::ForwardVector;
	float SavedDashTimeRemaining = 0.f;
	float SavedDashCooldownRemaining = 0.f;
	int32 SavedAirDashesUsed = 0;
	int32 SavedWallJumpsUsed = 0;
};

class FSAEVUSNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
{
public:
	explicit FSAEVUSNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement)
		: FNetworkPredictionData_Client_Character(ClientMovement)
	{
	}

	virtual FSavedMovePtr AllocateNewMove() override
	{
		return FSavedMovePtr(new FSAEVUSSavedMove());
	}
};

USAEVUSMovementComponent::USAEVUSMovementComponent()
{
	bWantsInitializeComponent = true;

	// Third-person action: the character turns toward where it moves; the camera is free.
	bOrientRotationToMovement = true;
}

void USAEVUSMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	ApplyMovementSettings();
}

ESAEVUSGait USAEVUSMovementComponent::GetCurrentGait() const
{
	if (bWantsToSprint)
	{
		return ESAEVUSGait::Sprint;
	}
	return bWantsToWalk ? ESAEVUSGait::Walk : ESAEVUSGait::Run;
}

float USAEVUSMovementComponent::GetGaitSpeed(ESAEVUSGait Gait) const
{
	const FSAEVUSGroundMovementTuning& Tuning = GetGroundTuning();
	switch (Gait)
	{
	case ESAEVUSGait::Walk:
		return Tuning.WalkSpeed;
	case ESAEVUSGait::Sprint:
		return Tuning.SprintSpeed;
	case ESAEVUSGait::Run:
	default:
		return Tuning.RunSpeed;
	}
}

const FSAEVUSGroundMovementTuning& USAEVUSMovementComponent::GetGroundTuning() const
{
	return MovementSettings ? MovementSettings->Ground : SAEVUSMovement::DefaultGroundTuning;
}

const FSAEVUSAirMovementTuning& USAEVUSMovementComponent::GetAirTuning() const
{
	return MovementSettings ? MovementSettings->Air : SAEVUSMovement::DefaultAirTuning;
}

const FSAEVUSDashTuning& USAEVUSMovementComponent::GetDashTuning() const
{
	return MovementSettings ? MovementSettings->Dash : SAEVUSMovement::DefaultDashTuning;
}

const FSAEVUSWallJumpTuning& USAEVUSMovementComponent::GetWallJumpTuning() const
{
	return MovementSettings ? MovementSettings->WallJump : SAEVUSMovement::DefaultWallJumpTuning;
}

void USAEVUSMovementComponent::SetMovementSettings(const USAEVUSMovementSettings* NewSettings)
{
	MovementSettings = NewSettings;
	ApplyMovementSettings();
}

ESAEVUSGait USAEVUSMovementComponent::GetAirGait() const
{
	return GetAirTuning().bLockGaitInAir ? AirGait : GetCurrentGait();
}

bool USAEVUSMovementComponent::IsInCoyoteWindow() const
{
	return IsFalling() && CoyoteTimeRemaining > 0.f;
}

void USAEVUSMovementComponent::BufferJumpRequest()
{
	if (IsFalling())
	{
		JumpBufferTimeRemaining = GetAirTuning().JumpBufferTime;
	}
}

int32 USAEVUSMovementComponent::GetAirDashesRemaining() const
{
	return FMath::Max(0, GetDashTuning().MaxAirDashes - AirDashesUsed);
}

bool USAEVUSMovementComponent::CanDash() const
{
	if (IsDashing() || DashCooldownRemaining > 0.f || GetDashTuning().Duration <= 0.f || IsCrouching())
	{
		return false;
	}
	if (IsMovingOnGround())
	{
		return true;
	}
	return IsFalling() && GetAirDashesRemaining() > 0;
}

void USAEVUSMovementComponent::StartDash()
{
	const FSAEVUSDashTuning& Tuning = GetDashTuning();

	// Acceleration is the replicated input, so server and client pick the same direction.
	const FVector InputDirection = Acceleration.GetSafeNormal2D();
	DashDirection = InputDirection.IsNearlyZero() ? UpdatedComponent->GetForwardVector().GetSafeNormal2D() : InputDirection;
	DashTimeRemaining = Tuning.Duration;

	float VerticalSpeed = Velocity.Z;
	if (IsFalling())
	{
		++AirDashesUsed;
		VerticalSpeed = Tuning.AirDashVerticalSpeed;
	}
	Velocity = DashDirection * Tuning.Speed;
	Velocity.Z = VerticalSpeed;

	if (Tuning.bFaceDashDirection)
	{
		MoveUpdatedComponent(FVector::ZeroVector, DashDirection.ToOrientationQuat(), /*bSweep*/ false);
	}
}

void USAEVUSMovementComponent::EndDash()
{
	const FSAEVUSDashTuning& Tuning = GetDashTuning();
	DashTimeRemaining = 0.f;
	DashCooldownRemaining = Tuning.Cooldown;

	const FVector Horizontal = FVector(Velocity.X, Velocity.Y, 0.f).GetClampedToMaxSize(Tuning.ExitSpeed);
	Velocity.X = Horizontal.X;
	Velocity.Y = Horizontal.Y;
}

int32 USAEVUSMovementComponent::GetWallJumpsRemaining() const
{
	return FMath::Max(0, GetWallJumpTuning().MaxWallJumps - WallJumpsUsed);
}

FVector USAEVUSMovementComponent::FindWallJumpNormal() const
{
	if (!HasValidData())
	{
		return FVector::ZeroVector;
	}
	const FSAEVUSWallJumpTuning& Tuning = GetWallJumpTuning();
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FCollisionShape Probe = GetPawnCapsuleCollisionShape(SHRINK_RadiusCustom, SAEVUSMovement::WallProbeSkin);
	const float ProbeDistance = Tuning.DetectionDistance + SAEVUSMovement::WallProbeSkin;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SAEVUSWallJumpProbe), false, CharacterOwner);
	FCollisionResponseParams ResponseParams;
	InitCollisionParams(QueryParams, ResponseParams);
	const ECollisionChannel Channel = UpdatedComponent->GetCollisionObjectType();

	// Contacts on the lower hemisphere are ledges at foot level (e.g. the edge just walked off),
	// not walls: a wall has to touch the capsule's straight side.
	const float LowestWallContactZ = Start.Z - Probe.GetCapsuleHalfHeight() + Probe.GetCapsuleRadius();

	FVector BestNormal = FVector::ZeroVector;
	float BestDistance = TNumericLimits<float>::Max();
	for (int32 Index = 0; Index < SAEVUSMovement::WallProbeDirections; ++Index)
	{
		const float Yaw = 360.f * Index / SAEVUSMovement::WallProbeDirections;
		const FVector Direction = FRotator(0.f, Yaw, 0.f).Vector();
		FHitResult Hit;
		if (!GetWorld()->SweepSingleByChannel(Hit, Start, Start + Direction * ProbeDistance, FQuat::Identity, Channel, Probe, QueryParams, ResponseParams)
			|| Hit.bStartPenetrating || FMath::Abs(Hit.ImpactNormal.Z) > Tuning.MaxWallNormalZ || Hit.ImpactPoint.Z < LowestWallContactZ)
		{
			continue;
		}
		if (Hit.Distance < BestDistance)
		{
			BestDistance = Hit.Distance;
			BestNormal = Hit.ImpactNormal.GetSafeNormal2D();
		}
	}
	return BestNormal;
}

bool USAEVUSMovementComponent::TryWallJump()
{
	// Only the first update of a jump press counts (JumpKeyHoldTime is reset by Jump() and grows
	// after each update), and not when that press already did a normal or coyote jump.
	const bool bJumpJustPressed = CharacterOwner && CharacterOwner->bPressedJump && CharacterOwner->JumpKeyHoldTime <= 0.f
		&& !CharacterOwner->bWasJumping;
	if (!bJumpJustPressed || !IsFalling() || GetWallJumpsRemaining() <= 0)
	{
		return false;
	}
	const FVector WallNormal = FindWallJumpNormal();
	if (WallNormal.IsNearlyZero())
	{
		return false;
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

float USAEVUSMovementComponent::GetMaxSpeed() const
{
	if (IsDashing())
	{
		return GetDashTuning().Speed;
	}
	if (IsCrouching())
	{
		return Super::GetMaxSpeed();
	}
	if (IsMovingOnGround())
	{
		return GetGaitSpeed(GetCurrentGait());
	}
	// Falling uses a gait cap too, so air control never drags a sprint jump down to run speed.
	if (IsFalling())
	{
		return GetGaitSpeed(GetAirGait());
	}
	return Super::GetMaxSpeed();
}

float USAEVUSMovementComponent::GetGravityZ() const
{
	if (IsDashing() && IsFalling() && GetDashTuning().bSuspendGravityDuringDash)
	{
		return 0.f;
	}
	return Super::GetGravityZ();
}

void USAEVUSMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	// A dash holds its speed and direction: no input steering, friction or braking.
	// Callers pass horizontal velocity here (walking and falling both handle Z themselves).
	if (IsDashing() && HasValidData() && !HasAnimRootMotion())
	{
		const float VerticalSpeed = Velocity.Z;
		Velocity = DashDirection * GetDashTuning().Speed;
		Velocity.Z = VerticalSpeed;
		return;
	}
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

bool USAEVUSMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
	const bool bJumped = Super::DoJump(bReplayingMoves, DeltaTime);
	if (bJumped)
	{
		CoyoteTimeRemaining = 0.f;
		JumpBufferTimeRemaining = 0.f;
		// Jumping out of a ground dash cancels it; the jump keeps the dash's exit speed.
		if (IsDashing())
		{
			EndDash();
		}
	}
	return bJumped;
}

void USAEVUSMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	// Super notifies the character, which resets its jump state on landing; buffered jumps go after it.
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	const bool bLeftGround = IsFalling() && (PreviousMovementMode == MOVE_Walking || PreviousMovementMode == MOVE_NavWalking);
	if (bLeftGround)
	{
		AirGait = GetCurrentGait();
		// A jump take-off already used the ground jump; only walking off a ledge opens the window.
		const bool bJumpedOff = CharacterOwner && CharacterOwner->bPressedJump;
		CoyoteTimeRemaining = bJumpedOff ? 0.f : GetAirTuning().CoyoteTime;
	}
	else if (!IsFalling())
	{
		CoyoteTimeRemaining = 0.f;
	}

	if (IsMovingOnGround())
	{
		AirDashesUsed = 0;
		WallJumpsUsed = 0;
	}
	else if (IsDashing() && !IsFalling())
	{
		// Dash only runs in walking and falling; any other mode ends it.
		EndDash();
	}

	if (IsMovingOnGround() && JumpBufferTimeRemaining > 0.f && CharacterOwner)
	{
		JumpBufferTimeRemaining = 0.f;
		CharacterOwner->Jump();
	}
}

void USAEVUSMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	CoyoteTimeRemaining = FMath::Max(0.f, CoyoteTimeRemaining - DeltaSeconds);
	JumpBufferTimeRemaining = FMath::Max(0.f, JumpBufferTimeRemaining - DeltaSeconds);

	if (IsDashing())
	{
		DashTimeRemaining = FMath::Max(0.f, DashTimeRemaining - DeltaSeconds);
		if (!IsDashing())
		{
			EndDash();
		}
	}
	else
	{
		DashCooldownRemaining = FMath::Max(0.f, DashCooldownRemaining - DeltaSeconds);
	}
}

void USAEVUSMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	TryWallJump();

	// The request is consumed either way: a dash pressed during cooldown is not queued.
	if (bWantsToDash)
	{
		bWantsToDash = false;
		if (CanDash())
		{
			StartDash();
		}
	}
}

void USAEVUSMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToSprint = (Flags & SAEVUSMovement::FlagSprint) != 0;
	bWantsToWalk = (Flags & SAEVUSMovement::FlagWalk) != 0;
	bWantsToDash = (Flags & SAEVUSMovement::FlagDash) != 0;
}

FNetworkPredictionData_Client* USAEVUSMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		USAEVUSMovementComponent* MutableThis = const_cast<USAEVUSMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FSAEVUSNetworkPredictionData_Client(*this);
	}
	return ClientPredictionData;
}

void USAEVUSMovementComponent::ApplyMovementSettings()
{
	const FSAEVUSGroundMovementTuning& Ground = GetGroundTuning();
	// Systems that ignore gait (navigation, AI) read MaxWalkSpeed, so it mirrors the default gait.
	MaxWalkSpeed = Ground.RunSpeed;
	MaxAcceleration = Ground.MaxAcceleration;
	BrakingDecelerationWalking = Ground.BrakingDeceleration;
	GroundFriction = Ground.GroundFriction;
	RotationRate = FRotator(0.f, Ground.TurnRate, 0.f);

	const FSAEVUSAirMovementTuning& Air = GetAirTuning();
	JumpZVelocity = Air.JumpZVelocity;
	GravityScale = Air.GravityScale;
	AirControl = Air.AirControl;
	AirControlBoostMultiplier = Air.AirControlBoostMultiplier;
	AirControlBoostVelocityThreshold = Air.AirControlBoostVelocityThreshold;
	FallingLateralFriction = Air.FallingLateralFriction;
	BrakingDecelerationFalling = Air.BrakingDecelerationFalling;

	if (CharacterOwner)
	{
		CharacterOwner->JumpMaxHoldTime = Air.JumpMaxHoldTime;
	}
}
