// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * Parada de golpe (hit-stop): congela casi del todo a los actores de un impacto durante un
 * instante, bajando su CustomTimeDilation, y los devuelve a 1 al acabar. El tiempo se cuenta
 * en tiempo real, así la propia parada no la alarga.
 *
 * Es local: en red, la parada del atacante tendrá que ser solo visual (fase de red).
 */
class FSAEVUSParadaGolpe
{
public:
	/** Para a los actores dados Duracion segundos reales a la escala de tiempo Escala (0..1). */
	void Iniciar(TConstArrayView<AActor*> Actores, float Escala, float Duracion)
	{
		if (Duracion <= 0.f || Escala >= 1.f)
		{
			return;
		}
		Terminar();
		for (AActor* Actor : Actores)
		{
			if (Actor)
			{
				Actor->CustomTimeDilation = Escala;
				Parados.Add(Actor);
			}
		}
		TiempoRestante = Duracion;
	}

	/** Avanza la parada TiempoReal segundos (sin dilatar); devuelve a todos al acabar. */
	void Actualizar(float TiempoReal)
	{
		if (TiempoRestante <= 0.f)
		{
			return;
		}
		TiempoRestante -= TiempoReal;
		if (TiempoRestante <= 0.f)
		{
			Terminar();
		}
	}

	void Terminar()
	{
		for (const TWeakObjectPtr<AActor>& Actor : Parados)
		{
			if (Actor.IsValid())
			{
				Actor->CustomTimeDilation = 1.f;
			}
		}
		Parados.Reset();
		TiempoRestante = 0.f;
	}

	bool EstaActiva() const { return TiempoRestante > 0.f; }

private:
	TArray<TWeakObjectPtr<AActor>> Parados;
	float TiempoRestante = 0.f;
};
