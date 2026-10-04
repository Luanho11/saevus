// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSInfoZona.h"
#include "Engine/World.h"
#include "EngineUtils.h"

const ASAEVUSInfoZona* ASAEVUSInfoZona::Obtener(const UWorld* Mundo)
{
	if (!Mundo)
	{
		return nullptr;
	}
	TActorIterator<ASAEVUSInfoZona> Info(Mundo);
	return Info ? *Info : nullptr;
}
