// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SAEVUSControladorJugador.generated.h"

class UInputAction;
class UInputMappingContext;

/** Añade los contextos de entrada del jugador local y atiende la entrada propia del controlador (no del pawn). */
UCLASS()
class SAEVUS_API ASAEVUSControladorJugador : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetupInputComponent() override;

	/** Se añaden en orden, con prioridad 0, para el jugador local. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Entrada")
	TArray<TObjectPtr<UInputMappingContext>> ContextosEntrada;

	/** Alterna el HUD de depuración (SAEVUS.Depuracion.HUD). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Entrada|Depuracion")
	TObjectPtr<UInputAction> AccionAlternarHUD;

private:
	void AlternarHUDDepuracion();
};
