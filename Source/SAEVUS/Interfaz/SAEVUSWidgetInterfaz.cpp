// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSWidgetInterfaz.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Personaje/SAEVUSConfigEntrada.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Interfaz/SAEVUSEstiloInterfaz.h"
#include "Interfaz/SAEVUSTextoTeclas.h"
#include "Mundo/SAEVUSInfoZona.h"
#include "Styling/CoreStyle.h"

namespace SAEVUSInterfaz
{
	// Separaciones del boceto (unidades de interfaz a 1080p).
	// HUD (REF-08 A): separación entre el emblema y las barras, y entre las barras.
	constexpr float SeparacionEmblemaBarras = 18.f;
	constexpr float SeparacionBarras = 6.f;
	// El rombo girado ocupa su diagonal: el hueco que se le reserva.
	constexpr float Raiz2 = 1.41421356f;
	// Ojo del emblema: fracción del lado.
	constexpr float FraccionOjoEmblema = 0.28f;
	constexpr float SeparacionLineas = 4.f;
	constexpr float AlturaBarraJefeDesdeAbajo = 70.f;
	// El aviso de interacción, por encima de la barra del jefe.
	constexpr float AlturaInteraccionDesdeAbajo = 150.f;
	// Latido de la barra de alma mientras cura: amplitud de escala y rapidez (rad/s).
	constexpr float AmplitudLatido = 0.08f;
	constexpr float RapidezLatido = 12.f;
	// La sombra del texto lo despega de fondos claros.
	const FVector2D DesplazamientoSombra(1.5f, 1.5f);
	const FLinearColor ColorSombra(0.f, 0.f, 0.f, 0.85f);
	// Panel del aviso: relleno interior y separación con el título.
	const FMargin RellenoPanelAviso(28.f, 12.f);
	constexpr float SeparacionTituloPanel = 6.f;
	// Tecla: relleno del texto dentro de la caja y separación con lo de alrededor; la letra, algo menor.
	const FMargin RellenoTecla(7.f, 1.f);
	const FMargin SeparacionTecla(2.f, 2.f);
	constexpr int32 ReduccionLetraTecla = 2;

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

	// La tecla de interactuar se vuelve a mirar cada tanto (por si cambian los controles).
	constexpr float IntervaloBuscarTecla = 1.f;
	// Grosor de las rayas del título de zona.
	constexpr float GrosorRayaZona = 1.5f;

	void FijarOpacidad(UWidget* Widget, float Opacidad)
	{
		if (!FMath::IsNearlyEqual(Widget->GetRenderOpacity(), Opacidad))
		{
			Widget->SetRenderOpacity(Opacidad);
		}
	}
}

float SAEVUSInterfaz::OpacidadTituloZona(float Segundos, const FSAEVUSAjusteInterfaz& Ajuste)
{
	const float Desde = Segundos - Ajuste.RetardoTituloZona;
	const float Fundido = FMath::Max(Ajuste.FundidoTituloZona, KINDA_SMALL_NUMBER);
	if (Desde <= 0.f)
	{
		return 0.f;
	}
	if (Desde < Fundido)
	{
		return Desde / Fundido;
	}
	const float HastaSalida = Desde - Fundido - Ajuste.DuracionTituloZona;
	return HastaSalida <= 0.f ? 1.f : FMath::Clamp(1.f - HastaSalida / Fundido, 0.f, 1.f);
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
	Texto->SetFont(SAEVUSEstilo::FuenteTexto(Tamano));
	Texto->SetColorAndOpacity(FSlateColor(Ajuste.ColorTexto));
	Texto->SetShadowOffset(SAEVUSInterfaz::DesplazamientoSombra);
	Texto->SetShadowColorAndOpacity(SAEVUSInterfaz::ColorSombra);
	if (bCentrado)
	{
		Texto->SetJustification(ETextJustify::Center);
	}
	return Texto;
}

