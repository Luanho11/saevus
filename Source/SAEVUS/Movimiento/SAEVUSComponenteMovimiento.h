// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Movimiento/SAEVUSAjustesMovimiento.h"
#include "Movimiento/SAEVUSTiposMovimiento.h"
#include "SAEVUSComponenteMovimiento.generated.h"

/** Se emite al empezar un dash (no durante las repeticiones de corrección de red). Parámetro: dirección horizontal. */
DECLARE_MULTICAST_DELEGATE_OneParam(FSAEVUSEventoDash, const FVector& /*Direccion*/);
DECLARE_MULTICAST_DELEGATE(FSAEVUSEventoSaltoAereo);
DECLARE_MULTICAST_DELEGATE_OneParam(FSAEVUSEventoSaltoPared, const FVector& /*NormalPared*/);
/** Se emite al tocar suelo tras caer, con la velocidad de caída (positiva, cm/s). */
DECLARE_MULTICAST_DELEGATE_OneParam(FSAEVUSEventoAterrizaje, float /*VelocidadCaida*/);

/**
 * Movimiento del personaje de SAEVUS.
 *
 * Elige el paso (caminar / correr / esprint) y aplica los ajustes de USAEVUSAjustesMovimiento
 * al modelo de movimiento base. Las peticiones viajan en los movimientos guardados, así se
 * mantienen predichas en el cliente cuando haya red.
 *
 * El dash es un estado temporal dentro de caminar / caer; el salto de pared es un impulso
 * instantáneo al caer; la carrera por pared y la escalada son modos de movimiento personalizados
 * (ESAEVUSModoMovimientoPersonalizado). El movimiento en paredes vive en
 * SAEVUSComponenteMovimientoParedes.cpp. Ver Documentos/movimiento.md.
 */
