// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSWidgetPausa.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/GameUserSettings.h"
#include "Interfaz/SAEVUSEstiloInterfaz.h"
#include "Interfaz/SAEVUSMapa.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Nucleo/SAEVUSControladorJugador.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Nucleo/SAEVUSModoJuego.h"

#define LOCTEXT_NAMESPACE "SAEVUSPausa"

namespace SAEVUSPausa
{
	const float Sensibilidades[] = { 0.5f, 0.75f, 1.f, 1.5f, 2.f };
	constexpr float PasoVolumen = 0.25f;
	constexpr int32 NumeroCalidades = 3;
	constexpr int32 TamanoTitulo = 40;
	// Separación entre la columna de opciones y el mapa.
	constexpr float SeparacionMapa = 48.f;
	constexpr int32 TamanoTituloPrincipal = 88;
	// El menú principal tapa el juego algo más que la pausa.
	const FLinearColor ColorVeloPrincipal(0.f, 0.f, 0.02f, 0.88f);
	constexpr int32 TamanoBoton = 20;
	constexpr float SeparacionBotones = 4.f;
	constexpr float AnchoRellenoBoton = 28.f;
	const FLinearColor ColorVelo(0.f, 0.f, 0.02f, 0.72f);
	const FLinearColor ColorTexto(0.85f, 0.9f, 1.f);
	// Raya fina bajo el título.
	const FVector2D TamanoRaya(280.f, 2.f);
	const FLinearColor ColorRaya(0.6f, 0.68f, 0.85f, 0.6f);
	constexpr float SeparacionRaya = 18.f;

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