UTextBlock* USAEVUSWidgetInterfaz::CrearTitulo(FName Nombre, int32 Tamano)
{
	UTextBlock* Titulo = CrearTexto(Nombre, Tamano, true);
	Titulo->SetFont(SAEVUSEstilo::FuenteTitulo(Tamano));
	Titulo->SetTextTransformPolicy(ETextTransformPolicy::ToUpper);
	return Titulo;
}

void USAEVUSWidgetInterfaz::RellenarLineas(UVerticalBox* Caja, const FString& Texto, int32 Tamano)
{
	using namespace SAEVUSInterfaz;
	Caja->ClearChildren();
	TArray<FString> Lineas;
	Texto.ParseIntoArray(Lineas, TEXT("\n"), false);
	for (const FString& Linea : Lineas)
	{
		UHorizontalBox* Fila = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		for (const FSAEVUSTrozoTexto& Trozo : TrocearTeclas(Linea))
		{
			UTextBlock* Bloque = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			Bloque->SetText(FText::FromString(Trozo.Texto));
			if (Trozo.bTecla)
			{
				Bloque->SetFont(SAEVUSEstilo::FuenteTecla(Tamano - ReduccionLetraTecla));
				Bloque->SetColorAndOpacity(FSlateColor(SAEVUSEstilo::ColorTextoTecla()));
				UBorder* Tecla = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
				Tecla->SetBrush(SAEVUSEstilo::PincelTecla());
				Tecla->SetPadding(RellenoTecla);
				Tecla->SetContent(Bloque);
				UHorizontalBoxSlot* Ranura = Fila->AddChildToHorizontalBox(Tecla);
				Ranura->SetVerticalAlignment(VAlign_Center);
				Ranura->SetPadding(SeparacionTecla);
			}
			else
			{
				Bloque->SetFont(SAEVUSEstilo::FuenteTexto(Tamano));
				Bloque->SetColorAndOpacity(FSlateColor(Ajuste.ColorTexto));
				Fila->AddChildToHorizontalBox(Bloque)->SetVerticalAlignment(VAlign_Center);
			}
		}
		UVerticalBoxSlot* RanuraFila = Caja->AddChildToVerticalBox(Fila);
		RanuraFila->SetHorizontalAlignment(HAlign_Center);
		RanuraFila->SetPadding(FMargin(0.f, SeparacionLineas * 0.5f));
	}
}

