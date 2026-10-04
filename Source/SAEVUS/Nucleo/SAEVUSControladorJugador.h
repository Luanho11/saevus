// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SAEVUSControladorJugador.generated.h"

class UInputAction;
class UInputMappingContext;
class USAEVUSWidgetPausa;

/** Añade los contextos de entrada del jugador local y atiende la entrada propia del controlador (no del pawn). */
UCLASS()
class SAEVUS_API ASAEVUSControladorJugador : public APlayerController
{
	GENERATED_BODY()

public:
	ASAEVUSControladorJugador();

	/** Abre o cierra el menú de pausa (pausa el juego y muestra el cursor). */
	void AlternarPausa();

	bool EstaEnMenuPausa() const;

	/** Vuelve al último punto de control (o al inicio) y quita la pausa. */
	void ReiniciarEnPuntoControl();

	/** Menú principal con el juego en pausa (al arrancar los mapas que lo piden, ver ASAEVUSModoJuego). */
	void MostrarMenuPrincipal();
	/** "Jugar": cierra el menú principal y quita la pausa. */
	void CerrarMenuPrincipal();
	/** "Continuar": lleva al jugador al punto de control guardado de este mapa y cierra el menú. */
	void ContinuarPartidaGuardada();
	bool EstaEnMenuPrincipal() const;

	/** Multiplicador de la entrada de mirar (ratón y stick). Se guarda entre sesiones. */
	float ObtenerSensibilidad() const { return Sensibilidad; }
	void FijarSensibilidad(float Nueva);

	/** Volumen general (0–1). Se aplica al audio y se guarda entre sesiones. */
	float ObtenerVolumen() const { return Volumen; }
	void FijarVolumen(float Nuevo);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Abre y cierra el menú de pausa. Debe disparar también en pausa (bTriggerWhenPaused). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Entrada")
	TObjectPtr<UInputAction> AccionPausa;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Interfaz")
	TSubclassOf<USAEVUSWidgetPausa> ClaseMenuPausa;

	/** Se añaden en orden, con prioridad 0, para el jugador local. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Entrada")
	TArray<TObjectPtr<UInputMappingContext>> ContextosEntrada;

	/** Alterna el HUD de depuración (SAEVUS.Depuracion.HUD). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Entrada|Depuracion")
	TObjectPtr<UInputAction> AccionAlternarHUD;

private:
	void AlternarHUDDepuracion();
	/** Añade el menú a la pantalla con el juego en pausa y el cursor visible, o lo quita y devuelve el control. */
	void AbrirMenu(USAEVUSWidgetPausa* Menu);
	void CerrarMenu(USAEVUSWidgetPausa* Menu);

	UPROPERTY(Transient)
	TObjectPtr<USAEVUSWidgetPausa> MenuPrincipal;

	UPROPERTY(Transient)
	TObjectPtr<USAEVUSWidgetPausa> MenuPausa;

	float Sensibilidad = 1.f;
	float Volumen = 1.f;
};
