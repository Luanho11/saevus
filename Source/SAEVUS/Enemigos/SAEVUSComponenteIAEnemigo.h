// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Enemigos/SAEVUSTiposEnemigo.h"
#include "SAEVUSComponenteIAEnemigo.generated.h"

class ASAEVUSEnemigo;
class USAEVUSComponenteVida;

DECLARE_MULTICAST_DELEGATE_OneParam(FSAEVUSEventoFaseEnemigo, int32 /*Fase*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FSAEVUSEventoEstadoEnemigo, ESAEVUSEstadoEnemigo /*NuevoEstado*/);

/**
 * IA de estados de un enemigo: patrulla, persigue, avisa, ataca y se recupera; se aturde al
 * recibir golpes y hace daño por contacto. Todo lo manda FSAEVUSAjusteEnemigo de su dueño. Sin
 * navegación: va en línea recta (arenas de tutorial); el movimiento no cae por bordes.
 * Ver Documentos/enemigos.md.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteIAEnemigo : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteIAEnemigo();

	/** Objetivo al que atacar. Sin fijar, se usa el pawn del primer jugador. */
	void FijarObjetivo(APawn* NuevoObjetivo) { Objetivo = NuevoObjetivo; }
	APawn* ObtenerObjetivo() const;

	/** Ajuste del tipo de enemigo (de sus datos, o el del código sin datos). */
	const FSAEVUSAjusteEnemigo& ObtenerAjuste() const;

	ESAEVUSEstadoEnemigo ObtenerEstado() const { return Estado; }
	float ObtenerTiempoEstado() const { return TiempoEstado; }
	/** Ataque elegido (avisando, atacando o recuperándose), o nulo. */
	const FSAEVUSAtaqueEnemigo* ObtenerAtaqueActual() const;
	int32 ObtenerFase() const { return Fase; }

	/** Progreso del aviso en curso, de 0 a 1 (para el aspecto). */
	float ObtenerProgresoAviso() const;

	/** Interrumpe lo que esté haciendo (golpe recibido), salvo si no se aturde. */
	void Aturdir();

	/** Fuerza un estado sin pasar por las condiciones (pruebas y escenas guionizadas). */
	void ForzarEstado(ESAEVUSEstadoEnemigo NuevoEstado) { CambiarEstado(NuevoEstado); }

	/** Despierta al enemigo contra el objetivo y no lo pierde por distancia (una arena de jefe empieza el combate). */
	void Despertar();

	/** Vuelve al reposo como al empezar: fase 0, sin ataque ni alerta (el jugador murió o reinició). */
	void Reiniciar();

	/** Dónde apareció: a donde vuelve al reiniciarse. */
	const FVector& ObtenerOrigen() const { return Origen; }

	/** Pasa a muerto; ya no actúa. */
	void Morir();

	/** Avanza la IA DeltaTime segundos (lo llama el tick; los tests lo llaman a mano). */
	void Actualizar(float DeltaTime);

	FSAEVUSEventoFaseEnemigo AlCambiarFase;

	/** Se emite al cambiar de estado (sonido, efectos). */
	FSAEVUSEventoEstadoEnemigo AlCambiarEstado;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador);

	void CambiarEstado(ESAEVUSEstadoEnemigo NuevoEstado);
	void ActualizarReposo();
	/** Cierto si el objetivo está lo bastante cerca en altura y, si se exige, sin pared por medio (se vuelve a mirar cada pocas décimas). */
	bool PuedeDetectarObjetivo();
	/** Rodea al objetivo esperando su turno de ataque (no tiene ficha). */
	void EsperarTurno(float Distancia);
	/** Voladores: dirección de movimiento con la componente vertical que lleva a AlturaObjetivo. */
	FVector ConAltura(const FVector& DireccionHorizontal, float AlturaObjetivo) const;
	/** Voladores: vuelve a volar al salir del aturdimiento (el golpe los hace caer). */
	void MantenerVuelo();
	/** Reposo lejano: la IA y el movimiento (si está de pie) se actualizan cada IntervaloReposoLejos. */
	void FijarReposoLejano(bool bLejos);
	void ActualizarPerseguir();
	void ActualizarTelegrafiar();
	void ActualizarAtacar();
	void EmpezarAtaque();
	void LanzarProyectil(const FSAEVUSAtaqueEnemigo& Ataque);
	/** Elige el siguiente ataque válido a esta distancia (en rotación); falso si no hay. */
	bool ElegirAtaque(float Distancia);
	/** Cierto si el objetivo está a una altura que el ataque alcanza. */
	bool AlcanzaEnAltura(const FSAEVUSAtaqueEnemigo& Ataque) const;
	void ComprobarContacto();
	void GolpearObjetivo(float Danio);
	void MoverHacia(const FVector& Direccion, float Escala);
	void Encarar(const FVector& Direccion);
	void ActualizarFase();

	const FSAEVUSFaseEnemigo& ObtenerFaseActual() const;
	bool FaseAdmiteAtaque(int32 IndiceAtaque) const;
	/** Distancia horizontal al objetivo, o un número enorme sin objetivo vivo. */
	float DistanciaObjetivo() const;
	FVector DireccionObjetivo() const;
	bool ObjetivoVivo() const;

	ASAEVUSEnemigo* ObtenerEnemigo() const;

	TWeakObjectPtr<APawn> Objetivo;
	// Caché del pawn del jugador (una búsqueda por fotograma) y de su vida (mientras sea el mismo pawn).
	mutable TWeakObjectPtr<APawn> ObjetivoJugador;
	mutable uint64 FotogramaObjetivo = 0;
	mutable TWeakObjectPtr<const APawn> PawnVidaObjetivo;
	mutable TWeakObjectPtr<const USAEVUSComponenteVida> VidaObjetivo;
	ESAEVUSEstadoEnemigo Estado = ESAEVUSEstadoEnemigo::Reposo;
	float TiempoEstado = 0.f;
	int32 IndiceAtaque = INDEX_NONE;
	int32 SiguienteAtaque = 0;
	int32 Fase = 0;
	FVector Origen = FVector::ZeroVector;
	float SentidoPatrulla = 1.f;
	/** Eje de la patrulla: hacia donde mira el enemigo al aparecer (no el eje X del mundo). */
	FVector EjePatrulla = FVector::ForwardVector;
	/** Tiempo que lleva sin avanzar al patrullar (da la vuelta al toparse con una pared). */
	float TiempoBloqueado = 0.f;
	/** Resultado de la última comprobación de vista y cuándo se hizo. */
	bool bVeObjetivo = false;
	float TiempoSinMirar = 1.f;
	/** Ficha de ataque del director de combate. */
	bool bTieneFicha = false;
	/** El índice de ataque que se iba a usar antes de quedarse sin ficha (para no saltarse el siguiente de la rotación). */
	int32 SiguienteAtaqueAntes = 0;
	FVector DireccionAtaque = FVector::ForwardVector;
	bool bGolpeAplicado = false;

	/** Para elegir ataques al azar (con la semilla del enemigo) y no repetir. */
	FRandomStream AleatorioAtaques;
	int32 UltimoAtaque = INDEX_NONE;
	/** La onda del aterrizaje de un salto se aplica una sola vez. */
	bool bOndaAplicada = false;

	/** Rugido del cambio de fase: mientras queda tiempo no ataca ni se mueve. */
	float TiempoTransicionRestante = 0.f;

public:
	bool EstaEnTransicion() const { return TiempoTransicionRestante > 0.f; }

private:
	/** Mientras dura, el enemigo persigue sin perder al objetivo por distancia (le acaban de herir). */
	float TiempoAlertaRestante = 0.f;

	/** El DeltaTime de la actualización en curso (para el giro limitado). */
	float DeltaActual = 0.f;
	bool bDejoSuelo = false;
};
