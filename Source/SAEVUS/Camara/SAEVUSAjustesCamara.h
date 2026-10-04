// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "SAEVUSAjustesCamara.generated.h"

/**
 * Ajuste de la cámara de seguimiento en tercera persona. Los inicializadores son la única
 * fuente de valores por defecto; un USAEVUSAjustesCamara los sobrescribe.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteCamara
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encuadre", meta = (ClampMin = "0", ForceUnits = "cm"))
	float LongitudBrazo = 400.f;

	/** Desplazamiento en el extremo del brazo. Y positivo saca al personaje del centro (sobre el hombro). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encuadre")
	FVector DesplazamientoExtremo = FVector(0.f, 50.f, 60.f);

	/** Radio de la sonda de colisión que acerca la cámara delante de las paredes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encuadre", meta = (ClampMin = "0", ForceUnits = "cm"))
	float TamanoSonda = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Retraso")
	bool bRetrasoPosicion = true;

	/** Valores altos siguen al personaje más de cerca. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Retraso", meta = (ClampMin = "0", EditCondition = "bRetrasoPosicion"))
	float VelocidadRetrasoPosicion = 12.f;

	/** Límite de cuánto puede quedarse atrás la cámara. 0 = sin límite. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Retraso", meta = (ClampMin = "0", ForceUnits = "cm", EditCondition = "bRetrasoPosicion"))
	float DistanciaMaximaRetraso = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Campo de visión", meta = (ClampMin = "5", ClampMax = "170", ForceUnits = "deg"))
	float CampoVisionBase = 90.f;

	/** Campo de visión a velocidad de esprint; se mezcla desde el base por encima de la velocidad de carrera. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Campo de visión", meta = (ClampMin = "5", ClampMax = "170", ForceUnits = "deg"))
	float CampoVisionEsprint = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Campo de visión", meta = (ClampMin = "0"))
	float VelocidadInterpCampoVision = 6.f;

	/**
	 * Post-proceso de contorno dibujado (M_PP_Contorno, por profundidad: 5 lecturas, apto para la
	 * Vega 8). Nulo = sin contorno. Grosor, umbral y color son parámetros del material.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo")
	TObjectPtr<UMaterialInterface> MaterialContorno;

	/** Oscurece los bordes de la pantalla: centra la mirada y da tono sombrío. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo", meta = (ClampMin = "0", ClampMax = "1"))
	float IntensidadVineta = 0.55f;

	/** Saturación global (1 = sin cambio). Por debajo de 1, colores apagados de mundo en ruinas. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo", meta = (ClampMin = "0", ClampMax = "2"))
	float Saturacion = 0.8f;

	/** Sacudida de cámara en los impactos (modelo de "trauma": cada evento suma; la sacudida es su cuadrado). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida")
	bool bSacudida = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", ForceUnits = "cm", EditCondition = "bSacudida"))
	float DesplazamientoMaxSacudida = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", ForceUnits = "deg", EditCondition = "bSacudida"))
	float GiroMaxSacudida = 2.f;

	/** Trauma que se pierde por segundo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", EditCondition = "bSacudida"))
	float RecuperacionSacudida = 2.5f;

	/** Rapidez del ruido de la sacudida (ciclos por segundo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", EditCondition = "bSacudida"))
	float FrecuenciaSacudida = 25.f;

	/** Trauma por evento (0..1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "bSacudida"))
	float SacudidaGolpeDado = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "bSacudida"))
	float SacudidaGolpeRecibido = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "bSacudida"))
	float SacudidaBloqueo = 0.4f;
};

/** Perfil de cámara editable asignado a USAEVUSComponenteCamara. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSAjustesCamara : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camara")
	FSAEVUSAjusteCamara Camara;
};
