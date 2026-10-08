// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

/** Cero de los tipos del muelle (FVector no se inicia solo a cero). */
template <typename T>
inline T SAEVUSCeroMuelle() { return static_cast<T>(0); }

template <>
inline FVector SAEVUSCeroMuelle<FVector>() { return FVector::ZeroVector; }

/**
 * Muelle amortiguado de segundo orden: el valor persigue a su objetivo con inercia, se pasa un poco y se
 * asienta (amortiguación < 1) o llega sin pasarse (1). Es lo que da peso a la animación procedural: la
 * cadera que se queda atrás al girar, el tronco que sigue hacia delante al frenar, la capa que ondea.
 *
 * Frecuencia en Hz (cuántas oscilaciones por segundo), Amortiguacion adimensional (0 = rebota siempre,
 * 1 = crítica). Se integra en subpasos de tamaño fijo, así es estable con fotogramas largos.
 */
template <typename T>
struct TSAEVUSMuelle
{
	T Valor = SAEVUSCeroMuelle<T>();
	T Velocidad = SAEVUSCeroMuelle<T>();

	/** Avanza DeltaTime segundos hacia Objetivo. */
	void Avanzar(const T& Objetivo, float Frecuencia, float Amortiguacion, float DeltaTime)
	{
		if (DeltaTime <= 0.f)
		{
			return;
		}
		const float Omega = UE_TWO_PI * FMath::Max(Frecuencia, 0.01f);
		const float Rigidez = Omega * Omega;
		const float Friccion = 2.f * FMath::Max(Amortiguacion, 0.f) * Omega;
		// Subpasos: el muelle más rígido (Omega alto) con un fotograma largo no debe explotar.
		const int32 Pasos = FMath::Clamp(FMath::CeilToInt(DeltaTime / PasoMaximo), 1, MaxSubpasos);
		const float Paso = DeltaTime / static_cast<float>(Pasos);
		for (int32 Indice = 0; Indice < Pasos; ++Indice)
		{
			// Euler semiimplícito: primero la velocidad, luego la posición con la velocidad nueva.
			Velocidad += ((Objetivo - Valor) * Rigidez - Velocidad * Friccion) * Paso;
			Valor += Velocidad * Paso;
		}
	}

	/** Golpe instantáneo: suma Impulso a la velocidad (aterrizajes, golpes recibidos, arranques). */
	void Impulsar(const T& Impulso) { Velocidad += Impulso; }

	/** Fija el valor sin inercia (al empezar, al reaparecer). */
	void Fijar(const T& NuevoValor)
	{
		Valor = NuevoValor;
		Velocidad = SAEVUSCeroMuelle<T>();
	}

private:
	// 1/240 s: estable hasta frecuencias de ~30 Hz con amortiguación baja.
	static constexpr float PasoMaximo = 1.f / 240.f;
	static constexpr int32 MaxSubpasos = 16;
};

using FSAEVUSMuelle = TSAEVUSMuelle<float>;
using FSAEVUSMuelleVector = TSAEVUSMuelle<FVector>;
