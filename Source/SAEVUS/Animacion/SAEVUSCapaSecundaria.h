// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Animacion/SAEVUSMuelle.h"
#include "CoreMinimal.h"
#include "SAEVUSCapaSecundaria.generated.h"

/**
 * Ajustes del movimiento secundario: lo que hace que el cuerpo no gire ni se pare como una sola pieza.
 * Frecuencias en Hz y amortiguaciones adimensionales (ver TSAEVUSMuelle); ángulos en grados, distancias en cm.
 */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteSecundario
{
	GENERATED_BODY()

	// --- Giro por capas: la cabeza se adelanta, el pecho sigue y la cadera llega la última. ---

	/** Rapidez con la que la cadera alcanza el giro del cuerpo (más alto = se queda menos atrás). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0.1", ForceUnits = "Hz"))
	float FrecuenciaCadera = 2.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0", ClampMax = "2"))
	float AmortiguacionCadera = 0.6f;

	/** Lo más que la cadera se queda atrás del giro. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0", ClampMax = "90", ForceUnits = "deg"))
	float MaxRetrasoCadera = 55.f;

	/** Parte del retraso de la cadera que también lleva el pecho (0 = el pecho gira con la cámara, 1 = con la cadera). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionRetrasoPecho = 0.35f;

	/** La cabeza se adelanta al giro: grados por cada grado/s de velocidad de giro. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0"))
	float AdelantoCabeza = 0.07f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0", ClampMax = "60", ForceUnits = "deg"))
	float MaxAdelantoCabeza = 22.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0.1", ForceUnits = "Hz"))
	float FrecuenciaCabeza = 3.5f;

	/** En una acción (ataque, dash) el cuerpo tiene que ir con la cámara: el retraso se queda en esta fracción. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionRetrasoEnAccion = 0.25f;

	/** Quieto, si la cadera se queda atrás más de esto, los pies dan un paso para alcanzarla. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "1", ForceUnits = "deg"))
	float UmbralPasoGiro = 26.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float DuracionPasoGiro = 0.3f;

	/** Mientras da el paso, la cadera alcanza el giro con esta frecuencia (más rápida). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0.1", ForceUnits = "Hz"))
	float FrecuenciaPasoGiro = 4.5f;

	/** Corriendo, el cuerpo se inclina hacia dentro de la curva: grados por cada grado/s de giro a la velocidad de referencia. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0"))
	float LadeoCurva = 0.035f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0", ClampMax = "45", ForceUnits = "deg"))
	float MaxLadeoCurva = 14.f;

	/** Corriendo de lado (GunZ), el cuerpo se inclina hacia donde va. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Giro por capas", meta = (ClampMin = "0", ClampMax = "30", ForceUnits = "deg"))
	float LadeoLateral = 7.f;

	// --- Inercia: el tronco sigue cuando las piernas frenan y se queda atrás cuando arrancan. ---

	/**
	 * Empujón al tronco por cada cm/s que cambia la velocidad del cuerpo (grados/s por cm/s): como un péndulo, el
	 * tronco recibe el cambio al revés. No depende de los fps ni de si el cambio fue de golpe o en varios fotogramas.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inercia", meta = (ClampMin = "0"))
	float EscalaInercia = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inercia", meta = (ClampMin = "0", ClampMax = "45", ForceUnits = "deg"))
	float MaxInercia = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inercia", meta = (ClampMin = "0.1", ForceUnits = "Hz"))
	float FrecuenciaInercia = 2.4f;

	/** Baja: el tronco se pasa y vuelve (un balanceo al frenar). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inercia", meta = (ClampMin = "0", ClampMax = "2"))
	float AmortiguacionInercia = 0.42f;

	/** La cabeza compensa esta parte del balanceo del tronco: la mirada se queda estable. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inercia", meta = (ClampMin = "0", ClampMax = "1"))
	float EstabilizacionCabeza = 0.7f;

	// --- Rebote: la cadera cede y rebota al aterrizar o al recibir un golpe. ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rebote", meta = (ClampMin = "0.1", ForceUnits = "Hz"))
	float FrecuenciaRebote = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rebote", meta = (ClampMin = "0", ClampMax = "2"))
	float AmortiguacionRebote = 0.38f;

	/** Velocidad hacia abajo (cm/s) que da a la cadera un aterrizaje de fuerza 1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rebote", meta = (ClampMin = "0"))
	float ImpulsoAterrizaje = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rebote", meta = (ClampMin = "0", ForceUnits = "cm"))
	float MaxRebote = 22.f;

	// --- Respiración: lenta en calma, rápida y honda tras el esfuerzo. ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respiracion", meta = (ClampMin = "0", ForceUnits = "Hz"))
	float FrecuenciaRespiracionCalma = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respiracion", meta = (ClampMin = "0", ForceUnits = "Hz"))
	float FrecuenciaRespiracionAgitada = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respiracion", meta = (ClampMin = "0", ForceUnits = "deg"))
	float AmplitudRespiracionCalma = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respiracion", meta = (ClampMin = "0", ForceUnits = "deg"))
	float AmplitudRespiracionAgitada = 4.f;

	/** Cansancio ganado por segundo de esfuerzo (esprint, ataques, dash) y perdido por segundo de calma (0–1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respiracion", meta = (ClampMin = "0"))
	float SubidaCansancio = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respiracion", meta = (ClampMin = "0"))
	float BajadaCansancio = 0.12f;

	// --- Reposo vivo: el peso pasa de un pie a otro y la mirada vaga. ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0", ForceUnits = "Hz"))
	float FrecuenciaVaivenPeso = 0.11f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DesplazamientoPeso = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0", ForceUnits = "deg"))
	float LadeoPeso = 2.5f;

	/** La rodilla sin peso se dobla estos grados. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0", ForceUnits = "deg"))
	float FlexionRodillaSinPeso = 8.f;

	/** Mirada ociosa (ruido suave): giro y cabeceo máximos y su ritmo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0", ForceUnits = "deg"))
	float GiroMiradaOciosa = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0", ForceUnits = "deg"))
	float CabeceoMiradaOciosa = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0", ForceUnits = "Hz"))
	float RitmoMiradaOciosa = 0.16f;

	/** Tras este tiempo quieto sale un gesto de reposo (mirar atrás, girar la hoja, estirar los hombros). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "1", ForceUnits = "s"))
	float TiempoGestoReposo = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0.2", ForceUnits = "s"))
	float DuracionGestoReposo = 2.2f;

	/** Variación aleatoria de la espera entre gestos (0,4 = entre un 60 % y un 140 % del tiempo base). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reposo vivo", meta = (ClampMin = "0", ClampMax = "0.8"))
	float VariacionEsperaGesto = 0.4f;
};

/** Gestos de reposo que salen tras un rato quieto, por turno. */
enum class ESAEVUSGestoReposo : uint8
{
	Ninguno,
	MirarAtras,
	GirarHoja,
	EstirarHombros
};

/** Lo que el cuerpo hace este fotograma, para la capa secundaria. */
struct FSAEVUSEntradaSecundaria
{
	float DeltaTime = 0.f;
	/** Giro del actor en el mundo (grados). */
	float Guinada = 0.f;
	/** Velocidad horizontal en el mundo (X, Y de UE). */
	FVector2D Velocidad = FVector2D::ZeroVector;
	/** Velocidad horizontal a la que la locomoción vale 1. */
	float VelocidadReferencia = 550.f;
	bool bEnSuelo = true;
	/** Atacando, en dash o en otra acción: el cuerpo va con la cámara. */
	bool bAccion = false;
	/** Esprint, ataque o dash: cansa. */
	bool bEsfuerzo = false;
	/** Quieto y libre (sin acción, ni sentado, ni curando): puede respirar a gusto y hacer gestos de reposo. */
	bool bReposoLibre = false;
	/** Escala de tiempo de los muelles: 1 = humano; más de 1 = más pesado y lento (un jefe). */
	float Peso = 1.f;
};

/** Resultado de la capa secundaria: desplazamientos que la animación suma a la pose. Ángulos en grados. */
struct FSAEVUSSalidaSecundaria
{
	// Giro (guiñada) de cada tramo respecto a la pose, positivo hacia la derecha.
	float GiroCadera = 0.f;
	float GiroColumna = 0.f;
	float GiroPecho = 0.f;
	float GiroCuello = 0.f;
	float GiroCabeza = 0.f;

	/** Balanceo del tronco por inercia: X = hacia delante, Y = hacia la derecha. */
	FVector2D InerciaTronco = FVector2D::ZeroVector;
	/** Lo que la cabeza compensa del balanceo (ya con el signo contrario). */
	FVector2D InerciaCabeza = FVector2D::ZeroVector;
	/** Ladeo de todo el cuerpo sobre los pies (curvas, carrera de lado, vaivén del peso), positivo a la derecha. */
	float Ladeo = 0.f;
	/** Bajada de la cadera por el rebote (cm, positiva hacia abajo). */
	float Rebote = 0.f;
	/** Respiración: grados de pecho (positivo = se hincha y sube) y subida de hombros. */
	float Respiracion = 0.f;
	/** Paso de giro en curso (0 = ninguno, si no fase 0–1) y hacia qué lado (+1 derecha). */
	float PasoGiro = 0.f;
	float LadoPasoGiro = 0.f;
	/** Vaivén del peso de -1 (pie izquierdo) a +1 (pie derecho), ya con su fuerza. */
	float VaivenPeso = 0.f;
	/** Mirada ociosa: giro y cabeceo de la cabeza. */
	FVector2D MiradaOciosa = FVector2D::ZeroVector;
	/** Gesto de reposo en curso y su avance (0–1). */
	ESAEVUSGestoReposo GestoReposo = ESAEVUSGestoReposo::Ninguno;
	float AvanceGestoReposo = 0.f;
	/** Cansancio actual (0–1). */
	float Cansancio = 0.f;
};

/**
 * Movimiento secundario del cuerpo, independiente del esqueleto: recibe giro y velocidad del actor y devuelve
 * desplazamientos con muelles (giro por capas, inercia del tronco, rebote, respiración, reposo vivo). La
 * animación procedural los suma a la pose de cada fotograma. Sirve igual al protagonista y a los enemigos.
 */
class SAEVUS_API FSAEVUSCapaSecundaria
{
public:
	/** Avanza un fotograma y deja el resultado en ObtenerSalida(). */
	void Actualizar(const FSAEVUSAjusteSecundario& Ajuste, const FSAEVUSEntradaSecundaria& Entrada);

	/** Aterrizaje: la cadera cede según la fuerza (1 = caída de referencia). */
	void Aterrizar(const FSAEVUSAjusteSecundario& Ajuste, float Fuerza);

	/**
	 * Empujón al tronco (golpe recibido, retroceso de un impacto, arranque de un dash): grados por segundo
	 * en los ejes del cuerpo (X = hacia delante, Y = hacia la derecha) y bajada de la cadera en cm/s.
	 */
	void Empujar(const FVector2D& GiroTronco, float Hundimiento);

	/** Olvida todo el movimiento (reaparición, cambio de malla). */
	void Reiniciar();

	const FSAEVUSSalidaSecundaria& ObtenerSalida() const { return Salida; }

private:
	FSAEVUSSalidaSecundaria Salida;

	FSAEVUSMuelle RetrasoCadera;
	FSAEVUSMuelle AdelantoCabeza;
	FSAEVUSMuelle LadeoCurva;
	FSAEVUSMuelleVector Inercia;
	FSAEVUSMuelle Rebote;

	bool bIniciada = false;
	float GuinadaAnterior = 0.f;
	FVector2D VelocidadAnterior = FVector2D::ZeroVector;
	float Tiempo = 0.f;
	float FaseRespiracion = 0.f;
	float Cansancio = 0.f;
	float TiempoPasoGiro = 0.f;
	float LadoPasoGiro = 0.f;
	float TiempoQuieto = 0.f;
	float TiempoGestoReposo = 0.f;
	ESAEVUSGestoReposo GestoReposo = ESAEVUSGestoReposo::Ninguno;
	ESAEVUSGestoReposo UltimoGestoReposo = ESAEVUSGestoReposo::Ninguno;
	/** Factor de la espera hasta el próximo gesto (se sortea tras cada gesto). */
	float FactorEsperaGesto = 1.f;
	FRandomStream Azar;
	/** Fase propia de cada cuerpo, para que dos enemigos iguales no respiren ni miren al unísono. */
	float Desfase = 0.f;

public:
	/** Fija el desfase del ruido y los ritmos (por ejemplo, con el identificador del actor). */
	void FijarDesfase(float NuevoDesfase)
	{
		Desfase = NuevoDesfase;
		Azar.Initialize(FMath::FloorToInt(NuevoDesfase * 1000.f));
	}
};
