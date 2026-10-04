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
 * Menú construido en C++ como la interfaz: título con letras espaciadas y opciones de solo texto
 * (se iluminan al pasar por encima o con el foco del mando o del teclado). Dos modos:
 * - Principal: Tutorial, Mazmorra, Continuar (si hay un punto de control guardado para el mapa),
 *   Opciones y Salir (ASAEVUSControladorJugador::MostrarMenuPrincipal).
 * - Pausa: Continuar, Volver al punto de control, Opciones, Menú principal y Salir, con el mapa de
 *   la zona a la derecha (ASAEVUSControladorJugador::AlternarPausa).
 * Opciones (calidad, sensibilidad, volumen y volver) sustituye a la lista principal en el mismo sitio.
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

	/** Cierto si se ven las opciones (calidad, sensibilidad, volumen) en vez de la lista principal. */
	bool EstaEnOpciones() const { return bEnOpciones; }
	/** Cambia entre la lista principal y las opciones, y da el foco al primer botón que se vea. */
	void MostrarOpciones(bool bOpciones);

private:
	void Construir();
	UButton* CrearBoton(UVerticalBox* Columna, FName Nombre, const FText& Texto, TArray<TObjectPtr<UButton>>& Grupo, UTextBlock** OutTexto = nullptr);
	void ActualizarTextos();

	UFUNCTION()
	void AlContinuar();

	UFUNCTION()
	void AlReiniciar();

	UFUNCTION()
	void AlTutorial();

	UFUNCTION()
	void AlMazmorra();

	UFUNCTION()
	void AlContinuarPartida();

	UFUNCTION()
	void AlOpciones();

	UFUNCTION()
	void AlVolver();

	UFUNCTION()
	void AlMenuPrincipal();

	UFUNCTION()
	void AlCambiarCalidad();

	UFUNCTION()
	void AlCambiarSensibilidad();

	UFUNCTION()
	void AlCambiarVolumen();

	UFUNCTION()
	void AlSalir();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoCalidad;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoSensibilidad;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextoVolumen;

	/** Mapa de la zona (solo en la pausa). */
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSWidgetMapa> Mapa;

	ESAEVUSModoMenu Modo = ESAEVUSModoMenu::Pausa;

	/** Todos los botones, en orden (para el resaltado del foco). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> Botones;

	/** Lista principal y lista de opciones: se ve una u otra. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> BotonesInicio;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> BotonesOpciones;

	/** "Continuar" del menú principal: solo con un punto de control guardado para este mapa. */
	UPROPERTY(Transient)
	TObjectPtr<UButton> BotonContinuarPartida;

	bool bHayPartidaGuardada = false;
	bool bEnOpciones = false;

	/** Si cada botón tenía el foco el último fotograma (solo se cambia el estilo al cambiar). */
	TArray<bool> FocoBotones;
};
