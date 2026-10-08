// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSTiposCombate.generated.h"

/** En qué punto de su línea de tiempo está un ataque. */
UENUM(BlueprintType)
enum class ESAEVUSFaseAtaque : uint8
{
	Ninguna,
	/** Preparación: todavía no golpea. */
	Preparacion,
	/** El volumen de golpe está activo. */
	Activa,
	/** Recuperación: no golpea; el siguiente ataque puede encadenarse desde EncadenarDesde. */
	Recuperacion
};

/** Hacia dónde va un ataque cuerpo a cuerpo: de frente o hacia arriba (la inclinación de la cámara lo elige). */
UENUM(BlueprintType)
enum class ESAEVUSDireccionAtaque : uint8
{
	Frontal,
	Arriba
};

/**
 * Animación de un ataque: cada gesto tiene su trayectoria de la hoja, su giro del tronco, su paso y su
 * asentamiento (SAEVUSGestosAtaque.cpp). Va en los datos del ataque, así cada golpe del repertorio se
 * ve distinto y uno nuevo solo tiene que elegir el suyo.
 */
UENUM(BlueprintType)
enum class ESAEVUSGestoAtaque : uint8
{
	/** Tajo horizontal de derecha a izquierda, con el pie derecho adelante. */
	TajoDerecha,
	/** Revés: desde la cadera izquierda hacia arriba a la derecha, con el pie izquierdo adelante. */
	Reves,
	/** Remate: la hoja por encima de la cabeza a dos manos y abajo hasta el suelo, zancada honda. */
	Remate,
	/** Estocada a fondo: la hoja recogida junto al pecho sale recta hacia delante (tajo en carrera). */
	Estocada,
	/** Tajo hacia arriba: desde abajo delante hasta por encima de la cabeza, el cuerpo arqueado atrás. */
	TajoArriba,
	/** En el aire: diagonal de arriba a la derecha a abajo a la izquierda, las piernas recogidas. */
	AereoDiagonal,
	/** En el aire: revés horizontal con el cuerpo torcido y las piernas al lado contrario. */
	AereoReves,
	/** En el aire: tajo vertical con todo el cuerpo volcado hacia delante, las piernas atrás. */
	AereoRemate,
	/** Giro completo con la hoja extendida (ataque cargado). */
	Giratorio
};

/** Acciones del jugador que pueden cortar un ataque, cada una con su ventana en los datos del ataque. */
UENUM(BlueprintType)
enum class ESAEVUSAccionCancelacion : uint8
{
	Dash,
	Salto,
	CambioArma,
	Guardia
};

/**
 * Un ataque cuerpo a cuerpo, con tiempos en segundos desde su inicio. La línea de tiempo es el
 * ataque: fases, volumen de golpe, estocada y ventanas de cancelación. La animación se deriva de ella.
 */
USTRUCT(BlueprintType)
struct FSAEVUSDefinicionAtaque
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque")
	FName Nombre = TEXT("Ataque");

	/** Animación del ataque (trayectoria de la hoja, tronco, paso). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque")
	ESAEVUSGestoAtaque Gesto = ESAEVUSGestoAtaque::TajoDerecha;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoPreparacion = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoActivo = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoRecuperacion = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0"))
	float Danio = 10.f;

	/**
	 * Zona de golpe en abanico (GunZ): todo lo que esté a menos de AlcanceAbanico del centro del cuerpo
	 * (a AlturaGolpe), dentro de ±AnguloAbanico del frente y a ±MitadAlturaAbanico de altura. Es lo que
	 * dibuja el arco del tajo: lo que se ve es lo que golpea. AnguloAbanico 0 = esfera (RadioGolpe).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AlcanceAbanico = 170.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ClampMax = "180", ForceUnits = "deg"))
	float AnguloAbanico = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ForceUnits = "cm"))
	float MitadAlturaAbanico = 70.f;

	/** Esfera de golpe (sin abanico), delante del personaje mientras el ataque está activo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioGolpe = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ForceUnits = "cm"))
	float DesplazamientoFrontalGolpe = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ForceUnits = "cm"))
	float AlturaGolpe = 20.f;

	/** Movimiento guiado por la espada: velocidad hacia delante mantenida desde el inicio del ataque. 0 = nada. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadEstocada = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionEstocada = 0.12f;

	/** Multiplicador del tope de velocidad en el suelo mientras dura este ataque (no limita la estocada). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ClampMax = "1"))
	float EscalaVelocidadSuelo = 0.25f;

	/** Al golpear, el atacante sale despedido hacia atrás (horizontal) a esta velocidad. 0 = nada. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacto", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float RetrocesoAtacante = 250.f;

	/** Velocidad con la que el golpe empuja a un personaje golpeado, en la dirección del tajo. 0 = nada. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacto", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float EmpujeObjetivo = 350.f;

	/** Parada de golpe (hit-stop) en tiempo real al acertar. 0 = sin parada. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacto", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionParadaGolpe = 0.06f;


	/** En el aire: multiplica la gravedad mientras dura el ataque (menos de 1 = flota, GunZ). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aire", meta = (ClampMin = "0"))
	float EscalaGravedadEnAire = 1.f;

	/** En el aire: fracción de la velocidad de caída que se conserva al empezar el ataque (1 = toda). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aire", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionCaidaConservada = 1.f;

	/** Desde este tiempo, una pulsación de ataque en buffer inicia el siguiente ataque de la cadena. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancelaciones", meta = (ForceUnits = "s"))
	float EncadenarDesde = 0.18f;

	/** Desde este tiempo un dash puede cortar el ataque. Negativo: nunca. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancelaciones", meta = (ForceUnits = "s"))
	float CancelarConDashDesde = 0.18f;

	/** Desde este tiempo un salto puede cortar el ataque. Negativo: nunca. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancelaciones", meta = (ForceUnits = "s"))
	float CancelarConSaltoDesde = 0.18f;

	/** Desde este tiempo un cambio de arma puede cortar el ataque. Negativo: nunca. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancelaciones", meta = (ForceUnits = "s"))
	float CancelarConCambioArmaDesde = 0.18f;

	/** Desde este tiempo la guardia puede cortar el ataque (tajo → guardia, GunZ). Negativo: nunca. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancelaciones", meta = (ForceUnits = "s"))
	float CancelarConGuardiaDesde = 0.18f;

	float ObtenerTiempoTotal() const { return TiempoPreparacion + TiempoActivo + TiempoRecuperacion; }

	ESAEVUSFaseAtaque ObtenerFaseEn(float Tiempo) const
	{
		if (Tiempo < TiempoPreparacion)
		{
			return ESAEVUSFaseAtaque::Preparacion;
		}
		if (Tiempo < TiempoPreparacion + TiempoActivo)
		{
			return ESAEVUSFaseAtaque::Activa;
		}
		return Tiempo < ObtenerTiempoTotal() ? ESAEVUSFaseAtaque::Recuperacion : ESAEVUSFaseAtaque::Ninguna;
	}

	float ObtenerCancelacionDesde(ESAEVUSAccionCancelacion Accion) const
	{
		switch (Accion)
		{
		case ESAEVUSAccionCancelacion::Dash:
			return CancelarConDashDesde;
		case ESAEVUSAccionCancelacion::Salto:
			return CancelarConSaltoDesde;
		case ESAEVUSAccionCancelacion::Guardia:
			return CancelarConGuardiaDesde;
		case ESAEVUSAccionCancelacion::CambioArma:
		default:
			return CancelarConCambioArmaDesde;
		}
	}
};

/**
 * Los ataques cuerpo a cuerpo de un personaje: una cadena en el suelo, otra en el aire, y el
 * manejo de la entrada. El constructor tiene el repertorio por defecto; un USAEVUSDatosRepertorioMelee lo sobrescribe.
 */
