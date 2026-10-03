// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Efectos/SAEVUSComponenteParticulas.h"
#include "SAEVUSComponenteEfectosCombate.generated.h"

class USAEVUSComponenteAlma;
class USAEVUSComponenteCombate;

/** Cómo se ven los efectos de golpe, alma y curación. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteEfectosCombate
{
	GENERATED_BODY()

	/** Chispas en el actor golpeado por un ataque cuerpo a cuerpo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0"))
	int32 ParticulasGolpe = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadGolpe = 350.f;

	/** Destello que sube del cuerpo al completar una curación. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0"))
	int32 ParticulasCuracion = 24;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadCuracion = 220.f;

	/** Partículas por segundo que se juntan alrededor del cuerpo mientras cura. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0"))
	float ParticulasPorSegundoCurando = 30.f;

	/** Radio del que salen las partículas que convergen al curar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioConvergencia = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Dispersion = 140.f;
};

/**
 * Partículas de combate del personaje: chispas al golpear, partículas que convergen mientras
 * cura y un destello al completar la curación. Escucha USAEVUSComponenteCombate::AlGolpear y los
 * eventos de USAEVUSComponenteAlma del dueño.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteEfectosCombate : public USAEVUSComponenteParticulas
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Efectos")
	FSAEVUSAjusteEfectosCombate AjusteEfectos;

private:
	UFUNCTION()
	void AlGolpear(AActor* ActorGolpeado, float Danio);

	void AlCurar(float VidaRecuperada);

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteAlma> Alma;

	FDelegateHandle ManejadorCurar;
	float AcumuladoEmision = 0.f;
};
