// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSWidgetInterfaz.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Interfaz/SAEVUSEstiloInterfaz.h"
#include "Styling/CoreStyle.h"

namespace SAEVUSInterfaz
{
	// Separaciones del boceto (unidades de interfaz a 1080p).
	constexpr float SeparacionMascaras = 6.f;
	constexpr float SeparacionOrbeMascaras = 18.f;
	constexpr float SeparacionLineas = 4.f;
	constexpr float AlturaBarraJefeDesdeAbajo = 70.f;
	// El aviso de interacción, por encima de la barra del jefe.
	constexpr float AlturaInteraccionDesdeAbajo = 150.f;
	// Latido del orbe mientras cura: amplitud de escala y rapidez (rad/s).
	constexpr float AmplitudLatido = 0.08f;
	constexpr float RapidezLatido = 12.f;
	// La sombra del texto lo despega de fondos claros.
	const FVector2D DesplazamientoSombra(1.5f, 1.5f);
	const FLinearColor ColorSombra(0.f, 0.f, 0.f, 0.85f);

	// Cambiar textos, barras o visibilidad invalida el diseño de Slate: solo se tocan si cambian.
	void FijarTexto(UTextBlock* Texto, const FText& Nuevo)
	{
		if (!Texto->GetText().EqualTo(Nuevo))
		{
			Texto->SetText(Nuevo);
		}
	}

	void FijarVisible(UWidget* Widget, bool bVisible)
	{
		const ESlateVisibility Nueva = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
		if (Widget->GetVisibility() != Nueva)
		{
			Widget->SetVisibility(Nueva);
		}
	}

	void FijarPorcentaje(UProgressBar* Barra, float Fraccion)
	{
		if (!FMath::IsNearlyEqual(Barra->GetPercent(), Fraccion))
		{
			Barra->SetPercent(Fraccion);
		}
	}

	using SAEVUSEstilo::PincelRedondo;
}

TSharedRef<SWidget> USAEVUSWidgetInterfaz::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Construir();
	}
	return Super::RebuildWidget();
}

UTextBlock* USAEVUSWidgetInterfaz::CrearTexto(FName Nombre, int32 Tamano, bool bCentrado)
{
	UTextBlock* Texto = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Nombre);
	Texto->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", Tamano));
	Texto->SetColorAndOpacity(FSlateColor(Ajuste.ColorTexto));
	Texto->SetShadowOffset(SAEVUSInterfaz::DesplazamientoSombra);
	Texto->SetShadowColorAndOpacity(SAEVUSInterfaz::ColorSombra);
	if (bCentrado)
	{
		Texto->SetJustification(ETextJustify::Center);
	}
	return Texto;
}

