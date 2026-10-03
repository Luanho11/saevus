// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSConfigEntrada.generated.h"

class UInputAction;

/**
 * Acciones de entrada que enlaza el personaje. Los contextos (qué teclas disparan cada acción)
 * los añade el controlador del jugador, así las acciones no dependen del dispositivo.
 */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSConfigEntrada : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Eje 2D: X = derecha, Y = adelante. */
	UPROPERTY(EditDefaultsOnly, Category = "Movimiento")
	TObjectPtr<UInputAction> AccionMover;

	/** Mirar en 2D con el stick del mando. */
	UPROPERTY(EditDefaultsOnly, Category = "Camara")
	TObjectPtr<UInputAction> AccionMirar;

	/** Mirar en 2D con el ratón. Separada para que las plataformas táctiles puedan excluirla. */
	UPROPERTY(EditDefaultsOnly, Category = "Camara")
	TObjectPtr<UInputAction> AccionMirarRaton;

	UPROPERTY(EditDefaultsOnly, Category = "Movimiento")
	TObjectPtr<UInputAction> AccionSaltar;

	/** Booleana, mantenida. */
	UPROPERTY(EditDefaultsOnly, Category = "Movimiento")
	TObjectPtr<UInputAction> AccionEsprint;

	/** Booleana, mantenida. */
	UPROPERTY(EditDefaultsOnly, Category = "Movimiento")
	TObjectPtr<UInputAction> AccionCaminar;

	/** Booleana, pulsada (mando). En teclado el dash es doble toque de dirección. */
	UPROPERTY(EditDefaultsOnly, Category = "Movimiento")
	TObjectPtr<UInputAction> AccionDash;

	/** Dash con doble toque de una dirección de movimiento (WASD). */
	UPROPERTY(EditDefaultsOnly, Category = "Movimiento|Doble toque")
	bool bDashDobleToque = true;

	/** Tiempo máximo entre las dos pulsaciones del doble toque. */
	UPROPERTY(EditDefaultsOnly, Category = "Movimiento|Doble toque", meta = (ClampMin = "0.05", ClampMax = "1", ForceUnits = "s"))
	float VentanaDobleToque = 0.25f;

	/** Valor del eje de movimiento a partir del cual una dirección cuenta como pulsada. */
	UPROPERTY(EditDefaultsOnly, Category = "Movimiento|Doble toque", meta = (ClampMin = "0.1", ClampMax = "1"))
	float UmbralDobleToque = 0.5f;

	/** Booleana, mantenida: ataque cuerpo a cuerpo, o gatillo del arma de fuego equipada. */
	UPROPERTY(EditDefaultsOnly, Category = "Combate")
	TObjectPtr<UInputAction> AccionAtacar;

	/** Booleana, pulsada. */
	UPROPERTY(EditDefaultsOnly, Category = "Armas")
	TObjectPtr<UInputAction> AccionRecargar;

	/** Booleana, pulsada. */
	UPROPERTY(EditDefaultsOnly, Category = "Armas")
	TObjectPtr<UInputAction> AccionSiguienteArma;

	/** Booleanas, pulsadas. El índice i elige la ranura i del arsenal. */
	UPROPERTY(EditDefaultsOnly, Category = "Armas")
	TArray<TObjectPtr<UInputAction>> AccionesRanuraArma;

	/** Booleana, mantenida: gasta alma para curar (en el suelo). */
	UPROPERTY(EditDefaultsOnly, Category = "Combate")
	TObjectPtr<UInputAction> AccionCurar;
};
