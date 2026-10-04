// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Mundo/SAEVUSInteractuable.h"
#include "SAEVUSBanco.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/**
 * Banco (Hollow Knight): sentarse con la acción de interactuar cura del todo, rellena la munición,
 * fija el punto de control (se reaparece delante del banco) y lo guarda. Levantarse: moverse,
 * saltar, atacar o volver a interactuar.
 */
UCLASS()
class SAEVUS_API ASAEVUSBanco : public AActor, public ISAEVUSInteractuable
{
	GENERATED_BODY()

public:
	ASAEVUSBanco();

	//~ Inicio de ISAEVUSInteractuable
	virtual FText ObtenerTextoInteraccion() const override;
	virtual void Interactuar(ASAEVUSPersonaje* Personaje) override;
	//~ Fin de ISAEVUSInteractuable

	/** Donde reaparece el jugador: delante del banco, mirando hacia donde mira el banco. */
	FTransform ObtenerPuntoReaparicion() const;

	int32 ObtenerUsos() const { return Usos; }

protected:
	virtual void BeginPlay() override;

	/** Zona en la que se puede sentar. */
	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UBoxComponent> Zona;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Asiento;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Respaldo;

	/** Distancia por delante del banco a la que se reaparece. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Banco", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaReaparicion = 120.f;

private:
	UFUNCTION()
	void AlEntrar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe);

	UFUNCTION()
	void AlSalir(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice);

	int32 Usos = 0;
};
