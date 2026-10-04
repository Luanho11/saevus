// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSProyectil.generated.h"

class UMaterialInterface;
class USphereComponent;
class UStaticMeshComponent;

/**
 * Proyectil de enemigo: vuela recto, choca con el escenario y hace daño al jugador al tocarlo
 * (la guardia lo para si llega de frente). Un tajo lo devuelve: invierte su dirección, va más
 * rápido y entonces daña a los enemigos. Sin físicas: se mueve con barrido en su tick.
 */
UCLASS()
class SAEVUS_API ASAEVUSProyectil : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSProyectil();

	/** Lo lanza en Direccion a Velocidad con Danio; Lanzador no recibe su propio proyectil. */
	void Lanzar(const FVector& Direccion, float NuevaVelocidad, float NuevoDanio, AActor* Lanzador, UMaterialInterface* Material);

	bool FueDevuelto() const { return bDevuelto; }
	const FVector& ObtenerVelocidad() const { return Velocidad; }

	/** Un golpe (tajo del jugador) lo devuelve. No cuenta como daño: devuelve 0. */
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USphereComponent> Esfera;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Malla;

	/** Segundos de vida como mucho. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Proyectil", meta = (ClampMin = "0.1", ForceUnits = "s"))
	float Duracion = 4.f;

	/** Multiplica la velocidad al devolverlo. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Proyectil", meta = (ClampMin = "0"))
	float EscalaVelocidadDevuelto = 1.6f;

private:
	UFUNCTION()
	void AlSolapar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe);

	void Impactar(AActor* Objetivo);

	FVector Velocidad = FVector::ZeroVector;
	float Danio = 0.f;
	float Edad = 0.f;
	bool bDevuelto = false;
	TWeakObjectPtr<AActor> Lanzador;
};
