// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSPalanca.generated.h"

class ASAEVUSPuertaArena;
class UBoxComponent;
class UStaticMeshComponent;

/**
 * Palanca (Hollow Knight): golpearla (tajo o disparo) la acciona una vez y abre sus puertas.
 * Sirve para atajos: una puerta sin enemigos asignados se queda cerrada hasta que una palanca
 * la abre. No da alma ni munición (el golpe no cuenta como daño hecho).
 */
UCLASS()
class SAEVUS_API ASAEVUSPalanca : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSPalanca();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Acciona la palanca (lo hace un golpe): abre las puertas, gira el brazo y muestra el aviso. */
	void Accionar();

	bool EstaAccionada() const { return bAccionada; }
	const TArray<TObjectPtr<ASAEVUSPuertaArena>>& ObtenerPuertas() const { return Puertas; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UBoxComponent> Caja;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Base;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USceneComponent> Pivote;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Brazo;

	UPROPERTY(EditInstanceOnly, Category = "SAEVUS|Palanca")
	TArray<TObjectPtr<ASAEVUSPuertaArena>> Puertas;

	/** Inclinación del brazo antes y después de accionarla (alabeo, en grados). */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Palanca")
	float AnguloReposo = -40.f;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Palanca")
	float AnguloAccionada = 40.f;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Palanca", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionGiro = 0.25f;

	/** Aviso al accionarla (vacío = ninguno). */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Palanca")
	FText TituloAlAccionar;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Palanca", meta = (MultiLine = "true"))
	FText TextoAlAccionar;

private:
	bool bAccionada = false;
	float TiempoGiro = 0.f;
};
