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

	/**
	 * El campo de visión se abre al esprintar. Apagado, queda fijo (comodidad: el movimiento de la imagen
	 * con cada dash encadenado marea a algunos jugadores, auditoría ANIMVFX-13).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Campo de visión")
	bool bCampoVisionDinamico = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Campo de visión", meta = (ClampMin = "0"))
	float VelocidadInterpCampoVision = 6.f;

	/**
	 * Post-proceso dibujado (M_PP_Dibujado, una sola pasada barata para la Vega 8): bandas de luz
	 * planas con sombra fría, tinta por profundidad y por normales, niebla en capas y grano de papel.
	 * Nulo = sin él. Todos sus valores son parámetros del material.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo")
	TObjectPtr<UMaterialInterface> MaterialDibujado;

	/** Oscurece los bordes de la pantalla: centra la mirada y da tono sombrío. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo", meta = (ClampMin = "0", ClampMax = "1"))
	float IntensidadVineta = 0.55f;

	/** Saturación global (1 = sin cambio). Por debajo de 1, colores apagados de mundo en ruinas. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo", meta = (ClampMin = "0", ClampMax = "2"))
	float Saturacion = 0.8f;

	/** Desenfoque de movimiento (0 = ninguno): emborrona las líneas del dibujo y cuesta en la Vega 8. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo", meta = (ClampMin = "0", ClampMax = "1"))
	float DesenfoqueMovimiento = 0.f;

	/**
	 * Gradación de mundo en ruinas (dirección artística): las sombras tiran a azul frío y las luces a oro viejo,
	 * como una pintura antigua; ganancia por canal de cada zona tonal (1 = sin cambio).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo")
	FLinearColor TinteSombras = FLinearColor(0.9f, 0.96f, 1.12f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo")
	FLinearColor TinteLuces = FLinearColor(1.1f, 1.0f, 0.84f);

	/** Contraste de las sombras (más de 1 = negros más hondos, el tono oscuro del juego). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo", meta = (ClampMin = "0.5", ClampMax = "2"))
	float ContrasteSombras = 1.12f;

	/** Resplandor (bloom) suave en las luces fuertes: braseros, filo, ecos. 0 = apagado; barato a esta intensidad. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo", meta = (ClampMin = "0", ClampMax = "4"))
	float Resplandor = 0.45f;

	/** Desde qué brillo resplandece (más alto = solo lo muy brillante). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estilo", meta = (ClampMin = "0"))
	float UmbralResplandor = 1.2f;

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
	float SacudidaGolpeDado = 0.5f;

	/** Un aterrizaje más rápido que esto (cm/s) sacude la cámara, hasta SacudidaAterrizajeMaxima. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", ForceUnits = "cm/s", EditCondition = "bSacudida"))
	float VelocidadAterrizajeSacudida = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sacudida", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "bSacudida"))
	float SacudidaAterrizajeMaxima = 0.5f;

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
