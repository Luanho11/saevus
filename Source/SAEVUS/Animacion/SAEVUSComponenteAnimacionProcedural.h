// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SAEVUSComponenteAnimacionProcedural.generated.h"

class UPoseableMeshComponent;
class USAEVUSComponenteAlma;
class USAEVUSComponenteArmas;
class USAEVUSComponenteCombate;
class USAEVUSComponenteMovimiento;
class USAEVUSComponenteVida;
struct FSAEVUSDefinicionAtaque;
enum class ESAEVUSDireccionAtaque : uint8;

/** Huesos del esqueleto propio del caballero (Herramientas/Arte/blender_caballero.py). */
UENUM()
enum class ESAEVUSHueso : uint8
{
	Raiz, Pelvis, Columna, Pecho, Cuello, Cabeza, Capa1, Capa2,
	HombroD, BrazoD, AntebrazoD, ManoD, MusloD, PiernaD, PieD,
	HombroI, BrazoI, AntebrazoI, ManoI, MusloI, PiernaI, PieI,
	Arma, ArmaFuego,
	Total UMETA(Hidden)
};

/** Amplitudes, ritmos y suavizado de la animación procedural. Todo en grados, cm y segundos. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteAnimacion
{
	GENERATED_BODY()

	/** Distancia recorrida en un ciclo completo de pasos (dos zancadas). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ClampMin = "10", ForceUnits = "cm"))
	float LongitudCiclo = 190.f;

	/** Velocidad a la que las amplitudes de locomoción valen 1 (por encima crecen un poco). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float VelocidadReferencia = 550.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion")
	float GiroMuslo = 38.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion")
	float FlexionRodilla = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion")
	float GiroBrazo = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion")
	float InclinacionCarrera = 9.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "cm"))
	float BotePaso = 4.f;

	/** Respiración en reposo: grados y ciclos por segundo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo")
	float AmplitudRespiracion = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo")
	float FrecuenciaRespiracion = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float InclinacionDash = 38.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paredes")
	float InclinacionCarreraPared = 22.f;

	/** Recogida de piernas del doble salto: duración y grados extra de muslo y rodilla en el pico. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Doble salto", meta = (ForceUnits = "s"))
	float DuracionRecogidaSaltoAereo = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Doble salto")
	float RecogidaSaltoAereo = 45.f;

	/** Rapidez del suavizado hacia la pose objetivo (1/s). Más alto = más seco. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0"))
	float Suavizado = 14.f;

	/** Suavizado durante ataques y dash: más alto para que los golpes se lean secos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0"))
	float SuavizadoAccion = 32.f;

	/** Suavizado de la capa: bajo, para que se quede atrás y ondee. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0"))
	float SuavizadoCapa = 6.f;

	/** Duración y profundidad de la flexión al aterrizar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje", meta = (ForceUnits = "s"))
	float DuracionAterrizaje = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje")
	float FlexionAterrizaje = 45.f;

	/** Retroceso del brazo al disparar (grados) y su recuperación (1/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armas")
	float RetrocesoDisparo = 14.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armas")
	float RecuperacionRetroceso = 12.f;

	/** Agachado de la curación: bajada de la raíz (cm) y flexión de rodillas (grados). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ForceUnits = "cm"))
	float BajadaCuracion = 14.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación")
	float FlexionCuracion = 50.f;

	/** Reacción al recibir un golpe: el torso y la cabeza se echan atrás y vuelven. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ForceUnits = "s"))
	float DuracionReaccionGolpe = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido")
	float InclinacionReaccionGolpe = 22.f;

	/** Largo de la hoja del aguijón (cm), desde el hueso del arma: lo usa la estela. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armas", meta = (ClampMin = "0", ForceUnits = "cm"))
	float LongitudHoja = 62.f;

	/** Parpadeos por segundo del cuerpo durante la invulnerabilidad tras un golpe. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0"))
	float FrecuenciaParpadeo = 12.f;
};

/**
 * Animación procedural del caballero sobre una malla poseable con esqueleto propio.
 *
 * Cada fotograma construye una pose objetivo a partir del estado del juego (movimiento, paredes,
 * dash, ataques por su línea de tiempo, armas, muerte) y suaviza la pose actual hacia ella, así
 * los cambios de estado y las cancelaciones salen fluidos. La pose es, por hueso, una dirección
 * absoluta a la que apuntar o un giro relativo, compuestos sobre la pose de reposo por un FK propio.
 *
 * Ejes del espacio de la malla (el personaje mira a +Y): Adelante = +Y, Derecha = -X, Arriba = +Z.
 * Es un prototipo: lo sustituirán animaciones hechas a mano cuando haya diseño final.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteAnimacionProcedural : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteAnimacionProcedural();

	/** Malla a animar. Si no se fija, se busca un UPoseableMeshComponent en el dueño. */
	void FijarMalla(UPoseableMeshComponent* NuevaMalla);

	/** Cierto si la malla tiene todos los huesos del esqueleto del caballero. */
	bool EsqueletoValido() const { return bEsqueletoValido; }

	/** Nombre de hueso de cada entrada de ESAEVUSHueso. */
	static FName NombreHueso(ESAEVUSHueso Hueso);

	/**
	 * Base y punta de la hoja del aguijón en el mundo, según la pose actual. Falso si no hay
	 * esqueleto o el aguijón está oculto (arma de fuego equipada).
	 */
	bool ObtenerHoja(FVector& OutBase, FVector& OutPunta) const;

	/** Avanza la animación DeltaTime segundos (lo llama el tick; los tests lo llaman a mano). */
	void Actualizar(float DeltaTime);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Animacion")
	FSAEVUSAjusteAnimacion Ajuste;

