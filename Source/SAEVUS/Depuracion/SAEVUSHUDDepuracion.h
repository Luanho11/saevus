// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SAEVUSHUDDepuracion.generated.h"

/** Texto superpuesto con valores en vivo. Solo se dibuja con SAEVUS.Depuracion.HUD activo (F6). */
UCLASS()
class SAEVUS_API ASAEVUSHUDDepuracion : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	/** Esquina superior izquierda del bloque de texto, en píxeles de pantalla. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Depuracion")
	FVector2D OrigenPantalla = FVector2D(24.f, 120.f);

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Depuracion", meta = (ClampMin = "0"))
	float AltoLinea = 18.f;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Depuracion", meta = (ClampMin = "0"))
	float EscalaTexto = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Depuracion")
	FLinearColor ColorTexto = FLinearColor(0.2f, 1.f, 0.4f);
};
