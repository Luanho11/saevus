// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Tutorial/SAEVUSVolumenJugador.h"
#include "SAEVUSPuntoControl.generated.h"

/** Al entrar, el jugador reaparecerá aquí (posición y orientación del actor) en vez de en el inicio. */
UCLASS()
class SAEVUS_API ASAEVUSPuntoControl : public ASAEVUSVolumenJugador
{
	GENERATED_BODY()

protected:
	virtual void AlEntrarJugador(ASAEVUSPersonaje* Personaje) override;
};
