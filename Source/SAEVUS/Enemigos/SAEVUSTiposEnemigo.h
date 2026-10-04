// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SAEVUSTiposEnemigo.generated.h"

/** Estados de la IA de un enemigo. Ver Documentos/enemigos.md. */
UENUM(BlueprintType)
enum class ESAEVUSEstadoEnemigo : uint8
{
	/** Patrulla alrededor de su origen hasta ver al objetivo. */
	Reposo,
	/** Va hacia el objetivo hasta tener un ataque a distancia. */
	Perseguir,
	/** Se para y avisa del ataque que viene (lectura para el jugador). */
	Telegrafiar,
	Atacar,
	/** Ventana de castigo tras atacar. */
	Recuperarse,
	/** Golpeado: no actúa un momento. */
	Aturdido,
	Muerto
};

/** Forma de un ataque de enemigo. */
UENUM(BlueprintType)
enum class ESAEVUSTipoAtaqueEnemigo : uint8
{
	/** Carrera recta hacia donde estaba el objetivo al acabar de avisar. */
	Embestida,
	/** Salto en arco hacia el objetivo. */
	Salto,
	/** Golpe en una esfera delante del enemigo. */
	Golpe,
	/** Lanza un proyectil hacia el objetivo (ASAEVUSProyectil): se bloquea con la guardia y se devuelve con un tajo. */
	Proyectil,
	/** Voladores: picado en línea recta (también en vertical) hacia donde estaba el objetivo al acabar de avisar. */
	Picado
};

/** Un ataque de enemigo. Tiempos en segundos; se escalan con la fase. */
USTRUCT(BlueprintType)
struct FSAEVUSAtaqueEnemigo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque")
	FName Nombre = TEXT("Embestida");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque")
	ESAEVUSTipoAtaqueEnemigo Tipo = ESAEVUSTipoAtaqueEnemigo::Embestida;

	/** Solo se elige con el objetivo entre estas distancias (horizontales, de centro a centro). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elección", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaMinima = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elección", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaMaxima = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoTelegrafiar = 0.5f;

	/** Embestida: lo que dura. Salto: tiempo máximo en el aire. Golpe: tiempo hasta pasar a recuperarse. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float Duracion = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoRecuperacion = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0"))
	float Danio = 20.f;

	/** Embestida: velocidad de la carrera. Salto: velocidad horizontal del salto. Proyectil: su velocidad. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Velocidad = 1000.f;

	/** Salto: velocidad vertical al despegar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadVertical = 0.f;

	/** Golpe: esfera de radio RadioGolpe a AlcanceGolpe delante del enemigo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioGolpe = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AlcanceGolpe = 120.f;

	/** Proyectil: cuántos salen a la vez, repartidos en abanico horizontal (1 = uno recto al objetivo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Proyectil", meta = (ClampMin = "1"))
	int32 NumeroProyectiles = 1;

	/** Proyectil: apertura total del abanico; el del centro va al objetivo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Proyectil", meta = (ClampMin = "0", ClampMax = "180", ForceUnits = "deg"))
	float AperturaAbanico = 0.f;
};

/** Una fase de un enemigo (los jefes cambian de fase al perder vida). */
USTRUCT(BlueprintType)
struct FSAEVUSFaseEnemigo
{
	GENERATED_BODY()

	/** La fase empieza cuando la vida baja a esta fracción del máximo (1 = desde el principio). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fase", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionVida = 1.f;

	/** Multiplica la velocidad de movimiento y la de los ataques. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fase", meta = (ClampMin = "0.1"))
	float EscalaVelocidad = 1.f;

	/** Multiplica los tiempos de aviso y recuperación (menos de 1 = más agresivo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fase", meta = (ClampMin = "0.1"))
	float EscalaTiempos = 1.f;

	/** Índices en Ataques disponibles en esta fase. Vacío = todos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fase")
	TArray<int32> Ataques;
};

/**
 * Un tipo de enemigo: cuerpo, sentidos, contacto, ataques y fases. El constructor da el mini
 * enemigo por defecto (un rastrero que embiste); los Data Asset definen el resto.
 */
USTRUCT(BlueprintType)
struct SAEVUS_API FSAEVUSAjusteEnemigo
{
	GENERATED_BODY()

	FSAEVUSAjusteEnemigo();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo")
	FText NombreVisible;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo", meta = (ClampMin = "1"))
	float VidaMaxima = 30.f;

