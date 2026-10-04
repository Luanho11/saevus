// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSPuntoControl.h"
#include "Engine/World.h"
#include "Nucleo/SAEVUSModoJuego.h"

void ASAEVUSPuntoControl::AlEntrarJugador(ASAEVUSPersonaje* Personaje)
{
	if (ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>())
	{
		ModoJuego->FijarPuntoControl(GetActorTransform());
	}
}