void USAEVUSWidgetInterfaz::Construir()
{
	using namespace SAEVUSInterfaz;
	UCanvasPanel* Lienzo = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Lienzo"));
	WidgetTree->RootWidget = Lienzo;

	// Arriba a la izquierda: orbe de alma y, a su derecha, la fila de máscaras.
	UHorizontalBox* Vitales = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Vitales"));
	UCanvasPanelSlot* RanuraVitales = Lienzo->AddChildToCanvas(Vitales);
	RanuraVitales->SetAnchors(FAnchors(0.f, 0.f));
	RanuraVitales->SetPosition(Ajuste.Margen);
	RanuraVitales->SetAutoSize(true);

	OrbeAlma = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("OrbeAlma"));
	// Orbe: barra redonda que se llena de abajo arriba; su tamaño lo da el del pincel de fondo.
	FProgressBarStyle EstiloOrbe;
	EstiloOrbe.BackgroundImage = PincelRedondo(Ajuste.ColorFondo, Ajuste.ColorMascaraVacia);
	EstiloOrbe.BackgroundImage.ImageSize = FVector2D(Ajuste.DiametroOrbe);
	EstiloOrbe.FillImage = PincelRedondo(Ajuste.ColorAlma);
	EstiloOrbe.FillImage.ImageSize = FVector2D(Ajuste.DiametroOrbe);
	EstiloOrbe.MarqueeImage = EstiloOrbe.FillImage;
	OrbeAlma->SetWidgetStyle(EstiloOrbe);
	OrbeAlma->SetBarFillType(EProgressBarFillType::BottomToTop);
	OrbeAlma->SetPercent(0.f);
	OrbeAlma->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	UHorizontalBoxSlot* RanuraOrbe = Vitales->AddChildToHorizontalBox(OrbeAlma);
	RanuraOrbe->SetVerticalAlignment(VAlign_Center);
	RanuraOrbe->SetPadding(FMargin(0.f, 0.f, SeparacionOrbeMascaras, 0.f));

	FilaMascaras = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Mascaras"));
	UHorizontalBoxSlot* RanuraFila = Vitales->AddChildToHorizontalBox(FilaMascaras);
	RanuraFila->SetVerticalAlignment(VAlign_Center);

	// Abajo a la derecha: arma y munición.
	UVerticalBox* BloqueArma = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Arma"));
	UCanvasPanelSlot* RanuraArma = Lienzo->AddChildToCanvas(BloqueArma);
	RanuraArma->SetAnchors(FAnchors(1.f, 1.f));
	RanuraArma->SetAlignment(FVector2D(1.f, 1.f));
	RanuraArma->SetPosition(-Ajuste.Margen);
	RanuraArma->SetAutoSize(true);
	TextoArma = CrearTexto(TEXT("TextoArma"), Ajuste.TamanoTextoArma, false);
	TextoMunicion = CrearTexto(TEXT("TextoMunicion"), Ajuste.TamanoTextoArma, false);
	BloqueArma->AddChildToVerticalBox(TextoArma)->SetHorizontalAlignment(HAlign_Right);
	BloqueArma->AddChildToVerticalBox(TextoMunicion)->SetHorizontalAlignment(HAlign_Right);

	// Arriba al centro: aviso del tutorial.
	BloqueAviso = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Aviso"));
	UCanvasPanelSlot* RanuraAviso = Lienzo->AddChildToCanvas(BloqueAviso);
	RanuraAviso->SetAnchors(FAnchors(0.5f, Ajuste.AlturaAviso));
	RanuraAviso->SetAlignment(FVector2D(0.5f, 0.f));
	RanuraAviso->SetAutoSize(true);
	TituloAviso = CrearTexto(TEXT("TituloAviso"), Ajuste.TamanoTituloAviso, true);
	TextoAviso = CrearTexto(TEXT("TextoAviso"), Ajuste.TamanoTextoAviso, true);
	BloqueAviso->AddChildToVerticalBox(TituloAviso)->SetHorizontalAlignment(HAlign_Center);
	UVerticalBoxSlot* RanuraTextoAviso = BloqueAviso->AddChildToVerticalBox(TextoAviso);
	RanuraTextoAviso->SetHorizontalAlignment(HAlign_Center);
	RanuraTextoAviso->SetPadding(FMargin(0.f, SeparacionLineas, 0.f, 0.f));
	BloqueAviso->SetVisibility(ESlateVisibility::Collapsed);

	// Abajo al centro: nombre y barra del jefe activo.
	BloqueJefe = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Jefe"));
	UCanvasPanelSlot* RanuraJefe = Lienzo->AddChildToCanvas(BloqueJefe);
	RanuraJefe->SetAnchors(FAnchors(0.5f, 1.f));
	RanuraJefe->SetAlignment(FVector2D(0.5f, 1.f));
	RanuraJefe->SetPosition(FVector2D(0.f, -AlturaBarraJefeDesdeAbajo));
	RanuraJefe->SetAutoSize(true);
	NombreJefe = CrearTexto(TEXT("NombreJefe"), Ajuste.TamanoTextoArma, true);
	BloqueJefe->AddChildToVerticalBox(NombreJefe)->SetHorizontalAlignment(HAlign_Center);
	BarraJefe = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("BarraJefe"));
	FProgressBarStyle EstiloJefe;
	EstiloJefe.BackgroundImage = PincelRedondo(Ajuste.ColorFondo);
	EstiloJefe.BackgroundImage.ImageSize = Ajuste.TamanoBarraJefe;
	EstiloJefe.FillImage = PincelRedondo(Ajuste.ColorVidaJefe);
	EstiloJefe.FillImage.ImageSize = Ajuste.TamanoBarraJefe;
	EstiloJefe.MarqueeImage = EstiloJefe.FillImage;
	BarraJefe->SetWidgetStyle(EstiloJefe);
	UVerticalBoxSlot* RanuraBarra = BloqueJefe->AddChildToVerticalBox(BarraJefe);
	RanuraBarra->SetPadding(FMargin(0.f, SeparacionLineas, 0.f, 0.f));
	BloqueJefe->SetVisibility(ESlateVisibility::Collapsed);

	// Abajo al centro, encima del jefe: lo que se puede usar ("E   Sentarse").
	TextoInteraccion = CrearTexto(TEXT("TextoInteraccion"), Ajuste.TamanoTextoArma, true);
	UCanvasPanelSlot* RanuraInteraccion = Lienzo->AddChildToCanvas(TextoInteraccion);
	RanuraInteraccion->SetAnchors(FAnchors(0.5f, 1.f));
	RanuraInteraccion->SetAlignment(FVector2D(0.5f, 1.f));
	RanuraInteraccion->SetPosition(FVector2D(0.f, -AlturaInteraccionDesdeAbajo));
	RanuraInteraccion->SetAutoSize(true);
	TextoInteraccion->SetVisibility(ESlateVisibility::Collapsed);

	// Pantalla de muerte: velo negro a pantalla completa con un texto, por encima de todo.
	VeloMuerte = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("VeloMuerte"));
	VeloMuerte->SetBrushColor(FLinearColor::Black);
	VeloMuerte->SetHorizontalAlignment(HAlign_Center);
	VeloMuerte->SetVerticalAlignment(VAlign_Center);
	UCanvasPanelSlot* RanuraVelo = Lienzo->AddChildToCanvas(VeloMuerte);
	RanuraVelo->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	RanuraVelo->SetOffsets(FMargin(0.f));
	TextoMuerte = CrearTexto(TEXT("TextoMuerte"), Ajuste.TamanoTextoMuerte, true);
	TextoMuerte->SetText(NSLOCTEXT("SAEVUSInterfaz", "HasCaido", "Has caído"));
	VeloMuerte->SetContent(TextoMuerte);
	VeloMuerte->SetRenderOpacity(0.f);
	VeloMuerte->SetVisibility(ESlateVisibility::Collapsed);
}

