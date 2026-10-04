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
};

/**
 * Sonido boceto de un enemigo: aviso, ataque y muerte, sintetizados al empezar y disparados por
 * los cambios de estado de su IA. Suena en la posición del enemigo.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteSonidoEnemigo : public UActorComponent
{
	GENERATED_BODY()

public:
	int32 ObtenerReproducciones() const { return Reproducciones; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Sonido")
	FSAEVUSAjusteSonidosEnemigo Ajuste;

private:
	void AlCambiarEstado(ESAEVUSEstadoEnemigo NuevoEstado);
	void Reproducir(const TArray<uint8>& Datos, float Duracion);

	TArray<uint8> DatosAviso;
	TArray<uint8> DatosAtaque;
	TArray<uint8> DatosMuerte;
	FDelegateHandle Manejador;
	int32 Reproducciones = 0;
};
