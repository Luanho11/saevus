// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Interfaz/SAEVUSEstadoInterfaz.h"
#include "SAEVUSWidgetInterfaz.generated.h"

class UBorder;
class UPanelWidget;
class UCanvasPanel;
class UHorizontalBox;
class UImage;
class UProgressBar;
class UTextBlock;
class UVerticalBox;

/**
 * Aspecto de la interfaz de juego según REF-08 A (Documentos/ARTE_SAEVUS.md, UI): emblema en rombo con marco
 * fino, barra de vida roja larga y fina, barra de alma azul debajo, paneles azul noche; todo aquí para
 * cambiarlo sin tocar el código. Colores lineales medidos en la referencia.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteInterfaz
{
	GENERATED_BODY()

	/** Vida que vale cada máscara (Hollow Knight muestra la vida en máscaras). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vida", meta = (ClampMin = "1"))
	float VidaPorMascara = 20.f;

	/** Barra de vida (REF-08 A): larga y fina, roja (#964843), sobre fondo oscuro con un marco gris fino. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vida")
	FVector2D TamanoBarraVida = FVector2D(380.f, 11.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vida")
	FLinearColor ColorVida = FLinearColor(0.30f, 0.06f, 0.056f);

	/** Lado del emblema en rombo (antes de girarlo 45°), con su marco y el ojo cian del protagonista. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vida")
	float LadoEmblema = 46.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vida")
	FLinearColor ColorOjoEmblema = FLinearColor(0.16f, 0.65f, 0.79f);

	/** Barra del alma (recurso), más corta y fina, azul (#2B4A62 aclarado para leerse). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alma")
	FVector2D TamanoBarraAlma = FVector2D(260.f, 6.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alma")
	FLinearColor ColorAlma = FLinearColor(0.078f, 0.21f, 0.38f);

	/** Color del alma cuando alcanza para curar: brilla más. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Alma")
	FLinearColor ColorAlmaLista = FLinearColor(0.35f, 0.65f, 0.95f);

	/** Marco fino gris de barras y emblema (#40464D) y fondo de las barras. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FLinearColor ColorMarco = FLinearColor(0.05f, 0.06f, 0.074f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FLinearColor ColorFondoBarra = FLinearColor(0.004f, 0.006f, 0.009f, 0.85f);

	/** Texto de títulos del HUD (nombre del jefe): hueso #DFC6AD (REF-08 B). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FLinearColor ColorTextoHueso = FLinearColor(0.74f, 0.56f, 0.42f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FLinearColor ColorFondo = FLinearColor(0.02f, 0.02f, 0.03f, 0.75f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FLinearColor ColorTexto = FLinearColor(0.85f, 0.9f, 1.f);

	/** Margen de los bloques respecto al borde de la pantalla (unidades de interfaz a 1080p). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General")
	FVector2D Margen = FVector2D(48.f, 40.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textos", meta = (ClampMin = "6"))
	int32 TamanoTituloAviso = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textos", meta = (ClampMin = "6"))
	int32 TamanoTextoAviso = 18;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Textos", meta = (ClampMin = "6"))
	int32 TamanoTextoArma = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Avisos", meta = (ClampMin = "0", ForceUnits = "s"))
	float FundidoAviso = 0.3f;

	/** Altura del aviso del tutorial como fracción de la pantalla (desde arriba). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Avisos", meta = (ClampMin = "0", ClampMax = "1"))
	float AlturaAviso = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jefe")
	FVector2D TamanoBarraJefe = FVector2D(640.f, 9.f);

	/** Título de zona al entrar (Hollow Knight): espera, fundido de entrada, tiempo a la vista y fundido de salida. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zona", meta = (ClampMin = "0", ForceUnits = "s"))
	float RetardoTituloZona = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zona", meta = (ClampMin = "0.01", ForceUnits = "s"))
	float FundidoTituloZona = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zona", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionTituloZona = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zona", meta = (ClampMin = "6"))
	int32 TamanoTituloZona = 64;

	/** Altura del título de zona como fracción de la pantalla (desde arriba); por debajo del aviso del tutorial. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zona", meta = (ClampMin = "0", ClampMax = "1"))
	float AlturaTituloZona = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jefe")
	FLinearColor ColorVidaJefe = FLinearColor(0.30f, 0.06f, 0.056f);

	/** Pantalla de muerte: opacidad del negro, y tiempos del fundido a negro y de vuelta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Muerte", meta = (ClampMin = "0", ClampMax = "1"))
	float OpacidadMuerte = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Muerte", meta = (ClampMin = "0.01", ForceUnits = "s"))
	float FundidoMuerte = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Muerte", meta = (ClampMin = "0.01", ForceUnits = "s"))
	float FundidoReaparicion = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Muerte", meta = (ClampMin = "6"))
	int32 TamanoTextoMuerte = 44;
};

namespace SAEVUSInterfaz
{
	/** Opacidad del título de zona a los Segundos de entrar: 0, sube, se queda a 1 y baja a 0. */
	SAEVUS_API float OpacidadTituloZona(float Segundos, const FSAEVUSAjusteInterfaz& Ajuste);
}

/**
 * Interfaz de juego: máscaras de vida y orbe de alma (arriba a la izquierda), arma y munición
 * (abajo a la derecha), aviso del tutorial (arriba al centro) y barra del jefe activo (abajo al
 * centro). El árbol de widgets se construye en C++ con formas redondeadas de Slate, sin
 * texturas ni assets; escala con las reglas de DPI del proyecto. Una subclase Blueprint puede
 * sustituirlo cuando haya dirección de arte.
 */
UCLASS()
class SAEVUS_API USAEVUSWidgetInterfaz : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Muestra Estado (lo llama el tick; expuesto para pruebas y previsualización). */
	void Aplicar(const FSAEVUSEstadoInterfaz& Estado);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& Geometria, float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Interfaz")
	FSAEVUSAjusteInterfaz Ajuste;

private:
	void Construir();
	/** Barra fina con marco (REF-08 A); devuelve la barra, ya dentro de su marco en Caja. */
	UProgressBar* CrearBarra(FName Nombre, UPanelWidget* Caja, const FVector2D& Tamano, const FLinearColor& Color);
	UTextBlock* CrearTexto(FName Nombre, int32 Tamano, bool bCentrado);
	/** Título: letras espaciadas en mayúsculas. */
	UTextBlock* CrearTitulo(FName Nombre, int32 Tamano);
	/** Rehace las líneas de Caja con Texto: una fila por línea y las [teclas] dibujadas como teclas. */
	void RellenarLineas(class UVerticalBox* Caja, const FString& Texto, int32 Tamano);

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> BarraVida;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> BarraAlma;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoArma;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoMunicion;

	/** Lo que se puede usar cerca ("[E]  Sentarse"), con la tecla dibujada. */
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> CajaInteraccion;

	FString TextoInteraccionMostrado;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> BloqueTituloZona;

	/** Tiempo de mundo al construirse la interfaz: de ahí cuenta el título de zona (se para con la pausa). */
	double TiempoEntrada = 0.0;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> BloqueAviso;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TituloAviso;

	/** Líneas del aviso (con teclas), dentro de un panel translúcido. */
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> LineasAviso;

	FString TextoAvisoMostrado;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> BloqueJefe;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NombreJefe;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> BarraJefe;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FaseJefe;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> VeloMuerte;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoMuerte;

	/** Opacidad actual del velo de muerte (sube y baja con los fundidos). */
	float OpacidadVelo = 0.f;
};