void USAEVUSWidgetInterfaz::Construir()
{
	using namespace SAEVUSInterfaz;
	UCanvasPanel* Lienzo = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Lienzo"));
	WidgetTree->RootWidget = Lienzo;

	// Debajo de todo: el destello rojo de los golpes recibidos, a pantalla completa.
	DestelloDanio = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DestelloDanio"));
	DestelloDanio->SetBrush(SAEVUSEstilo::PincelRecto(Ajuste.ColorDestelloDanio));
	UCanvasPanelSlot* RanuraDestello = Lienzo->AddChildToCanvas(DestelloDanio);
	RanuraDestello->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	RanuraDestello->SetOffsets(FMargin(0.f));
	DestelloDanio->SetRenderOpacity(0.f);
	DestelloDanio->SetVisibility(ESlateVisibility::Collapsed);

	// Arriba a la izquierda (REF-08 A): emblema en rombo y, a su derecha, la barra de vida y debajo la de alma.
	UHorizontalBox* Vitales = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Vitales"));
	UCanvasPanelSlot* RanuraVitales = Lienzo->AddChildToCanvas(Vitales);
	RanuraVitales->SetAnchors(FAnchors(0.f, 0.f));
	RanuraVitales->SetPosition(Ajuste.Margen);
	RanuraVitales->SetAutoSize(true);

	// Emblema: rombo oscuro con marco fino y el ojo cian del protagonista en el centro.
	USizeBox* HuecoEmblema = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HuecoEmblema"));
	HuecoEmblema->SetWidthOverride(Ajuste.LadoEmblema * Raiz2);
	HuecoEmblema->SetHeightOverride(Ajuste.LadoEmblema * Raiz2);
	UBorder* Emblema = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Emblema"));
	Emblema->SetBrush(SAEVUSEstilo::PincelRecto(Ajuste.ColorFondoBarra, Ajuste.ColorMarco, 2.f));
	Emblema->SetPadding(FMargin(Ajuste.LadoEmblema * (0.5f - FraccionOjoEmblema * 0.5f)));
	UImage* Ojo = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("OjoEmblema"));
	Ojo->SetBrush(SAEVUSEstilo::PincelRecto(Ajuste.ColorOjoEmblema));
	Emblema->SetContent(Ojo);
	Emblema->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Emblema->SetRenderTransformAngle(45.f);
	USizeBox* LadoEmblema = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("LadoEmblema"));
	LadoEmblema->SetWidthOverride(Ajuste.LadoEmblema);
	LadoEmblema->SetHeightOverride(Ajuste.LadoEmblema);
	LadoEmblema->SetContent(Emblema);
	HuecoEmblema->SetContent(LadoEmblema);
	if (USizeBoxSlot* RanuraLado = Cast<USizeBoxSlot>(LadoEmblema->Slot))
	{
		RanuraLado->SetHorizontalAlignment(HAlign_Center);
		RanuraLado->SetVerticalAlignment(VAlign_Center);
	}
	UHorizontalBoxSlot* RanuraEmblema = Vitales->AddChildToHorizontalBox(HuecoEmblema);
	RanuraEmblema->SetVerticalAlignment(VAlign_Center);
	RanuraEmblema->SetPadding(FMargin(0.f, 0.f, SeparacionEmblemaBarras, 0.f));

	UVerticalBox* Barras = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Barras"));
	Vitales->AddChildToHorizontalBox(Barras)->SetVerticalAlignment(VAlign_Center);
	UProgressBar* Rastro = nullptr;
	BarraVida = CrearBarra(TEXT("BarraVida"), Barras, Ajuste.TamanoBarraVida, Ajuste.ColorVida, &Rastro);
	RastroVida = Rastro;
	BarraAlma = CrearBarra(TEXT("BarraAlma"), Barras, Ajuste.TamanoBarraAlma, Ajuste.ColorAlma);
	BarraAlma->SetRenderTransformPivot(FVector2D(0.f, 0.5f));

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
	TituloAviso = CrearTitulo(TEXT("TituloAviso"), Ajuste.TamanoTituloAviso);
	BloqueAviso->AddChildToVerticalBox(TituloAviso)->SetHorizontalAlignment(HAlign_Center);
	UBorder* PanelAviso = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PanelAviso"));
	PanelAviso->SetBrush(SAEVUSEstilo::PincelPanel());
	PanelAviso->SetPadding(RellenoPanelAviso);
	LineasAviso = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LineasAviso"));
	PanelAviso->SetContent(LineasAviso);
	UVerticalBoxSlot* RanuraPanel = BloqueAviso->AddChildToVerticalBox(PanelAviso);
	RanuraPanel->SetHorizontalAlignment(HAlign_Center);
	RanuraPanel->SetPadding(FMargin(0.f, SeparacionTituloPanel, 0.f, 0.f));
	BloqueAviso->SetVisibility(ESlateVisibility::Collapsed);

	// Abajo al centro: nombre y barra del jefe activo.
	BloqueJefe = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Jefe"));
	UCanvasPanelSlot* RanuraJefe = Lienzo->AddChildToCanvas(BloqueJefe);
	RanuraJefe->SetAnchors(FAnchors(0.5f, 1.f));
	RanuraJefe->SetAlignment(FVector2D(0.5f, 1.f));
	RanuraJefe->SetPosition(FVector2D(0.f, -AlturaBarraJefeDesdeAbajo));
	RanuraJefe->SetAutoSize(true);
	// Nombre en la tipografía de títulos, color hueso, con la fase a la derecha (REF-08 A: "Rey Goblin ... Fase 2").
	UHorizontalBox* CabeceraJefe = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CabeceraJefe"));
	NombreJefe = CrearTitulo(TEXT("NombreJefe"), Ajuste.TamanoTextoArma + 4);
	NombreJefe->SetColorAndOpacity(FSlateColor(Ajuste.ColorTextoHueso));
	CabeceraJefe->AddChildToHorizontalBox(NombreJefe)->SetVerticalAlignment(VAlign_Bottom);
	FaseJefe = CrearTitulo(TEXT("FaseJefe"), Ajuste.TamanoTextoArma - 4);
	FaseJefe->SetColorAndOpacity(FSlateColor(Ajuste.ColorVidaJefe * 2.f));
	UHorizontalBoxSlot* RanuraFase = CabeceraJefe->AddChildToHorizontalBox(FaseJefe);
	RanuraFase->SetVerticalAlignment(VAlign_Bottom);
	RanuraFase->SetPadding(FMargin(SeparacionEmblemaBarras, 0.f, 0.f, 2.f));
	BloqueJefe->AddChildToVerticalBox(CabeceraJefe)->SetHorizontalAlignment(HAlign_Center);
	BarraJefe = CrearBarra(TEXT("BarraJefe"), BloqueJefe, Ajuste.TamanoBarraJefe, Ajuste.ColorVidaJefe, &Rastro);
	RastroJefe = Rastro;
	// La barra está en un solapamiento dentro del marco: el marco es el hijo de la caja.
	UVerticalBoxSlot* RanuraBarra = Cast<UVerticalBoxSlot>(BarraJefe->GetParent()->GetParent()->Slot);
	RanuraBarra->SetHorizontalAlignment(HAlign_Center);
	RanuraBarra->SetPadding(FMargin(0.f, SeparacionLineas, 0.f, 0.f));
	BloqueJefe->SetVisibility(ESlateVisibility::Collapsed);

	// Título de zona al entrar: nombre grande y subtítulo, centrados en el tercio de arriba.
	BloqueTituloZona = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TituloZona"));
	UCanvasPanelSlot* RanuraTituloZona = Lienzo->AddChildToCanvas(BloqueTituloZona);
	RanuraTituloZona->SetAnchors(FAnchors(0.5f, Ajuste.AlturaTituloZona));
	RanuraTituloZona->SetAlignment(FVector2D(0.5f, 0.5f));
	RanuraTituloZona->SetAutoSize(true);
	UTextBlock* NombreZona = CrearTitulo(TEXT("NombreZona"), Ajuste.TamanoTituloZona);
	NombreZona->SetColorAndOpacity(FSlateColor(Ajuste.ColorTextoHueso));
	UTextBlock* SubtituloZona = CrearTexto(TEXT("SubtituloZona"), Ajuste.TamanoTextoAviso, true);
	// Dos rayas finas color hueso enmarcan el nombre y crecen desde el centro al aparecer.
	auto CrearRaya = [&]()
	{
		USizeBox* Raya = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Raya->SetHeightOverride(GrosorRayaZona);
		Raya->SetWidthOverride(0.f);
		UImage* Linea = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		Linea->SetBrush(SAEVUSEstilo::PincelRecto(Ajuste.ColorTextoHueso * 0.7f));
		Raya->SetContent(Linea);
		UVerticalBoxSlot* Ranura = BloqueTituloZona->AddChildToVerticalBox(Raya);
		Ranura->SetHorizontalAlignment(HAlign_Center);
		Ranura->SetPadding(FMargin(0.f, SeparacionTituloPanel * 2.f));
		RayasZona.Add(Raya);
	};
	BloqueTituloZona->AddChildToVerticalBox(SubtituloZona)->SetHorizontalAlignment(HAlign_Center);
	CrearRaya();
	BloqueTituloZona->AddChildToVerticalBox(NombreZona)->SetHorizontalAlignment(HAlign_Center);
	CrearRaya();
	Zona = ASAEVUSInfoZona::Obtener(GetWorld());
	if (Zona.IsValid())
	{
		NombreZona->SetText(Zona->Nombre);
		SubtituloZona->SetText(Zona->Subtitulo);
	}
	BloqueTituloZona->SetRenderOpacity(0.f);
	BloqueTituloZona->SetVisibility(ESlateVisibility::Collapsed);
	TiempoEntrada = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

	// Abajo al centro, encima del jefe: lo que se puede usar ("[E]  Sentarse", con la tecla dibujada).
	CajaInteraccion = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Interaccion"));
	UCanvasPanelSlot* RanuraInteraccion = Lienzo->AddChildToCanvas(CajaInteraccion);
	RanuraInteraccion->SetAnchors(FAnchors(0.5f, 1.f));
	RanuraInteraccion->SetAlignment(FVector2D(0.5f, 1.f));
	RanuraInteraccion->SetPosition(FVector2D(0.f, -AlturaInteraccionDesdeAbajo));
	RanuraInteraccion->SetAutoSize(true);
	CajaInteraccion->SetVisibility(ESlateVisibility::Collapsed);

	// Pantalla de muerte: la pantalla se oscurece y, a lo ancho, una banda negra con el texto en rojo
	// sangre seca, que crece despacio. Por encima de todo.
	VeloMuerte = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("VeloMuerte"));
	VeloMuerte->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.6f));
	VeloMuerte->SetHorizontalAlignment(HAlign_Fill);
	VeloMuerte->SetVerticalAlignment(VAlign_Center);
	UCanvasPanelSlot* RanuraVelo = Lienzo->AddChildToCanvas(VeloMuerte);
	RanuraVelo->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	RanuraVelo->SetOffsets(FMargin(0.f));
	USizeBox* AltoBanda = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("AltoBandaMuerte"));
	AltoBanda->SetHeightOverride(Ajuste.AlturaBandaMuerte);
	UBorder* Banda = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BandaMuerte"));
	Banda->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.8f));
	Banda->SetHorizontalAlignment(HAlign_Center);
	Banda->SetVerticalAlignment(VAlign_Center);
	TextoMuerte = CrearTitulo(TEXT("TextoMuerte"), Ajuste.TamanoTextoMuerte);
	TextoMuerte->SetText(NSLOCTEXT("SAEVUSInterfaz", "HasCaido", "Has caído"));
	TextoMuerte->SetColorAndOpacity(FSlateColor(Ajuste.ColorTextoMuerte));
	TextoMuerte->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Banda->SetContent(TextoMuerte);
	AltoBanda->SetContent(Banda);
	VeloMuerte->SetContent(AltoBanda);
	VeloMuerte->SetRenderOpacity(0.f);
	VeloMuerte->SetVisibility(ESlateVisibility::Collapsed);
}

