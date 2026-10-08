// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

enum class ESAEVUSGestoAtaque : uint8;

/**
 * Pose de un gesto de ataque en un instante (SAEVUSGestosAtaque.cpp tiene las claves de cada gesto).
 *
 * Las direcciones van en los ejes del cuerpo: X = derecha, Y = adelante, Z = arriba (la animación las pasa
 * al espacio de la malla). Ángulos en grados: GiroPecho y GiroCadera positivos giran hacia la derecha,
 * Inclinacion positiva va hacia delante, Ladeo positivo hacia la derecha, Volteo positivo vuelca todo el
 * cuerpo hacia delante y GiroRaiz gira todo el cuerpo sobre sí mismo (positivo hacia la derecha).
 */
struct FSAEVUSPoseGesto
{
	FVector Brazo = FVector::ZeroVector;
	FVector Hoja = FVector::ZeroVector;
	float Codo = 0.f;
	FVector BrazoLibre = FVector::ZeroVector;
	float CodoLibre = 0.f;
	float GiroPecho = 0.f;
	float Inclinacion = 0.f;
	float Ladeo = 0.f;
	float GiroCadera = 0.f;
	float GiroRaiz = 0.f;
	float Volteo = 0.f;
	/** En el suelo, cuánto de la zancada del gesto (0–1); en el aire, cuánto se recogen las piernas. */
	float Postura = 0.f;
	/** Bajada de la cadera (cm); negativa sube (de puntillas). */
	float Bajada = 0.f;
};

/** Cómo se pisa en el suelo durante un gesto. */
struct FSAEVUSPisadaGesto
{
	/** El pie izquierdo va delante (revés): la zancada se refleja. */
	bool bPieIzquierdoDelante = false;
	/** Escala de la zancada de ataque del ajuste de animación (1 = la de siempre). */
	float EscalaZancada = 1.f;
};

namespace SAEVUSGestos
{
	/**
	 * Tiempo del gesto en fases: de 0 a 1 la preparación, de 1 a 2 la fase activa y de 2 a 3 la recuperación,
	 * sea cual sea la duración de cada una en los datos del ataque.
	 */
	SAEVUS_API float TiempoEnFases(float Tiempo, float Preparacion, float Activo, float Recuperacion);

	/** Pose del gesto en TiempoFases (0–3). */
	SAEVUS_API FSAEVUSPoseGesto Evaluar(ESAEVUSGestoAtaque Gesto, float TiempoFases);

	SAEVUS_API FSAEVUSPisadaGesto Pisada(ESAEVUSGestoAtaque Gesto);

	/** Cierto si el gesto es aéreo (piernas recogidas en vez de zancada). */
	SAEVUS_API bool EsAereo(ESAEVUSGestoAtaque Gesto);
}
