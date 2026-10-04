// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SAEVUSInteractuable.generated.h"

class ASAEVUSPersonaje;

UINTERFACE(MinimalAPI)
class USAEVUSInteractuable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Algo del mundo que el jugador usa con la acción de interactuar (bancos). Al acercarse se lo
 * anuncia al personaje (ASAEVUSPersonaje::FijarInteractuable) y la interfaz muestra su texto.
 */
class SAEVUS_API ISAEVUSInteractuable
{
	GENERATED_BODY()

public:
	/** Lo que se hace al interactuar, para la interfaz ("Sentarse"). */
	virtual FText ObtenerTextoInteraccion() const = 0;

	virtual void Interactuar(ASAEVUSPersonaje* Personaje) = 0;
};
