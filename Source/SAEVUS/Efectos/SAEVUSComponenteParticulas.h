// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Components/InstancedStaticMeshComponent.h"
#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "SAEVUSComponenteParticulas.generated.h"

/** Comportamiento de las partículas de un emisor. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteParticulas
{
	GENERATED_BODY()

	/** Partículas vivas a la vez como mucho (tamaño del pool de instancias). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particulas", meta = (ClampMin = "1", ClampMax = "512"))
	int32 MaxParticulas = 96;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particulas", meta = (ClampMin = "0.01", ForceUnits = "s"))
	float Vida = 0.45f;

	/** Escala de la malla al nacer y al morir (la malla por defecto es un cubo de 100 cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particulas", meta = (ClampMin = "0"))
	float TamanoInicial = 0.16f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particulas", meta = (ClampMin = "0"))
	float TamanoFinal = 0.f;

	/** Frenado de la velocidad por segundo (0 = sin frenado). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particulas", meta = (ClampMin = "0"))
	float Arrastre = 5.f;

	/** Aceleración vertical constante: positiva sube (humo, polvo), negativa cae (chispas). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particulas", meta = (ForceUnits = "cm/s^2"))
	float AceleracionVertical = 120.f;

	/** Giro aleatorio máximo por segundo, para que no se vean todas iguales. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particulas", meta = (ClampMin = "0", ForceUnits = "deg/s"))
	float GiroMaximo = 360.f;
};

/**
 * Partículas sencillas por CPU sobre mallas instanciadas: un pool fijo de instancias que se
 * mueven, encogen y desaparecen. Barato (sin translucidez, sin sombras, sin colisión) y sin
 * assets externos; sirve para polvo, estelas y chispas de los bocetos.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteParticulas : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteParticulas();

	/** Lanza una partícula en Posicion (mundo) con Velocidad. Si el pool está lleno, reemplaza la más vieja. */
	void Emitir(const FVector& Posicion, const FVector& Velocidad);

	/** Lanza Cantidad partículas con VelocidadBase más una dispersión aleatoria de hasta Dispersion cm/s. */
	void EmitirRafaga(const FVector& Posicion, const FVector& VelocidadBase, int32 Cantidad, float Dispersion);

	int32 ObtenerParticulasActivas() const;

	const FSAEVUSAjusteParticulas& ObtenerAjuste() const { return Ajuste; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Particulas")
	FSAEVUSAjusteParticulas Ajuste;

	FRandomStream Aleatorio;

private:
	struct FParticula
	{
		FVector Posicion = FVector::ZeroVector;
		FVector Velocidad = FVector::ZeroVector;
		FRotator Rotacion = FRotator::ZeroRotator;
		FRotator Giro = FRotator::ZeroRotator;
		float Edad = 0.f;
		bool bViva = false;
	};

	/** Crea las instancias del pool la primera vez que hacen falta. */
	void PrepararPool();
	void ActualizarInstancias();

	TArray<FParticula> Particulas;
	TArray<FTransform> Transformaciones;
	int32 SiguienteHueco = 0;
	bool bInstanciasSucias = false;
};
