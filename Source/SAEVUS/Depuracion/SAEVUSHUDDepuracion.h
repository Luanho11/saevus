// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Interfaz/SAEVUSWidgetInterfaz.h"
#include "SAEVUSHUDDepuracion.generated.h"

/**
 * HUD de SAEVUS: añade la interfaz de juego (USAEVUSWidgetInterfaz) y dibuja encima los valores
 * en vivo de depuración (solo con SAEVUS.Depuracion.HUD activo, F6).
 */
UCLASS()
class SAEVUS_API ASAEVUSHUDDepuracion : public AHUD
{
	GENERATED_BODY()

public:
	ASAEVUSHUDDepuracion();

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

	/** Interfaz de juego que se añade a la pantalla al empezar. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Interfaz")
	TSubclassOf<USAEVUSWidgetInterfaz> ClaseInterfaz;

	virtual void BeginPlay() override;

private:
	void DibujarDepuracion();

	UPROPERTY(Transient)
	TObjectPtr<USAEVUSWidgetInterfaz> Interfaz;
};