UProgressBar* USAEVUSWidgetInterfaz::CrearBarra(FName Nombre, UPanelWidget* Caja, const FVector2D& Tamano, const FLinearColor& Color, UProgressBar** OutRastro)
{
	using namespace SAEVUSInterfaz;
	// Marco fino gris alrededor de la barra; la barra, recta, sobre fondo casi negro; debajo, el rastro del daño.
	UBorder* Marco = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Marco->SetBrush(SAEVUSEstilo::PincelRecto(Ajuste.ColorFondoBarra, Ajuste.ColorMarco));
	Marco->SetPadding(FMargin(1.f));
	FProgressBarStyle Estilo;
	Estilo.BackgroundImage = SAEVUSEstilo::PincelRecto(FLinearColor::Transparent);
	Estilo.BackgroundImage.ImageSize = Tamano;
	Estilo.FillImage = SAEVUSEstilo::PincelRecto(FLinearColor::White);
	Estilo.FillImage.ImageSize = Tamano;
	Estilo.MarqueeImage = Estilo.FillImage;
	auto Nueva = [&](FName NombreBarra, const FLinearColor& ColorBarra)
	{
		UProgressBar* Barra = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), NombreBarra);
		Barra->SetWidgetStyle(Estilo);
		Barra->SetFillColorAndOpacity(ColorBarra);
		Barra->SetPercent(1.f);
		return Barra;
	};
	UOverlay* Capas = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	if (OutRastro)
	{
		*OutRastro = Nueva(*(Nombre.ToString() + TEXT("Rastro")), Ajuste.ColorRastroDanio);
		Capas->AddChildToOverlay(*OutRastro);
	}
	UProgressBar* Barra = Nueva(Nombre, Color);
	Capas->AddChildToOverlay(Barra);
	Marco->SetContent(Capas);
	UPanelSlot* Ranura = Caja->AddChild(Marco);
	if (UVerticalBoxSlot* RanuraVertical = Cast<UVerticalBoxSlot>(Ranura))
	{
		RanuraVertical->SetPadding(FMargin(0.f, 0.f, 0.f, SeparacionBarras));
		RanuraVertical->SetHorizontalAlignment(HAlign_Left);
	}
	return Barra;
}

