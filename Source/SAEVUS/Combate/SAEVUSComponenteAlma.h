// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SAEVUSComponenteAlma.generated.h"

class USAEVUSComponenteCombate;
class USAEVUSComponenteMovimiento;
class USAEVUSComponenteVida;

DECLARE_MULTICAST_DELEGATE_OneParam(FSAEVUSEventoAlma, float /*Cantidad*/);

/** Valores del alma y la curación. Primera pasada; todo ajustable. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteAlma
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alma", meta = (ClampMin = "1"))
	float AlmaMaxima = 99.f;

	/** Alma que da cada actor golpeado por un ataque cuerpo a cuerpo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alma", meta = (ClampMin = "0"))
	float AlmaPorGolpe = 11.f;

	/** Alma que cuesta cada curación completa. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0"))
	float CosteCuracion = 33.f;

	/** Tiempo manteniendo la acción de curar hasta que la curación se completa. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float DuracionCuracion = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0"))
	float VidaPorCuracion = 25.f;
};

/**
 * Alma y curación, al estilo Hollow Knight: los golpes cuerpo a cuerpo llenan el alma, y mantener
 * la acción de curar en el suelo la gasta para recuperar vida. Mientras cura el personaje no se
 * mueve; recibir daño, dejar el suelo o soltar la acción interrumpe la curación sin gastar alma.
 * Mantenida, encadena curaciones mientras quede alma y falte vida.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteAlma : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteAlma();

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Alma")
	float ObtenerAlma() const { return Alma; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Alma")
	float ObtenerAlmaMaxima() const { return Ajuste.AlmaMaxima; }

	const FSAEVUSAjusteAlma& ObtenerAjuste() const { return Ajuste; }

	/** Suma alma hasta el máximo. */
	void GanarAlma(float Cantidad);

	/** Acción de curar pulsada (cierto) o soltada (falso). */
	void FijarCuracionPedida(bool bPedida);

	/** Corta la curación en curso sin gastar alma (saltos, ataques, dash, daño); hay que volver a pulsar. */
	void CancelarCuracion();

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Alma")
	bool EstaCurando() const { return bCurando; }

	/** Progreso de la curación en curso, de 0 a 1. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Alma")
	float ObtenerProgresoCuracion() const;

	/** Cierto si ahora mismo se podría empezar a curar. */
	bool PuedeCurar() const;

	/** Se emite al ganar alma y al completar una curación (con la vida recuperada). */
	FSAEVUSEventoAlma AlGanarAlma;
	FSAEVUSEventoAlma AlCurar;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Alma")
	FSAEVUSAjusteAlma Ajuste;

private:
	UFUNCTION()
	void AlGolpear(AActor* ActorGolpeado, float Danio);

	UFUNCTION()
	void AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador);

	void FijarCurando(bool bNuevo);
	/** Corta el progreso sin olvidar que la acción sigue pulsada. */
	void PausarCuracion();
	USAEVUSComponenteMovimiento* ObtenerMovimiento() const;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteVida> Vida;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteCombate> Combate;

	float Alma = 0.f;
	bool bCuracionPedida = false;
	bool bCurando = false;
	float TiempoCuracion = 0.f;
};
