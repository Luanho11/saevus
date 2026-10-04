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
	const FString Opciones = Llegada.IsNone() ? FString() : FString::Printf(TEXT("portal=%s"), *Llegada.ToString());
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Mapa, true, Opciones);
}
