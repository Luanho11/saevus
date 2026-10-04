// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Efectos/SAEVUSComponenteParticulas.h"
#include "SAEVUSComponentePolvoMovimiento.generated.h"

class USAEVUSComponenteMovimiento;

/** Cuánto polvo levanta cada movimiento. */
USTRUCT(BlueprintType)
struct FSAEVUSAjustePolvoMovimiento
{
	GENERATED_BODY()

	/** Aterrizaje: anillo de polvo a los pies; con VelocidadCaidaReferencia salen ParticulasAterrizaje. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje", meta = (ClampMin = "0"))
	int32 ParticulasAterrizaje = 18;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float VelocidadCaidaReferencia = 900.f;

	/** Por debajo de esta velocidad de caída no se levanta polvo (bajar un escalón). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadCaidaMinima = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadAnillo = 320.f;

	/** Carrera por pared: polvo por segundo donde los pies pisan la pared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paredes", meta = (ClampMin = "0"))
	float ParticulasPorSegundoPared = 45.f;

	/** Salto de pared: estallido contra la pared, hacia fuera. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paredes", meta = (ClampMin = "0"))
	int32 ParticulasSaltoPared = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paredes", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadSaltoPared = 300.f;

	/** Dash en el suelo: polvo por segundo a ras de suelo, hacia atrás. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0"))
	float ParticulasPorSegundoDashSuelo = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Dispersion = 90.f;
};

/**
 * Polvo que levanta el cuerpo al moverse (GunZ): anillo al aterrizar (según la caída), rastro donde los
 * pies pisan la pared al correr por ella o escalar, estallido contra la pared en el salto de pared y polvo
 * a ras de suelo en el dash. Partículas por CPU en mallas instanciadas, como la estela del dash.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponentePolvoMovimiento : public USAEVUSComponenteParticulas
{
	GENERATED_BODY()

public:
	USAEVUSComponentePolvoMovimiento();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Anillo de aterrizaje para una velocidad de caída (positiva hacia abajo). Expuesto para pruebas. */
	void EmitirAterrizaje(float VelocidadCaida);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Polvo")
	FSAEVUSAjustePolvoMovimiento AjustePolvo;

private:
	void AlSaltoPared(const FVector& NormalPared);
	USAEVUSComponenteMovimiento* ObtenerMovimiento() const;
	FVector ObtenerPies() const;
	void EmitirContinuo(float PorSegundo, float DeltaTime, const FVector& Posicion, const FVector& Velocidad);

	FDelegateHandle ManejadorSaltoPared;
	bool bEstabaEnAire = false;
	float VelocidadVerticalAire = 0.f;
	float AcumuladoEmision = 0.f;
};
