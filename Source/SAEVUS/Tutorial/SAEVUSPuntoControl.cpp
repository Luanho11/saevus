// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSPuntoControl.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Engine/World.h"
#include "Nucleo/SAEVUSModoJuego.h"
#include "Personaje/SAEVUSPersonaje.h"

void ASAEVUSPuntoControl::AlEntrarJugador(ASAEVUSPersonaje* Personaje)
{
	if (ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>())
	{
		ModoJuego->FijarPuntoControl(GetActorTransform());
	}
	// Descansar en un punto de control rellena la munición.
	if (USAEVUSComponenteArmas* Armas = Personaje ? Personaje->ObtenerArmas() : nullptr)
	{
		Armas->RellenarMunicion();
	}
}
