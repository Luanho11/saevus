// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSParedRompible.generated.h"

class UStaticMeshComponent;

/**
 * Pared falsa (Hollow Knight): un bloque que cae tras GolpesParaRomper golpes (tajos o disparos)
 * y deja ver lo que esconde. Cada golpe tiembla y suena; no da alma ni munición.
 */
UCLASS()
class SAEVUS_API ASAEVUSParedRompible : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSParedRompible();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Tick(float DeltaSeconds) override;

	bool EstaRota() const { return bRota; }
	int32 ObtenerGolpes() const { return Golpes; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Malla;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Pared", meta = (ClampMin = "1"))
	int32 GolpesParaRomper = 3;

	/** Temblor de cada golpe: desplazamiento y duración. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Pared", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AmplitudTemblor = 6.f;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Pared", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionTemblor = 0.12f;

private:
	void Romper();

	int32 Golpes = 0;
	bool bRota = false;
	float TiempoTemblor = 0.f;
	/** Posición quieta: el temblor se aplica sobre ella (la malla es la raíz). */
	FVector PosicionReposo = FVector::ZeroVector;
};
