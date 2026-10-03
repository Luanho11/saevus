// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Efectos/SAEVUSComponenteParticulas.h"
#include "SAEVUSComponenteEstelaDash.generated.h"

class USAEVUSComponenteMovimiento;

/** Cómo se ve la estela del dash. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteEstelaDash
{
	GENERATED_BODY()

	/** Partículas de golpe al empezar el dash (el "estallido" de salida). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "0"))
	int32 ParticulasRafaga = 14;

	/** Partículas por segundo mientras dura el dash. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "0"))
	float ParticulasPorSegundo = 90.f;

	/** Velocidad hacia atrás (contraria al dash) con la que salen las partículas. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadRetroceso = 250.f;

	/** Dispersión aleatoria de la velocidad de cada partícula. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Dispersion = 160.f;

	/** Altura de emisión respecto al centro de la cápsula (negativa = cerca de los pies). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ForceUnits = "cm"))
	float AlturaEmision = -70.f;

	/** Altura de la bocanada del doble salto respecto al centro de la cápsula (bajo los pies). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Doble salto", meta = (ForceUnits = "cm"))
	float AlturaEmisionSaltoAereo = -90.f;
};

/**
 * Estela de partículas del dash, al estilo GunZ: un estallido al empezar y un rastro mientras
 * dura, y una bocanada bajo los pies en el doble salto. Escucha AlIniciarDash y AlSaltoAereo
 * del USAEVUSComponenteMovimiento del personaje dueño.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteEstelaDash : public USAEVUSComponenteParticulas
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Estela")
	FSAEVUSAjusteEstelaDash AjusteEstela;

private:
	void AlIniciarDash(const FVector& Direccion);
	void AlSaltoAereo();
	FVector ObtenerPuntoEmision() const;
	USAEVUSComponenteMovimiento* ObtenerMovimiento() const;

	FDelegateHandle ManejadorDash;
	FDelegateHandle ManejadorSaltoAereo;
	/** Fracción de partícula pendiente entre fotogramas, así el ritmo no depende de los fps. */
	float AcumuladoEmision = 0.f;
};
