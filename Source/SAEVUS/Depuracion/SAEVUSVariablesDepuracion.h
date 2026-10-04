// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

/** Interruptores de depuración respaldados por variables de consola: funcionan desde la consola, los .ini y las teclas. */
namespace SAEVUSDepuracion
{
	/** SAEVUS.Depuracion.HUD */
	SAEVUS_API bool HUDActivo();
	SAEVUS_API void ActivarHUD(bool bActivo);

	/** SAEVUS.Depuracion.Combate: dibuja las esferas de golpe y las trazas de disparo. */
	SAEVUS_API bool DibujoCombateActivo();

	/** SAEVUS.Depuracion.NumerosDanio: números de daño flotantes sobre quien recibe el golpe. */
	SAEVUS_API bool NumerosDanioActivos();
	SAEVUS_API void ActivarNumerosDanio(bool bActivo);
}
