// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSDirectorCombate.generated.h"

/**
 * Director de combate: reparte «fichas de ataque». Solo unos pocos enemigos a la vez pueden avisar o atacar; el
 * resto rodea al jugador esperando su turno. Así un grupo es una coreografía legible y no cuatro goblins
 * atacando en el mismo fotograma (auditoría EA-8). Los jefes no piden ficha.
 */
UCLASS()
class SAEVUS_API USAEVUSDirectorCombate : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Cuántos enemigos pueden estar avisando o atacando a la vez. */
	static constexpr int32 MaximoAtacantes = 2;

	/** Cierto si Enemigo ya tiene ficha o queda alguna libre (y se la queda). */
	bool PedirFicha(const UObject* Enemigo);

	/** Devuelve la ficha (no pasa nada si no la tenía). */
	void SoltarFicha(const UObject* Enemigo);

	int32 ObtenerFichasUsadas();

private:
	/** Quita las fichas de enemigos que ya no existen. */
	void Depurar();

	TSet<TWeakObjectPtr<const UObject>> Atacantes;
};
