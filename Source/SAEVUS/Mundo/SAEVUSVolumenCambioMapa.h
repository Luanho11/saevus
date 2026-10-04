// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Tutorial/SAEVUSVolumenJugador.h"
#include "SAEVUSVolumenCambioMapa.generated.h"

/**
 * Al entrar el jugador, abre otro mapa (paso del tutorial a la Zona 1, entre zonas). Con Llegada,
 * el jugador aparece en el APlayerStart del otro mapa con esa etiqueta (opción "portal" de UE).
 */
UCLASS()
class SAEVUS_API ASAEVUSVolumenCambioMapa : public ASAEVUSVolumenJugador
{
	GENERATED_BODY()

public:
	const TSoftObjectPtr<UWorld>& ObtenerMapa() const { return Mapa; }
	FName ObtenerLlegada() const { return Llegada; }

protected:
	virtual void AlEntrarJugador(ASAEVUSPersonaje* Personaje) override;

	UPROPERTY(EditInstanceOnly, Category = "SAEVUS|Mapa")
	TSoftObjectPtr<UWorld> Mapa;

	/** Etiqueta (PlayerStartTag) del inicio del otro mapa donde aparecer; vacía = el inicio normal. */
	UPROPERTY(EditInstanceOnly, Category = "SAEVUS|Mapa")
	FName Llegada;
};
