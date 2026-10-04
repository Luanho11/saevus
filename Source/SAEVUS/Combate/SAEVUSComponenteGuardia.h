// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SAEVUSComponenteGuardia.generated.h"

class USAEVUSComponenteArmas;
class USAEVUSComponenteCombate;
class USAEVUSComponenteMovimiento;
class USAEVUSComponenteVida;

DECLARE_MULTICAST_DELEGATE_TwoParams(FSAEVUSEventoBloqueo, AActor* /*Atacante*/, const FVector& /*PuntoContacto*/);

/** Valores de la guardia. Primera pasada; todo ajustable. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteGuardia
{
	GENERATED_BODY()

	/** Media apertura del cono que cubre la guardia, desde el frente del cuerpo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guardia", meta = (ClampMin = "0", ClampMax = "180", ForceUnits = "deg"))
	float MediaAperturaCobertura = 70.f;

	/** Fracción del daño que para (1 = todo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guardia", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionBloqueada = 1.f;

	/** Tope de velocidad en el suelo en guardia (fracción de la normal). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guardia", meta = (ClampMin = "0", ClampMax = "1"))
	float EscalaVelocidad = 0.35f;

	/** Al bloquear, el jugador retrocede y el atacante sale despedido: el choque se siente. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bloqueo", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float RetrocesoPropio = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bloqueo", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float EmpujeAtacante = 450.f;

	/** Entre dos reacciones de bloqueo (efectos y empujes); el daño se para siempre. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bloqueo", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoEntreReacciones = 0.2f;

	/**
	 * Contraataque (parry): un golpe bloqueado en este tiempo desde que se levanta la guardia
	 * aturde al atacante, da alma y un instante de invulnerabilidad. 0 = sin contraataque.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contraataque", meta = (ClampMin = "0", ForceUnits = "s"))
	float VentanaContraataque = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contraataque", meta = (ClampMin = "0"))
	float AlmaPorContraataque = 22.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contraataque", meta = (ClampMin = "0", ForceUnits = "s"))
	float InvulnerabilidadContraataque = 0.3f;

	/** Distancia delante del cuerpo a la que salen las chispas del bloqueo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bloqueo", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaContacto = 60.f;
};

/**
 * Guardia (GunZ): mantenida con un arma cuerpo a cuerpo, bloquea el daño que llega de frente
 * (golpes y disparos de un causante dentro del cono), con retroceso propio, empuje al atacante,
 * chispas y sonido. Es una acción de cancelación más: pedida durante un tajo, entra en cuanto la
 * ventana `Guardia` del ataque lo permite (tajo → guardia, la "mariposa"). En guardia el cuerpo
 * mira hacia la cámara y se mueve despacio. Atacar la suelta; mantenida, vuelve tras el ataque.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteGuardia : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteGuardia();

	/** Acción de guardia pulsada (cierto) o soltada (falso). */
	void FijarGuardiaPedida(bool bPedida);

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Guardia")
	bool EstaEnGuardia() const { return bEnGuardia; }

	bool EstaPedida() const { return bPedida; }

	/** Sale de la guardia sin olvidar que sigue pulsada (un ataque la interrumpe). */
	void Interrumpir();

	/** Cierto si un causante en Posicion queda dentro del cono de la guardia. */
	bool CubreDesde(const FVector& Posicion) const;

	int32 ObtenerBloqueos() const { return Bloqueos; }

	/** Se emite al bloquear (como mucho una vez cada TiempoEntreReacciones). */
	FSAEVUSEventoBloqueo AlBloquear;

	/** Se emite en cada contraataque (bloqueo dentro de la ventana). */
	FSAEVUSEventoBloqueo AlContraatacar;

	int32 ObtenerContraataques() const { return Contraataques; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Guardia")
	FSAEVUSAjusteGuardia Ajuste;

private:
	/** Filtro de daño de la vida: devuelve lo que queda tras la guardia. */
	float FiltrarDanio(float Cantidad, AActor* Causante);
	bool PuedeEntrar() const;
	void Entrar();
	void Salir();
	void ReaccionarBloqueo(AActor* Atacante);
	void Contraatacar(AActor* Atacante);
	USAEVUSComponenteMovimiento* ObtenerMovimiento() const;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteCombate> Combate;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteArmas> Armas;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteVida> Vida;

	bool bPedida = false;
	bool bEnGuardia = false;
	bool bOrientabaAlMovimiento = true;
	double UltimaReaccion = -1.0e9;
	int32 Bloqueos = 0;
	int32 Contraataques = 0;
	/** Tiempo de mundo en que se levantó la guardia (para la ventana de contraataque). */
	double InicioGuardia = 0.0;
};
