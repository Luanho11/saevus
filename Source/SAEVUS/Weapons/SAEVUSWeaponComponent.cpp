// Copyright SAEVUS. All Rights Reserved.

#include "Weapons/SAEVUSWeaponComponent.h"
#include "Combat/SAEVUSCombatComponent.h"
#include "Core/SAEVUSCollisionChannels.h"
#include "Debug/SAEVUSDebugCVars.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Movement/SAEVUSMovementComponent.h"

namespace SAEVUSWeapons
{
	// Debug drawing of shots (SAEVUS.Debug.Combat).
	constexpr float DebugShotLifetime = 1.f;
}

USAEVUSWeaponComponent::USAEVUSWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USAEVUSWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetMagazines();
	if (Loadout.IsValidIndex(StartingSlot))
	{
		Equip(StartingSlot);
	}
}

void USAEVUSWeaponComponent::SetLoadout(const TArray<const USAEVUSWeaponDefinition*>& NewLoadout, int32 StartSlot)
{
	Loadout.Reset();
	for (const USAEVUSWeaponDefinition* Weapon : NewLoadout)
	{
		Loadout.Add(Weapon);
	}
	CurrentSlot = PendingSlot = INDEX_NONE;
	SwitchTimeRemaining = ReloadTimeRemaining = FireCooldown = 0.f;
	bShotRequested = false;
	ResetMagazines();
	if (Loadout.IsValidIndex(StartSlot))
	{
		Equip(StartSlot);
	}
}

void USAEVUSWeaponComponent::ResetMagazines()
{
	Magazines.SetNumZeroed(Loadout.Num());
	for (int32 Slot = 0; Slot < Loadout.Num(); ++Slot)
	{
		const USAEVUSWeaponDefinition* Weapon = Loadout[Slot];
		Magazines[Slot] = (Weapon && Weapon->IsFirearm()) ? Weapon->Firearm.MagazineSize : 0;
	}
}

const USAEVUSWeaponDefinition* USAEVUSWeaponComponent::GetCurrentWeapon() const
{
	return Loadout.IsValidIndex(CurrentSlot) ? Loadout[CurrentSlot].Get() : nullptr;
}

bool USAEVUSWeaponComponent::IsFirearmEquipped() const
{
	const USAEVUSWeaponDefinition* Weapon = GetCurrentWeapon();
	return Weapon && Weapon->IsFirearm();
}

int32 USAEVUSWeaponComponent::GetAmmo(int32 Slot) const
{
	return Magazines.IsValidIndex(Slot) ? Magazines[Slot] : 0;
}

bool USAEVUSWeaponComponent::CanSwitchTo(int32 Slot) const
{
	return Loadout.IsValidIndex(Slot) && Loadout[Slot] && Slot != PendingSlot && (Slot != CurrentSlot || IsSwitching());
}

bool USAEVUSWeaponComponent::RequestSwitch(int32 Slot)
{
	if (!CanSwitchTo(Slot))
	{
		return false;
	}
	// Switching drops a reload in progress and any queued shot.
	ReloadTimeRemaining = 0.f;
	bShotRequested = false;
	PendingSlot = Slot;
	SwitchTimeRemaining = Loadout[Slot]->EquipTime;
	if (SwitchTimeRemaining <= 0.f)
	{
		Equip(Slot);
	}
	return true;
}

int32 USAEVUSWeaponComponent::GetNextSlot() const
{
	if (Loadout.Num() < 2)
	{
		return INDEX_NONE;
	}
	const int32 From = IsSwitching() ? PendingSlot : CurrentSlot;
	return (FMath::Max(From, 0) + 1) % Loadout.Num();
}

void USAEVUSWeaponComponent::Equip(int32 Slot)
{
	CurrentSlot = Slot;
	PendingSlot = INDEX_NONE;
	SwitchTimeRemaining = 0.f;

	const USAEVUSWeaponDefinition* Weapon = GetCurrentWeapon();
	if (Weapon && !Weapon->IsFirearm())
	{
		if (USAEVUSCombatComponent* Combat = GetOwner()->FindComponentByClass<USAEVUSCombatComponent>())
		{
			Combat->SetMoveset(Weapon->Moveset);
		}
	}
}

void USAEVUSWeaponComponent::SetTriggerHeld(bool bHeld)
{
	bTriggerHeld = bHeld;
	if (bHeld)
	{
		bShotRequested = true;
	}
}

void USAEVUSWeaponComponent::RequestReload()
{
	const USAEVUSWeaponDefinition* Weapon = GetCurrentWeapon();
	if (!Weapon || !Weapon->IsFirearm() || IsSwitching() || IsReloading() || GetCurrentAmmo() >= Weapon->Firearm.MagazineSize)
	{
		return;
	}
	ReloadTimeRemaining = Weapon->Firearm.ReloadTime;
	bShotRequested = false;
	if (ReloadTimeRemaining <= 0.f)
	{
		Magazines[CurrentSlot] = Weapon->Firearm.MagazineSize;
	}
}

void USAEVUSWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsSwitching())
	{
		SwitchTimeRemaining -= DeltaTime;
		if (SwitchTimeRemaining <= 0.f)
		{
			Equip(PendingSlot);
		}
	}
	if (IsReloading())
	{
		ReloadTimeRemaining -= DeltaTime;
		if (ReloadTimeRemaining <= 0.f)
		{
			ReloadTimeRemaining = 0.f;
			if (const USAEVUSWeaponDefinition* Weapon = GetCurrentWeapon())
			{
				Magazines[CurrentSlot] = Weapon->Firearm.MagazineSize;
			}
		}
	}
	TryFire(DeltaTime);
}

void USAEVUSWeaponComponent::TryFire(float DeltaTime)
{
	// Carry at most one frame of overshoot, so automatic fire keeps its rate without banking shots.
	FireCooldown = FMath::Max(FireCooldown - DeltaTime, -DeltaTime);

	const USAEVUSWeaponDefinition* Weapon = GetCurrentWeapon();
	if (!Weapon || !Weapon->IsFirearm() || IsSwitching() || IsReloading())
	{
		return;
	}
	const FSAEVUSFirearmTuning& Tuning = Weapon->Firearm;
	const bool bWantsShot = bShotRequested || (bTriggerHeld && Tuning.bAutomatic);
	if (!bWantsShot || FireCooldown > 0.f)
	{
		return;
	}
	bShotRequested = false;
	if (Magazines[CurrentSlot] <= 0)
	{
		// Dry trigger: reload instead.
		RequestReload();
		return;
	}
	--Magazines[CurrentSlot];
	FireCooldown += Tuning.FireInterval;
	FireShot(Tuning);
}

void USAEVUSWeaponComponent::GetAimViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->GetController())
	{
		Pawn->GetController()->GetPlayerViewPoint(OutLocation, OutRotation);
		return;
	}
	GetOwner()->GetActorEyesViewPoint(OutLocation, OutRotation);
}

void USAEVUSWeaponComponent::FireShot(const FSAEVUSFirearmTuning& Tuning)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}
	++ShotsFired;

	FVector ViewLocation;
	FRotator ViewRotation;
	GetAimViewPoint(ViewLocation, ViewRotation);
	const FVector AimDirection = ViewRotation.Vector();
	const FVector Muzzle = Owner->GetActorTransform().TransformPosition(MuzzleOffset);

	// The body turns to where the shot goes.
	if (const ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (USAEVUSMovementComponent* Movement = Cast<USAEVUSMovementComponent>(Character->GetCharacterMovement()))
		{
			Movement->SnapFacing(AimDirection);
		}
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SAEVUSWeaponShot), true, Owner);
	const float SpreadRadians = FMath::DegreesToRadians(Tuning.SpreadDegrees);
	// The view ray starts behind the character in third person, so it is longer than the range.
	const float ViewRayLength = Tuning.Range + FVector::Dist(ViewLocation, Muzzle);

	for (int32 Pellet = 0; Pellet < Tuning.PelletsPerShot; ++Pellet)
	{
		const FVector Direction = SpreadRadians > 0.f ? SpreadStream.VRandCone(AimDirection, SpreadRadians) : AimDirection;

		// 1) What the view ray points at: the aim point.
		FHitResult ViewHit;
		const FVector ViewEnd = ViewLocation + Direction * ViewRayLength;
		const FVector AimPoint = World->LineTraceSingleByChannel(ViewHit, ViewLocation, ViewEnd, SAEVUSCollision::WeaponTrace, QueryParams) ? ViewHit.ImpactPoint : ViewEnd;

		// 2) The shot itself, from the muzzle toward the aim point, so cover in front of the character counts.
		FVector ShotDirection = (AimPoint - Muzzle).GetSafeNormal();
		if (FVector::DotProduct(ShotDirection, AimDirection) <= 0.f)
		{
			// Aim point behind the muzzle (something between camera and character): shoot along the view.
			ShotDirection = Direction;
		}
		const FVector ShotEnd = Muzzle + ShotDirection * Tuning.Range;
		FHitResult ShotHit;
		const bool bHit = World->LineTraceSingleByChannel(ShotHit, Muzzle, ShotEnd, SAEVUSCollision::WeaponTrace, QueryParams);
		AActor* HitActor = bHit ? ShotHit.GetActor() : nullptr;
		const FVector Impact = bHit ? ShotHit.ImpactPoint : ShotEnd;

		if (HitActor)
		{
			UGameplayStatics::ApplyPointDamage(HitActor, Tuning.Damage, ShotDirection, ShotHit, Owner->GetInstigatorController(), Owner, UDamageType::StaticClass());
		}
		if (SAEVUSDebug::IsDebugCombatEnabled())
		{
			DrawDebugLine(World, Muzzle, Impact, HitActor ? FColor::Red : FColor::Yellow, false, SAEVUSWeapons::DebugShotLifetime);
		}
		OnWeaponFired.Broadcast(Muzzle, Impact, HitActor);
	}
}
