// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SAEVUSComponenteEscudo.generated.h"

/**
 * Escudo de un enemigo (FSAEVUSAjusteEnemigo::AnguloEscudo): filtra el daño de su vida y bloquea
 * lo que llega de frente. Un golpe fuerte lo rompe un rato. Sin ángulo no hace nada.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteEscudo : public UActorComponent
{
	GENERATED_BODY()

public:
	bool EstaRoto() const;
	int32 ObtenerBloqueos() const { return Bloqueos; }

	/** Daño que pasa del golpe de Causante: 0 si lo bloquea. */
	float Filtrar(float Cantidad, AActor* Causante);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	double RotoHasta = -1.0;
	int32 Bloqueos = 0;
};
