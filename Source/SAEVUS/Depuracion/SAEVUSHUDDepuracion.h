// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SAEVUSHUDDepuracion.generated.h"

/**
 * Texto superpuesto: los avisos del tutorial (siempre) y valores en vivo de depuración (solo con
 * SAEVUS.Depuracion.HUD activo, F6).
 */
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

	/** Altura del aviso de tutorial como fracción de la pantalla (desde arriba). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Tutorial", meta = (ClampMin = "0", ClampMax = "1"))
	float AlturaAviso = 0.12f;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Tutorial", meta = (ClampMin = "0"))
	float EscalaTituloAviso = 1.4f;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Tutorial", meta = (ClampMin = "0"))
	float EscalaTextoAviso = 1.f;

	/** Fundido de entrada y salida del aviso. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Tutorial", meta = (ClampMin = "0", ForceUnits = "s"))
	float FundidoAviso = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Tutorial")
	FLinearColor ColorAviso = FLinearColor(0.85f, 0.9f, 1.f);

private:
	void DibujarAvisoTutorial();
	void DibujarDepuracion();
};
