// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Tutorial/SAEVUSVolumenJugador.h"
#include "SAEVUSVolumenCaida.generated.h"

/**
 * Fondo de un foso (Hollow Knight): caer quita DanioCaida y devuelve al jugador a PuntoRetorno
 * (normalmente el borde del que saltó), sin pasar por la muerte salvo que el daño la cause.
 */
UCLASS()
class SAEVUS_API ASAEVUSVolumenCaida : public ASAEVUSVolumenJugador
{
	GENERATED_BODY()

public:
	ASAEVUSVolumenCaida();

	/** Fija el punto de retorno en coordenadas de mundo. */
	void FijarPuntoRetorno(const FVector& PuntoMundo) { PuntoRetorno = GetActorTransform().InverseTransformPosition(PuntoMundo); }

protected:
	virtual void AlEntrarJugador(ASAEVUSPersonaje* Personaje) override;

	/** Dónde reaparece el jugador tras caer (relativo al volumen; se edita con su manejador). */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Caida", meta = (MakeEditWidget = "true"))
	FVector PuntoRetorno = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Caida", meta = (ClampMin = "0"))
	float DanioCaida = 10.f;
};
