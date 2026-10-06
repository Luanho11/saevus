// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSKitInstancias.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;

/**
 * Decorado de un mapa con las piezas del kit modular (REF-07, Documentos/ARTE_SAEVUS.md, Mazmorra) dibujadas
 * como instancias: un componente de mallas instanciadas por cada malla, así cientos de piezas cuestan pocas
 * llamadas de dibujo (Vega 8). Sin colisión: la colisión la siguen dando los bloques probados del mapa (ocultos
 * en el juego). Lo rellena el script de assets (saevus_crear_assets.py) con AnadirPieza.
 */
UCLASS()
class SAEVUS_API ASAEVUSKitInstancias : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSKitInstancias();

	/** Añade una pieza (transformación en el mundo) con sus materiales por ranura; crea el componente de esa malla si falta. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Kit")
	void AnadirPieza(UStaticMesh* Malla, const FTransform& Transformacion, const TArray<UMaterialInterface*>& Materiales);

	/** Quita todas las piezas. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Kit")
	void Vaciar();

	/** Número total de piezas (para la verificación). */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Kit")
	int32 NumeroPiezas() const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> Instancias;
};
