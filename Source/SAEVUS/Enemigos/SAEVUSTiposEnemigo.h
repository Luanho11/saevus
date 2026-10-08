// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
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

/**
 * Animación de un ataque de enemigo: cómo se prepara (el aviso que lee el jugador), cómo golpea y cómo se
 * queda. Automatico la saca del tipo de ataque; los demás dan a cada ataque su propio gesto.
 */
UENUM(BlueprintType)
enum class ESAEVUSGestoEnemigo : uint8
{
	Automatico,
	/** Arma alzada atrás por encima del hombro y tajo cruzado hacia delante y abajo. */
	Tajo,
	/** A dos manos por encima de la cabeza y hasta el suelo, con todo el cuerpo. */
	Mazazo,
	/** Revés con la mano libre abierta, girando el tronco. */
	Manotazo,
	/** El arma recogida atrás y lanzada recta al frente. */
	Estocada,
	/** Brazo atrás y lanzamiento por encima del hombro, con paso. */
	Lanzamiento,
	/** Agachado y echado hacia delante, el arma por delante: carrera. */
	Embestida,
	/** Encogido para saltar, recogido en el aire con el arma arriba, golpe al caer. */
	Salto
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

	/** Su animación (Automatico: según el tipo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ataque")
	ESAEVUSGestoEnemigo Gesto = ESAEVUSGestoEnemigo::Automatico;

	/** El gesto que se ve: el elegido o, en automático, el del tipo de ataque. */
	ESAEVUSGestoEnemigo ObtenerGesto() const
	{
		if (Gesto != ESAEVUSGestoEnemigo::Automatico)
		{
			return Gesto;
		}
		switch (Tipo)
		{
		case ESAEVUSTipoAtaqueEnemigo::Embestida:
			return ESAEVUSGestoEnemigo::Embestida;
		case ESAEVUSTipoAtaqueEnemigo::Salto:
			return ESAEVUSGestoEnemigo::Salto;
		case ESAEVUSTipoAtaqueEnemigo::Proyectil:
			return ESAEVUSGestoEnemigo::Lanzamiento;
		default:
			return ESAEVUSGestoEnemigo::Tajo;
		}
	}

