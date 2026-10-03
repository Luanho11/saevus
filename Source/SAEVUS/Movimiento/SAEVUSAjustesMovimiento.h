// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SAEVUSAjustesMovimiento.generated.h"

/**
 * Ajuste del movimiento en el suelo. Los inicializadores son la única fuente de valores
 * por defecto; un USAEVUSAjustesMovimiento los sobrescribe por personaje.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteMovimientoSuelo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paso", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadCaminar = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paso", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadCorrer = 550.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Paso", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadEsprint = 850.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respuesta", meta = (ClampMin = "0", ForceUnits = "cm/s^2"))
	float AceleracionMaxima = 2400.f;

	/** Deceleración cuando no hay entrada de movimiento. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respuesta", meta = (ClampMin = "0", ForceUnits = "cm/s^2"))
	float DeceleracionFrenado = 2000.f;

	/** Valores altos dan cambios de dirección más secos y menos deslizamiento. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respuesta", meta = (ClampMin = "0"))
	float FriccionSuelo = 8.f;

	/** Rapidez con la que el personaje gira hacia donde se mueve. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respuesta", meta = (ClampMin = "0", ForceUnits = "deg/s"))
	float VelocidadGiro = 720.f;
};

/** Ajuste del salto y del aire. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteMovimientoAire
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadSalto = 700.f;

	/** Tiempo que el botón de salto mantenido sigue aplicando VelocidadSalto. 0 = salto de altura fija. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoMaximoMantenerSalto = 0.f;

	/** Multiplicador de la gravedad del mundo. Mayor que 1 da un arco más pesado y seco. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gravedad", meta = (ClampMin = "0"))
	float EscalaGravedad = 1.75f;

	/** Fracción de la aceleración de suelo disponible en el aire (0 = nada, 1 = toda). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Control aéreo", meta = (ClampMin = "0", ClampMax = "1"))
	float ControlAire = 0.35f;

	/** Multiplicador del control aéreo mientras la velocidad horizontal está bajo el umbral. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Control aéreo", meta = (ClampMin = "0"))
	float MultiplicadorImpulsoControlAire = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Control aéreo", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float UmbralImpulsoControlAire = 25.f;

	/** Fricción lateral al caer. 0 conserva el impulso del despegue. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Control aéreo", meta = (ClampMin = "0"))
	float FriccionLateralCaida = 0.f;

	/** Deceleración lateral al caer sin entrada. 0 conserva el impulso del despegue. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Control aéreo", meta = (ClampMin = "0", ForceUnits = "cm/s^2"))
	float DeceleracionFrenadoCaida = 0.f;

	/**
	 * Mantiene en el aire el tope de velocidad del paso usado al despegar: el esprint no se puede
	 * activar (ni soltar) en pleno salto. Desactivado, el tope sigue al paso pedido.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Control aéreo")
	bool bBloquearPasoEnAire = true;

	/** Margen tras salir caminando de un borde en el que el salto de suelo sigue permitido. 0 lo desactiva. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asistencia de salto", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoCoyote = 0.12f;

	/** Un salto pulsado hasta este tiempo antes de aterrizar se ejecuta al tocar suelo. 0 lo desactiva. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asistencia de salto", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoBufferSalto = 0.12f;

	/** Saltos en el aire (doble salto) antes de aterrizar. 0 los desactiva. Ver Documentos/movimiento.md. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Doble salto", meta = (ClampMin = "0"))
	int32 MaxSaltosAereos = 1;

	/** Velocidad vertical que fija el doble salto (sustituye a la de caída: siempre sube lo mismo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Doble salto", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadDobleSalto = 650.f;
};

/**
 * Dash: ráfaga corta a velocidad constante en la dirección de la entrada (o hacia delante sin
 * entrada), en el suelo o en el aire. Ver Documentos/movimiento.md.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteDash
{
	GENERATED_BODY()

	/** Velocidad horizontal mantenida durante todo el dash. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Velocidad = 1600.f;

	/** Duración del dash; la distancia es más o menos Velocidad x Duracion. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "s"))
	float Duracion = 0.18f;

	/** Al terminar el dash (o cancelarlo con un salto) la velocidad horizontal se limita a esto. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadSalida = 700.f;

	/** Tiempo tras terminar un dash antes de poder empezar otro. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "s"))
	float Enfriamiento = 0.3f;

	/** Gira el cuerpo hacia la dirección del dash al empezar, para que se lea bien. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	bool bGirarHaciaDash = true;

	/** Dashes permitidos por cada estancia en el aire; se recargan al aterrizar. 0 desactiva el dash aéreo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash aéreo", meta = (ClampMin = "0"))
	int32 MaxDashesAereos = 1;

	/** Velocidad vertical fijada al empezar un dash aéreo (0 = plano). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash aéreo", meta = (ForceUnits = "cm/s"))
	float VelocidadVerticalDashAereo = 0.f;

	/** Sin gravedad mientras dura un dash aéreo, así se mantiene la velocidad vertical de arriba. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash aéreo")
	bool bSuspenderGravedadEnDash = true;

	/** La cámara no gira mientras dura el dash (como en GunZ); la dirección del dash queda fija. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	bool bBloquearCamaraEnDash = true;

	/** Invulnerabilidad desde el inicio del dash (esquiva). 0 = ninguna. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionInvulnerable = 0.15f;
};

/**
 * Salto de pared: un salto pulsado en el aire junto a una pared casi vertical empuja al
 * personaje lejos de ella. Ver Documentos/movimiento.md.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteSaltoPared
{
	GENERATED_BODY()

	/** Velocidad horizontal alejándose de la pared, según su normal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto de pared", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadAlejarse = 650.f;

	/** Velocidad vertical que fija el salto de pared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto de pared", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadSubida = 750.f;

	/** Fracción de la velocidad paralela a la pared que se conserva (0 = nada, 1 = toda). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto de pared", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionVelocidadTangencial = 0.5f;

	/** Saltos de pared permitidos por estancia en el aire; se recargan al aterrizar. 0 los desactiva. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto de pared", meta = (ClampMin = "0"))
	int32 MaxSaltosPared = 3;

	/** Mayor hueco entre la cápsula y la pared que todavía cuenta como tocarla. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detección", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaDeteccion = 30.f;

	/** Superficies con normal Z mayor que esto son suelos o rampas, no paredes (0.3 son unos 72 grados). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detección", meta = (ClampMin = "0", ClampMax = "1"))
	float MaxNormalZPared = 0.3f;

	/** Un salto de pared devuelve los dashes aéreos, así dash y salto de pared se encadenan. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto de pared")
	bool bRecargaDashesAereos = true;

	/** Un salto de pared devuelve también los dobles saltos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto de pared")
	bool bRecargaSaltosAereos = true;

	/** Gira el cuerpo hacia la nueva dirección horizontal, para que se lea bien. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salto de pared")
	bool bGirarHaciaSalto = true;
};

/**
 * Carrera por pared: en el aire, moviéndose a lo largo de una pared con entrada que no apunta
 * lejos de ella, el personaje corre por la pared un tiempo limitado con menos gravedad.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteCarreraPared
{
	GENERATED_BODY()

	/** Velocidad a lo largo de la pared durante la carrera. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Carrera por pared", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Velocidad = 750.f;

	/** Carrera más larga posible; después cae. 0 desactiva la carrera por pared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Carrera por pared", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionMaxima = 1.2f;

	/** Multiplicador de la gravedad del personaje mientras está en la pared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Carrera por pared", meta = (ClampMin = "0"))
	float EscalaGravedad = 0.3f;

	/** Velocidad paralela a la pared necesaria para empezar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entrada", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadMinimaEntrada = 300.f;

	/** Al empezar, la velocidad de subida se limita a esto; la de caída se anula. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entrada", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float MaxVelocidadSubidaEntrada = 300.f;

	/** Cayendo más rápido que esto, no se agarra la pared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entrada", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float MaxVelocidadCaidaEntrada = 600.f;

	/** Mayor hueco entre la cápsula y la pared para empezar o seguir corriendo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detección", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaDeteccion = 40.f;

	/** Superficies con normal Z mayor que esto no sirven para correr. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detección", meta = (ClampMin = "0", ClampMax = "1"))
	float MaxNormalZPared = 0.3f;

	/** Entrada de movimiento apuntando lejos de la pared más que esto (producto con la normal) suelta la pared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Carrera por pared", meta = (ClampMin = "0", ClampMax = "1"))
	float UmbralSoltarEntrada = 0.7f;

	/** Empezar una carrera por pared devuelve los dashes aéreos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Carrera por pared")
	bool bRecargaDashesAereos = true;
};

/**
 * Escalada de pared: saltando de frente a una pared con entrada hacia ella, el personaje sube
 * corriendo un tiempo limitado; luego hace una voltereta hacia atrás, o sube a la cornisa si la alcanza.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteEscaladaPared
{
	GENERATED_BODY()

	/** Velocidad de subida; la altura es más o menos Velocidad x DuracionMaxima. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escalada", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Velocidad = 650.f;

	/** Escalada más larga posible; luego voltereta lejos de la pared. 0 desactiva la escalada. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escalada", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionMaxima = 0.55f;

	/** Escaladas permitidas por estancia en el aire; se recargan al aterrizar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escalada", meta = (ClampMin = "0"))
	int32 MaxEscaladas = 1;

	/** Velocidad hacia la pared necesaria para empezar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entrada", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadMinimaEntrada = 200.f;

	/** Cuán de frente deben ir el movimiento y la entrada (producto con la dirección hacia la pared; 0.85 son unos 32 grados). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entrada", meta = (ClampMin = "0", ClampMax = "1"))
	float UmbralFrontal = 0.85f;

	/** Cayendo más rápido que esto, no se agarra la pared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entrada", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float MaxVelocidadCaidaEntrada = 600.f;

	/** Mayor hueco entre la cápsula y la pared para empezar o seguir escalando. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detección", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaDeteccion = 40.f;

	/** Superficies con normal Z mayor que esto no se pueden escalar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detección", meta = (ClampMin = "0", ClampMax = "1"))
	float MaxNormalZPared = 0.3f;

	/** Entrada de movimiento apuntando lejos de la pared más que esto (producto con la normal) suelta la pared. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escalada", meta = (ClampMin = "0", ClampMax = "1"))
	float UmbralSoltarEntrada = 0.7f;

	/** Voltereta al final de la escalada: velocidad alejándose de la pared y hacia arriba. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salida", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadVolteretaAlejarse = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salida", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadVolteretaSubida = 250.f;

	/** Al llegar arriba: velocidad hacia delante para subir a la cornisa (se conserva la de subida). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Salida", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadAvanceCornisa = 300.f;

	/** Empezar una escalada devuelve los dashes aéreos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escalada")
	bool bRecargaDashesAereos = true;
};

/** Perfil de movimiento editable asignado a USAEVUSComponenteMovimiento. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSAjustesMovimiento : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suelo")
	FSAEVUSAjusteMovimientoSuelo Suelo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aire")
	FSAEVUSAjusteMovimientoAire Aire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash")
	FSAEVUSAjusteDash Dash;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Salto de pared")
	FSAEVUSAjusteSaltoPared SaltoPared;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Carrera por pared")
	FSAEVUSAjusteCarreraPared CarreraPared;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Escalada")
	FSAEVUSAjusteEscaladaPared Escalada;
};
