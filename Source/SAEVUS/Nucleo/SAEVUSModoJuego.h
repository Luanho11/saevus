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
	const FTransform& ObtenerPuntoControl() const { return PuntoControl; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Reaparicion", meta = (ClampMin = "0", ForceUnits = "s"))
	float RetardoReaparicion = 3.f;

	/** Cuánto tiempo queda el cuerpo tras la reaparición. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Reaparicion", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionCadaver = 10.f;

private:
	void ReaparecerJugador(TWeakObjectPtr<AController> Controlador);

	FTransform PuntoControl;
	bool bTienePuntoControl = false;
};
