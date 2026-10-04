// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSEstiloInterfaz.h"
#include "Styling/CoreStyle.h"

namespace SAEVUSEstilo
{
	namespace
	{
		const FLinearColor ColorBotonEncima(0.6f, 0.7f, 0.95f, 0.1f);
		const FLinearColor ColorBotonPulsado(0.6f, 0.7f, 0.95f, 0.2f);
		const FLinearColor ColorBordeBotonEncima(0.75f, 0.82f, 1.f, 0.45f);
		// Relleno interior del botón (horizontal, vertical) y desplazamiento del texto al pulsar.
		const FMargin RellenoBoton(32.f, 6.f);
		const FMargin RellenoBotonPulsado(32.f, 7.f, 32.f, 5.f);
		// Letras espaciadas de los títulos (milésimas de eme) y contorno del texto corrido.
		constexpr int32 EspaciadoTitulo = 220;
		constexpr int32 GrosorContornoTexto = 1;
		const FLinearColor ColorContornoTexto(0.f, 0.f, 0.f, 0.85f);
		// Tecla: caja clara con texto oscuro; panel: oscuro con filo frío.
		const FLinearColor ColorTecla(0.86f, 0.89f, 0.96f, 0.95f);
		const FLinearColor ColorBordeTecla(1.f, 1.f, 1.f, 0.7f);
		const FLinearColor ColorLetraTecla(0.03f, 0.035f, 0.06f, 1.f);
		constexpr float RadioTecla = 4.f;
		const FLinearColor ColorPanel(0.008f, 0.01f, 0.02f, 0.62f);
		const FLinearColor ColorBordePanel(0.6f, 0.7f, 0.9f, 0.25f);
		constexpr float RadioPanel = 10.f;
		constexpr float GrosorBordeFino = 1.f;

		FSlateBrush PincelEsquinas(const FLinearColor& Color, const FLinearColor& Borde, float Radio)
		{
			FSlateBrush Pincel;
			Pincel.DrawAs = ESlateBrushDrawType::RoundedBox;
			Pincel.TintColor = FSlateColor(Color);
			Pincel.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
			Pincel.OutlineSettings.CornerRadii = FVector4(Radio, Radio, Radio, Radio);
			Pincel.OutlineSettings.Color = FSlateColor(Borde);
			Pincel.OutlineSettings.Width = GrosorBordeFino;
			return Pincel;
		}
	}

	FSlateBrush PincelRedondo(const FLinearColor& Color, const FLinearColor& Borde)
	{
		FSlateBrush Pincel;
		Pincel.DrawAs = ESlateBrushDrawType::RoundedBox;
		Pincel.TintColor = FSlateColor(Color);
		Pincel.OutlineSettings.RoundingType = ESlateBrushRoundingType::HalfHeightRadius;
		Pincel.OutlineSettings.Color = FSlateColor(Borde);
		Pincel.OutlineSettings.Width = Borde.A > 0.f ? GrosorBorde : 0.f;
		return Pincel;
	}

	FSlateFontInfo FuenteTitulo(int32 Tamano)
	{
		FSlateFontInfo Fuente = FCoreStyle::GetDefaultFontStyle(TEXT("Light"), Tamano);
		Fuente.LetterSpacing = EspaciadoTitulo;
		return Fuente;
	}

	FSlateFontInfo FuenteTexto(int32 Tamano)
	{
		FFontOutlineSettings Contorno;
		Contorno.OutlineSize = GrosorContornoTexto;
		Contorno.OutlineColor = ColorContornoTexto;
		return FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), Tamano, Contorno);
	}

	FSlateFontInfo FuenteTecla(int32 Tamano)
	{
		return FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), Tamano);
	}

	FSlateBrush PincelTecla()
	{
		return PincelEsquinas(ColorTecla, ColorBordeTecla, RadioTecla);
	}

	FLinearColor ColorTextoTecla()
	{
		return ColorLetraTecla;
	}

	FSlateBrush PincelPanel()
	{
		return PincelEsquinas(ColorPanel, ColorBordePanel, RadioPanel);
	}

	FButtonStyle EstiloBotonMenu()
	{
		FButtonStyle Estilo;
		Estilo.SetNormal(FSlateNoResource());
		Estilo.SetHovered(PincelRedondo(ColorBotonEncima, ColorBordeBotonEncima));
		Estilo.SetPressed(PincelRedondo(ColorBotonPulsado, ColorBordeBotonEncima));
		Estilo.SetNormalPadding(RellenoBoton);
		Estilo.SetPressedPadding(RellenoBotonPulsado);
		return Estilo;
	}

	FButtonStyle EstiloBotonMenuEnfocado()
	{
		FButtonStyle Estilo = EstiloBotonMenu();
		Estilo.SetNormal(PincelRedondo(ColorBotonEncima, ColorBordeBotonEncima));
		return Estilo;
	}
}
