// Copyright SAEVUS. Todos los derechos reservados.

#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "HAL/IConsoleManager.h"

namespace SAEVUSDepuracion
{
	static TAutoConsoleVariable<bool> CVarHUD(
		TEXT("SAEVUS.Depuracion.HUD"),
		false,
		TEXT("Muestra el HUD de depuración de SAEVUS (paso, velocidad, modo de movimiento, combate, armas)."),
		ECVF_Cheat);

	static TAutoConsoleVariable<bool> CVarCombate(
		TEXT("SAEVUS.Depuracion.Combate"),
		false,
		TEXT("Dibuja las esferas de golpe activas y las trazas de disparo (amarillo: sin golpe, rojo: golpe)."),
		ECVF_Cheat);

	static TAutoConsoleVariable<bool> CVarNumerosDanio(
		TEXT("SAEVUS.Depuracion.NumerosDanio"),
		false,
		TEXT("Muestra números de daño flotantes sobre quien recibe el golpe (blancos: enemigos y muñecos; rojos: el jugador)."),
		ECVF_Cheat);

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

	bool NumerosDanioActivos()
	{
		return CVarNumerosDanio.GetValueOnGameThread();
	}

	void ActivarNumerosDanio(bool bActivo)
	{
		CVarNumerosDanio->Set(bActivo, ECVF_SetByConsole);
	}
}
