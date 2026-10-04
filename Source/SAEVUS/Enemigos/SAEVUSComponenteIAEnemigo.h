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
	ESAEVUSEstadoEnemigo Estado = ESAEVUSEstadoEnemigo::Reposo;
	float TiempoEstado = 0.f;
	int32 IndiceAtaque = INDEX_NONE;
	int32 SiguienteAtaque = 0;
	int32 Fase = 0;
	FVector Origen = FVector::ZeroVector;
	float SentidoPatrulla = 1.f;
	FVector DireccionAtaque = FVector::ForwardVector;
	bool bGolpeAplicado = false;
	bool bDejoSuelo = false;
};
