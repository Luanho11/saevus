// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "SAEVUSComponenteEstelaEspada.generated.h"

class USAEVUSComponenteAnimacionProcedural;
class USAEVUSComponenteCombate;

/** Cómo se ve la estela de la espada (REF-08 E: media luna blanca azulada que sigue al filo). */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteEstelaEspada
{
	GENERATED_BODY()

	/** Lo que tarda en desvanecerse cada tramo de la estela desde que la hoja pasa por él. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "0.01", ForceUnits = "s"))
	float VidaTramo = 0.14f;

	/** Desde dónde de la hoja (0 = guarda, 1 = punta) empieza la cinta: el borde interior de la media luna. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionInterior = 0.3f;

	/** Hasta dónde llega el borde exterior, en fracciones de la hoja (> 1: un poco más allá de la punta). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "0.1"))
	float FraccionExterior = 1.1f;

	/** El ataque cargado deja una estela más ancha y brillante (REF-08 E, "ataque cargado"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "1"))
	float EscalaCargado = 1.35f;

	/** Opacidad del borde interior respecto al exterior (la luz se concentra en el filo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "0", ClampMax = "1"))
	float OpacidadInterior = 0.f;

	/** Entre dos fotogramas se intercala un tramo cada estos grados de giro de la hoja: la media luna sale curva. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "1", ForceUnits = "deg"))
	float GradosPorTramo = 6.f;

	/** Tramos guardados como mucho: limita los triángulos (dos por tramo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela", meta = (ClampMin = "2"))
	int32 MaxTramos = 64;
};

/**
 * Estela de la espada en media luna (REF-08 E): mientras el tajo está en su fase activa guarda, cada fotograma,
 * la hoja (de la guarda a la punta, USAEVUSComponenteAnimacionProcedural::ObtenerHoja) y dibuja una cinta que
 * une esas posiciones y se desvanece en VidaTramo. Malla en espacio de mundo: la estela se queda en el aire donde
 * pasó la hoja. El material (aditivo, sin luz, de dos caras) toma la opacidad del color de vértice.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteEstelaEspada : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	explicit USAEVUSComponenteEstelaEspada(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Tramos vivos de la estela (para tests y depuración). */
	int32 ObtenerNumeroTramos() const { return Tramos.Num(); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Estela")
	FSAEVUSAjusteEstelaEspada AjusteEstela;

private:
	struct FTramo
	{
		FVector Base;
		FVector Punta;
		float Edad = 0.f;
		float Escala = 1.f;
	};

	void Reconstruir();

	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteCombate> Combate;

	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteAnimacionProcedural> Animacion;

	TArray<FTramo> Tramos;
	TArray<FVector> Vertices;
	TArray<int32> Triangulos;
	TArray<FLinearColor> Colores;
	/** Tramos con los que se creó la sección de malla (con los mismos solo se actualiza). */
	int32 NumTramosSeccion = 0;
};
