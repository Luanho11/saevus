// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

/** Piezas de estilo compartidas por la interfaz y los menús (sin texturas: todo con pinceles de Slate). */
namespace SAEVUSEstilo
{
	/** Grosor del borde de los pinceles con borde. */
	constexpr float GrosorBorde = 2.f;

	/** Pincel redondeado sin textura: círculo o píldora (radio = media altura). Borde transparente = sin borde. */
	SAEVUS_API FSlateBrush PincelRedondo(const FLinearColor& Color, const FLinearColor& Borde = FLinearColor::Transparent);

	/**
	 * Tipografía (Roboto del motor, sin fuentes externas): títulos en Roboto fina con letras espaciadas,
	 * como un rótulo grabado; texto corrido en Roboto normal con un contorno oscuro fino, legible sobre
	 * cualquier fondo; teclas en negrita.
	 */
	SAEVUS_API FSlateFontInfo FuenteTitulo(int32 Tamano);
	SAEVUS_API FSlateFontInfo FuenteTexto(int32 Tamano);
	SAEVUS_API FSlateFontInfo FuenteTecla(int32 Tamano);

	/** Tecla dibujada en los avisos: caja clara de esquinas poco redondeadas, con el texto oscuro. */
	SAEVUS_API FSlateBrush PincelTecla();
	SAEVUS_API FLinearColor ColorTextoTecla();

	/** Panel translúcido de los avisos: rectángulo oscuro de esquinas redondeadas con un filo frío. */
	SAEVUS_API FSlateBrush PincelPanel();

	/** Botón de menú de solo texto: sin fondo; al pasar por encima (o con el foco) aparece un velo claro. */
	SAEVUS_API FButtonStyle EstiloBotonMenu();

	/** El mismo botón con el foco del mando o del teclado: se ve como al pasar el ratón por encima. */
	SAEVUS_API FButtonStyle EstiloBotonMenuEnfocado();
}