void USAEVUSWidgetInterfaz::Aplicar(const FSAEVUSEstadoInterfaz& Estado)
{
	using namespace SAEVUSInterfaz;
	if (!BarraVida)
	{
		return;
	}
	// La vida es la suma de las máscaras del estado (cada una vale VidaPorMascara).
	float Llenas = 0.f;
	for (const float Mascara : Estado.Mascaras)
	{
		Llenas += Mascara;
	}
	FijarPorcentaje(BarraVida, Estado.Mascaras.Num() > 0 ? Llenas / Estado.Mascaras.Num() : 0.f);

	FijarPorcentaje(BarraAlma, Estado.FraccionAlma);
	// Al ganar alma la barra se ilumina un instante; si alcanza para curar, brilla más.
	const FLinearColor ColorBaseAlma = Estado.bAlmaParaCurar ? Ajuste.ColorAlmaLista : Ajuste.ColorAlma;
	BarraAlma->SetFillColorAndOpacity(FMath::Lerp(ColorBaseAlma, FLinearColor::White, BrilloAlma * 0.7f));
	// Al curar la barra de alma late.
	const float Latido = Estado.bCurando ? 1.f + AmplitudLatido * FMath::Sin(GetWorld()->GetTimeSeconds() * RapidezLatido) : 1.f;
	if (!FMath::IsNearlyEqual(BarraAlma->GetRenderTransform().Scale.Y, Latido))
	{
		BarraAlma->SetRenderScale(FVector2D(1.f, Latido));
	}

	FijarTexto(TextoArma, Estado.TextoArma);
	FijarVisible(TextoArma, !Estado.TextoArma.IsEmpty());
	FijarTexto(TextoMunicion, Estado.TextoMunicion);
	FijarVisible(TextoMunicion, !Estado.TextoMunicion.IsEmpty());

	FijarVisible(BloqueAviso, Estado.bHayAviso);
	if (Estado.bHayAviso)
	{
		FijarTexto(TituloAviso, Estado.TituloAviso);
		// Las líneas solo se rehacen si cambia el texto.
		if (Estado.TextoAviso.ToString() != TextoAvisoMostrado)
		{
			TextoAvisoMostrado = Estado.TextoAviso.ToString();
			RellenarLineas(LineasAviso, TextoAvisoMostrado, Ajuste.TamanoTextoAviso);
		}
		FijarOpacidad(BloqueAviso, Estado.OpacidadAviso);
	}

	FijarVisible(CajaInteraccion, !Estado.TextoInteraccion.IsEmpty());
	if (!Estado.TextoInteraccion.IsEmpty() && Estado.TextoInteraccion.ToString() != TextoInteraccionMostrado)
	{
		TextoInteraccionMostrado = Estado.TextoInteraccion.ToString();
		RellenarLineas(CajaInteraccion, TextoInteraccionMostrado, Ajuste.TamanoTextoArma);
	}
	FijarVisible(BloqueJefe, Estado.bJefeVisible);
	if (Estado.bJefeVisible)
	{
		FijarTexto(NombreJefe, Estado.NombreJefe);
		FijarPorcentaje(BarraJefe, Estado.FraccionVidaJefe);
		FijarTexto(FaseJefe, Estado.FaseJefe > 1 ? FText::Format(NSLOCTEXT("SAEVUS", "FaseJefe", "Fase {0}"), Estado.FaseJefe) : FText::GetEmpty());
	}
}

