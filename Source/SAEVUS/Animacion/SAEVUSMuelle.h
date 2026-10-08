// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

/** Cero de los tipos del muelle (FVector no se inicia solo a cero). */
template <typename T>
inline T SAEVUSCeroMuelle() { return static_cast<T>(0); }

template <>
inline FVector SAEVUSCeroMuelle<FVector>() { return FVector::ZeroVector; }

/** Comprobación de valores finitos para los tipos del muelle. */
inline bool SAEVUSEsFinitoMuelle(float Valor) { return FMath::IsFinite(Valor); }
inline bool SAEVUSEsFinitoMuelle(const FVector& Valor) { return !Valor.ContainsNaN(); }

/**
 * Muelle amortiguado de segundo orden: el valor persigue a su objetivo con inercia, se pasa un poco y se
 * asienta (amortiguación < 1) o llega sin pasarse (1). Es lo que da peso a la animación procedural: la
 * cadera que se queda atrás al girar, el tronco que sigue hacia delante al frenar, la capa que ondea.
 *
 * Frecuencia en Hz (cuántas oscilaciones por segundo), Amortiguacion adimensional (0 = rebota siempre,
 * 1 = crítica). Se integra con Euler implícito en subpasos: es estable con cualquier fotograma y frecuencia
 * (el semiimplícito explotaba a NaN con la cadera a ~40 Hz y un tirón de 0,1 s, auditoría ANIMVFX-1).
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
		// Un valor no finito (NaN o infinito) no se recupera solo: se descarta y se parte del objetivo.
		if (!SAEVUSEsFinitoMuelle(Valor) || !SAEVUSEsFinitoMuelle(Velocidad))
		{
			Fijar(Objetivo);
			return;
		}
		const float Omega = UE_TWO_PI * FMath::Clamp(Frecuencia, 0.01f, FrecuenciaMaxima);
		const float Rigidez = Omega * Omega;
		const float Friccion = 2.f * FMath::Max(Amortiguacion, 0.f) * Omega;
		const float TiempoTotal = FMath::Min(DeltaTime, TiempoMaximo);
		const int32 Pasos = FMath::Clamp(FMath::CeilToInt(TiempoTotal / PasoMaximo), 1, MaxSubpasos);
		const float Paso = TiempoTotal / static_cast<float>(Pasos);
		// Euler implícito: v' = (v + h·k·(objetivo − x)) / (1 + h·c + h²·k); x' = x + h·v'. Incondicionalmente estable.
		const float Divisor = 1.f + Paso * Friccion + Paso * Paso * Rigidez;
		for (int32 Indice = 0; Indice < Pasos; ++Indice)
		{
			Velocidad = (Velocidad + (Objetivo - Valor) * (Paso * Rigidez)) / Divisor;
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
	// 1/240 s: precisión (el implícito amortigua de más con pasos largos, pero nunca explota).
	static constexpr float PasoMaximo = 1.f / 240.f;
	static constexpr int32 MaxSubpasos = 16;
	/** Un fotograma más largo se recorta: tras un tirón no hace falta simular medio segundo de muelle. */
	static constexpr float TiempoMaximo = 0.1f;
	static constexpr float FrecuenciaMaxima = 20.f;
};

using FSAEVUSMuelle = TSAEVUSMuelle<float>;
using FSAEVUSMuelleVector = TSAEVUSMuelle<FVector>;
