// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Tutorial/SAEVUSVolumenJugador.h"
#include "SAEVUSVolumenTutorial.generated.h"

/** Muestra un aviso de tutorial en pantalla cuando el jugador entra. */
UCLASS()
class SAEVUS_API ASAEVUSVolumenTutorial : public ASAEVUSVolumenJugador
{
	GENERATED_BODY()

public:
	const FText& ObtenerTitulo() const { return Titulo; }

protected:
	virtual void AlEntrarJugador(ASAEVUSPersonaje* Personaje) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Tutorial")
	FText Titulo;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Tutorial", meta = (MultiLine = "true"))
	FText Texto;

	/** Segundos en pantalla. 0 = hasta el siguiente aviso. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Tutorial", meta = (ClampMin = "0", ForceUnits = "s"))
	float Duracion = 9.f;
};
