// Copyright SAEVUS. All Rights Reserved.

#include "Movement/SAEVUSMovementComponent.h"
#include "GameFramework/Character.h"

namespace SAEVUSMovement
{
	// Gait requests ride in the custom compressed-flag bits reserved for games.
	constexpr uint8 FlagSprint = FSavedMove_Character::FLAG_Custom_0;
	constexpr uint8 FlagWalk = FSavedMove_Character::FLAG_Custom_1;

	const FSAEVUSGroundMovementTuning DefaultGroundTuning;
	const FSAEVUSAirMovementTuning DefaultAirTuning;
}

/** Saved move carrying the gait requests so they replay correctly during client correction. */
class FSAEVUSSavedMove : public FSavedMove_Character
{
public:
	using Super = FSavedMove_Character;

	virtual void Clear() override
	{
		Super::Clear();
		bSavedWantsToSprint = false;
		bSavedWantsToWalk = false;
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
		return Flags;
	}

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override
	{
		const FSAEVUSSavedMove* Other = static_cast<const FSAEVUSSavedMove*>(NewMove.Get());
		if (bSavedWantsToSprint != Other->bSavedWantsToSprint || bSavedWantsToWalk != Other->bSavedWantsToWalk)
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
		}
	}

	virtual void PrepMoveFor(ACharacter* Character) override
	{
		Super::PrepMoveFor(Character);
		if (USAEVUSMovementComponent* Movement = Cast<USAEVUSMovementComponent>(Character->GetCharacterMovement()))
		{
			Movement->bWantsToSprint = bSavedWantsToSprint;
			Movement->bWantsToWalk = bSavedWantsToWalk;
		}
	}

private:
	bool bSavedWantsToSprint = false;
	bool bSavedWantsToWalk = false;
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

void USAEVUSMovementComponent::SetMovementSettings(const USAEVUSMovementSettings* NewSettings)
{
	MovementSettings = NewSettings;
	ApplyMovementSettings();
}

float USAEVUSMovementComponent::GetMaxSpeed() const
{
	// Falling uses the gait cap too, so air control never drags a sprint jump down to run speed.
	if ((IsMovingOnGround() || IsFalling()) && !IsCrouching())
	{
		return GetGaitSpeed(GetCurrentGait());
	}
	return Super::GetMaxSpeed();
}

void USAEVUSMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToSprint = (Flags & SAEVUSMovement::FlagSprint) != 0;
	bWantsToWalk = (Flags & SAEVUSMovement::FlagWalk) != 0;
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
