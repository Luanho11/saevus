// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Tutorial/SAEVUSVolumenCaida.h"
#include "SAEVUSPeligro.generated.h"

class UInstancedStaticMeshComponent;

/**
 * Pinchos (Hollow Knight): tocarlos quita vida y devuelve al jugador al borde (como una caída).
 * No dan alma ni reciben daño (el tajo hacia abajo y el rebote se retiraron). La caja es la zona
 * peligrosa; los pinchos visibles (conos del motor) la llenan según su tamaño.
 */
UCLASS()
class SAEVUS_API ASAEVUSPeligro : public ASAEVUSVolumenCaida
{
	GENERATED_BODY()

public:
	ASAEVUSPeligro();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UInstancedStaticMeshComponent> Pinchos;

	/** Separación entre pinchos. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Peligro", meta = (ClampMin = "10", ForceUnits = "cm"))
	float SeparacionPinchos = 60.f;

private:
	/** Coloca un pincho cada SeparacionPinchos sobre la base de la caja. */
	void ColocarPinchos();
};