void USAEVUSWidgetInterfaz::AjustarMascaras(int32 Numero)
{
	using namespace SAEVUSInterfaz;
	if (!FilaMascaras || Mascaras.Num() == Numero)
	{
		return;
	}
	FilaMascaras->ClearChildren();
	Mascaras.Reset();
	for (int32 Indice = 0; Indice < Numero; ++Indice)
	{
		UImage* Mascara = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		FSlateBrush Pincel = PincelRedondo(FLinearColor::White);
		Pincel.ImageSize = Ajuste.TamanoMascara;
		Mascara->SetBrush(Pincel);
		UHorizontalBoxSlot* Ranura = FilaMascaras->AddChildToHorizontalBox(Mascara);
		Ranura->SetPadding(FMargin(0.f, 0.f, SeparacionMascaras, 0.f));
		Ranura->SetVerticalAlignment(VAlign_Center);
		Mascaras.Add(Mascara);
	}
}

void USAEVUSWidgetInterfaz::Aplicar(const FSAEVUSEstadoInterfaz& Estado)
{
	using namespace SAEVUSInterfaz;
	if (!OrbeAlma)
	{
		return;
	}
	AjustarMascaras(Estado.Mascaras.Num());
	for (int32 Indice = 0; Indice < Mascaras.Num(); ++Indice)
	{
		// Máscara partida: se mezcla entre vacía y llena.
		const FLinearColor Color = FMath::Lerp(Ajuste.ColorMascaraVacia, Ajuste.ColorMascaraLlena, Estado.Mascaras[Indice]);
		if (!Mascaras[Indice]->GetColorAndOpacity().Equals(Color))
		{
			Mascaras[Indice]->SetColorAndOpacity(Color);
		}
	}

	FijarPorcentaje(OrbeAlma, Estado.FraccionAlma);
	OrbeAlma->SetFillColorAndOpacity(Estado.bAlmaParaCurar ? Ajuste.ColorAlmaLista : Ajuste.ColorAlma);
	// Al curar el orbe late.
	const float Latido = Estado.bCurando ? 1.f + AmplitudLatido * FMath::Sin(GetWorld()->GetTimeSeconds() * RapidezLatido) : 1.f;
	OrbeAlma->SetRenderScale(FVector2D(Latido));

	FijarTexto(TextoArma, Estado.TextoArma);
	FijarTexto(TextoMunicion, Estado.TextoMunicion);
	FijarVisible(TextoMunicion, !Estado.TextoMunicion.IsEmpty());

	FijarVisible(BloqueAviso, Estado.bHayAviso);
	if (Estado.bHayAviso)
	{
		FijarTexto(TituloAviso, Estado.TituloAviso);
		FijarTexto(TextoAviso, Estado.TextoAviso);
		BloqueAviso->SetRenderOpacity(Estado.OpacidadAviso);
	}

	FijarVisible(TextoInteraccion, !Estado.TextoInteraccion.IsEmpty());
	if (!Estado.TextoInteraccion.IsEmpty())
	{
		FijarTexto(TextoInteraccion, Estado.TextoInteraccion);
	}
	FijarVisible(BloqueJefe, Estado.bJefeVisible);
	if (Estado.bJefeVisible)
	{
		FijarTexto(NombreJefe, Estado.NombreJefe);
		FijarPorcentaje(BarraJefe, Estado.FraccionVidaJefe);
	}
}

void USAEVUSWidgetInterfaz::NativeTick(const FGeometry& Geometria, float DeltaTime)
{
	Super::NativeTick(Geometria, DeltaTime);
	const FSAEVUSEstadoInterfaz Estado = FSAEVUSEstadoInterfaz::Calcular(GetOwningPlayerPawn(), GetWorld(), Ajuste.VidaPorMascara, Ajuste.FundidoAviso);
	if (VeloMuerte)
	{
		// Fundido a negro al caer, y de vuelta (más rápido) al reaparecer.
		const float Objetivo = Estado.bJugadorMuerto ? Ajuste.OpacidadMuerte : 0.f;
		const float Rapidez = Ajuste.OpacidadMuerte / (Estado.bJugadorMuerto ? Ajuste.FundidoMuerte : Ajuste.FundidoReaparicion);
		OpacidadVelo = FMath::FInterpConstantTo(OpacidadVelo, Objetivo, DeltaTime, Rapidez);
		VeloMuerte->SetRenderOpacity(OpacidadVelo);
		SAEVUSInterfaz::FijarVisible(VeloMuerte, OpacidadVelo > 0.f);
	}
	Aplicar(Estado);
}
