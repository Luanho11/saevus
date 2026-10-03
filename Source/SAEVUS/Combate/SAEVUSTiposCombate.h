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

/** Hacia dónde va un ataque cuerpo a cuerpo (tajos de Hollow Knight: de frente, arriba, abajo en el aire). */
UENUM(BlueprintType)
enum class ESAEVUSDireccionAtaque : uint8
{
	Frontal,
	Arriba,
	/** Solo en el aire; en el suelo se convierte en frontal. */
	Abajo
};

/** Acciones del jugador que pueden cortar un ataque, cada una con su ventana en los datos del ataque. */
UENUM(BlueprintType)
enum class ESAEVUSAccionCancelacion : uint8
{
	Dash,
	Salto,
	CambioArma
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoPreparacion = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoActivo = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoRecuperacion = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0"))
	float Danio = 10.f;

	/** Esfera de golpe, delante del personaje mientras el ataque está activo. */
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

	/** Rebote (pogo): al acertar en el aire, la velocidad vertical pasa a este valor y se recargan dash y doble salto. 0 = sin rebote. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacto", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadRebote = 0.f;

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

	/** Tajo hacia abajo, solo en el aire; rebota al acertar (pogo). No encadena. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repertorio")
	FSAEVUSDefinicionAtaque TajoAbajo;

	/**
	 * Adaptación a 3D del "arriba/abajo + ataque": con la cámara inclinada al menos este ángulo
	 * hacia arriba sale el tajo hacia arriba; inclinada hacia abajo hasta UmbralInclinacionAbajo, el tajo hacia abajo.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dirección", meta = (ClampMin = "0", ClampMax = "90", ForceUnits = "deg"))
	float UmbralInclinacionArriba = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dirección", meta = (ClampMin = "-90", ClampMax = "0", ForceUnits = "deg"))
	float UmbralInclinacionAbajo = -40.f;

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

/** Repertorio cuerpo a cuerpo editable asignado a USAEVUSComponenteCombate. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSDatosRepertorioMelee : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Repertorio")
	FSAEVUSRepertorioMelee Repertorio;
};
