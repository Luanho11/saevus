// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSFragmentoMascara.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * Fragmento de máscara (Hollow Knight): al tocarlo, el jugador lo recoge; cada pocos fragmentos
 * (ASAEVUSPersonaje, FSAEVUSAjusteProgreso) suma una máscara de vida. Se guarda por Identificador:
 * uno recogido ya no aparece.
 */
UCLASS()
class SAEVUS_API ASAEVUSFragmentoMascara : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSFragmentoMascara();

	virtual void Tick(float DeltaSeconds) override;

	/** Identificador con el que se guarda: el puesto a mano o, sin él, uno derivado del mapa y del actor. */
	FName ObtenerIdentificador() const;
	void FijarIdentificador(FName Nuevo) { Identificador = Nuevo; }

	/** Lo recoge Personaje (lo llama el solape; los tests, a mano). */
	void Recoger(class ASAEVUSPersonaje* Personaje);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USphereComponent> Zona;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Malla;

	/** Único en todo el juego: con él se guarda que ya se recogió. */
	UPROPERTY(EditInstanceOnly, Category = "SAEVUS|Fragmento")
	FName Identificador;

	/** Giro lento para que se vea desde lejos (grados por segundo). */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Fragmento")
	float VelocidadGiro = 90.f;

private:
	UFUNCTION()
	void AlEntrar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe);
};
