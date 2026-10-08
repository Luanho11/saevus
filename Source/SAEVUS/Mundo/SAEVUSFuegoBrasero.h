// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSFuegoBrasero.generated.h"

class UPointLightComponent;
class USAEVUSComponenteParticulas;

/** Cómo arde un brasero. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteFuego
{
	GENERATED_BODY()

	/** Intensidad media de la luz (candelas) y cuánto titila arriba y abajo (fracción). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luz", meta = (ClampMin = "0"))
	float Intensidad = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luz", meta = (ClampMin = "0", ClampMax = "1"))
	float Titileo = 0.28f;

	/** Rapidez del titileo (ruido suave, no aleatorio puro: la llama respira y da tirones). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luz", meta = (ClampMin = "0"))
	float RitmoTitileo = 3.2f;

	/** La luz baila un poco de sitio con la llama (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luz", meta = (ClampMin = "0", ForceUnits = "cm"))
	float Baile = 6.f;

	/** Brasas que suben por segundo, su velocidad y dispersión. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brasas", meta = (ClampMin = "0"))
	float BrasasPorSegundo = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brasas", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadBrasas = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Brasas", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float DispersionBrasas = 35.f;

	/** Sin verse durante este tiempo, el fuego descansa (ni titila ni suelta brasas). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "0", ForceUnits = "s"))
	float DescansoSinVerse = 0.5f;

	/** La luz se actualiza a este ritmo (Hz), no cada fotograma: cada cambio de luz cuesta en el render. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "1"))
	float ActualizacionesLuzPorSegundo = 20.f;
};

/**
 * Fuego de un brasero (atmósfera): luz cálida que titila y baila con ruido suave, y brasas que suben y se
 * apagan. Lo coloca el script de assets sobre el bloque del brasero. Sin translucidez ni sombras; descansa
 * cuando no se ve.
 */
UCLASS()
class SAEVUS_API ASAEVUSFuegoBrasero : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSFuegoBrasero();

	virtual void Tick(float DeltaSeconds) override;

	/** Intensidad de la luz ahora (tests). */
	float ObtenerIntensidadActual() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UPointLightComponent> Luz;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USAEVUSComponenteParticulas> Brasas;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Fuego")
	FSAEVUSAjusteFuego Ajuste;

private:
	float Tiempo = 0.f;
	float TiempoLuz = 0.f;
	float AcumuladoBrasas = 0.f;
	/** Fase propia: dos braseros no titilan a la vez. */
	float Desfase = 0.f;
	FVector PosicionLuz = FVector::ZeroVector;
};
