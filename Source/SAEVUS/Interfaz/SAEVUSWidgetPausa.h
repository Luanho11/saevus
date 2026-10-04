// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "SAEVUSWidgetPausa.generated.h"

class UButton;
class USAEVUSWidgetMapa;
class UTextBlock;
class UVerticalBox;

/** Opciones del menú de pausa: niveles de calidad y sensibilidades que se recorren al pulsar. */
namespace SAEVUSPausa
{
	/** Nivel de escalabilidad siguiente en la rotación Baja (0) → Media (1) → Alta (2) → Baja. */
	SAEVUS_API int32 SiguienteCalidad(int32 Actual);

	/** Sensibilidad siguiente en la rotación 0,5 → 0,75 → 1 → 1,5 → 2 → 0,5. */
	SAEVUS_API float SiguienteSensibilidad(float Actual);

	SAEVUS_API FText NombreCalidad(int32 Nivel);

	/** Volumen siguiente en la rotación 0 → 25 → 50 → 75 → 100 % → 0. */
	SAEVUS_API float SiguienteVolumen(float Actual);
}

/** Para qué se abre el menú: la pausa en partida o el menú principal al arrancar. */
UENUM()
enum class ESAEVUSModoMenu : uint8
{
	Pausa,
	Principal
};

/**
 * Menú boceto, construido en C++ como la interfaz, en dos modos con las mismas opciones:
 * - Pausa: continuar, volver al último punto de control, calidad, sensibilidad y salir
 *   (ASAEVUSControladorJugador::AlternarPausa).
 * - Principal: título, jugar, continuar desde el punto de control guardado (si lo hay para el
 *   mapa), calidad, sensibilidad y salir (ASAEVUSControladorJugador::MostrarMenuPrincipal).
 */
UCLASS()
class SAEVUS_API USAEVUSWidgetPausa : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	/** Resalta el botón con el foco (mando y teclado no tienen "pasar por encima"). */
	virtual void NativeTick(const FGeometry& Geometria, float DeltaTime) override;

public:
	/** Antes de añadirlo a la pantalla. */
	void FijarModo(ESAEVUSModoMenu NuevoModo) { Modo = NuevoModo; }
	ESAEVUSModoMenu ObtenerModo() const { return Modo; }

	/** El primer botón visible, para darle el foco al abrir (mando y teclado: cruceta o flechas y confirmar). */
	UWidget* ObtenerPrimerBoton() const;
	/** Cierto si algún botón del menú tiene el foco del jugador. */
	bool TieneFocoAlgunBoton() const;

private:
	void Construir();
	UButton* CrearBoton(UVerticalBox* Columna, FName Nombre, UTextBlock*& OutTexto);
	void ActualizarTextos();

	UFUNCTION()
	void AlContinuar();

	UFUNCTION()
	void AlReiniciar();

	UFUNCTION()
	void AlJugar();

	UFUNCTION()
	void AlContinuarPartida();

	UFUNCTION()
	void AlCambiarCalidad();

	UFUNCTION()
	void AlCambiarSensibilidad();

	UFUNCTION()
	void AlCambiarVolumen();

	UFUNCTION()
	void AlSalir();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoContinuar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoReiniciar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoCalidad;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoSensibilidad;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoVolumen;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoSalir;

	/** Mapa de la zona (solo en la pausa). */
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSWidgetMapa> Mapa;

	ESAEVUSModoMenu Modo = ESAEVUSModoMenu::Pausa;

	/** Botones en orden de la columna. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> Botones;

	/** Si cada botón tenía el foco el último fotograma (solo se cambia el estilo al cambiar). */
	TArray<bool> FocoBotones;
};
