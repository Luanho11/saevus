// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSWidgetPausa.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Nucleo/SAEVUSControladorJugador.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "SAEVUSPausa"

namespace SAEVUSPausa
{
	const float Sensibilidades[] = { 0.5f, 0.75f, 1.f, 1.5f, 2.f };
	constexpr int32 NumeroCalidades = 3;
	constexpr int32 TamanoTitulo = 40;
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
}

UButton* USAEVUSWidgetPausa::CrearBoton(UVerticalBox* Columna, FName Nombre, UTextBlock*& OutTexto)
{
	using namespace SAEVUSPausa;
	UButton* Boton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Nombre);
	OutTexto = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	OutTexto->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", TamanoBoton));
	OutTexto->SetJustification(ETextJustify::Center);
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
	UBorder* Velo = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Velo"));
	Velo->SetBrushColor(ColorVelo);
	Velo->SetHorizontalAlignment(HAlign_Center);
	Velo->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Velo;

	UVerticalBox* Columna = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Opciones"));
	Velo->SetContent(Columna);

	UTextBlock* Titulo = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Titulo"));
	Titulo->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", TamanoTitulo));
	Titulo->SetColorAndOpacity(FSlateColor(ColorTexto));
	Titulo->SetText(LOCTEXT("Pausa", "Pausa"));
	Titulo->SetJustification(ETextJustify::Center);
	Columna->AddChildToVerticalBox(Titulo)->SetPadding(FMargin(AnchoRellenoBoton, 0.f, AnchoRellenoBoton, SeparacionBotones * 2.f));

	UTextBlock* Texto = nullptr;
	CrearBoton(Columna, TEXT("Continuar"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlContinuar);
	TextoContinuar = Texto;
	CrearBoton(Columna, TEXT("Reiniciar"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlReiniciar);
	TextoReiniciar = Texto;
	CrearBoton(Columna, TEXT("Calidad"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlCambiarCalidad);
	TextoCalidad = Texto;
	CrearBoton(Columna, TEXT("Sensibilidad"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlCambiarSensibilidad);
	TextoSensibilidad = Texto;
	CrearBoton(Columna, TEXT("Salir"), Texto)->OnClicked.AddDynamic(this, &ThisClass::AlSalir);
	TextoSalir = Texto;
}

void USAEVUSWidgetPausa::ActualizarTextos()
{
	if (!TextoContinuar)
	{
		return;
	}
	TextoContinuar->SetText(LOCTEXT("Continuar", "Continuar"));
	TextoReiniciar->SetText(LOCTEXT("Reiniciar", "Volver al último punto de control"));
	const UGameUserSettings* Ajustes = UGameUserSettings::GetGameUserSettings();
	const int32 Calidad = Ajustes ? Ajustes->GetOverallScalabilityLevel() : 1;
	TextoCalidad->SetText(FText::Format(LOCTEXT("Calidad", "Calidad: {0}"), SAEVUSPausa::NombreCalidad(Calidad)));
	const ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer());
	const float Sensibilidad = Controlador ? Controlador->ObtenerSensibilidad() : 1.f;
	TextoSensibilidad->SetText(FText::Format(LOCTEXT("Sensibilidad", "Sensibilidad del ratón: {0}"), FText::AsNumber(Sensibilidad)));
	TextoSalir->SetText(LOCTEXT("Salir", "Salir del juego"));
}

void USAEVUSWidgetPausa::AlContinuar()
{
	if (ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer()))
	{
		Controlador->AlternarPausa();
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

void USAEVUSWidgetPausa::AlSalir()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

#undef LOCTEXT_NAMESPACE
