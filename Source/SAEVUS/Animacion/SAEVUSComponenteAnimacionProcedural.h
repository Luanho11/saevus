// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Animacion/SAEVUSCapaSecundaria.h"
#include "Animacion/SAEVUSMuelle.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SAEVUSComponenteAnimacionProcedural.generated.h"

class UCharacterMovementComponent;
class UPoseableMeshComponent;
class USAEVUSComponenteAlma;
class USAEVUSComponenteGuardia;
class USAEVUSComponenteArmas;
class USAEVUSComponenteCombate;
class USAEVUSComponenteIAEnemigo;
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
	// Punta de la capa (tiras de REF-02): al final para no mover los índices de los demás huesos.
	Capa3,
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

	/** Inclinación del tronco hacia delante a la velocidad de referencia (crece con el esprint). REF-04 n.º 2–3: cuerpo muy inclinado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float InclinacionCarrera = 18.f;

	/** La cadera baja estos centímetros a la velocidad de referencia: zancada larga y baja (REF-04). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "cm"))
	float BajadaCarrera = 7.f;

	/**
	 * Brazo de la espada al correr (REF-04 n.º 2–3): el brazo se echa atrás estos grados y la hoja
	 * apunta atrás, casi horizontal, tantos grados desde la vertical.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float GradosBrazoArmaCarrera = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float GradosHojaCarrera = 70.f;

	/** Brazo libre adelantado y doblado al correr (REF-04 n.º 2–3). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float GradosBrazoLibreCarrera = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float FlexionCodoLibreCarrera = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "cm"))
	float BotePaso = 4.f;

	/**
	 * GunZ: el cuerpo mira a la cámara y corre en ocho direcciones. Las piernas oscilan en la
	 * dirección del paso; de lado, la cadera gira hacia él estos grados y el pecho lo compensa.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float GiroCaderaLateral = 35.f;

	/** Corriendo hacia atrás, fracción de la inclinación de carrera (el cuerpo se echa atrás). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionInclinacionAtras = 0.5f;

	/**
	 * Reposo con la espada lista (REF-04 n.º 1, REF-01): la hoja baja en diagonal, adelantada y
	 * abierta estos grados desde la vertical, con la punta lejos del suelo; rodillas algo dobladas.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo", meta = (ForceUnits = "deg"))
	float GradosHojaReposoAdelante = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo", meta = (ForceUnits = "deg"))
	float GradosHojaReposoAfuera = 28.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo", meta = (ForceUnits = "deg"))
	float FlexionRodillaReposo = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float InclinacionDash = 38.f;

	/**
	 * Dash a los lados o hacia atrás: voltereta recogida de estos grados en la dirección del dash (0 = sin voltereta).
	 * REF-04 n.º 7–8 y REF-08 D muestran la misma estocada baja que el dash frontal, inclinada hacia donde va: 0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "deg"))
	float GradosVolteretaDash = 0.f;

	/** Desde este producto escalar con el frente (1 = recto delante) el dash es frontal: estocada en vez de voltereta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "-1", ClampMax = "1"))
	float UmbralDashFrontal = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paredes")
	float InclinacionCarreraPared = 22.f;

	/**
	 * Giro del cuerpo sobre sí mismo al saltar desde una pared (0 = sin giro), y su duración. REF-04 n.º 10: sin
	 * giro, empuje con la mano contra la pared y el cuerpo abierto (0); la duración es la del empuje.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paredes", meta = (ClampMin = "0", ForceUnits = "deg"))
	float GradosGiroSaltoPared = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paredes", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float DuracionGiroSaltoPared = 0.4f;

	/** Aterrizaje fuerte (REF-04 n.º 11): desde esta fuerza (fracción de la caída de referencia) cuclillas abiertas con una mano al suelo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje", meta = (ClampMin = "0"))
	float FuerzaAterrizajeFuerte = 0.85f;

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

	/** La capa la mueven sus muelles (Capa); la pose la sigue de cerca con este suavizado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0"))
	float SuavizadoCapa = 30.f;

	/** Cada tramo de la capa responde con este factor de la frecuencia del de arriba (< 1: la punta llega más tarde y ondea). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0.1", ClampMax = "1"))
	float RetrasoCadenaCapa = 0.75f;

	/** Duración y profundidad de la flexión al aterrizar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje", meta = (ForceUnits = "s"))
	float DuracionAterrizaje = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje")
	float FlexionAterrizaje = 45.f;

	/** Velocidad de caída con la que la flexión del aterrizaje es completa: las caídas cortas flexionan menos y las largas, más. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aterrizaje", meta = (ClampMin = "1", ForceUnits = "cm/s"))
	float VelocidadCaidaAterrizaje = 900.f;

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

	/** Sentado en un banco: bajada de la raíz (cm) y flexión de caderas y rodillas (grados). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Banco", meta = (ForceUnits = "cm"))
	float BajadaSentado = 42.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Banco")
	float FlexionSentado = 85.f;

	/** Reacción al recibir un golpe: el torso y la cabeza se echan atrás y vuelven. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ForceUnits = "s"))
	float DuracionReaccionGolpe = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido")
	float InclinacionReaccionGolpe = 22.f;

	/** Mirada: la cabeza se gira hacia el enemigo vivo más cercano dentro de este radio. 0 = nunca. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirada", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioMirada = 1500.f;

	/** Giros máximos de la cabeza al mirar: a los lados y arriba/abajo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirada", meta = (ClampMin = "0", ClampMax = "90", ForceUnits = "deg"))
	float GiroMaximoCabeza = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mirada", meta = (ClampMin = "0", ClampMax = "90", ForceUnits = "deg"))
	float InclinacionMaximaCabeza = 30.f;

	/** Largo de la hoja de la espada (cm), desde el hueso del arma (la guarda) a la punta: lo usa la estela. Igual que LARGO_HOJA en blender_caballero.py (REF-02). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armas", meta = (ClampMin = "0", ForceUnits = "cm"))
	float LongitudHoja = 90.f;

	/**
	 * Tajos en el suelo (REF-04 n.º 12–13): zancada ancha y baja, el pie derecho adelante y el izquierdo atrás,
	 * los muslos abiertos hacia fuera; la estocada crece con la preparación y se asienta en la recuperación.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "deg"))
	float ZancadaAtaqueDelante = 38.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "deg"))
	float ZancadaAtaqueAtras = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "deg"))
	float AperturaAtaque = 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "deg"))
	float FlexionRodillaAtaqueDelante = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "deg"))
	float FlexionRodillaAtaqueAtras = 22.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "cm"))
	float BajadaAtaque = 16.f;

	/** Tajo en carrera (REF-04 n.º 14): el cuerpo echado hacia delante con la zancada del dash. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "deg"))
	float InclinacionTajoCarrera = 30.f;

	/** Tajos en el aire (REF-04 n.º 15): rodillas recogidas bajo el cuerpo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "deg"))
	float RecogidaPiernasAtaqueAereo = 55.f;

	/**
	 * Enemigos con esqueleto (goblins, REF-05/REF-06): encorvado de la especie, agachada y arma alzada
	 * al avisar (se lee el golpe que viene), giro del pecho y retroceso al quedar aturdido.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo", meta = (ForceUnits = "deg"))
	float EncorvadoEnemigo = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo", meta = (ForceUnits = "cm"))
	float BajadaAvisoEnemigo = 14.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo", meta = (ForceUnits = "deg"))
	float GiroPechoAvisoEnemigo = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo", meta = (ForceUnits = "deg"))
	float InclinacionAturdidoEnemigo = 28.f;

	/** Parpadeos por segundo del cuerpo durante la invulnerabilidad tras un golpe. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0"))
	float FrecuenciaParpadeo = 4.f;

	/** Empujón al tronco al recibir un golpe (grados/s, alejándose del que golpea) y hundimiento de la cadera (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0"))
	float ImpulsoGolpeRecibido = 320.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0"))
	float HundimientoGolpeRecibido = 90.f;

	/** Daño que mueve el cuerpo con la fuerza normal; la reacción escala con el daño real (de media a doble fuerza). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0.1"))
	float DanioReferenciaReaccion = 15.f;

	/** Movimiento secundario: giro por capas, inercia, rebote, respiración, reposo vivo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Secundario")
	FSAEVUSAjusteSecundario Secundario;

	// --- Locomoción viva: la cadera cae y gira con cada paso, la cabeza se estabiliza, los pies ruedan. ---

	/** La cadera cae hacia el lado de la pierna que se levanta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float CaidaCaderaPaso = 4.f;

	/** La cadera gira con la zancada (y el pecho al revés). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float GiroCaderaPaso = 7.f;

	/** El pie rueda: talón al apoyar delante, punta al despegar detrás. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float RodaduraPie = 22.f;

	/** Parte del balanceo del paso que la cabeza compensa (la mirada no bota). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ClampMin = "0", ClampMax = "1"))
	float EstabilizacionCabezaPaso = 0.8f;

	/** Por encima de la velocidad de referencia (esprint): inclinación y subida de rodillas extra. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float InclinacionEsprint = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float RodillasEsprint = 18.f;

	/** Los codos se doblan más cuando el brazo va adelante (bombeo de la carrera). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomocion", meta = (ForceUnits = "deg"))
	float BombeoCodo = 30.f;

	// --- Dash con identidad: arranque explosivo, estirado en vuelo y frenada. ---

	/** Fracción del dash que dura el arranque (el cuerpo se encoge y sale disparado). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float FraccionArranqueDash = 0.18f;

	/** Desde esta fracción del dash empieza la frenada (pies por delante, el tronco se echa atrás). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0.5", ClampMax = "1"))
	float FraccionFrenadaDash = 0.72f;

	/** Dash a un lado: el cuerpo se ladea hacia él estos grados y el brazo libre se abre de contrapeso. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ForceUnits = "deg"))
	float LadeoDashLateral = 26.f;

	/** Dash hacia atrás: el tronco se echa atrás y la espada se adelanta, en guardia. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ForceUnits = "deg"))
	float InclinacionDashAtras = 20.f;

	/** Hundimiento de la cadera al arrancar el dash (cm/s, la mitad) e impulso de la capa (grados/s, el doble). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0"))
	float ImpulsoDash = 140.f;

	// --- Capa con muelles: se queda atrás, ondea, se abre en las curvas y en los dash. ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Capa", meta = (ClampMin = "0.1", ForceUnits = "Hz"))
	float FrecuenciaCapa = 1.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Capa", meta = (ClampMin = "0", ClampMax = "2"))
	float AmortiguacionCapa = 0.32f;

	/** Grados que la capa se va atrás a la velocidad de esprint de referencia, y de lado al correr de lado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Capa", meta = (ForceUnits = "deg"))
	float AtrasCapa = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Capa", meta = (ForceUnits = "deg"))
	float LadoCapa = 30.f;

	// --- Golpe dado: el brazo rebota en el contacto. ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ForceUnits = "deg"))
	float RetrocesoImpacto = 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque", meta = (ClampMin = "0.1", ForceUnits = "Hz"))
	float FrecuenciaRetrocesoImpacto = 5.f;

	// --- Suavizado por tramos: lo de las puntas (manos, hoja, cabeza) llega después que el tronco. ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0.1", ClampMax = "2"))
	float FactorSuavizadoBrazos = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0.1", ClampMax = "2"))
	float FactorSuavizadoManos = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0.1", ClampMax = "2"))
	float FactorSuavizadoCabeza = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suavizado", meta = (ClampMin = "0.1", ClampMax = "3"))
	float FactorSuavizadoPiernas = 1.3f;

	/** Enemigos fuera de pantalla: la pose se actualiza con este intervalo (s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "0", ForceUnits = "s"))
	float IntervaloFueraDePantalla = 0.25f;

	/** LOD por distancia de los enemigos visibles: más allá de esta distancia (cm) se animan con IntervaloLejano. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaLejana = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "0", ForceUnits = "s"))
	float IntervaloLejano = 0.05f;

	/** Más allá de esta distancia (cm) el intervalo es IntervaloMuyLejano. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaMuyLejana = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "0", ForceUnits = "s"))
	float IntervaloMuyLejano = 0.1f;

	/** Intervalo de actualización de la pose (s) para un cuerpo visible a esta distancia (0 = cada fotograma). */
	float IntervaloPorDistancia(float Distancia) const
	{
		if (Distancia >= DistanciaMuyLejana) { return IntervaloMuyLejano; }
		if (Distancia >= DistanciaLejana) { return IntervaloLejano; }
		return 0.f;
	}

	/** La mirada busca enemigo cercano cada tanto, no cada fotograma (s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "0", ForceUnits = "s"))
	float IntervaloBusquedaMirada = 0.2f;

	/** Pies al suelo (ANIMVFX-3): cada pie baja o sube hasta el suelo real (escalones, rampas) con una traza corta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pies al suelo")
	bool bPiesAlSuelo = true;

	/** Cuánto se busca el suelo por encima y por debajo de la planta del pie (cm); fuera de eso el pie no se ajusta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pies al suelo", meta = (ClampMin = "1", ForceUnits = "cm"))
	float AlcanceTrazaPies = 35.f;

	/** Ajuste máximo de un pie respecto al suelo plano (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pies al suelo", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AjusteMaximoPies = 28.f;

	/** Por encima de esta altura sobre el suelo (cm) el pie está en el aire de la zancada y no se ajusta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pies al suelo", meta = (ClampMin = "0.1", ForceUnits = "cm"))
	float AlturaContactoPies = 9.f;

	/** Rapidez con la que el ajuste de cada pie y de la cadera sigue al suelo (1/s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pies al suelo", meta = (ClampMin = "0.1"))
	float VelocidadAjustePies = 16.f;
};

/**
 * Animación procedural del caballero (y de los goblins) sobre una malla poseable con esqueleto propio.
 *
 * Cada fotograma construye una pose objetivo a partir del estado del juego (movimiento, paredes, dash,
 * ataques con su gesto, armas, muerte), le suma la capa secundaria (giro por capas, inercia del tronco,
 * rebote, respiración, reposo vivo; FSAEVUSCapaSecundaria) y suaviza la pose actual hacia ella por
 * tramos: el tronco llega antes que las manos, la hoja y la cabeza. La capa y el rebote del brazo van
 * con muelles. La pose es, por hueso, una dirección absoluta a la que apuntar o un giro relativo,
 * compuestos sobre la pose de reposo por un FK propio.
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

	/** Opción del jugador: sin parpadeo tras un golpe el cuerpo se ve siempre (sigue siendo invulnerable). */
	void FijarParpadeoActivo(bool bActivo) { bParpadeoJugador = bActivo; }

	/** Avanza la animación DeltaTime segundos (lo llama el tick; los tests lo llaman a mano). */
	void Actualizar(float DeltaTime);

	/** Peso del cuerpo: 1 = humano; más es más lento y pesado (jefes). Lo fija el enemigo según su tipo. */
	void FijarPeso(float NuevoPeso) { Peso = FMath::Max(0.1f, NuevoPeso); }

	/** Movimiento secundario de este fotograma (tests, depuración). */
	const FSAEVUSSalidaSecundaria& ObtenerSecundaria() const { return CapaSecundaria.ObtenerSalida(); }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	/** Enemigos con esqueleto: la pose sale del estado de su IA (aviso, ataque, recuperación, aturdido). */
	void ConstruirPoseEnemigo(FPose& Pose, float DeltaTime);
	/** Velocidad del dueño (jugador o enemigo). */
	FVector VelocidadDueno() const;
	void PoseLocomocion(FPose& Pose, float Velocidad, float DeltaTime);
	void PoseAire(FPose& Pose, float VelocidadVertical);
	void PoseDash(FPose& Pose);
	void PoseCarreraPared(FPose& Pose, float DeltaTime);
	void PoseEscalada(FPose& Pose, float DeltaTime);
	/** El gesto del ataque (SAEVUSGestosAtaque) en su instante, con su pisada o sus piernas recogidas. */
	void PoseAtaque(FPose& Pose, const FSAEVUSDefinicionAtaque& Ataque, float Tiempo);
	void PoseDisparo(FPose& Pose);
	void PoseMuerte(FPose& Pose, float DeltaTime);
	void PoseCuracion(FPose& Pose);
	void PoseSentado(FPose& Pose);
	void PoseGuardia(FPose& Pose);
	/** Oculta y muestra el cuerpo durante la invulnerabilidad tras un golpe. */
	void AplicarParpadeo();
	/** Capa con muelles: atrás con la velocidad, de lado al correr de lado o girar, levantada al caer. */
	void PoseCapa(FPose& Pose, float DeltaTime);
	/** Suma a la pose el movimiento secundario (giros por capas, inercia, rebote, respiración, reposo vivo). */
	void AplicarSecundaria(FPose& Pose, float DeltaTime, bool bAccion, bool bEsfuerzo, bool bReposoLibre);
	/** Gestos de reposo (mirar atrás, girar la hoja, estirar los hombros) sobre la pose de reposo. */
	void PoseGestoReposo(FPose& Pose, const FSAEVUSSalidaSecundaria& Secundaria);

	/**
	 * Suma un giro (espacio de malla) al hueso: compuesto con su giro relativo, y las direcciones absolutas de
	 * todo lo que cuelga de él (brazos, piernas, hoja) giran igual, así el miembro va con el tramo que se mueve.
	 */
	void GirarTramo(FPose& Pose, ESAEVUSHueso Hueso, const FQuat& Giro) const;

	/** Compone la pose objetivo sobre la de reposo (FK): rotaciones objetivo en espacio de malla. */
	void ResolverObjetivo(const FPose& Pose, FQuat (&OutRotaciones)[static_cast<int32>(ESAEVUSHueso::Total)]) const;
	/** Suaviza las rotaciones actuales hacia las objetivo por tramos, recalcula posiciones y lo aplica a la malla. */
	void SuavizarYAplicar(const FPose& Pose, const FQuat (&Objetivo)[static_cast<int32>(ESAEVUSHueso::Total)], float DeltaTime, bool bAccion);
	/** Escribe las transformaciones de los huesos (espacio de componente) en la malla de una vez, sin buscar nombres. */
	void EscribirEnMalla(const FTransform (&Componente)[static_cast<int32>(ESAEVUSHueso::Total)]);
	/**
	 * Pies al suelo: traza hacia abajo bajo cada pie y, si el suelo no es plano, lo coloca encima con cinemática inversa
	 * de dos huesos (muslo y pierna). Además fija cuánto baja la cadera el próximo fotograma (AjusteCaderaSuelo, ≤ 0)
	 * para que el pie más bajo llegue.
	 */
	void AplicarPiesAlSuelo(FTransform (&Componente)[static_cast<int32>(ESAEVUSHueso::Total)], float DeltaTime);
	/** Altura del suelo bajo un punto del mundo respecto al origen de la malla (cm locales); falso si no hay suelo a mano. */
	bool AlturaSueloBajo(const FVector& PuntoMundo, float& OutAltura) const;

	UFUNCTION()
	void AlDisparar(FVector Boca, FVector Impacto, AActor* ActorGolpeado);

	void AlSaltoAereo();
	void AlSaltoPared(const FVector& NormalPared);
	void AlIniciarDash(const FVector& Direccion);
	void AlAterrizar(float VelocidadCaida);
	void AlImpactar(const FSAEVUSDefinicionAtaque& Ataque);
	/** Jefes: al entrar en una fase nueva, rugen (SAEVUSAnimacionEnemigo.cpp). */
	void AlCambiarFaseEnemigo(int32 Fase);

	/** Dirección horizontal del mundo pasada al espacio de la malla (unitaria); al frente si es nula. */
	FVector DireccionMalla(const FVector& DireccionMundo) const;

	UFUNCTION()
	void AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador);

	/** Brazos sueltos tras un golpe recibido (el tronco lo empuja la capa secundaria). */
	void PoseReaccionGolpe(FPose& Pose);
	/** La cabeza mira al enemigo cercano (o vaga, quieta) si ninguna otra pose la ocupa. */
	void PoseMirada(FPose& Pose, float DeltaTime);

	/** Dirección de reposo de un hueso en espacio de malla (hacia su hijo, o la fija de las hojas). */
	FVector DireccionReposo(int32 Hueso) const;

	UPROPERTY()
	TObjectPtr<UPoseableMeshComponent> Malla;

	// Componentes hermanos del dueño (viven lo mismo que él).
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteMovimiento> Movimiento;
	/** Movimiento de cualquier dueño (los enemigos usan el del motor). */
	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> MovimientoBase;
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteIAEnemigo> IA;
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteCombate> Combate;
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteArmas> Armas;
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteVida> Vida;
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteAlma> Alma;
	UPROPERTY(Transient)
	TObjectPtr<USAEVUSComponenteGuardia> Guardia;

	bool bEsqueletoValido = false;
	int32 IndiceMalla[static_cast<int32>(ESAEVUSHueso::Total)];
	int32 Padre[static_cast<int32>(ESAEVUSHueso::Total)];
	FTransform ReposoComponente[static_cast<int32>(ESAEVUSHueso::Total)];
	/** Para cada hueso, qué huesos cuelgan de él (bit i = hueso i; incluye el propio). */
	uint32 Subarbol[static_cast<int32>(ESAEVUSHueso::Total)];
	/** Pose de la malla entera en espacio de componente (reposo, con los huesos propios encima) y padre de cada hueso. */
	TArray<FTransform> ComponenteMalla;
	TArray<int32> PadreMalla;

	// Pose actual (ya suavizada) en espacio de malla.
	FQuat RotacionActual[static_cast<int32>(ESAEVUSHueso::Total)];
	FVector PosicionActual[static_cast<int32>(ESAEVUSHueso::Total)];
	FVector DesplazamientoRaizActual = FVector::ZeroVector;
	/** Pies al suelo: ajuste suavizado de cada pie (D, I) respecto al suelo plano y bajada de la cadera (cm, ≤ 0). */
	float AjustePieSuelo[2] = { 0.f, 0.f };
	float AjusteCaderaSuelo = 0.f;
	float EscalaArmaActual = 1.f;
	float EscalaArmaFuegoActual = 0.f;
	bool bPoseIniciada = false;
	bool bParpadeoJugador = true;
	float FaseCiclo = 0.f;
	float TiempoTotal = 0.f;
	float TiempoAterrizaje = 0.f;
	float TiempoMuerto = 0.f;
	float Retroceso = 0.f;
	float TiempoRecogidaSaltoAereo = 0.f;
	float TiempoReaccionGolpe = 0.f;
	bool bEstabaEnAire = false;
	float TiempoGiroSaltoPared = 0.f;
	/** Normal de la pared del último salto de pared (espacio de mundo): hacia ella empuja la mano. */
	FVector NormalSaltoPared = FVector::ZeroVector;
	float VelocidadVerticalAire = 0.f;
	/** Intensidad del último aterrizaje (según la velocidad de caída). */
	float FuerzaAterrizaje = 1.f;

	FSAEVUSCapaSecundaria CapaSecundaria;
	/** Tramos de la capa: X = hacia atrás, Y = hacia la derecha (grados). */
	FSAEVUSMuelleVector MuellesCapa[3];
	/** Rebote del brazo al acertar (grados). */
	FSAEVUSMuelle RetrocesoImpacto;
	float Peso = 1.f;
	/** Tiempo acumulado sin actualizar (enemigos fuera de pantalla). */
	float TiempoPendiente = 0.f;
	/** Enemigo al que mira la cabeza (se busca cada IntervaloBusquedaMirada). */
	TWeakObjectPtr<AActor> ObjetivoMirada;
	float TiempoBusquedaMirada = 0.f;
	/** Giro de la cabeza hacia lo que mira (muelle): X = giro, Y = cabeceo. */
	FSAEVUSMuelleVector GiroMirada;
	/** Lado del enemigo del que vino el último golpe recibido (espacio de malla), para la muerte. */
	FVector DireccionUltimoGolpe = FVector::ZeroVector;
	FDelegateHandle ManejadorDash;
	FDelegateHandle ManejadorImpacto;
	FDelegateHandle ManejadorSaltoAereo;
	FDelegateHandle ManejadorSaltoPared;
	FDelegateHandle ManejadorFase;
	FDelegateHandle ManejadorAterrizaje;
	/** Rugido de cambio de fase en curso (s que quedan). */
	float TiempoRugido = 0.f;
};
