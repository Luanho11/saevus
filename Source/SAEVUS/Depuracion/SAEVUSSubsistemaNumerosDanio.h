// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSSubsistemaNumerosDanio.generated.h"

/** Un número de daño flotante: dónde y cuándo se hizo el daño. */
struct FSAEVUSNumeroDanio
{
	FVector Posicion = FVector::ZeroVector;
	float Danio = 0.f;
	double Tiempo = 0.0;
	/** Daño recibido por el jugador (se pinta en otro color). */
	bool bAlJugador = false;
};

namespace SAEVUSNumerosDanio
{
	/** Segundos que vive un número en pantalla. */
	constexpr float Duracion = 0.9f;
	/** Lo que sube un número durante su vida, en cm. */
	constexpr float Subida = 90.f;
	/** Tope de números vivos; los más viejos se descartan. */
	constexpr int32 Maximo = 64;
}

/**
 * Números de daño flotantes para ajustar el equilibrio jugando (SAEVUS.Depuracion.NumerosDanio).
 * La vida y los muñecos apuntan aquí cada daño hecho; ASAEVUSHUDDepuracion los dibuja subiendo
 * y desvaneciéndose. Con la variable apagada no se apunta nada.
 */
UCLASS()
class SAEVUS_API USAEVUSSubsistemaNumerosDanio : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Apunta un daño en Posicion si los números están activos (no hace nada sin mundo o sin daño). */
	static void Registrar(const UObject* Contexto, const FVector& Posicion, float Danio, bool bAlJugador);

	/** Quita los números que ya cumplieron su duración y devuelve los vivos. */
	const TArray<FSAEVUSNumeroDanio>& ObtenerNumeros();

	/** Fracción de vida consumida (0 recién hecho, 1 a punto de desaparecer). */
	float ObtenerEdad(const FSAEVUSNumeroDanio& Numero) const;

private:
	TArray<FSAEVUSNumeroDanio> Numeros;
};
