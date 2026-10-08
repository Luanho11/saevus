// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SAEVUSGuardado.generated.h"

/** Celdas exploradas de un mapa (SAEVUSExploracion). */
USTRUCT()
struct FSAEVUSExploracionMapa
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FIntPoint> Celdas;
};

/**
 * Lo que se guarda entre sesiones: opciones que no guarda el motor (la calidad ya va en
 * GameUserSettings.ini), el punto de control de cada mapa y en qué mapa fue el último (para "Continuar").
 */
UCLASS()
class SAEVUS_API USAEVUSPartidaGuardada : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float Sensibilidad = 1.f;

	/** Volumen general (0–1). */
	UPROPERTY()
	float Volumen = 1.f;

	UPROPERTY()
	bool bTienePuntoControl = false;

	/** Mapa del último punto de control (nombre corto, sin prefijo de PIE): "Continuar" vuelve a él. */
	UPROPERTY()
	FName MapaPuntoControl;

	/** Punto de control del mapa MapaPuntoControl (partidas anteriores a PuntosControl). */
	UPROPERTY()
	FTransform PuntoControl;

	/** Punto de control de cada mapa (nombre corto). */
	UPROPERTY()
	TMap<FName, FTransform> PuntosControl;

	/** Fragmentos de máscara recogidos (por identificador). */
	UPROPERTY()
	TArray<FName> FragmentosMascara;

	/** Celdas exploradas por mapa (nombre corto). */
	UPROPERTY()
	TMap<FName, FSAEVUSExploracionMapa> Exploracion;
};

namespace SAEVUSGuardado
{
	/** Ranura del juego; los tests usan otra. */
	SAEVUS_API extern const TCHAR* const RanuraPorDefecto;

	/** La partida de Ranura, o una nueva con los valores por defecto si no existe o no se puede leer. */
	SAEVUS_API USAEVUSPartidaGuardada* Cargar(const FString& Ranura);
	SAEVUS_API bool Guardar(USAEVUSPartidaGuardada* Partida, const FString& Ranura);

	/** Nombre corto del mapa de Mundo, igual en PIE y en el juego. */
	SAEVUS_API FName NombreMapa(const UWorld* Mundo);

	/** Volumen general del dispositivo de audio del mundo (0–1). */
	SAEVUS_API void AplicarVolumen(const UWorld* Mundo, float Volumen);
}

/**
 * Partida guardada durante la sesión: se carga al arrancar y se escribe al cambiar algo. Con
 * -SAEVUSSinGuardar (pruebas de humo, recorrido) no escribe, así no pisa la partida del jugador.
 */
UCLASS()
class SAEVUS_API USAEVUSSubsistemaGuardado : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Coleccion) override;
	/** Al cerrar el juego se escribe en el acto (una escritura en otro hilo podría no acabar). */
	virtual void Deinitialize() override;

	/** El subsistema de la instancia de juego de Contexto; nulo sin instancia (mundos de prueba). */
	static USAEVUSSubsistemaGuardado* Obtener(const UObject* Contexto);

	float ObtenerSensibilidad() const { return Partida->Sensibilidad; }
	void GuardarSensibilidad(float Sensibilidad);

	float ObtenerVolumen() const { return Partida->Volumen; }
	void GuardarVolumen(float Volumen);

	int32 ObtenerNumeroFragmentos() const { return Partida->FragmentosMascara.Num(); }
	bool FragmentoRecogido(FName Identificador) const { return Partida->FragmentosMascara.Contains(Identificador); }
	void GuardarFragmento(FName Identificador);

	/** Celdas exploradas del mapa (vacío si nunca se exploró). */
	TArray<FIntPoint> ObtenerExploracion(FName Mapa) const;
	void GuardarExploracion(FName Mapa, const TArray<FIntPoint>& Celdas);

	void GuardarPuntoControl(const UWorld* Mundo, const FTransform& Punto);
	/** Cierto si hay un punto de control guardado para el mapa de Mundo. */
	bool ObtenerPuntoControl(const UWorld* Mundo, FTransform& OutPunto) const;
	/** Mapa del último punto de control guardado (NAME_None si nunca hubo): adonde lleva "Continuar". */
	FName ObtenerMapaUltimoPuntoControl() const { return Partida->bTienePuntoControl ? Partida->MapaPuntoControl : NAME_None; }

private:
	void Guardar();

	UPROPERTY()
	TObjectPtr<USAEVUSPartidaGuardada> Partida;

	bool bGuardarActivo = true;
};
