// Copyright SAEVUS. All Rights Reserved.

#include "Debug/SAEVUSDebugHUD.h"
#include "Combat/SAEVUSCombatComponent.h"
#include "Combat/SAEVUSHealthComponent.h"
#include "Weapons/SAEVUSWeaponComponent.h"
#include "Debug/SAEVUSDebugCVars.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Movement/SAEVUSMovementComponent.h"

void ASAEVUSDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!SAEVUSDebug::IsDebugHUDEnabled())
	{
		return;
	}

	const ACharacter* Character = Cast<ACharacter>(GetOwningPawn());
	const USAEVUSMovementComponent* Movement = Character ? Cast<USAEVUSMovementComponent>(Character->GetCharacterMovement()) : nullptr;
	if (!Movement)
	{
		return;
	}

	const TArray<FString> Lines = {
		TEXT("SAEVUS DEBUG  (F6 / SAEVUS.Debug.HUD)"),
		FString::Printf(TEXT("Gait: %s"), *UEnum::GetDisplayValueAsText(Movement->GetCurrentGait()).ToString()),
		FString::Printf(TEXT("Speed: %.0f / %.0f cm/s"), Movement->Velocity.Size2D(), Movement->GetMaxSpeed()),
		FString::Printf(TEXT("Vertical speed: %.0f cm/s"), Movement->Velocity.Z),
		FString::Printf(TEXT("Mode: %s"), *Movement->GetMovementName()),
		FString::Printf(TEXT("Dash: %s, air dashes left %d"), Movement->IsDashing() ? TEXT("active") : (Movement->CanDash() ? TEXT("ready") : TEXT("blocked")),
			Movement->GetAirDashesRemaining()),
		FString::Printf(TEXT("Wall jumps left: %d, climbs left %d, on wall %.2f s"), Movement->GetWallJumpsRemaining(),
			Movement->GetWallClimbsRemaining(), Movement->GetWallTimeRemaining()),
		FString::Printf(TEXT("Location: %s"), *Character->GetActorLocation().ToCompactString()),
	};

	FString AttackLine = TEXT("Attack: none");
	if (const USAEVUSCombatComponent* Combat = Character->FindComponentByClass<USAEVUSCombatComponent>())
	{
		if (const FSAEVUSAttackDefinition* Attack = Combat->GetCurrentAttack())
		{
			AttackLine = FString::Printf(TEXT("Attack: %s #%d %s %.2f s"), *Attack->Name.ToString(), Combat->GetComboIndex() + 1,
				*UEnum::GetDisplayValueAsText(Combat->GetAttackPhase()).ToString(), Combat->GetAttackTime());
		}
		if (Combat->HasBufferedAttack())
		{
			AttackLine += TEXT(" (buffered)");
		}
	}

	FString WeaponLine = TEXT("Weapon: none");
	if (const USAEVUSWeaponComponent* Weapons = Character->FindComponentByClass<USAEVUSWeaponComponent>())
	{
		if (const USAEVUSWeaponDefinition* Weapon = Weapons->GetCurrentWeapon())
		{
			WeaponLine = FString::Printf(TEXT("Weapon: %s [%d/%d]"), *Weapon->DisplayName.ToString(), Weapons->GetCurrentSlot() + 1, Weapons->GetSlotCount());
			if (Weapon->IsFirearm())
			{
				WeaponLine += FString::Printf(TEXT(" ammo %d/%d"), Weapons->GetCurrentAmmo(), Weapon->Firearm.MagazineSize);
			}
		}
		if (Weapons->IsSwitching())
		{
			WeaponLine += FString::Printf(TEXT(" (switching to %d)"), Weapons->GetPendingSlot() + 1);
		}
		if (Weapons->IsReloading())
		{
			WeaponLine += TEXT(" (reloading)");
		}
	}

	if (const USAEVUSHealthComponent* Health = Character->FindComponentByClass<USAEVUSHealthComponent>())
	{
		WeaponLine += FString::Printf(TEXT("   Health: %.0f/%.0f%s"), Health->GetHealth(), Health->GetMaxHealth(), Health->IsDead() ? TEXT(" DEAD") : TEXT(""));
	}

	FVector2D Cursor = ScreenOrigin;
	for (const FString& Line : Lines)
	{
		DrawText(Line, TextColor, Cursor.X, Cursor.Y, GEngine->GetSmallFont(), TextScale);
		Cursor.Y += LineHeight;
	}
	DrawText(AttackLine, TextColor, Cursor.X, Cursor.Y, GEngine->GetSmallFont(), TextScale);
	Cursor.Y += LineHeight;
	DrawText(WeaponLine, TextColor, Cursor.X, Cursor.Y, GEngine->GetSmallFont(), TextScale);
}
