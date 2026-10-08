// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

/**
 * Parada de golpe (hit-stop): congela casi del todo a los actores de un impacto durante un
 * instante, bajando su CustomTimeDilation, y les devuelve la dilatación que tenían al acabar.
 * El tiempo se cuenta en tiempo real del mundo, así la propia parada no la alarga y una escala
 * 0 (congelados del todo) también termina.
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
				Parados.Add({ Actor, Actor->CustomTimeDilation });
				Actor->CustomTimeDilation = FMath::Max(0.f, Escala);
			}
		}
		TiempoRestante = Duracion;
	}

	/** Avanza la parada TiempoReal segundos (tiempo del mundo, sin la dilatación de los actores); devuelve a todos al acabar. */
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
		for (const FParado& Parado : Parados)
		{
			if (Parado.Actor.IsValid())
			{
				Parado.Actor->CustomTimeDilation = Parado.DilatacionPrevia;
			}
		}
		Parados.Reset();
		TiempoRestante = 0.f;
	}

	bool EstaActiva() const { return TiempoRestante > 0.f; }

private:
	struct FParado
	{
		TWeakObjectPtr<AActor> Actor;
		/** Dilatación que tenía antes de la parada (otra mecánica pudo cambiarla). */
		float DilatacionPrevia = 1.f;
	};

	TArray<FParado> Parados;
	float TiempoRestante = 0.f;
};
