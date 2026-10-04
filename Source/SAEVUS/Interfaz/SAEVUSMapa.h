// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SAEVUSMapa.generated.h"

class UCanvasPanel;

/** Un rectángulo del mapa: huella en planta de un bloque y la altura de su cara de arriba. */
struct FSAEVUSBloqueMapa
{
	FBox2D Caja = FBox2D(ForceInit);
	float Altura = 0.f;
};

/** Cómo pasar de coordenadas del mundo (X, Y) a píxeles del mapa: Pixel = Desplazamiento + (Punto - Origen) * Escala. */
struct FSAEVUSEncuadreMapa
{
	FVector2D Origen = FVector2D::ZeroVector;
	float Escala = 1.f;
	FVector2D Desplazamiento = FVector2D::ZeroVector;

	FVector2D APixel(const FVector2D& Punto) const { return Desplazamiento + (Punto - Origen) * Escala; }
};

/** El mapa se calcula aparte del widget: se prueba sin pantalla. */
namespace SAEVUSMapa
{
	/** Etiqueta de los actores que no salen en el mapa (fondos de fosos). */
	SAEVUS_API extern const FName EtiquetaSinMapa;

	/** Bloques con colisión del mundo (mallas estáticas), sin los marcados con EtiquetaSinMapa. */
	SAEVUS_API TArray<FSAEVUSBloqueMapa> RecogerBloques(const UWorld* Mundo);

	/** Encuadre que mete Limites en un rectángulo de Tamano píxeles con Margen, centrado y sin deformar. */
	SAEVUS_API FSAEVUSEncuadreMapa Encuadrar(const FBox2D& Limites, const FVector2D& Tamano, float Margen);
}

/**
 * Mapa boceto de la zona en la pausa: vista cenital generada de la geometría (más claro = más
 * alto), puertas cerradas, puntos de control y el jugador. Solo lo explorado
 * (USAEVUSSubsistemaExploracion); el encuadre es el de toda la zona. Se rehace al abrir la pausa.
 */
UCLASS()
class SAEVUS_API USAEVUSWidgetMapa : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Vuelve a dibujar el mapa con el estado actual del mundo. */
	void Refrescar();

	static const FVector2D Tamano;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void Construir();
	void AnadirRectangulo(const FVector2D& Posicion, const FVector2D& TamanoRectangulo, const FLinearColor& Color, bool bRedondo = false);

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> Lienzo;
};
