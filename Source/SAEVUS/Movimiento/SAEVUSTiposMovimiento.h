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

/** Submodos SAEVUS de MOVE_Custom (UCharacterMovementComponent::CustomMovementMode). */
UENUM(BlueprintType)
enum class ESAEVUSModoMovimientoPersonalizado : uint8
{
	Ninguno = 0 UMETA(Hidden),
	CarreraPared,
	EscaladaPared
};
