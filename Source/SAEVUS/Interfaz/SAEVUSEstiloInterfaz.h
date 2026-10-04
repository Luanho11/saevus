// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

/** Piezas de estilo compartidas por la interfaz y los menús (sin texturas: todo con pinceles de Slate). */
namespace SAEVUSEstilo
{
	/** Grosor del borde de los pinceles con borde. */
	constexpr float GrosorBorde = 2.f;

	/** Pincel redondeado sin textura: círculo o píldora (radio = media altura). Borde transparente = sin borde. */
	SAEVUS_API FSlateBrush PincelRedondo(const FLinearColor& Color, const FLinearColor& Borde = FLinearColor::Transparent);

	/** Botón de menú: píldora oscura translúcida que se aclara al pasar por encima y al pulsar. */
	SAEVUS_API FButtonStyle EstiloBotonMenu();

	/** El mismo botón con el foco del mando o del teclado: se ve como al pasar el ratón por encima. */
	SAEVUS_API FButtonStyle EstiloBotonMenuEnfocado();
}
