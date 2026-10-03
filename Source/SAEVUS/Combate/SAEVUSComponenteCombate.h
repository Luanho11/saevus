// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Combate/SAEVUSTiposCombate.h"
#include "Components/ActorComponent.h"
#include "SAEVUSComponenteCombate.generated.h"

class ACharacter;
class USAEVUSComponenteMovimiento;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSAEVUSFirmaGolpeAtaque, AActor*, ActorGolpeado, float, Danio);

/**
 * Base del cuerpo a cuerpo: ejecuta las líneas de tiempo de ataque de un repertorio, guarda las
 * pulsaciones en buffer, encadena combos, aplica golpes y decide cuándo una acción de movimiento
 * puede cancelar un ataque.
 *
 * Los efectos de movimiento (estocada, velocidad reducida en el suelo, giro) pasan por
 * USAEVUSComponenteMovimiento, que se actualiza después de este componente. Ver Documentos/combate.md.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteCombate : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteCombate();

	/** Pulsación de ataque desde cualquier entrada; empieza o encadena un ataque en cuanto se pueda. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Combate")
	void PedirAtaque();

	/**
	 * Pregunta si una acción de movimiento puede ocurrir ahora. Sin atacar: sí. Atacando: solo
	 * dentro de la ventana de cancelación del ataque para esa acción, y entonces el ataque termina.
	 */
	bool IntentarCancelarPor(ESAEVUSAccionCancelacion Accion);

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Combate")
	bool EstaAtacando() const { return AtaqueActual != nullptr; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Combate")
	ESAEVUSFaseAtaque ObtenerFaseAtaque() const;

	/** El ataque en curso, o nulo. */
	const FSAEVUSDefinicionAtaque* ObtenerAtaqueActual() const { return AtaqueActual; }

	/** Posición del ataque en curso en su cadena (0 = primero), o INDEX_NONE. */
	int32 ObtenerIndiceCombo() const { return IndiceCombo; }
	bool EsCadenaAerea() const { return bCadenaAerea; }
	float ObtenerTiempoAtaque() const { return TiempoAtaque; }
	bool TieneAtaqueEnBuffer() const { return TiempoBufferAtaqueRestante > 0.f; }

	const FSAEVUSRepertorioMelee& ObtenerRepertorio() const;

	/** Cambia el repertorio. Nulo vuelve al del código. Termina un ataque en curso. */
	void FijarRepertorio(const USAEVUSDatosRepertorioMelee* NuevoRepertorio);

	/** Se dispara una vez por cada actor golpeado por un ataque. */
	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Combate")
	FSAEVUSFirmaGolpeAtaque AlGolpear;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/** Repertorio opcional; sin él se usan los valores por defecto de FSAEVUSRepertorioMelee. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Combate")
	TObjectPtr<const USAEVUSDatosRepertorioMelee> DatosRepertorio;

private:
	/** Empieza o encadena un ataque si la pulsación en buffer se puede usar ya. */
	void ProcesarAtaqueEnBuffer();
	/** Los ataques empiezan en el suelo o en el aire, no en paredes; un dash puede cortarse para atacar. */
	bool PuedeAtacarAhora() const;
	void IniciarAtaque(bool bEnAire, int32 Indice);
	void TerminarAtaque();
	void DetectarGolpes(const FSAEVUSDefinicionAtaque& Ataque);

	ACharacter* ObtenerPersonaje() const;
	USAEVUSComponenteMovimiento* ObtenerMovimiento() const;

	const FSAEVUSDefinicionAtaque* AtaqueActual = nullptr;
	int32 IndiceCombo = INDEX_NONE;
	bool bCadenaAerea = false;
	float TiempoAtaque = 0.f;
	float TiempoBufferAtaqueRestante = 0.f;

	/** Actores ya golpeados por el ataque en curso; cada uno recibe un golpe por ataque. */
	TArray<TWeakObjectPtr<AActor>> ActoresGolpeados;
};
