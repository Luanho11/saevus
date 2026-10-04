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
	if (Llegada.IsNone())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, Mapa);
		return;
	}
	// UE elige el inicio por el "portal" de la URL (mapa#Llegada); la opción ?portal= le dice al modo
	// de juego que se llegó por un paso.
	const FString Destino = FString::Printf(TEXT("%s#%s"), *Mapa.GetLongPackageName(), *Llegada.ToString());
	UGameplayStatics::OpenLevel(this, FName(*Destino), true, FString::Printf(TEXT("portal=%s"), *Llegada.ToString()));
}