UCLASS(ClassGroup = (SAEVUS))
class SAEVUS_API USAEVUSComponenteMovimiento : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteMovimiento();

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Movimiento")
	void PedirEsprint(bool bPedido) { bQuiereEsprint = bPedido; }

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Movimiento")
	void PedirCaminar(bool bPedido) { bQuiereCaminar = bPedido; }

	/** El esprint manda sobre caminar; sin ninguno de los dos, el personaje corre. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	ESAEVUSPaso ObtenerPasoActual() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	float ObtenerVelocidadPaso(ESAEVUSPaso Paso) const;

	/** Paso que limita la velocidad en el aire; el del despegue mientras bBloquearPasoEnAire esté activo. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	ESAEVUSPaso ObtenerPasoAire() const;

	/** Cierto al caer poco después de salir caminando de un borde, cuando el salto de suelo sigue permitido. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	bool EnVentanaCoyote() const;

	/** Recuerda un salto pulsado en el aire para que se ejecute al aterrizar (ver TiempoBufferSalto). */
	void GuardarSaltoEnBuffer();

	/** Pide un dash para la próxima actualización de movimiento; se descarta si no se puede entonces. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Movimiento")
	void PedirDash() { bQuiereDash = true; DireccionDashPedida = FVector::ZeroVector; }

	/**
	 * Pide un dash en una dirección concreta (horizontal, en el mundo), p. ej. la del doble toque.
	 * Sin dirección válida se comporta como PedirDash (entrada de movimiento u orientación).
	 */
	void PedirDashEnDireccion(const FVector& Direccion) { bQuiereDash = true; DireccionDashPedida = Direccion.GetSafeNormal2D(); }

	/** Dirección del dash en curso (o del último). */
	FVector ObtenerDireccionDash() const { return DireccionDash; }

	/** Se emite al empezar cada dash. */
	FSAEVUSEventoDash AlIniciarDash;

	/** Se emite al hacer un doble salto (no en las repeticiones de corrección del cliente). */
	FSAEVUSEventoSaltoAereo AlSaltoAereo;

	/** Se emite al saltar desde una pared, con su normal (no en las repeticiones de corrección del cliente). */
	FSAEVUSEventoSaltoPared AlSaltoPared;

	/** Se emite al aterrizar tras caer, con la velocidad de caída (animación, polvo; no en las repeticiones del cliente). */
	FSAEVUSEventoAterrizaje AlAterrizar;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	bool EstaEnDash() const { return TiempoDashRestante > 0.f; }

	/** Avance del dash en curso: 0 al empezar, 1 al terminar (1 fuera de un dash). */
	float ObtenerFraccionDash() const
	{
		return EstaEnDash() && DuracionDashActual > 0.f ? FMath::Clamp(1.f - TiempoDashRestante / DuracionDashActual, 0.f, 1.f) : 1.f;
	}

	/** Termina el dash antes de tiempo, con la salida normal (límite de velocidad, enfriamiento). */
	void CancelarDash();

	/**
	 * Movimiento dirigido por una acción, como la estocada de la espada: velocidad horizontal fija
	 * en Direccion x Velocidad durante Duracion, en el suelo o en el aire. El dash tiene prioridad.
	 */
	void IniciarEstocada(const FVector& Direccion, float Velocidad, float Duracion);

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	bool EstaEnEstocada() const { return TiempoEstocadaRestante > 0.f; }

	/**
	 * Escala el tope de velocidad en el suelo mientras una acción (ataque, curación, guardia) compromete al
	 * personaje. 1 = nada. Cada fuente tiene la suya y se aplica la más baja.
	 */
	void FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad Fuente, float Escala)
	{
		const int32 Indice = static_cast<int32>(Fuente);
		if (ensure(Indice >= 0 && Indice < static_cast<int32>(ESAEVUSFuenteEscalaVelocidad::Total)))
		{
			EscalasVelocidadAccion[Indice] = Escala;
		}
	}

	/** Escala de velocidad que se aplica ahora: la más baja de todas las fuentes. */
	float ObtenerEscalaVelocidadAccion() const;

	/** Escala la gravedad mientras una acción lo pide (tajos aéreos que flotan). 1 = nada. */
	void FijarEscalaGravedadAccion(float Escala) { EscalaGravedadAccion = Escala; }

	/** Gira el cuerpo de golpe hacia Direccion (horizontal). */
	void GirarHacia(const FVector& Direccion);

	/** Cierto si el cuerpo sigue a la cámara (bMirarHaciaCamara con controlador): las acciones no lo giran. */
	bool MiraHaciaCamara() const;

	/** Orientación del cuerpo para una acción hacia Direccion: la de ahora si mira a la cámara. */
	FQuat OrientacionCuerpo(const FVector& Direccion) const;

	/** Dashes aéreos que quedan antes de aterrizar. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	int32 DashesAereosRestantes() const;

	/** Cierto si un dash pedido ahora empezaría (sin contar la petición pendiente). */
	bool PuedeDash() const;

	/** Saltos de pared que quedan antes de aterrizar. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	int32 SaltosParedRestantes() const;

	/** Con arma de fuego, el dash es la voltereta (DashArmaFuego). Lo fija el personaje al pedir el dash. */
	void FijarDashArmaFuego(bool bArmaFuego) { bDashArmaFuego = bArmaFuego; }

	/** Dobles saltos que quedan antes de aterrizar. */
	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	int32 SaltosAereosRestantes() const;

	/** Devuelve los dashes aéreos y los dobles saltos (al aterrizar). */
	void RecargarRecursosAereos();

	/** Corta la estocada en curso (p. ej. al golpear algo). */
	void TerminarEstocada() { TiempoEstocadaRestante = 0.f; }

	/**
	 * Busca una pared casi vertical a menos de DistanciaDeteccion de la cápsula.
	 * Devuelve su normal horizontal (alejándose de la pared), o cero si no hay.
	 */
	FVector BuscarNormalSaltoPared() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	bool EstaEnCarreraPared() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Movimiento")
	bool EstaEscalando() const;

	/** Tiempo que queda en la carrera por pared o escalada actual; 0 fuera de una pared. */
	float TiempoEnParedRestante() const { return (EstaEnCarreraPared() || EstaEscalando()) ? TiempoParedRestante : 0.f; }

	/** Escaladas que quedan antes de aterrizar. */
	int32 EscaladasRestantes() const;

	/** Normal horizontal de la pared en la que corre o escala (alejándose de ella); cero fuera de una pared. */
	FVector ObtenerNormalPared() const { return (EstaEnCarreraPared() || EstaEscalando()) ? NormalPared : FVector::ZeroVector; }

	const FSAEVUSAjusteMovimientoSuelo& ObtenerAjusteSuelo() const;
	/** Recorta la subida del salto al soltar el botón (salto variable); no hace nada si no sube o va en otro modo. */
	void CortarSaltoAlSoltar();

	const FSAEVUSAjusteMovimientoAire& ObtenerAjusteAire() const;
	const FSAEVUSAjusteDash& ObtenerAjusteDash() const;
	const FSAEVUSAjusteSaltoPared& ObtenerAjusteSaltoPared() const;
	const FSAEVUSAjusteCarreraPared& ObtenerAjusteCarreraPared() const;
	const FSAEVUSAjusteEscaladaPared& ObtenerAjusteEscalada() const;

	/** Cambia el perfil de movimiento y lo aplica en el acto. Nulo vuelve a los valores del código. */
	void FijarAjustesMovimiento(const USAEVUSAjustesMovimiento* NuevosAjustes);

	//~ Inicio de la interfaz de UCharacterMovementComponent
	using Super::DoJump;
	virtual void InitializeComponent() override;
	virtual float GetMaxSpeed() const override;
	virtual float GetGravityZ() const override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual bool DoJump(bool bReplayingMoves, float DeltaTime) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual FString GetMovementName() const override;
	//~ Fin de la interfaz de UCharacterMovementComponent

protected:
	//~ Inicio de la interfaz de UCharacterMovementComponent
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	//~ Fin de la interfaz de UCharacterMovementComponent

