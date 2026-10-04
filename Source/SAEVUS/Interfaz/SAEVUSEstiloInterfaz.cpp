// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSEstiloInterfaz.h"

namespace SAEVUSEstilo
{
	namespace
	{
		const FLinearColor ColorBotonNormal(0.03f, 0.035f, 0.06f, 0.85f);
		const FLinearColor ColorBotonEncima(0.12f, 0.15f, 0.26f, 0.95f);
		const FLinearColor ColorBotonPulsado(0.22f, 0.27f, 0.42f, 1.f);
		const FLinearColor ColorBordeBoton(0.45f, 0.5f, 0.7f, 0.5f);
		const FLinearColor ColorBordeBotonEncima(0.75f, 0.82f, 1.f, 0.9f);
		// Relleno interior del botón (horizontal, vertical) y desplazamiento del texto al pulsar.
		const FMargin RellenoBoton(24.f, 6.f);
		const FMargin RellenoBotonPulsado(24.f, 7.f, 24.f, 5.f);
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

	FButtonStyle EstiloBotonMenu()
	{
		FButtonStyle Estilo;
		Estilo.SetNormal(PincelRedondo(ColorBotonNormal, ColorBordeBoton));
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
