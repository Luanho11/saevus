// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Async/Future.h"
#include "Audio/SAEVUSMusica.h"
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSSubsistemaMusica.generated.h"

class UAudioComponent;
class USoundWaveProcedural;

/** Volúmenes y ritmos de la mezcla de la música. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteMusica
{
	GENERATED_BODY()

	/** Volumen de la música respecto a los efectos (el volumen general de las opciones va encima). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Musica", meta = (ClampMin = "0", ClampMax = "1"))
	float Volumen = 0.5f;

	/** Volumen de la calma mientras suena la pelea (queda debajo, como un fondo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Musica", meta = (ClampMin = "0", ClampMax = "1"))
	float CalmaEnPelea = 0.3f;

	/** Un enemigo a esta distancia o menos que persigue o ataca cuenta como pelea. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Musica", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioPelea = 2500.f;

	/** La pelea sigue sonando este tiempo tras el último enemigo activo (no se corta a cada respiro). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Musica", meta = (ClampMin = "0", ForceUnits = "s"))
	float EsperaFinPelea = 4.f;

	/** Rapidez de los fundidos (volumen por segundo): entrar es más rápido que salir. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Musica", meta = (ClampMin = "0.01"))
	float FundidoEntrada = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Musica", meta = (ClampMin = "0.01"))
	float FundidoSalida = 0.3f;

	/** Cada cuánto se mira el estado de los enemigos (s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Musica", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float IntervaloDecision = 0.25f;
};

/**
 * Música del mundo: las tres pistas (SAEVUSMusica) suenan a la vez en bucle y se funden según lo que pasa:
 * calma al explorar, pelea con enemigos activos cerca, jefe con un jefe despierto. Las pistas se generan en
 * otro hilo al empezar (sin parón) y no se crean sin dispositivo de audio (tests, servidor, -NoSound).
 */
UCLASS()
class SAEVUS_API USAEVUSSubsistemaMusica : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Externo) const override;
	virtual void OnWorldBeginPlay(UWorld& Mundo) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	/** Suena también en los menús (el juego en pausa). */
	virtual bool IsTickableWhenPaused() const override { return true; }

	/** Pista que manda ahora (la de más volumen objetivo). */
	ESAEVUSPistaMusica ObtenerPistaActual() const { return PistaActual; }

	/** Decide la pista según los enemigos que hay (lo llama el tick; los tests a mano). */
	ESAEVUSPistaMusica DecidirPista(float DeltaTime);

private:
	/** Crea las ondas y los componentes de audio cuando las pistas ya están generadas. */
	void EmpezarASonar();

	FSAEVUSAjusteMusica Ajuste;

	/** Pistas generadas en otro hilo (bytes PCM); vacío mientras se generan. */
	TFuture<TArray<TArray<uint8>>> Generacion;
	TArray<TSharedPtr<const TArray<uint8>>> Bucles;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAudioComponent>> Componentes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USoundWaveProcedural>> Ondas;

	float Volumenes[static_cast<int32>(ESAEVUSPistaMusica::Total)] = { 0.f, 0.f, 0.f };
	ESAEVUSPistaMusica PistaActual = ESAEVUSPistaMusica::Exploracion;
	float TiempoDesdePelea = TNumericLimits<float>::Max();
	float TiempoDecision = 0.f;
	bool bGenerando = false;
};