USTRUCT(BlueprintType)
struct SAEVUS_API FSAEVUSRepertorioMelee
{
	GENERATED_BODY()

	FSAEVUSRepertorioMelee();

	/** Ataques encadenados en orden en el suelo; la cadena vuelve a empezar tras el último. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repertorio")
	TArray<FSAEVUSDefinicionAtaque> ComboSuelo;

	/** Ataques encadenados en orden cuando el primero empieza en el aire. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repertorio")
	TArray<FSAEVUSDefinicionAtaque> ComboAire;

	/** Tajo hacia arriba (suelo y aire). No encadena. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repertorio")
	FSAEVUSDefinicionAtaque TajoArriba;


	/**
	 * Tajo en carrera (GunZ): atacar durante un dash. Sigue en la dirección del dash con su propia
	 * estocada rápida y pega más fuerte. No encadena. Necesita bAtaqueTerminaDash.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repertorio")
	FSAEVUSDefinicionAtaque TajoEnCarrera;

	/**
	 * Ataque cargado (Hollow Knight, arte del aguijón): mantener el ataque TiempoCarga y soltar.
	 * Tajo giratorio alrededor del cuerpo. No encadena.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repertorio")
	FSAEVUSDefinicionAtaque AtaqueCargado;

	/** Tiempo manteniendo el ataque (sin estar atacando) hasta tener la carga lista. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entrada", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float TiempoCarga = 0.8f;

	/**
	 * Adaptación a 3D del "arriba/abajo + ataque": con la cámara inclinada al menos este ángulo
	 * hacia arriba sale el tajo hacia arriba.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dirección", meta = (ClampMin = "0", ClampMax = "90", ForceUnits = "deg"))
	float UmbralInclinacionArriba = 30.f;


	/** Escala de tiempo de los actores durante la parada de golpe (0 = congelados, 1 = sin parada). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacto", meta = (ClampMin = "0", ClampMax = "1"))
	float EscalaTiempoParada = 0.05f;

	/** Una pulsación de ataque se recuerda este tiempo hasta que puede empezar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entrada", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoBufferEntrada = 0.2f;

	/** Un ataque pulsado durante un dash termina el dash y empieza en el acto (si no, espera). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cancelaciones")
	bool bAtaqueTerminaDash = true;
};

namespace SAEVUSCombate
{
	/**
	 * Etiqueta de actor de los blancos de práctica (muñecos): cuentan como golpe (alma, chispas) aunque no
	 * tengan USAEVUSComponenteVida. El resto del escenario que recibe daño (puertas, plataformas) no cuenta.
	 */
	SAEVUS_API extern const FName EtiquetaBlancoPractica;

	/** Sentido del barrido de un gesto vista desde arriba: +1 de derecha a izquierda, −1 al revés. */
	SAEVUS_API float SentidoBarrido(ESAEVUSGestoAtaque Gesto);
}

/** Repertorio cuerpo a cuerpo editable asignado a USAEVUSComponenteCombate. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSDatosRepertorioMelee : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Repertorio")
	FSAEVUSRepertorioMelee Repertorio;
};