	void FijarVisibles(const TArray<TObjectPtr<UButton>>& Grupo, bool bVisible)
	{
		for (UButton* Boton : Grupo)
		{
			if (Boton)
			{
				Boton->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			}
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
	// Siempre se abre en la lista principal.
	MostrarOpciones(false);
	ActualizarTextos();
	if (Mapa)
	{
		Mapa->Refrescar();
	}
}

UButton* USAEVUSWidgetPausa::CrearBoton(UVerticalBox* Columna, FName Nombre, const FText& Texto, TArray<TObjectPtr<UButton>>& Grupo, UTextBlock** OutTexto)
{
	using namespace SAEVUSPausa;
	UButton* Boton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Nombre);
	Boton->SetStyle(SAEVUSEstilo::EstiloBotonMenu());
	UTextBlock* Etiqueta = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Etiqueta->SetFont(SAEVUSEstilo::FuenteTitulo(TamanoBoton));
	Etiqueta->SetTextTransformPolicy(ETextTransformPolicy::ToUpper);
	Etiqueta->SetJustification(ETextJustify::Center);
	Etiqueta->SetColorAndOpacity(FSlateColor(ColorTexto));
	Etiqueta->SetText(Texto);
	Boton->AddChild(Etiqueta);
	Botones.Add(Boton);
	FocoBotones.Add(false);
	Grupo.Add(Boton);
	UVerticalBoxSlot* Ranura = Columna->AddChildToVerticalBox(Boton);
	Ranura->SetPadding(FMargin(0.f, SeparacionBotones));
	Ranura->SetHorizontalAlignment(HAlign_Fill);
	if (OutTexto)
	{
		*OutTexto = Etiqueta;
	}
	return Boton;
}

void USAEVUSWidgetPausa::NativeTick(const FGeometry& Geometria, float DeltaTime)
{
	Super::NativeTick(Geometria, DeltaTime);
	for (int32 Indice = 0; Indice < Botones.Num(); ++Indice)
	{
		UButton* Boton = Botones[Indice];
		const bool bFoco = Boton && (Boton->HasKeyboardFocus() || Boton->HasUserFocus(GetOwningPlayer()));
		if (Boton && bFoco != FocoBotones[Indice])
		{
			FocoBotones[Indice] = bFoco;
			Boton->SetStyle(bFoco ? SAEVUSEstilo::EstiloBotonMenuEnfocado() : SAEVUSEstilo::EstiloBotonMenu());
		}
	}
}

UWidget* USAEVUSWidgetPausa::ObtenerPrimerBoton() const
{
	for (UButton* Boton : Botones)
	{
		if (Boton && Boton->GetVisibility() == ESlateVisibility::Visible)
		{
			return Boton;
		}
	}
	return nullptr;
}

bool USAEVUSWidgetPausa::TieneFocoAlgunBoton() const
{
	for (const UButton* Boton : Botones)
	{
		if (Boton && (Boton->HasKeyboardFocus() || Boton->HasUserFocus(GetOwningPlayer())))
		{
			return true;
		}
	}
	return false;
}

void USAEVUSWidgetPausa::MostrarOpciones(bool bOpciones)
{
	using namespace SAEVUSPausa;
	bEnOpciones = bOpciones;
	FijarVisibles(BotonesInicio, !bOpciones);
	FijarVisibles(BotonesOpciones, bOpciones);
	if (BotonContinuarPartida && !bOpciones && !bHayPartidaGuardada)
	{
		BotonContinuarPartida->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (IsInViewport())
	{
		if (UWidget* Primero = ObtenerPrimerBoton())
		{
			Primero->SetUserFocus(GetOwningPlayer());
		}
	}
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

	// Título con letras espaciadas y una raya fina debajo.
	UTextBlock* Titulo = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Titulo"));
	Titulo->SetFont(SAEVUSEstilo::FuenteTitulo(bPrincipal ? TamanoTituloPrincipal : TamanoTitulo));
	Titulo->SetTextTransformPolicy(ETextTransformPolicy::ToUpper);
	Titulo->SetColorAndOpacity(FSlateColor(ColorTexto));
	Titulo->SetText(bPrincipal ? LOCTEXT("TituloJuego", "SAEVUS") : LOCTEXT("Pausa", "Pausa"));
	Titulo->SetJustification(ETextJustify::Center);
	Columna->AddChildToVerticalBox(Titulo)->SetPadding(FMargin(AnchoRellenoBoton, 0.f));
	UImage* Raya = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Raya"));
	FSlateBrush PincelRaya;
	PincelRaya.TintColor = FSlateColor(ColorRaya);
	PincelRaya.ImageSize = TamanoRaya;
	Raya->SetBrush(PincelRaya);
	UVerticalBoxSlot* RanuraRaya = Columna->AddChildToVerticalBox(Raya);
	RanuraRaya->SetHorizontalAlignment(HAlign_Center);
	RanuraRaya->SetPadding(FMargin(0.f, SeparacionRaya * 0.5f, 0.f, SeparacionRaya));

	if (bPrincipal)
	{
		CrearBoton(Columna, TEXT("Tutorial"), LOCTEXT("Tutorial", "Tutorial"), BotonesInicio)->OnClicked.AddDynamic(this, &ThisClass::AlTutorial);
		CrearBoton(Columna, TEXT("Mazmorra"), LOCTEXT("Mazmorra", "Mazmorra"), BotonesInicio)->OnClicked.AddDynamic(this, &ThisClass::AlMazmorra);
		// "Continuar" solo si hay un punto de control guardado para este mapa.
		FTransform Punto;
		const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(GetOwningPlayer());
		bHayPartidaGuardada = Guardado && Guardado->ObtenerPuntoControl(GetWorld(), Punto);
		BotonContinuarPartida = CrearBoton(Columna, TEXT("ContinuarPartida"), LOCTEXT("ContinuarPartida", "Continuar"), BotonesInicio);
		BotonContinuarPartida->OnClicked.AddDynamic(this, &ThisClass::AlContinuarPartida);
	}
	else
	{
		CrearBoton(Columna, TEXT("Continuar"), LOCTEXT("Continuar", "Continuar"), BotonesInicio)->OnClicked.AddDynamic(this, &ThisClass::AlContinuar);
		CrearBoton(Columna, TEXT("Reiniciar"), LOCTEXT("Reiniciar", "Volver al punto de control"), BotonesInicio)->OnClicked.AddDynamic(this, &ThisClass::AlReiniciar);
	}
	CrearBoton(Columna, TEXT("Ajustes"), LOCTEXT("Opciones", "Opciones"), BotonesInicio)->OnClicked.AddDynamic(this, &ThisClass::AlOpciones);
	if (!bPrincipal)
	{
		CrearBoton(Columna, TEXT("MenuPrincipal"), LOCTEXT("MenuPrincipal", "Menú principal"), BotonesInicio)->OnClicked.AddDynamic(this, &ThisClass::AlMenuPrincipal);
	}
	CrearBoton(Columna, TEXT("Salir"), LOCTEXT("Salir", "Salir"), BotonesInicio)->OnClicked.AddDynamic(this, &ThisClass::AlSalir);

	UTextBlock* Etiqueta = nullptr;
	CrearBoton(Columna, TEXT("Calidad"), FText::GetEmpty(), BotonesOpciones, &Etiqueta)->OnClicked.AddDynamic(this, &ThisClass::AlCambiarCalidad);
	TextoCalidad = Etiqueta;
	CrearBoton(Columna, TEXT("Sensibilidad"), FText::GetEmpty(), BotonesOpciones, &Etiqueta)->OnClicked.AddDynamic(this, &ThisClass::AlCambiarSensibilidad);
	TextoSensibilidad = Etiqueta;
	CrearBoton(Columna, TEXT("Volumen"), FText::GetEmpty(), BotonesOpciones, &Etiqueta)->OnClicked.AddDynamic(this, &ThisClass::AlCambiarVolumen);
	TextoVolumen = Etiqueta;
	CrearBoton(Columna, TEXT("Volver"), LOCTEXT("Volver", "Volver"), BotonesOpciones)->OnClicked.AddDynamic(this, &ThisClass::AlVolver);
	MostrarOpciones(false);
}

void USAEVUSWidgetPausa::ActualizarTextos()
{
	if (!TextoCalidad)
	{
		return;
	}
	const UGameUserSettings* Ajustes = UGameUserSettings::GetGameUserSettings();
	const int32 Calidad = Ajustes ? Ajustes->GetOverallScalabilityLevel() : 1;
	TextoCalidad->SetText(FText::Format(LOCTEXT("Calidad", "Calidad: {0}"), SAEVUSPausa::NombreCalidad(Calidad)));
	const ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer());
	const float Sensibilidad = Controlador ? Controlador->ObtenerSensibilidad() : 1.f;
	TextoSensibilidad->SetText(FText::Format(LOCTEXT("Sensibilidad", "Sensibilidad del ratón: {0}"), FText::AsNumber(Sensibilidad)));
	const float Volumen = Controlador ? Controlador->ObtenerVolumen() : 1.f;
	TextoVolumen->SetText(FText::Format(LOCTEXT("Volumen", "Volumen: {0} %"), FText::AsNumber(FMath::RoundToInt32(Volumen * 100.f))));
}

void USAEVUSWidgetPausa::AlContinuar()
{
	if (ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer()))
	{
		Controlador->AlternarPausa();
	}
}

void USAEVUSWidgetPausa::AlTutorial()
{
	const ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>();
	ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer());
	if (Controlador && ModoJuego)
	{
		Controlador->JugarEn(ModoJuego->ObtenerRutaMapaTutorial());
	}
}

void USAEVUSWidgetPausa::AlMazmorra()
{
	const ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>();
	ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer());
	if (Controlador && ModoJuego)
	{
		Controlador->JugarEn(ModoJuego->ObtenerRutaMapaMazmorra());
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

void USAEVUSWidgetPausa::AlOpciones()
{
	ActualizarTextos();
	MostrarOpciones(true);
}

void USAEVUSWidgetPausa::AlVolver()
{
	MostrarOpciones(false);
}

void USAEVUSWidgetPausa::AlMenuPrincipal()
{
	// El tutorial arranca con el menú principal.
	const ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>();
	ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(GetOwningPlayer());
	if (Controlador && ModoJuego)
	{
		Controlador->IrAMapa(ModoJuego->ObtenerRutaMapaTutorial());
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
