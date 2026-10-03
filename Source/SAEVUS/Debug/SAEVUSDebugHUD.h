// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SAEVUSDebugHUD.generated.h"

/** Canvas overlay with live movement values. Drawn only while SAEVUS.Debug.HUD is on. */
UCLASS()
class SAEVUS_API ASAEVUSDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	/** Top-left corner of the text block, in screen pixels. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Debug")
	FVector2D ScreenOrigin = FVector2D(24.f, 120.f);

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Debug", meta = (ClampMin = "0"))
	float LineHeight = 18.f;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Debug", meta = (ClampMin = "0"))
	float TextScale = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Debug")
	FLinearColor TextColor = FLinearColor(0.2f, 1.f, 0.4f);
};