protected:
	/** Perfil opcional; sin él se usan los valores por defecto de los structs de ajuste. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Movimiento")
	TObjectPtr<const USAEVUSAjustesMovimiento> AjustesMovimiento;

private:
	/** Pasa los ajustes activos a las propiedades del movimiento base. */
	void AplicarAjustes();

	/** Impulsa lejos de la pared si el salto se acaba de pulsar en el aire junto a una. Cierto si lo hizo. */
	bool IntentarSaltoPared();
	/** Doble salto: una pulsación nueva en el aire, fuera del tiempo coyote. */
	bool IntentarSaltoAereo();
	/** Cierto si, cayendo, se tocará suelo caminable dentro del tiempo de buffer de salto. */
	bool SueloDentroDelBuffer() const;
	void ImpulsarDesdePared(const FVector& NormalEmpuje);

	/** Pared casi vertical más cercana encontrada barriendo la cápsula en Direcciones (horizontales, unitarias). */
	bool BuscarParedCercana(TConstArrayView<FVector> Direcciones, float Distancia, float MaxNormalZPared, FHitResult& OutGolpe) const;

	/** Empieza una carrera por pared al caer junto a una pared con entrada de dirección. Cierto si lo hizo. */
	bool IntentarIniciarCarreraPared();
	void FisicaCarreraPared(float DeltaTime, int32 Iteraciones);

	/** Empieza una escalada al saltar de frente a una pared con entrada hacia ella. Cierto si lo hizo. */
	bool IntentarIniciarEscalada();
	void FisicaEscalada(float DeltaTime, int32 Iteraciones);

	/** Vuelve a encontrar la pared en la que está el personaje (detrás de -NormalPared) y sigue su normal. */
	bool SondearParedActual(float DistanciaDeteccion, float MaxNormalZPared, FHitResult& OutGolpePared);
	/**
	 * Mueve según Velocity, pegado a un pequeño hueco de la pared de GolpePared; aterriza si toca
	 * suelo caminable al bajar. Devuelve el desplazamiento.
	 */
	FVector MoverJuntoAPared(float DeltaTime, int32 Iteraciones, const FHitResult& GolpePared, const FQuat& Orientacion);
	/** Deja la pared, pasa a caer y simula ahí el resto de la actualización. */
	void SoltarPared(float TiempoRestante, int32 Iteraciones);

	void IniciarDash();
	/** Termina el dash: limita la velocidad horizontal a VelocidadSalida y arranca el enfriamiento. */
	void TerminarDash();

	friend class FSAEVUSMovimientoGuardado;

	bool bQuiereEsprint = false;
	bool bQuiereCaminar = false;
	bool bQuiereDash = false;
	/** Dirección pedida explícitamente para el próximo dash; cero = usar la entrada de movimiento. */
	FVector DireccionDashPedida = FVector::ZeroVector;

	// Movimiento de acción dirigido por el combate. Estado local, todavía fuera del movimiento
	// guardado (el combate no tiene red hasta la fase de red).
	FVector DireccionEstocada = FVector::ForwardVector;
	float VelocidadEstocada = 0.f;
	float TiempoEstocadaRestante = 0.f;
	float EscalasVelocidadAccion[static_cast<int32>(ESAEVUSFuenteEscalaVelocidad::Total)];
	float EscalaGravedadAccion = 1.f;

	// Estado del dash. Lo guarda y restaura FSAEVUSMovimientoGuardado para que las repeticiones tras una corrección coincidan.
	FVector DireccionDash = FVector::ForwardVector;
	float TiempoDashRestante = 0.f;
	/** Duración del dash en curso (solo para la animación; no viaja en los movimientos guardados). */
	float DuracionDashActual = 0.f;
	float EnfriamientoDashRestante = 0.f;
	int32 DashesAereosUsados = 0;
	int32 SaltosParedUsados = 0;
	int32 SaltosAereosUsados = 0;
	bool bDashArmaFuego = false;

	// Estado de carrera por pared / escalada, también guardado y restaurado por FSAEVUSMovimientoGuardado.
	FVector NormalPared = FVector::ZeroVector;
	FVector DireccionCarreraPared = FVector::ZeroVector;
	float TiempoParedRestante = 0.f;
	/** Normal de la última pared recorrida; esa pared no se puede volver a recorrer antes de aterrizar. */
	FVector NormalUltimaCarreraPared = FVector::ZeroVector;
	int32 EscaladasUsadas = 0;

	// Estado local derivado de los cambios de modo. Fuera del movimiento guardado: tras una
	// corrección del servidor puede desfasarse una transición (aceptable por ahora).
	ESAEVUSPaso PasoAire = ESAEVUSPaso::Correr;
	float TiempoCoyoteRestante = 0.f;
	float TiempoBufferSaltoRestante = 0.f;
	/** Velocidad vertical del último paso en el aire: la del choque con el suelo al aterrizar. */
	float VelocidadVerticalEnAire = 0.f;
};

namespace SAEVUSMovimiento
{
	/** El movimiento SAEVUS de Actor (un personaje); nulo si no tiene. Lo usan los componentes hermanos. */
	SAEVUS_API USAEVUSComponenteMovimiento* DelActor(const AActor* Actor);
}
