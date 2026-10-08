// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSExploracion.generated.h"

struct FSAEVUSBloqueMapa;

/** Celdas del suelo en planta: lo explorado se recuerda por celdas (mapa de la pausa). */
namespace SAEVUSExploracion
{
	/** Lado de una celda: el ancho de un pasillo. */
	constexpr float TamanoCelda = 800.f;
	/** Celdas alrededor del jugador que se descubren a la vez (1 = un cuadrado de 3 × 3). */
	constexpr int32 RadioDescubrir = 1;
	/** Cada cuánto se guarda lo nuevo explorado. */
	constexpr float IntervaloGuardado = 5.f;

	SAEVUS_API FIntPoint Celda(const FVector& Posicion);
	SAEVUS_API FBox2D CajaCelda(const FIntPoint& Celda);

	/** Los trozos de cada bloque que caen en celdas exploradas (un bloque largo se ve solo donde se ha estado). */
	SAEVUS_API TArray<FSAEVUSBloqueMapa> RecortarBloques(const TArray<FSAEVUSBloqueMapa>& Bloques, const TSet<FIntPoint>& Celdas);
}

/**
 * Exploración del mapa (Hollow Knight): marca las celdas por las que pasa el jugador y las guarda
 * por mapa en la partida. El mapa de la pausa solo dibuja lo explorado.
 */
UCLASS()
class SAEVUS_API USAEVUSSubsistemaExploracion : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& Mundo) override;
	virtual void Deinitialize() override;

	//~ Inicio de la interfaz de FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ Fin de la interfaz de FTickableGameObject

	/** Descubre las celdas alrededor de Posicion; cierto si alguna era nueva. */
	bool Descubrir(const FVector& Posicion);

	const TSet<FIntPoint>& ObtenerCeldas() const { return Celdas; }

private:
	void Guardar();

	TSet<FIntPoint> Celdas;
	bool bSucio = false;
	float TiempoSinGuardar = 0.f;
	/** Celda en la que estaba el jugador el último fotograma (se descubre solo al cambiar). */
	FIntPoint CeldaActual = FIntPoint::ZeroValue;
	bool bHayCeldaActual = false;
};
