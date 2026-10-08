// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSPuertaArena.generated.h"

class ASAEVUSEnemigo;
class UStaticMeshComponent;

/**
 * Puerta (un bloque) que se hunde en el suelo cuando mueren todos sus enemigos: cierra una
 * arena del tutorial hasta vaciarla. Sin enemigos asignados se queda cerrada.
 */
UCLASS()
class SAEVUS_API ASAEVUSPuertaArena : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSPuertaArena();

	void FijarEnemigos(const TArray<ASAEVUSEnemigo*>& NuevosEnemigos);

	bool EstaAbierta() const { return bAbierta; }

	/** Sin enemigos asignados es una compuerta: solo la abre una palanca (ASAEVUSPalanca) o Abrir(). */
	bool TieneEnemigos() const { return !Enemigos.IsEmpty(); }

	/** Abre la puerta (sin colisión al instante; se hunde en DuracionApertura). Silenciosa: ya hundida, sin sonido ni aviso. */
	void Abrir(bool bSilenciosa = false);

	/** Vuelve a subir y a bloquear (las puertas de una arena de jefe se cierran al empezar el combate). */
	void Cerrar();

	/** Fracción abierta ahora: 0 cerrada, 1 hundida del todo. */
	float ObtenerFraccionApertura() const { return FraccionApertura; }

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Malla;

	UPROPERTY(EditInstanceOnly, Category = "SAEVUS|Puerta")
	TArray<TObjectPtr<ASAEVUSEnemigo>> Enemigos;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Puerta", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionApertura = 1.2f;

	/** Empieza abierta (la entrada de una arena de jefe: se cierra cuando empieza el combate). */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Puerta")
	bool bAbiertaAlInicio = false;

	/** Aviso al abrirse (vacío = ninguno). */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Puerta")
	FText TituloAlAbrir;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Puerta", meta = (MultiLine = "true"))
	FText TextoAlAbrir;

private:
	void AlMorirEnemigo(ASAEVUSEnemigo* Enemigo);
	bool QuedanEnemigos() const;

	FDelegateHandle Manejador;
	bool bAbierta = false;
	float FraccionApertura = 0.f;
	FVector PosicionCerrada = FVector::ZeroVector;
};
