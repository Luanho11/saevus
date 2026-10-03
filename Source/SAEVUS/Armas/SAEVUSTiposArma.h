// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSTiposArma.generated.h"

class USAEVUSDatosRepertorioMelee;

UENUM(BlueprintType)
enum class ESAEVUSTipoArma : uint8
{
	/** Los ataques salen de USAEVUSComponenteCombate con el repertorio del arma. */
	CuerpoACuerpo,
	/** Disparos hitscan desde USAEVUSComponenteArmas. */
	ArmaFuego
};

/** Ajuste de un arma de fuego hitscan. Los inicializadores describen una pistola semiautomática sencilla. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteArmaFuego
{
	GENERATED_BODY()

	/** Daño por perdigón. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arma de fuego", meta = (ClampMin = "0"))
	float Danio = 20.f;

	/** Trazas por disparo; más de 1 para armas de dispersión. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arma de fuego", meta = (ClampMin = "1"))
	int32 PerdigonesPorDisparo = 1;

	/** Tiempo mínimo entre disparos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arma de fuego", meta = (ClampMin = "0", ForceUnits = "s"))
	float IntervaloDisparo = 0.25f;

	/** Sigue disparando mientras se mantiene el gatillo; si no, un disparo por pulsación. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arma de fuego")
	bool bAutomatica = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Munición", meta = (ClampMin = "1"))
	int32 TamanoCargador = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Munición", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoRecarga = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puntería", meta = (ClampMin = "0", ForceUnits = "cm"))
	float Alcance = 5000.f;

	/** Medio ángulo del cono en el que cae cada perdigón al azar. 0 = precisión perfecta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puntería", meta = (ClampMin = "0", ClampMax = "45", ForceUnits = "deg"))
	float DispersionGrados = 0.5f;
};

/** Un arma del arsenal: un repertorio cuerpo a cuerpo o un arma de fuego, más su tiempo de equipado. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSDefinicionArma : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma")
	FName NombreVisible = TEXT("Arma");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma")
	ESAEVUSTipoArma Tipo = ESAEVUSTipoArma::ArmaFuego;

	/** Tiempo desde la petición de cambio hasta que el arma se puede usar. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoEquipar = 0.25f;

	/** Solo cuerpo a cuerpo. Nulo usa el repertorio por defecto del código. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cuerpo a cuerpo", meta = (EditCondition = "Tipo == ESAEVUSTipoArma::CuerpoACuerpo"))
	TObjectPtr<const USAEVUSDatosRepertorioMelee> Repertorio;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arma de fuego", meta = (EditCondition = "Tipo == ESAEVUSTipoArma::ArmaFuego"))
	FSAEVUSAjusteArmaFuego ArmaFuego;

	bool EsArmaFuego() const { return Tipo == ESAEVUSTipoArma::ArmaFuego; }
};
