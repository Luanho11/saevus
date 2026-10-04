// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSWidgetPausa.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/GameUserSettings.h"
#include "Interfaz/SAEVUSEstiloInterfaz.h"
#include "Interfaz/SAEVUSMapa.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Nucleo/SAEVUSControladorJugador.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "SAEVUSPausa"

namespace SAEVUSPausa
{
	const float Sensibilidades[] = { 0.5f, 0.75f, 1.f, 1.5f, 2.f };
	constexpr float PasoVolumen = 0.25f;
	constexpr int32 NumeroCalidades = 3;
	constexpr int32 TamanoTitulo = 40;
	// Separación entre la columna de opciones y el mapa.
	constexpr float SeparacionMapa = 48.f;
	constexpr int32 TamanoTituloPrincipal = 96;
	constexpr int32 TamanoSubtitulo = 18;
	// El menú principal tapa el juego algo más que la pausa.
	const FLinearColor ColorVeloPrincipal(0.f, 0.f, 0.02f, 0.85f);
	const FLinearColor ColorSubtitulo(0.55f, 0.6f, 0.75f);
	constexpr int32 TamanoBoton = 20;
	constexpr float SeparacionBotones = 8.f;
	constexpr float AnchoRellenoBoton = 28.f;
	const FLinearColor ColorVelo(0.f, 0.f, 0.02f, 0.7f);
	const FLinearColor ColorTexto(0.85f, 0.9f, 1.f);

	int32 SiguienteCalidad(int32 Actual)
	{
		return (FMath::Clamp(Actual, 0, NumeroCalidades - 1) + 1) % NumeroCalidades;
	}

	float SiguienteSensibilidad(float Actual)
	{
		for (const float Valor : Sensibilidades)
		{
			if (Valor > Actual + KINDA_SMALL_NUMBER)
			{
				return Valor;
			}
		}
		return Sensibilidades[0];
	}

	float SiguienteVolumen(float Actual)
	{
		// Al paso más cercano y uno más; pasado el 100 % vuelve a 0.
		const float Siguiente = FMath::RoundToFloat(Actual / PasoVolumen) * PasoVolumen + PasoVolumen;
		return Siguiente > 1.f + KINDA_SMALL_NUMBER ? 0.f : Siguiente;
	}

	FText NombreCalidad(int32 Nivel)
	{
		switch (Nivel)
		{
		case 0:
			return LOCTEXT("Baja", "Baja");
		case 1:
			return LOCTEXT("Media", "Media");
		default:
			return LOCTEXT("Alta", "Alta");
		}
	}
}

TSharedRef<SWidget> USAEVUSWidgetPausa::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Construir();
	}
	return Super::RebuildWidget();
}

void USAEVUSWidgetPausa::NativeConstruct()
{
	Super::NativeConstruct();
	ActualizarTextos();
	if (Mapa)
	{
		Mapa->Refrescar();
	}
}

UButton* USAEVUSWidgetPausa::CrearBoton(UVerticalBox* Columna, FName Nombre, UTextBlock*& OutTexto)
{
	using namespace SAEVUSPausa;
	UButton* Boton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Nombre);
	Boton->SetStyle(SAEVUSEstilo::EstiloBotonMenu());
	OutTexto = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	OutTexto->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", TamanoBoton));
	OutTexto->SetJustification(ETextJustify::Center);
	OutTexto->SetColorAndOpacity(FSlateColor(ColorTexto));
	Boton->AddChild(OutTexto);
	UVerticalBoxSlot* Ranura = Columna->AddChildToVerticalBox(Boton);
	Ranura->SetPadding(FMargin(0.f, SeparacionBotones));
	Ranura->SetHorizontalAlignment(HAlign_Fill);
	return Boton;
}

