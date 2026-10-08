// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/EngineTypes.h"

/** Canales de colisión del proyecto. Nombres y respuestas por defecto en Config/DefaultEngine.ini. */
namespace SAEVUSColision
{
	/**
	 * Canal de traza "Arma": disparos hitscan y la hoja (lo que el tajo puede tocar tiene que bloquearlo).
	 * Bloquea por defecto, también a los pawns (a diferencia de Visibility).
	 */
	constexpr ECollisionChannel TrazaArma = ECC_GameTraceChannel1;

	/**
	 * Zona que solo nota al jugador (avisos, bancos, fragmentos, pasos): consultas sin física, ignora todo menos
	 * los pawns, a los que solapa. Tampoco la toca la hoja.
	 */
	inline void ConfigurarZonaJugador(UPrimitiveComponent* Zona)
	{
		Zona->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
		Zona->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Zona->SetCollisionResponseToAllChannels(ECR_Ignore);
		Zona->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Zona->SetGenerateOverlapEvents(true);
	}
}
