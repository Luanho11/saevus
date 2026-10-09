// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

/** Resultado de resolver una pierna de dos huesos (muslo y pierna) hacia un objetivo. */
struct FSAEVUSResultadoPierna
{
	/** Posición de la rodilla. */
	FVector Rodilla = FVector::ZeroVector;
	/** Posición del pie (el objetivo, recortado al alcance de la pierna). */
	FVector Pie = FVector::ZeroVector;
};

namespace SAEVUSCinematicaInversa
{
	/** Fracción del largo total que la pierna nunca supera al estirarse (evita la rodilla bloqueada). */
	constexpr float FraccionAlcanceMaxima = 0.999f;

	/**
	 * Cinemática inversa de dos huesos. Cadera fija, largos de muslo y pierna, objetivo del pie y dirección hacia la
	 * que apunta la rodilla (se proyecta sobre el plano perpendicular a la cadera-pie). Si el objetivo no se alcanza,
	 * el pie queda en la línea hacia él, a lo máximo que llega la pierna.
	 */
	inline FSAEVUSResultadoPierna ResolverPierna(const FVector& Cadera, float LargoMuslo, float LargoPierna, const FVector& ObjetivoPie, const FVector& DireccionRodilla)
	{
		FSAEVUSResultadoPierna Resultado;
		const FVector HaciaObjetivo = ObjetivoPie - Cadera;
		const float DistanciaObjetivo = static_cast<float>(HaciaObjetivo.Size());
		if (DistanciaObjetivo < UE_KINDA_SMALL_NUMBER || LargoMuslo < UE_KINDA_SMALL_NUMBER || LargoPierna < UE_KINDA_SMALL_NUMBER)
		{
			Resultado.Rodilla = Cadera;
			Resultado.Pie = ObjetivoPie;
			return Resultado;
		}
		const FVector Eje = HaciaObjetivo / DistanciaObjetivo;
		const float AlcanceMaximo = (LargoMuslo + LargoPierna) * FraccionAlcanceMaxima;
		const float AlcanceMinimo = FMath::Abs(LargoMuslo - LargoPierna) + 1.f;
		const float Distancia = FMath::Clamp(DistanciaObjetivo, AlcanceMinimo, AlcanceMaximo);
		// Ley de cosenos: distancia desde la cadera hasta el pie de la perpendicular de la rodilla, y altura de la rodilla.
		const float Base = (LargoMuslo * LargoMuslo - LargoPierna * LargoPierna + Distancia * Distancia) / (2.f * Distancia);
		const float Altura = FMath::Sqrt(FMath::Max(0.f, LargoMuslo * LargoMuslo - Base * Base));
		FVector Polo = DireccionRodilla - Eje * static_cast<float>(FVector::DotProduct(DireccionRodilla, Eje));
		if (!Polo.Normalize())
		{
			// Rodilla alineada con el eje: cualquier perpendicular sirve.
			Polo = FVector::CrossProduct(Eje, FVector::UpVector);
			if (!Polo.Normalize())
			{
				Polo = FVector::ForwardVector;
			}
		}
		Resultado.Rodilla = Cadera + Eje * Base + Polo * Altura;
		Resultado.Pie = Cadera + Eje * Distancia;
		return Resultado;
	}
}
