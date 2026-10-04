// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Audio/SAEVUSSintesis.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SAEVUSComponenteSonido.generated.h"

class USAEVUSComponenteVida;

/** Los sonidos boceto del personaje y sus volúmenes por categoría. */
USTRUCT(BlueprintType)
struct SAEVUS_API FSAEVUSAjusteSonidos
{
	GENERATED_BODY()

	FSAEVUSAjusteSonidos();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volumen", meta = (ClampMin = "0", ClampMax = "1"))
	float VolumenGeneral = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volumen", meta = (ClampMin = "0", ClampMax = "1"))
	float VolumenCombate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Volumen", meta = (ClampMin = "0", ClampMax = "1"))
	float VolumenMovimiento = 0.7f;

	/** Combate: el barrido del tajo, el impacto, el disparo, el daño recibido y la curación. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combate")
	FSAEVUSSonidoSintetizado Tajo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combate")
	FSAEVUSSonidoSintetizado Golpe;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combate")
	FSAEVUSSonidoSintetizado Disparo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combate")
	FSAEVUSSonidoSintetizado Danio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combate")
	FSAEVUSSonidoSintetizado Curacion;

	/** Choque metálico del bloqueo de la guardia. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combate")
	FSAEVUSSonidoSintetizado Bloqueo;

	/** Contraataque: campanada brillante, distinta del bloqueo normal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combate")
	FSAEVUSSonidoSintetizado Contraataque;

	/** Movimiento: dash y doble salto. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento")
	FSAEVUSSonidoSintetizado Dash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento")
	FSAEVUSSonidoSintetizado DobleSalto;
};

/**
 * Sonido boceto del personaje: sintetiza cada efecto una vez al empezar y lo reproduce cuando
 * llega su evento (tajo, golpe, disparo, daño, curación, dash, doble salto). Sin archivos de audio.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteSonido : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Sonidos reproducidos desde el inicio (pruebas y depuración). */
	int32 ObtenerReproducciones() const { return Reproducciones; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Sonido")
	FSAEVUSAjusteSonidos Ajuste;

private:
	/** Un efecto ya sintetizado, en bytes PCM listos para la onda procedural. */
	struct FEfecto
	{
		TArray<uint8> Datos;
		float Duracion = 0.f;
	};

	enum class EEfecto : uint8 { Tajo, Golpe, Disparo, Danio, Curacion, Bloqueo, Contraataque, Dash, DobleSalto, Total };

	void Preparar(EEfecto Efecto, const FSAEVUSSonidoSintetizado& Sonido);
	void Reproducir(EEfecto Efecto, float VolumenCategoria);

	void AlIniciarAtaque();
	void AlIniciarDash(const FVector& Direccion);
	void AlSaltoAereo();
	void AlCurar(float VidaRecuperada);
	void AlBloquear(AActor* Atacante, const FVector& PuntoContacto);
	void AlContraatacar(AActor* Atacante, const FVector& PuntoContacto);

	UFUNCTION()
	void AlGolpear(AActor* ActorGolpeado, float Cantidad);

	UFUNCTION()
	void AlDisparar(FVector Boca, FVector Impacto, AActor* ActorGolpeado);

	UFUNCTION()
	void AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador);

	FEfecto Efectos[static_cast<int32>(EEfecto::Total)];
	TArray<TPair<TWeakObjectPtr<UObject>, FDelegateHandle>> Manejadores;
	int32 Reproducciones = 0;
};
