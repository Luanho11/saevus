// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "SAEVUSWidgetPausa.generated.h"

class UButton;
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
}

/**
 * Menú de pausa boceto, construido en C++ como la interfaz: continuar, reiniciar en el último
 * punto de control, calidad gráfica, sensibilidad del ratón y salir. Lo abre y cierra el
 * controlador (ASAEVUSControladorJugador::AlternarPausa).
 */
UCLASS()
class SAEVUS_API USAEVUSWidgetPausa : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	void Construir();
	UButton* CrearBoton(UVerticalBox* Columna, FName Nombre, UTextBlock*& OutTexto);
	void ActualizarTextos();

	UFUNCTION()
	void AlContinuar();

	UFUNCTION()
	void AlReiniciar();

	UFUNCTION()
	void AlCambiarCalidad();

	UFUNCTION()
	void AlCambiarSensibilidad();

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
	TObjectPtr<UTextBlock> TextoSalir;
};
