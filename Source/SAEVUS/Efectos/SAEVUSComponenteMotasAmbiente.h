// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Efectos/SAEVUSComponenteParticulas.h"
#include "SAEVUSComponenteMotasAmbiente.generated.h"

/** Cuántas motas hay y cómo flotan. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteMotas
{
	GENERATED_BODY()

	/** Motas vivas alrededor del dueño (densidad del polvo en el aire). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motas", meta = (ClampMin = "0", ClampMax = "256"))
	int32 Cantidad = 70;

	/** Caja alrededor del dueño en la que nacen (medio lado horizontal y altura). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motas", meta = (ClampMin = "0", ForceUnits = "cm"))
	float Radio = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motas", meta = (ClampMin = "0", ForceUnits = "cm"))
	float Altura = 450.f;

	/** Deriva: velocidad máxima de cada mota (muy lenta). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motas", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Deriva = 14.f;
};

/**
 * Polvo en suspensión alrededor del jugador: motas diminutas y cálidas que flotan y se apagan, como en la luz
 * de una nave de catedral en ruinas (dirección artística). Partículas por CPU del pool común, sin translucidez.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteMotasAmbiente : public USAEVUSComponenteParticulas
{
	GENERATED_BODY()

public:
	USAEVUSComponenteMotasAmbiente();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Motas")
	FSAEVUSAjusteMotas AjusteMotas;
};
