// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "SAEVUSSintesis.generated.h"

class USoundWaveProcedural;
class USoundAttenuation;

/**
 * Un sonido boceto descrito por datos: un tono que barre de FrecuenciaInicio a FrecuenciaFin
 * mezclado con ruido filtrado, bajo una envolvente de ataque y caída. Sin archivos de audio:
 * se sintetiza al empezar la partida. Lo sustituirán sonidos hechos a mano.
 */
USTRUCT(BlueprintType)
struct FSAEVUSSonidoSintetizado
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sonido", meta = (ClampMin = "0.01", ForceUnits = "s"))
	float Duracion = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tono", meta = (ClampMin = "0", ForceUnits = "Hz"))
	float FrecuenciaInicio = 440.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tono", meta = (ClampMin = "0", ForceUnits = "Hz"))
	float FrecuenciaFin = 220.f;

	/** 0 = seno puro, 1 = casi cuadrada (más áspera). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tono", meta = (ClampMin = "0", ClampMax = "1"))
	float Aspereza = 0.f;

	/** 0 = solo tono, 1 = solo ruido. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ruido", meta = (ClampMin = "0", ClampMax = "1"))
	float MezclaRuido = 0.f;

	/** Paso bajo del ruido: 1 = sin filtrar (siseo), cerca de 0 = muy grave (golpe sordo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ruido", meta = (ClampMin = "0.001", ClampMax = "1"))
	float FiltroRuido = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Envolvente", meta = (ClampMin = "0", ForceUnits = "s"))
	float Ataque = 0.005f;

	/** Constante de caída exponencial tras el ataque. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Envolvente", meta = (ClampMin = "0.001", ForceUnits = "s"))
	float Caida = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sonido", meta = (ClampMin = "0", ClampMax = "1"))
	float Volumen = 0.6f;

	/** Semilla del ruido: el mismo sonido suena siempre igual. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ruido")
	int32 Semilla = 1;
};

/** Síntesis de muestras (PCM 16 bits mono). Funciones puras: se prueban sin dispositivo de audio. */
namespace SAEVUSSintesis
{
	/** Frecuencia de muestreo de los sonidos boceto: baja, suficiente para efectos y barata. */
	constexpr int32 FrecuenciaMuestreo = 22050;

	SAEVUS_API TArray<int16> Sintetizar(const FSAEVUSSonidoSintetizado& Sonido, int32 Muestreo = FrecuenciaMuestreo);

	/** Las muestras de Sintetizar como bytes PCM, listas para una onda procedural. */
	SAEVUS_API TArray<uint8> SintetizarBytes(const FSAEVUSSonidoSintetizado& Sonido);

	/**
	 * Atenuación común de los sonidos situados en el mundo (enemigos, puertas, palancas): se oyen cerca y con
	 * dirección, y se apagan con la distancia en lugar de sonar a pleno volumen en todo el mapa (auditoría EA-5).
	 */
	SAEVUS_API USoundAttenuation* ObtenerAtenuacion();

	/** Onda procedural de un solo uso con Datos (se consume al sonar). */
	SAEVUS_API USoundWaveProcedural* CrearOnda(UObject* Propietario, const TArray<uint8>& Datos, float Duracion);
}