private:
	/** Objetivo de un hueso: apuntar a una dirección (espacio de malla) o un giro relativo. */
	struct FObjetivo
	{
		bool bApuntar = false;
		FVector Direccion = FVector::ZeroVector;
		FQuat Giro = FQuat::Identity;
	};

	struct FPose
	{
		FObjetivo Huesos[static_cast<int32>(ESAEVUSHueso::Total)];
		FVector DesplazamientoRaiz = FVector::ZeroVector;
		float EscalaArma = 1.f;
		float EscalaArmaFuego = 0.f;
	};

	/** Lee la pose de reposo de la malla y resuelve los índices de hueso. */
	void PrepararEsqueleto();

	void ConstruirPoseObjetivo(FPose& Pose, float DeltaTime);
	void PoseLocomocion(FPose& Pose, float Velocidad, float DeltaTime);
	void PoseAire(FPose& Pose, float VelocidadVertical);
	void PoseDash(FPose& Pose);
	void PoseCarreraPared(FPose& Pose, float DeltaTime);
	void PoseEscalada(FPose& Pose, float DeltaTime);
	void PoseAtaque(FPose& Pose, const FSAEVUSDefinicionAtaque& Ataque, int32 IndiceCombo, bool bAereo, ESAEVUSDireccionAtaque Direccion, float Tiempo);
	void PoseDisparo(FPose& Pose);
	void PoseMuerte(FPose& Pose, float DeltaTime);
	void PoseCuracion(FPose& Pose);
	/** Oculta y muestra el cuerpo durante la invulnerabilidad tras un golpe. */
	void AplicarParpadeo();
	void PoseCapa(FPose& Pose, float Velocidad, float VelocidadVertical);

	/** Compone la pose objetivo sobre la de reposo (FK): rotaciones objetivo en espacio de malla. */
	void ResolverObjetivo(const FPose& Pose, FQuat (&OutRotaciones)[static_cast<int32>(ESAEVUSHueso::Total)]) const;
	/** Suaviza las rotaciones actuales hacia las objetivo, recalcula posiciones y lo aplica a la malla. */
	void SuavizarYAplicar(const FPose& Pose, const FQuat (&Objetivo)[static_cast<int32>(ESAEVUSHueso::Total)], float DeltaTime, bool bAccion);

	UFUNCTION()
	void AlDisparar(FVector Boca, FVector Impacto, AActor* ActorGolpeado);

	void AlSaltoAereo();

	UFUNCTION()
	void AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador);

	void PoseReaccionGolpe(FPose& Pose);

	/** Dirección de reposo de un hueso en espacio de malla (hacia su hijo, o la fija de las hojas). */
	FVector DireccionReposo(int32 Hueso) const;

	UPROPERTY()
	TObjectPtr<UPoseableMeshComponent> Malla;

	USAEVUSComponenteMovimiento* Movimiento = nullptr;
	USAEVUSComponenteCombate* Combate = nullptr;
	USAEVUSComponenteArmas* Armas = nullptr;
	USAEVUSComponenteVida* Vida = nullptr;
	USAEVUSComponenteAlma* Alma = nullptr;

	bool bEsqueletoValido = false;
	int32 IndiceMalla[static_cast<int32>(ESAEVUSHueso::Total)];
	int32 Padre[static_cast<int32>(ESAEVUSHueso::Total)];
	FTransform ReposoComponente[static_cast<int32>(ESAEVUSHueso::Total)];

	// Pose actual (ya suavizada) en espacio de malla.
	FQuat RotacionActual[static_cast<int32>(ESAEVUSHueso::Total)];
	FVector PosicionActual[static_cast<int32>(ESAEVUSHueso::Total)];
	FVector DesplazamientoRaizActual = FVector::ZeroVector;
	float EscalaArmaActual = 1.f;
	float EscalaArmaFuegoActual = 0.f;
	bool bPoseIniciada = false;
	float FaseCiclo = 0.f;
	float TiempoTotal = 0.f;
	float TiempoAterrizaje = 0.f;
	float TiempoMuerto = 0.f;
	float Retroceso = 0.f;
	float TiempoRecogidaSaltoAereo = 0.f;
	float TiempoReaccionGolpe = 0.f;
	bool bEstabaEnAire = false;
};
