// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSArenaJefe.generated.h"

class ASAEVUSEnemigo;
class ASAEVUSPuertaArena;
class UBoxComponent;

/**
 * Arena de jefe: al entrar el jugador en su zona se cierran las puertas de entrada y el jefe despierta; el
 * combate empieza de verdad y no se puede abandonar. Si el jugador muere (el jefe vuelve a reposo) las puertas
 * se abren y la arena se rearma; si gana, se abren y quedan abiertas (auditoría EA-3).
 */
UCLASS()
class SAEVUS_API ASAEVUSArenaJefe : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSArenaJefe();

	/** Cierra las puertas y despierta al jefe (lo hace el solape del jugador; los tests lo llaman a mano). */
	void Activar();

	bool EstaActiva() const { return bActiva; }

	/** Asigna el jefe y las puertas de entrada (el script del mapa y los tests; en el editor son propiedades). */
	void Configurar(ASAEVUSEnemigo* NuevoJefe, const TArray<ASAEVUSPuertaArena*>& NuevasPuertas);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UBoxComponent> Zona;

	/** El jefe de la arena. */
	UPROPERTY(EditInstanceOnly, Category = "SAEVUS|Arena")
	TObjectPtr<ASAEVUSEnemigo> Jefe;

	/** Puertas que se cierran al empezar el combate (empiezan abiertas, sin enemigos asignados). */
	UPROPERTY(EditInstanceOnly, Category = "SAEVUS|Arena")
	TArray<TObjectPtr<ASAEVUSPuertaArena>> PuertasEntrada;

private:
	UFUNCTION()
	void AlSolapar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe);

	/** Abre las puertas y rearma la arena (el jugador murió) o la deja abierta (el jefe cayó). */
	void Terminar(bool bJefeVencido);

	bool bActiva = false;
	/** Para no mirar el estado del combate en cada fotograma. */
	float TiempoSinComprobar = 0.f;
};
