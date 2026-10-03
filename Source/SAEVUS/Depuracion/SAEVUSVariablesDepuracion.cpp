// Copyright SAEVUS. Todos los derechos reservados.

#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "HAL/IConsoleManager.h"

namespace SAEVUSDepuracion
{
	static TAutoConsoleVariable<bool> CVarHUD(
		TEXT("SAEVUS.Depuracion.HUD"),
		false,
		TEXT("Muestra el HUD de depuración de SAEVUS (paso, velocidad, modo de movimiento, combate, armas)."),
		ECVF_Default);

	static TAutoConsoleVariable<bool> CVarCombate(
		TEXT("SAEVUS.Depuracion.Combate"),
		false,
		TEXT("Dibuja las esferas de golpe activas y las trazas de disparo (amarillo: sin golpe, rojo: golpe)."),
		ECVF_Default);

	bool HUDActivo()
	{
		return CVarHUD.GetValueOnGameThread();
	}

	void ActivarHUD(bool bActivo)
	{
		// Prioridad de consola, para que una tecla y un comando escrito se puedan pisar entre sí.
		CVarHUD->Set(bActivo, ECVF_SetByConsole);
	}

	bool DibujoCombateActivo()
	{
		return CVarCombate.GetValueOnGameThread();
	}
}