	/** Los jefes muestran barra de vida y no se aturden. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo")
	bool bEsJefe = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cuerpo", meta = (ClampMin = "10", ForceUnits = "cm"))
	float RadioCapsula = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cuerpo", meta = (ClampMin = "10", ForceUnits = "cm"))
	float MediaAlturaCapsula = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cuerpo")
	FLinearColor ColorCuerpo = FLinearColor(0.05f, 0.03f, 0.03f);

	/** Color de la máscara: brilla un poco, y mucho al avisar de un ataque. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cuerpo")
	FLinearColor ColorMascara = FLinearColor(0.9f, 0.45f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadMovimiento = 300.f;

	/**
	 * Vuela (Hollow Knight): sin gravedad, persigue a AlturaVuelo por encima del objetivo y patrulla a
	 * su altura de origen. Un golpe lo derriba un poco (cae mientras está aturdido) y luego vuelve a volar.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vuelo")
	bool bVolador = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vuelo", meta = (ClampMin = "0", ForceUnits = "cm", EditCondition = "bVolador"))
	float AlturaVuelo = 220.f;

	/** Frenada en el aire cuando deja de moverse (al avisar, al recuperarse). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vuelo", meta = (ClampMin = "0", ForceUnits = "cm/s^2", EditCondition = "bVolador"))
	float FrenadaVuelo = 2500.f;

	/** Balanceo vertical del cuerpo al volar (solo aspecto): amplitud y frecuencia. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vuelo", meta = (ClampMin = "0", ForceUnits = "cm", EditCondition = "bVolador"))
	float AmplitudAleteo = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vuelo", meta = (ClampMin = "0", ForceUnits = "Hz", EditCondition = "bVolador"))
	float FrecuenciaAleteo = 3.f;

	/** Ve al objetivo a esta distancia, y lo pierde más allá de RadioPerdida. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sentidos", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioDeteccion = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sentidos", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioPerdida = 1800.f;

	/**
	 * Enemigos a distancia: sin ataque posible y con el objetivo más cerca que esto, se alejan en
	 * vez de acercarse. 0 = nunca retroceden.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaRetirada = 0.f;

	/** Patrulla de ida y vuelta alrededor del origen. 0 = quieto. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioPatrulla = 250.f;

	/** Daño por tocar al enemigo (Hollow Knight: el contacto hace daño). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contacto", meta = (ClampMin = "0"))
	float DanioContacto = 10.f;

	/** Holgura añadida a la suma de radios para contar contacto. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Contacto", meta = (ClampMin = "0", ForceUnits = "cm"))
	float MargenContacto = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoAturdido = 0.35f;

	/** Multiplica los empujes que recibe (0 = inamovible). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0"))
	float EscalaEmpujeRecibido = 1.f;

	/** Tiempo entre morir y desaparecer (se encoge y suelta partículas). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Muerte", meta = (ClampMin = "0", ForceUnits = "s"))
	float DuracionMuerte = 0.6f;

	/**
	 * Rendimiento: en reposo y a más de RadioDeteccion × este factor del objetivo, el enemigo se
	 * queda quieto y su IA se actualiza cada IntervaloReposoLejos (nadie lo ve patrullar).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "1"))
	float FactorDistanciaReposoLejos = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rendimiento", meta = (ClampMin = "0", ForceUnits = "s"))
	float IntervaloReposoLejos = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataques")
	TArray<FSAEVUSAtaqueEnemigo> Ataques;

	/** Fases ordenadas de mayor a menor FraccionVida. Vacío = una sola fase sin escalas. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fases")
	TArray<FSAEVUSFaseEnemigo> Fases;
};

/** Tipo de enemigo editable, asignado a cada ASAEVUSEnemigo. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSDatosEnemigo : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemigo")
	FSAEVUSAjusteEnemigo Enemigo;

	/** Material de personaje dibujado; cada pieza usa una instancia dinámica con su color. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aspecto")
	TObjectPtr<UMaterialInterface> MaterialBase;

	/**
	 * Cuerpo boceto generado en Blender (Herramientas/Arte/blender_enemigos.py): ranura 0 = cuerpo,
	 * ranura 1 = máscara. Sin él, el enemigo se arma con formas del motor.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aspecto")
	TObjectPtr<UStaticMesh> MallaCuerpo;

	/** Material de las partículas de muerte. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aspecto")
	TObjectPtr<UMaterialInterface> MaterialParticulas;
};
