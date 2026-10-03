// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SAEVUSComponenteVida.generated.h"

class USAEVUSComponenteVida;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSAEVUSFirmaCambioVida, USAEVUSComponenteVida*, ComponenteVida, float, Delta, AActor*, Instigador);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSAEVUSFirmaMuerte, USAEVUSComponenteVida*, ComponenteVida, AActor*, Asesino);

/**
 * Vida compartida por el jugador y los enemigos. Escucha el daño del motor del dueño
 * (OnTakeAnyDamage), así los golpes de espada y los disparos llegan sin conocerla.
 * Muere una sola vez al llegar a cero; mientras está muerto ignora daño y curación hasta RestablecerVida().
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteVida : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteVida();

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Vida")
	float ObtenerVida() const { return Vida; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Vida")
	float ObtenerVidaMaxima() const { return VidaMaxima; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Vida")
	bool EstaMuerto() const { return bMuerto; }

	/** Quita vida; devuelve cuánta quitó de verdad (0 si está muerto o la cantidad no es positiva). */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Vida")
	float AplicarDanio(float Cantidad, AActor* Causante);

	/** Suma vida hasta VidaMaxima; devuelve cuánta sumó de verdad. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Vida")
	float Curar(float Cantidad);

	/** Ignora todo daño durante Duracion segundos de tiempo de mundo (alarga una invulnerabilidad en curso, nunca la acorta). */
	void HacerInvulnerable(float Duracion);

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Vida")
	bool EstaInvulnerable() const;

	/** Vida completa y vivo otra vez. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Vida")
	void RestablecerVida();

	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Vida")
	FSAEVUSFirmaCambioVida AlCambiarVida;

	/** Se dispara una vez, cuando la vida llega a cero. */
	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Vida")
	FSAEVUSFirmaMuerte AlMorir;

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Vida", meta = (ClampMin = "1"))
	float VidaMaxima = 100.f;

	/** Ignora el daño cuyo causante es el propio dueño (disparos propios, explosiones propias más adelante). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Vida")
	bool bIgnorarDanioPropio = true;

private:
	UFUNCTION()
	void ManejarDanioRecibido(AActor* ActorDaniado, float Danio, const UDamageType* TipoDanio, AController* InstigadoPor, AActor* Causante);

	float Vida = 0.f;
	bool bMuerto = false;
	/** Tiempo de mundo hasta el que se ignora el daño. */
	double InvulnerableHasta = 0.0;
};
