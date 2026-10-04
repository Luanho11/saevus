// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSVolumenTutorial.h"
#include "Engine/World.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"

void ASAEVUSVolumenTutorial::AlEntrarJugador(ASAEVUSPersonaje* Personaje)
{
	if (USAEVUSSubsistemaTutorial* Tutorial = GetWorld()->GetSubsystem<USAEVUSSubsistemaTutorial>())
	{
		Tutorial->MostrarMensaje(Titulo, Texto, Duracion);
	}
}
