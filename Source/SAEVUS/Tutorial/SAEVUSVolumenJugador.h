// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSVolumenJugador.generated.h"

class UBoxComponent;
class ASAEVUSPersonaje;

/**
 * Caja que reacciona cuando el jugador entra. Base de los volúmenes del tutorial (avisos,
 * puntos de control). El tamaño es la extensión de la caja.
 */
UCLASS(Abstract)
class SAEVUS_API ASAEVUSVolumenJugador : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSVolumenJugador();

	/** Lo que pasa cuando el jugador entra (lo llama el solapamiento; los tests, a mano). */
	void NotificarEntrada(ASAEVUSPersonaje* Personaje);

	bool HaSidoUsado() const { return bUsado; }

protected:
	virtual void BeginPlay() override;

	/** Reacción concreta de cada volumen. */
	virtual void AlEntrarJugador(ASAEVUSPersonaje* Personaje) {}

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UBoxComponent> Caja;

	/** Solo reacciona la primera vez. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Volumen")
	bool bSoloUnaVez = true;

private:
	UFUNCTION()
	void AlSolapar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe);

	bool bUsado = false;
};