void USAEVUSWidgetInterfaz::ActualizarTeclaInteraccion()
{
	// La tecla que de verdad está asignada a interactuar (la primera), en vez de una [E] fija.
	const ASAEVUSPersonaje* Personaje = Cast<ASAEVUSPersonaje>(GetOwningPlayerPawn());
	const APlayerController* Controlador = GetOwningPlayer();
	const ULocalPlayer* Jugador = Controlador ? Controlador->GetLocalPlayer() : nullptr;
	const UEnhancedInputLocalPlayerSubsystem* Entrada = Jugador ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Jugador) : nullptr;
	const USAEVUSConfigEntrada* Config = Personaje ? Personaje->ObtenerConfigEntrada() : nullptr;
	if (!Entrada || !Config || !Config->AccionInteractuar)
	{
		return;
	}
	for (const FKey& Tecla : Entrada->QueryKeysMappedToAction(Config->AccionInteractuar))
	{
		if (!Tecla.IsGamepadKey())
		{
			TeclaInteraccion = Tecla.GetDisplayName(false).ToString();
			return;
		}
	}
}

void USAEVUSWidgetInterfaz::AnimarBarras(const FSAEVUSEstadoInterfaz& Estado, float DeltaTime)
{
	using namespace SAEVUSInterfaz;
	// Rastro del daño: lo perdido queda claro un momento y luego se vacía; curar lo pisa en el acto.
	auto Rastro = [&](UProgressBar* Barra, float Fraccion, float& Mostrado, float& Ultima, float& Desde)
	{
		if (!Barra)
		{
			return;
		}
		if (Ultima >= 0.f && Fraccion < Ultima - UE_KINDA_SMALL_NUMBER)
		{
			Desde = 0.f;
		}
		Desde += DeltaTime;
		Ultima = Fraccion;
		Mostrado = Fraccion >= Mostrado || Desde < Ajuste.EsperaRastroDanio ? FMath::Max(Mostrado, Fraccion)
			: FMath::FInterpConstantTo(Mostrado, Fraccion, DeltaTime, Ajuste.RapidezRastroDanio);
		FijarPorcentaje(Barra, Mostrado);
	};
	float Llenas = 0.f;
	for (const float Mascara : Estado.Mascaras)
	{
		Llenas += Mascara;
	}
	const float FraccionVida = Estado.Mascaras.Num() > 0 ? Llenas / Estado.Mascaras.Num() : 0.f;
	// Un golpe recibido: destello rojo en toda la pantalla que se apaga.
	if (UltimaFraccionVida >= 0.f && FraccionVida < UltimaFraccionVida - UE_KINDA_SMALL_NUMBER)
	{
		Destello = 1.f;
	}
	Rastro(RastroVida, FraccionVida, FraccionRastroVida, UltimaFraccionVida, TiempoDesdeDanioVida);
	if (Estado.bJefeVisible)
	{
		Rastro(RastroJefe, Estado.FraccionVidaJefe, FraccionRastroJefe, UltimaFraccionJefe, TiempoDesdeDanioJefe);
	}
	else
	{
		UltimaFraccionJefe = -1.f;
		FraccionRastroJefe = 1.f;
	}
	if (UltimaFraccionAlma >= 0.f && Estado.FraccionAlma > UltimaFraccionAlma + UE_KINDA_SMALL_NUMBER)
	{
		BrilloAlma = 1.f;
	}
	UltimaFraccionAlma = Estado.FraccionAlma;
	BrilloAlma = FMath::Max(0.f, BrilloAlma - DeltaTime / Ajuste.DuracionBrilloAlma);
	Destello = FMath::Max(0.f, Destello - DeltaTime / Ajuste.DuracionDestelloDanio);
	if (DestelloDanio)
	{
		FijarOpacidad(DestelloDanio, Destello * Destello * Ajuste.OpacidadDestelloDanio);
		FijarVisible(DestelloDanio, Destello > 0.f);
	}
}

