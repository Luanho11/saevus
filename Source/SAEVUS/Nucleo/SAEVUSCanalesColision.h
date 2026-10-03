// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Engine/EngineTypes.h"

/** Canales de colisión del proyecto. Nombres y respuestas por defecto en Config/DefaultEngine.ini. */
namespace SAEVUSColision
{
	/** Canal de traza "Arma": disparos hitscan. Bloquea por defecto, también a los pawns (a diferencia de Visibility). */
	constexpr ECollisionChannel TrazaArma = ECC_GameTraceChannel1;
}
