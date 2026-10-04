// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Audio/SAEVUSSintesis.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSSubsistemaSonidoMundo.generated.h"

/** Sonidos de objetos del mundo (no de personajes). */
UENUM()
enum class ESAEVUSSonidoMundo : uint8
{
	/** Palanca accionada: chasquido metálico. */
	Palanca,
	/** Puerta o compuerta que se abre: retumbo grave de piedra. */
	Puerta,
	/** Sentarse en un banco: campanada suave y cálida. */
	Banco,
	/** Golpe contra un escudo: choque metálico seco. */
	Escudo,
	Total UMETA(Hidden)
};

/** Sonidos boceto del mundo. Valores de primera pasada. */
USTRUCT()
struct SAEVUS_API FSAEVUSAjusteSonidosMundo
{
	GENERATED_BODY()

	FSAEVUSAjusteSonidosMundo();

	UPROPERTY(EditAnywhere, Category = "Volumen", meta = (ClampMin = "0", ClampMax = "1"))
	float Volumen = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Sonidos")
	FSAEVUSSonidoSintetizado Palanca;

	UPROPERTY(EditAnywhere, Category = "Sonidos")
	FSAEVUSSonidoSintetizado Puerta;

	UPROPERTY(EditAnywhere, Category = "Sonidos")
	FSAEVUSSonidoSintetizado Banco;

	UPROPERTY(EditAnywhere, Category = "Sonidos")
	FSAEVUSSonidoSintetizado Escudo;

	const FSAEVUSSonidoSintetizado& Obtener(ESAEVUSSonidoMundo Sonido) const;
};

/**
 * Sonidos del mundo sintetizados por código: cada uno se sintetiza la primera vez que suena y
 * se guarda; después solo se crea la onda. Los objetos llaman a Reproducir con su posición.
 */
UCLASS()
class SAEVUS_API USAEVUSSubsistemaSonidoMundo : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Suena Sonido en Posicion (no hace nada sin mundo). */
	static void Reproducir(const UObject* Contexto, ESAEVUSSonidoMundo Sonido, const FVector& Posicion);

	int32 ObtenerReproducciones() const { return Reproducciones; }

private:
	void ReproducirEn(ESAEVUSSonidoMundo Sonido, const FVector& Posicion);

	FSAEVUSAjusteSonidosMundo Ajuste;
	/** Bytes PCM por sonido; vacíos hasta la primera vez. */
	TArray<uint8> Datos[static_cast<int32>(ESAEVUSSonidoMundo::Total)];
	int32 Reproducciones = 0;
};
