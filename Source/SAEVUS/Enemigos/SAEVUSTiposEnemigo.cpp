// Copyright SAEVUS. Todos los derechos reservados.

#include "Enemigos/SAEVUSTiposEnemigo.h"

#define LOCTEXT_NAMESPACE "SAEVUSEnemigos"

FSAEVUSAjusteEnemigo::FSAEVUSAjusteEnemigo()
{
	// Mini enemigo por defecto: un rastrero bajo que avisa y embiste. Valores de primera pasada.
	NombreVisible = LOCTEXT("Rastrero", "Rastrero");
	FSAEVUSAtaqueEnemigo Embestida;
	Ataques = { Embestida };
}

#undef LOCTEXT_NAMESPACE
