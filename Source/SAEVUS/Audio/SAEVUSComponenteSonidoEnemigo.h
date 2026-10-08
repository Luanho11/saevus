// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Audio/SAEVUSSintesis.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Enemigos/SAEVUSTiposEnemigo.h"
#include "SAEVUSComponenteSonidoEnemigo.generated.h"

/** Sonidos boceto de un enemigo. */
USTRUCT(BlueprintType)
struct SAEVUS_API FSAEVUSAjusteSonidosEnemigo
{
	GENERATED_BODY()

	FSAEVUSAjusteSonidosEnemigo();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volumen", meta = (ClampMin = "0", ClampMax = "1"))
	float Volumen = 0.7f;

	/** Gruñido que sube mientras avisa: lectura sonora del ataque que viene. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sonidos")
	FSAEVUSSonidoSintetizado Aviso;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sonidos")
	FSAEVUSSonidoSintetizado Ataque;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sonidos")
	FSAEVUSSonidoSintetizado Muerte;

	/** Voladores: un batido de alas por ciclo de aleteo, solo con el objetivo cerca. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sonidos")
	FSAEVUSSonidoSintetizado Aleteo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sonidos", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaAleteo = 1200.f;
};

/**
 * Sonido boceto de un enemigo: aviso, ataque y muerte, sintetizados al empezar y disparados por
 * los cambios de estado de su IA; los voladores además aletean. Suena en la posición del enemigo.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteSonidoEnemigo : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteSonidoEnemigo();

	int32 ObtenerReproducciones() const { return Reproducciones; }

	/** Prepara el aleteo y el tick si el enemigo vuela (al empezar y cuando cambian sus datos). */
	void ActualizarTipo();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Sonido")
	FSAEVUSAjusteSonidosEnemigo Ajuste;

private:
	void AlCambiarEstado(ESAEVUSEstadoEnemigo NuevoEstado);
	void Reproducir(const TArray<uint8>& Datos, float Duracion, float Volumen = 1.f, float Tono = 1.f);
	/** Tono según el tamaño (los grandes suenan graves) y una pequeña variación por reproducción (EA-6 y EA-29). */
	float TonoBase() const;

	TArray<uint8> DatosAviso;
	TArray<uint8> DatosAtaque;
	TArray<uint8> DatosMuerte;
	TArray<uint8> DatosAleteo;
	float TiempoAleteo = 0.f;
	ESAEVUSEstadoEnemigo EstadoAnterior = ESAEVUSEstadoEnemigo::Reposo;
	FRandomStream Variacion;
	FDelegateHandle Manejador;
	int32 Reproducciones = 0;
};
