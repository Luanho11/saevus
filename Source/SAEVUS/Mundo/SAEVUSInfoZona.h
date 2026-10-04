// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "SAEVUSInfoZona.generated.h"

/** Datos de la zona de un mapa (uno por mapa): el nombre que la interfaz muestra al entrar. */
UCLASS()
class SAEVUS_API ASAEVUSInfoZona : public AInfo
{
	GENERATED_BODY()

public:
	/** La del mundo, o nula si el mapa no tiene. */
	static const ASAEVUSInfoZona* Obtener(const UWorld* Mundo);

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Zona")
	FText Nombre;

	/** Línea pequeña bajo el nombre ("Zona 1"). */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Zona")
	FText Subtitulo;
};
