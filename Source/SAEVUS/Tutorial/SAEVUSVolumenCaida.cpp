// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSVolumenCaida.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Personaje/SAEVUSPersonaje.h"

ASAEVUSVolumenCaida::ASAEVUSVolumenCaida()
{
	bSoloUnaVez = false;
}

void ASAEVUSVolumenCaida::AlEntrarJugador(ASAEVUSPersonaje* Personaje)
{
	USAEVUSComponenteVida* Vida = Personaje->ObtenerVida();
	if (Vida->EstaMuerto())
	{
		return;
	}
	// Sin causante: el daño de caída no empuja al jugador.
	Vida->AplicarDanio(DanioCaida, nullptr);
	if (Vida->EstaMuerto())
	{
		return;
	}
	Personaje->GetCharacterMovement()->StopMovementImmediately();
	Personaje->SetActorLocation(GetActorTransform().TransformPosition(PuntoRetorno), false, nullptr, ETeleportType::TeleportPhysics);
}
