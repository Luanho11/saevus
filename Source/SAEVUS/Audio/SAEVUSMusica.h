// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "SAEVUSMusica.generated.h"

/** Pistas de música del juego; suenan a la vez y se mezclan según lo que pasa. */
UENUM(BlueprintType)
enum class ESAEVUSPistaMusica : uint8
{
	/** Calma: bordón grave, acordes lentos de re menor, campanas lejanas y viento. */
	Exploracion,
	/** Pelea: tambores graves y cuerdas en ostinato sobre los mismos acordes. */
	Combate,
	/** Jefe: más rápido y pesado, con metales y coro en modo frigio. */
	Jefe,
	Total UMETA(Hidden)
};

/**
 * Música generada por código (sin archivos de audio): cada pista es un bucle que se sintetiza una vez,
 * en otro hilo, al empezar la partida. Tono oscuro (re menor / re frigio), aire de catedral en ruinas:
 * reverberación larga, timbres graves y pocas notas. Funciones puras: se prueban sin dispositivo de audio.
 */
namespace SAEVUSMusica
{
	/** Frecuencia de muestreo de la música: baja (la música es grave y suave) y barata en memoria. */
	constexpr int32 Muestreo = 22050;

	/** Duración del bucle de la pista (s): compases enteros, así enlaza sin salto. */
	SAEVUS_API float DuracionPista(ESAEVUSPistaMusica Pista);

	/** Muestras (PCM 16 bits mono) del bucle de la pista; la cola de la reverberación ya va sumada al principio. */
	SAEVUS_API TArray<int16> GenerarPista(ESAEVUSPistaMusica Pista);
}
