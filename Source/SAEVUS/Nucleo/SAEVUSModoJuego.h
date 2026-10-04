// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SAEVUSModoJuego.generated.h"

/**
 * Modo de juego por defecto. Las clases nativas son respaldo; la subclase Blueprint
 * pone el pawn y el controlador Blueprint, que llevan las referencias a assets.
 * También manda la regla de reaparición: un jugador muerto vuelve tras RetardoReaparicion.
 */
UCLASS()
class SAEVUS_API ASAEVUSModoJuego : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASAEVUSModoJuego();

	/** Lo llama un personaje al morir. Programa la reaparición de su controlador (si tiene). */
	void NotificarMuerteJugador(AController* Controlador);

	/** Reaparición inmediata en el último punto de control (menú de pausa): el cuerpo actual desaparece. */
	void ReiniciarJugador(AController* Controlador);

	/** Desde ahora los jugadores reaparecen en Punto (puntos de control del tutorial); se guarda para "Continuar". */
	void FijarPuntoControl(const FTransform& Punto);

	bool TienePuntoControl() const { return bTienePuntoControl; }

	/**
	 * Cierto si este mapa abre con el menú principal (MapasConMenuPrincipal), salvo en las pruebas
	 * en el juego (-SAEVUSPruebaHumo, -SAEVUSRecorrido) o con -SAEVUSSinMenu.
	 */
	bool DebeMostrarMenuPrincipal() const;

	/** Inicio normal: un APlayerStart sin etiqueta (los etiquetados son llegadas desde otros mapas). */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** Si se llega por un paso de otro mapa, la llegada es el primer punto de control. */
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NuevoJugador) override;

	/** Cierto si se entró en el mapa por un paso (opción "portal" en la URL). */
	bool LlegoPorPaso() const;
	const FTransform& ObtenerPuntoControl() const { return PuntoControl; }

	/** Mapas a los que llevan los botones del menú principal. */
	const FString& ObtenerRutaMapaTutorial() const { return RutaMapaTutorial; }
	const FString& ObtenerRutaMapaMazmorra() const { return RutaMapaMazmorra; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Reaparicion", meta = (ClampMin = "0", ForceUnits = "s"))
	float RetardoReaparicion = 3.f;

	/** Mapas (nombre corto) que arrancan con el menú principal. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Menu")
	TArray<FName> MapasConMenuPrincipal;

	/** Ruta del mapa del tutorial (botón "Tutorial" y "Menú principal", que arranca en él). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Menu")
	FString RutaMapaTutorial = TEXT("/Game/SAEVUS/Mapas/L_Tutorial");

	/** Ruta del mapa de práctica (botón "Mazmorra"). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Menu")
	FString RutaMapaMazmorra = TEXT("/Game/SAEVUS/Mapas/L_Mazmorra");

	/** Cuánto tiempo queda el cuerpo tras la reaparición. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Reaparicion", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionCadaver = 10.f;

private:
	void ReaparecerJugador(TWeakObjectPtr<AController> Controlador);

	FTransform PuntoControl;
	bool bTienePuntoControl = false;
};