void USAEVUSWidgetInterfaz::NativeTick(const FGeometry& Geometria, float DeltaTime)
{
	using namespace SAEVUSInterfaz;
	Super::NativeTick(Geometria, DeltaTime);
	TiempoBuscarTecla -= DeltaTime;
	if (TiempoBuscarTecla <= 0.f)
	{
		TiempoBuscarTecla = IntervaloBuscarTecla;
		ActualizarTeclaInteraccion();
	}
	// La vida por máscara es la del personaje (la misma que suman los fragmentos); sin él, la del ajuste.
	const ASAEVUSPersonaje* Personaje = Cast<ASAEVUSPersonaje>(GetOwningPlayerPawn());
	const float VidaPorMascara = Personaje ? Personaje->ObtenerProgreso().VidaPorMascara : Ajuste.VidaPorMascara;
	const FSAEVUSEstadoInterfaz Estado = FSAEVUSEstadoInterfaz::Calcular(GetOwningPlayerPawn(), GetWorld(), VidaPorMascara, Ajuste.FundidoAviso, TeclaInteraccion);
	if (BloqueTituloZona && Zona.IsValid() && !bTituloZonaTerminado)
	{
		// Tiempo de mundo: con el menú principal (pausa) el título espera a que empiece la partida. Ya apagado, no se
		// vuelve a calcular.
		const float Segundos = static_cast<float>(GetWorld()->GetTimeSeconds() - TiempoEntrada);
		const float Opacidad = OpacidadTituloZona(Segundos, Ajuste);
		FijarOpacidad(BloqueTituloZona, Opacidad);
		FijarVisible(BloqueTituloZona, Opacidad > 0.f);
		for (USizeBox* Raya : RayasZona)
		{
			Raya->SetWidthOverride(Ajuste.AnchoRayasZona * Opacidad);
		}
		bTituloZonaTerminado = Opacidad <= 0.f && Segundos > Ajuste.RetardoTituloZona + Ajuste.FundidoTituloZona;
	}
	if (VeloMuerte)
	{
		// Fundido a oscuro al caer, y de vuelta (más rápido) al reaparecer; el texto crece despacio mientras se ve.
		const float Objetivo = Estado.bJugadorMuerto ? Ajuste.OpacidadMuerte : 0.f;
		const float Rapidez = Ajuste.OpacidadMuerte / (Estado.bJugadorMuerto ? Ajuste.FundidoMuerte : Ajuste.FundidoReaparicion);
		OpacidadVelo = FMath::FInterpConstantTo(OpacidadVelo, Objetivo, DeltaTime, Rapidez);
		TiempoMuerto = Estado.bJugadorMuerto ? TiempoMuerto + DeltaTime : 0.f;
		FijarOpacidad(VeloMuerte, OpacidadVelo / FMath::Max(Ajuste.OpacidadMuerte, UE_KINDA_SMALL_NUMBER));
		FijarVisible(VeloMuerte, OpacidadVelo > 0.f);
		if (OpacidadVelo > 0.f && TextoMuerte)
		{
			TextoMuerte->SetRenderScale(FVector2D(1.f + Ajuste.CrecimientoTextoMuerte * FMath::Min(1.f, TiempoMuerto / (Ajuste.FundidoMuerte * 3.f))));
		}
	}
	AnimarBarras(Estado, DeltaTime);
	Aplicar(Estado);
}
