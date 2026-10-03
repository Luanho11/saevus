// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSMunecoEntrenamiento.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Blanco para probar ataques: una cápsula del tamaño de un pawn que cuenta los golpes y el daño
 * que recibe y los muestra encima. Nunca muere, ni se mueve, ni contraataca (no es un enemigo).
 */
UCLASS()
class SAEVUS_API ASAEVUSMunecoEntrenamiento : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSMunecoEntrenamiento();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Muneco")
	int32 ObtenerGolpes() const { return Golpes; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Muneco")
	float ObtenerDanioTotal() const { return DanioTotal; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Muneco")
	float ObtenerUltimoDanio() const { return UltimoDanio; }

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Muneco")
	void ReiniciarContadores();

protected:
	virtual void BeginPlay() override;

private:
	void ActualizarLectura();

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UCapsuleComponent> Capsula;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Cuerpo;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UTextRenderComponent> Lectura;

	int32 Golpes = 0;
	float DanioTotal = 0.f;
	float UltimoDanio = 0.f;
};
