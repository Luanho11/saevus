// Copyright SAEVUS. All Rights Reserved.

#include "Debug/SAEVUSDebugHUD.h"
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
		FString::Printf(TEXT("Wall jumps left: %d, wall run %.2f s"), Movement->GetWallJumpsRemaining(), Movement->GetWallRunTimeRemaining()),
		FString::Printf(TEXT("Location: %s"), *Character->GetActorLocation().ToCompactString()),
	};

	FVector2D Cursor = ScreenOrigin;
	for (const FString& Line : Lines)
	{
		DrawText(Line, TextColor, Cursor.X, Cursor.Y, GEngine->GetSmallFont(), TextScale);
		Cursor.Y += LineHeight;
	}
}