void USAEVUSWidgetPausa::Construir()
{
	using namespace SAEVUSPausa;
	// Velo oscuro a pantalla completa con la columna de opciones en el centro.
	const bool bPrincipal = Modo == ESAEVUSModoMenu::Principal;
	UBorder* Velo = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Velo"));
	Velo->SetBrushColor(bPrincipal ? ColorVeloPrincipal : ColorVelo);
	Velo->SetHorizontalAlignment(HAlign_Center);
	Velo->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Velo;

	UVerticalBox* Columna = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Opciones"));
	if (bPrincipal)
	{
		Velo->SetContent(Columna);
	}
	else
	{
		// Pausa: opciones a la izquierda y el mapa de la zona a la derecha.
		UHorizontalBox* Fila = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Fila"));
		Velo->SetContent(Fila);
		Fila->AddChildToHorizontalBox(Columna)->SetVerticalAlignment(VAlign_Center);
		Mapa = CreateWidget<USAEVUSWidgetMapa>(this, USAEVUSWidgetMapa::StaticClass(), TEXT("Mapa"));
		UHorizontalBoxSlot* RanuraMapa = Fila->AddChildToHorizontalBox(Mapa);
		RanuraMapa->SetPadding(FMargin(SeparacionMapa, 0.f, 0.f, 0.f));
		RanuraMapa->SetVerticalAlignment(VAlign_Center);
	}

	UTextBlock* Titulo = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Titulo"));
	Titulo->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", bPrincipal ? TamanoTituloPrincipal : TamanoTitulo));
	Titulo->SetColorAndOpacity(FSlateColor(ColorTexto));
	Titulo->SetText(bPrincipal ? LOCTEXT("TituloJuego", "SAEVUS") : LOCTEXT("Pausa", "Pausa"));
	Titulo->SetJustification(ETextJustify::Center);
	Columna->AddChildToVerticalBox(Titulo)->SetPadding(FMargin(AnchoRellenoBoton, 0.f, AnchoRellenoBoton, bPrincipal ? 0.f : SeparacionBotones * 2.f));
	if (bPrincipal)
	{
		UTextBlock* Subtitulo = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Subtitulo"));
		Subtitulo->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", TamanoSubtitulo));
		Subtitulo->SetColorAndOpacity(FSlateColor(ColorSubtitulo));
		Subtitulo->SetText(LOCTEXT("Subtitulo", "Boceto jugable"));
		Subtitulo->SetJustification(ETextJustify::Center);
		Columna->AddChildToVerticalBox(Subtitulo)->SetPadding(FMargin(0.f, 0.f, 0.f, SeparacionBotones * 4.f));
	}

	UTextBlock* Texto = nullptr;
	if (bPrincipal)
	{
		CrearBoton(Columna, TEXT("Jugar"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlJugar);
		TextoContinuar = Texto;
		// "Continuar" solo si hay un punto de control guardado para este mapa.
		FTransform Punto;
		const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(GetOwningPlayer());
		UButton* BotonContinuar = CrearBoton(Columna, TEXT("ContinuarPartida"), Texto);
		BotonContinuar->OnClicked.AddDynamic(this, &ThisClass::AlContinuarPartida);
		BotonContinuar->SetVisibility(Guardado && Guardado->ObtenerPuntoControl(GetWorld(), Punto) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		TextoReiniciar = Texto;
	}
	else
	{
		CrearBoton(Columna, TEXT("Continuar"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlContinuar);
		TextoContinuar = Texto;
		CrearBoton(Columna, TEXT("Reiniciar"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlReiniciar);
		TextoReiniciar = Texto;
	}
	CrearBoton(Columna, TEXT("Calidad"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlCambiarCalidad);
	TextoCalidad = Texto;
	CrearBoton(Columna, TEXT("Sensibilidad"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlCambiarSensibilidad);
	TextoSensibilidad = Texto;
	CrearBoton(Columna, TEXT("Volumen"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlCambiarVolumen);
	TextoVolumen = Texto;
	CrearBoton(Columna, TEXT("Salir"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlSalir);
	TextoSalir = Texto;
}

void USAEVUSWidgetPausa::ActualizarTextos()
{
	if (!TextoContinuar)
	{
		return;
	}
	const bool bPrincipal = Modo == ESAEVUSModoMenu::Principal;
	TextoContinuar->SetText(bPrincipal ? LOCTEXT("Jugar", "Jugar") : LOCTEXT("Continuar", "Continuar"));
	TextoReiniciar->SetText(bPrincipal ? LOCTEXT("ContinuarPartida", "Continuar desde el último punto de control")
		: LOCTEXT("Reiniciar", "Volver al último punto de control"));
	const UGameUserSettings* Ajustes = UGameUserSettings::GetGameUserSettings();
	const int32 Calidad = Ajustes ? Ajustes->GetOverallScalabilityLevel() : 1;
	TextoCalidad->SetText(FText::Format(LOCTEXT("Calidad", "Calidad: {0}"), SAEVUSPausa::NombreCalidad(Calidad)));
	const ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer());
	const float Sensibilidad = Controlador ? Controlador->ObtenerSensibilidad() : 1.f;
	TextoSensibilidad->SetText(FText::Format(LOCTEXT("Sensibilidad", "Sensibilidad del ratón: {0}"), FText::AsNumber(Sensibilidad)));
	const float Volumen = Controlador ? Controlador->ObtenerVolumen() : 1.f;
	TextoVolumen->SetText(FText::Format(LOCTEXT("Volumen", "Volumen: {0} %"), FText::AsNumber(FMath::RoundToInt32(Volumen * 100.f))));
	TextoSalir->SetText(LOCTEXT("Salir", "Salir del juego"));
}

void USAEVUSWidgetPausa::AlContinuar()
{
	if (ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer()))
	{
		Controlador->AlternarPausa();
	}
}

void USAEVUSWidgetPausa::AlJugar()
{
	if (ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer()))
	{
		Controlador->CerrarMenuPrincipal();
	}
}

void USAEVUSWidgetPausa::AlContinuarPartida()
{
	if (ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer()))
	{
		Controlador->ContinuarPartidaGuardada();
	}
}

void USAEVUSWidgetPausa::AlReiniciar()
{
	if (ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer()))
	{
		Controlador->ReiniciarEnPuntoControl();
	}
}

void USAEVUSWidgetPausa::AlCambiarCalidad()
{
	if (UGameUserSettings* Ajustes = UGameUserSettings::GetGameUserSettings())
	{
		// -1 en "personalizado": se empieza por Baja.
		Ajustes->SetOverallScalabilityLevel(SAEVUSPausa::SiguienteCalidad(Ajustes->GetOverallScalabilityLevel()));
		Ajustes->ApplySettings(false);
	}
	ActualizarTextos();
}

void USAEVUSWidgetPausa::AlCambiarSensibilidad()
{
	if (ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer()))
	{
		Controlador->FijarSensibilidad(SAEVUSPausa::SiguienteSensibilidad(Controlador->ObtenerSensibilidad()));
	}
	ActualizarTextos();
}

void USAEVUSWidgetPausa::AlCambiarVolumen()
{
	if (ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer()))
	{
		Controlador->FijarVolumen(SAEVUSPausa::SiguienteVolumen(Controlador->ObtenerVolumen()));
	}
	ActualizarTextos();
}

void USAEVUSWidgetPausa::AlSalir()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

#undef LOCTEXT_NAMESPACE