	/** Solo se elige con el objetivo entre estas distancias (horizontales, de centro a centro). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elección", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaMinima = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elección", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaMaxima = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoTelegrafiar = 0.5f;

	/**
	 * Golpe y Embestida: diferencia de altura máxima con el objetivo para elegir este ataque. Con el jugador en una
	 * cornisa por encima, el enemigo no golpea al aire (auditoría EA-1).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Elección", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AlcanceVertical = 220.f;

	/** Embestida: lo que dura. Salto: tiempo máximo en el aire. Golpe: tiempo hasta pasar a recuperarse. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float Duracion = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoRecuperacion = 0.7f;

	/**
	 * Golpe y Proyectil: cuánto después de empezar el ataque llega el golpe (o sale el proyectil), para que el
	 * daño coincida con el barrido que se ve y no con el brazo todavía levantado (auditoría ANIMVFX-2).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tiempos", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoImpacto = 0.15f;

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

	/**
	 * Cuánto gira el cuerpo hacia el objetivo, en grados por segundo (0 = al instante). Con un giro limitado, al
	 * elegir ataque no se da la vuelta en un fotograma y se puede rodear al que avisa (auditoría EA-17).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo", meta = (ClampMin = "0", ForceUnits = "deg/s"))
	float VelocidadGiro = 540.f;

	/** El aviso solo sigue al objetivo durante esta fracción de su duración; el resto, el ataque va a donde apuntaba. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemigo", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionSeguimientoAviso = 0.7f;

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

	/**
	 * Diferencia de altura máxima con el objetivo para detectarlo (cm). Un enemigo no ve a quien está en una cornisa
	 * tres metros más arriba ni en el piso de abajo (auditoría EA-1). Los voladores suman su altura de vuelo.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm"))
	float AlturaMaximaDeteccion = 300.f;

	/** Solo detecta al objetivo si no hay una pared entre los dos. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento")
	bool bExigirLineaDeVision = true;

	/** Sin ficha de ataque, el enemigo rodea al objetivo a esta distancia o más (retrocede si está más cerca). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaEspera = 250.f;

	/** Patrulla de ida y vuelta alrededor del origen. 0 = quieto. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movimiento", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioPatrulla = 250.f;

	/**
	 * Escudo: bloquea los golpes que llegan de frente dentro de este ángulo total (0 = sin escudo).
	 * Bloquear no hace daño, no da alma y empuja al atacante; se rodea, se salta por encima (pogo)
	 * o se rompe con un golpe fuerte.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escudo", meta = (ClampMin = "0", ClampMax = "360", ForceUnits = "deg"))
	float AnguloEscudo = 0.f;

	/** Un golpe de frente con al menos este daño rompe el escudo (el giro cargado). 0 = no se rompe. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escudo", meta = (ClampMin = "0"))
	float DanioRompeEscudo = 28.f;

	/** Tiempo que el escudo queda roto (no bloquea). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escudo", meta = (ClampMin = "0", ForceUnits = "s"))
	float TiempoEscudoRoto = 2.5f;

	/** Velocidad con la que el escudo empuja al atacante al bloquear. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Escudo", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float EmpujeEscudo = 500.f;

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
	float DuracionMuerte = 1.5f;

	/**
	 * Peso de su animación: 1 = un goblin (nervioso, rápido); más es más lento y pesado (el Rey Goblin): los
	 * muelles van más despacio, los pasos retumban y la respiración es más honda.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animacion", meta = (ClampMin = "0.3", ClampMax = "4"))
	float PesoAnimacion = 1.f;

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

namespace SAEVUSEnemigo
{
	/** Aviso mínimo legible de un ataque (s): por debajo, el jugador no puede reaccionar. */
	constexpr float AvisoMinimoLegible = 0.25f;

	/**
	 * Revisa un tipo de enemigo y devuelve lo que está mal (vacío = bien): fases desordenadas, índices de ataque
	 * fuera de rango, distancias invertidas, avisos ilegibles, picados sin vuelo (auditoría EA-25).
	 */
	SAEVUS_API TArray<FString> Validar(const FSAEVUSAjusteEnemigo& Ajuste);
}

/** Tipo de enemigo editable, asignado a cada ASAEVUSEnemigo. */
UCLASS(BlueprintType, Const)
class SAEVUS_API USAEVUSDatosEnemigo : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Contexto) const override;
#endif

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

	/**
	 * Cuerpo con esqueleto (los 25 huesos del protagonista; Herramientas/Arte/blender_goblins.py): lo anima
	 * USAEVUSComponenteAnimacionProcedural desde el estado de la IA. Manda sobre MallaCuerpo. Origen en los pies,
	 * mirando a +Y, a tamaño real (la cápsula debe medir lo mismo).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aspecto")
	TObjectPtr<USkeletalMesh> MallaEsqueletica;

	/**
	 * Color de cada ranura de la malla esquelética, por nombre de material de Blender (las que falten usan
	 * ColorCuerpo). La ranura que empieza por "Arma" es la que brilla al avisar (sin color aquí, ColorMascara).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aspecto")
	TMap<FName, FLinearColor> ColoresPiezas;

	/**
	 * Pintura de cada ranura de la malla esquelética (parámetro "Textura" de MaterialBase), por nombre de material de
	 * Blender: la del cuerpo reconstruido desde las vistas de la referencia (ranura "Referencia"; Documentos/pipeline-assets.md).
	 * Una ranura con textura y sin color en ColoresPiezas va en blanco, para que se vea la pintura tal cual.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aspecto")
	TMap<FName, TObjectPtr<UTexture>> TexturasPiezas;

	/** Material de las partículas de muerte. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aspecto")
	TObjectPtr<UMaterialInterface> MaterialParticulas;
};
