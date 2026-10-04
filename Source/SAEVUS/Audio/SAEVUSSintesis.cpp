// Copyright SAEVUS. Todos los derechos reservados.

#include "Audio/SAEVUSSintesis.h"
#include "Math/RandomStream.h"

namespace SAEVUSSintesis
{
	// Forma de la onda áspera: el seno se satura hacia una cuadrada.
	constexpr float GananciaSaturacion = 8.f;
	constexpr float MaximoInt16 = 32767.f;

	TArray<int16> Sintetizar(const FSAEVUSSonidoSintetizado& Sonido, int32 Muestreo)
	{
		const int32 Numero = FMath::Max(1, FMath::RoundToInt32(Sonido.Duracion * Muestreo));
		TArray<int16> Muestras;
		Muestras.SetNumUninitialized(Numero);

		FRandomStream Aleatorio(Sonido.Semilla);
		float Fase = 0.f;
		float RuidoFiltrado = 0.f;
		for (int32 Indice = 0; Indice < Numero; ++Indice)
		{
			const float Tiempo = static_cast<float>(Indice) / Muestreo;
			const float Fraccion = static_cast<float>(Indice) / Numero;

			const float Envolvente = Tiempo < Sonido.Ataque
				? Tiempo / FMath::Max(Sonido.Ataque, UE_KINDA_SMALL_NUMBER)
				: FMath::Exp(-(Tiempo - Sonido.Ataque) / Sonido.Caida);

			const float Frecuencia = FMath::Lerp(Sonido.FrecuenciaInicio, Sonido.FrecuenciaFin, Fraccion);
			Fase += UE_TWO_PI * Frecuencia / Muestreo;
			const float Seno = FMath::Sin(Fase);
			const float Saturada = FMath::Clamp(Seno * GananciaSaturacion, -1.f, 1.f);
			const float Tono = FMath::Lerp(Seno, Saturada, Sonido.Aspereza);

			RuidoFiltrado += Sonido.FiltroRuido * (Aleatorio.FRandRange(-1.f, 1.f) - RuidoFiltrado);
			const float Valor = FMath::Lerp(Tono, RuidoFiltrado, Sonido.MezclaRuido) * Envolvente * Sonido.Volumen;
			Muestras[Indice] = static_cast<int16>(FMath::Clamp(Valor, -1.f, 1.f) * MaximoInt16);
		}
		return Muestras;
	}
}
