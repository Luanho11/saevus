// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "SAEVUSTiposMovimiento.generated.h"

/** Nivel de velocidad en el suelo que pide el jugador. */
UENUM(BlueprintType)
enum class ESAEVUSPaso : uint8
{
	Caminar,
	Correr,
	Esprint
};

/**
 * Quién frena al personaje en el suelo. Cada fuente guarda su propia escala y manda la más baja:
 * así curar, la guardia y un ataque no se pisan al terminar uno mientras sigue otro.
 */
UENUM(BlueprintType)
enum class ESAEVUSFuenteEscalaVelocidad : uint8
{
	Ataque,
	Curacion,
	Guardia,
	Total UMETA(Hidden)
};

/** Submodos SAEVUS de MOVE_Custom (UCharacterMovementComponent::CustomMovementMode). */
UENUM(BlueprintType)
enum class ESAEVUSModoMovimientoPersonalizado : uint8
{
	Ninguno = 0 UMETA(Hidden),
	CarreraPared,
	EscaladaPared
};
