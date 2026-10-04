// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSVolumenCambioMapa.h"
#include "Kismet/GameplayStatics.h"
#include "SAEVUS.h"

void ASAEVUSVolumenCambioMapa::AlEntrarJugador(ASAEVUSPersonaje* Personaje)
{
	if (Mapa.IsNull())
	{
		UE_LOG(LogSAEVUS, Warning, TEXT("%s no tiene mapa de destino"), *GetName());
		return;
	}
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Mapa);
}
