// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

/**
 * Detecta el doble toque de una dirección (pulsar, soltar, pulsar) en el eje de movimiento 2D.
 * Lógica pura: recibe el eje en cada actualización con su tiempo y devuelve la dirección tocada.
 *
 * Direcciones locales: X = derecha, Y = adelante (como el eje de IA_Move). Cada una de las cuatro
 * (adelante, atrás, derecha, izquierda) se sigue por separado, así mantener W y tocar dos veces D
 * cuenta como doble toque a la derecha.
 */
class FSAEVUSDetectorDobleToque
{
public:
	/** Ventana máxima entre la primera y la segunda pulsación, en segundos. */
	float Ventana = 0.25f;

	/** Valor del eje a partir del cual una dirección cuenta como pulsada. */
	float Umbral = 0.5f;

	/**
	 * Pasa el eje actual en el instante Tiempo (segundos). Devuelve la dirección local del doble
	 * toque (vector unitario en X o Y) si se acaba de completar uno; cero en otro caso.
	 */
	FVector2D Actualizar(const FVector2D& Eje, double Tiempo)
	{
		FVector2D Resultado = FVector2D::ZeroVector;
		for (int32 Indice = 0; Indice < NumDirecciones; ++Indice)
		{
			FDireccion& Direccion = Direcciones[Indice];
			const bool bPulsada = FVector2D::DotProduct(Eje, Vectores()[Indice]) >= Umbral;
			if (bPulsada && !Direccion.bPulsada)
			{
				// Flanco de subida: segunda pulsación dentro de la ventana = doble toque.
				if (Direccion.TiempoUltimaPulsacion >= 0.0 && Tiempo - Direccion.TiempoUltimaPulsacion <= Ventana)
				{
					Resultado = Vectores()[Indice];
					// Consumido: una tercera pulsación empieza otra serie.
					Direccion.TiempoUltimaPulsacion = -1.0;
				}
				else
				{
					Direccion.TiempoUltimaPulsacion = Tiempo;
				}
			}
			Direccion.bPulsada = bPulsada;
		}
		return Resultado;
	}

	void Reiniciar()
	{
		for (FDireccion& Direccion : Direcciones)
		{
			Direccion = FDireccion();
		}
	}

private:
	static constexpr int32 NumDirecciones = 4;

	static const FVector2D* Vectores()
	{
		static const FVector2D Lista[NumDirecciones] = { FVector2D(0.f, 1.f), FVector2D(0.f, -1.f), FVector2D(1.f, 0.f), FVector2D(-1.f, 0.f) };
		return Lista;
	}

	struct FDireccion
	{
		bool bPulsada = false;
		double TiempoUltimaPulsacion = -1.0;
	};

	FDireccion Direcciones[NumDirecciones];
};
