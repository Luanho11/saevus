// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSPlataformaMovil.generated.h"

class UStaticMeshComponent;

/**
 * Plataforma que va y viene entre su posición inicial y Recorrido (relativo), con una pausa en
 * cada extremo y aceleración suave. Lo que está encima viaja con ella (el movimiento de personaje
 * sigue a su base).
 */
UCLASS()
class SAEVUS_API ASAEVUSPlataformaMovil : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSPlataformaMovil();

	virtual void Tick(float DeltaSeconds) override;

	/** Posición en el instante Tiempo desde que empezó (función pura de los datos: se prueba sin mundo). */
	FVector PosicionEn(float Tiempo) const;

	/** Recorrido, velocidad y pausa de una vez (pruebas). */
	void Configurar(const FVector& NuevoRecorrido, float NuevaVelocidad, float NuevaPausa)
	{
		Recorrido = NuevoRecorrido;
		Velocidad = NuevaVelocidad;
		PausaExtremo = NuevaPausa;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Malla;

	/** Desplazamiento hasta el otro extremo, en espacio del mundo. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Plataforma")
	FVector Recorrido = FVector(400.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Plataforma", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float Velocidad = 200.f;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Plataforma", meta = (ClampMin = "0", ForceUnits = "s"))
	float PausaExtremo = 0.8f;

private:
	FVector Inicio = FVector::ZeroVector;
	float Tiempo = 0.f;
};
