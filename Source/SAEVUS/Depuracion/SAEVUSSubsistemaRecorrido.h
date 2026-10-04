// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSSubsistemaRecorrido.generated.h"

class ASAEVUSMunecoEntrenamiento;
class ASAEVUSPersonaje;
class ASAEVUSPuertaArena;
class ASAEVUSVolumenTutorial;

/**
 * Recorrido del tutorial en el juego real (`-SAEVUSRecorrido`, Herramientas/Ejecutar-Recorrido.ps1):
 * lleva al jugador (invulnerable) a cada aviso y comprueba que aparece, con una captura por
 * sección; dispara con la pistola a una diana alta (un muñeco elevado); después vacía cada arena
 * y comprueba que su puerta se abre y que el jefe se activa.
 * Escribe "[SAEVUS recorrido]" en el log y cierra el juego. No existe en Shipping.
 */
UCLASS()
class SAEVUS_API USAEVUSSubsistemaRecorrido : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//~ Inicio de la interfaz de FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ Fin de la interfaz de FTickableGameObject

private:
	enum class EPaso : uint8
	{
		EsperarJugador,
		Avisos,
		Diana,
		Arenas,
		Hecho
	};

	void Preparar(ASAEVUSPersonaje* Jugador);
	void ActualizarAvisos(ASAEVUSPersonaje* Jugador);
	void ActualizarDiana(ASAEVUSPersonaje* Jugador);
	void ActualizarArenas(ASAEVUSPersonaje* Jugador);
	void Comprobar(const FString& Descripcion, bool bBien);
	void Terminar();
	void Llevar(ASAEVUSPersonaje* Jugador, const FVector& Posicion);

	EPaso Paso = EPaso::EsperarJugador;
	float TiempoPaso = 0.f;
	float TiempoTotal = 0.f;
	int32 Indice = 0;
	bool bLlevado = false;
	bool bVaciada = false;
	bool bCapturaEnemigos = false;
	/** Diana: 0 colocar y equipar, 1 apuntar y disparar, 2 comprobar. */
	int32 SubpasoDiana = 0;
	TWeakObjectPtr<ASAEVUSMunecoEntrenamiento> Diana;
	TArray<TWeakObjectPtr<ASAEVUSVolumenTutorial>> Avisos;
	TArray<TWeakObjectPtr<ASAEVUSPuertaArena>> Puertas;
	int32 Fallos = 0;
};
