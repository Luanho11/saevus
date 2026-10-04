// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSSubsistemaRecorrido.generated.h"

class ASAEVUSBanco;
class ASAEVUSFragmentoMascara;
class ASAEVUSParedRompible;
class ASAEVUSMunecoEntrenamiento;
class ASAEVUSPalanca;
class ASAEVUSPersonaje;
class ASAEVUSPuertaArena;
class ASAEVUSVolumenTutorial;

/**
 * Recorrido del tutorial en el juego real (`-SAEVUSRecorrido`, Herramientas/Ejecutar-Recorrido.ps1):
 * lleva al jugador (invulnerable) a cada aviso y comprueba que aparece, con una captura por
 * sección; dispara con la pistola a una diana alta (un muñeco elevado, si el mapa la tiene);
 * después vacía cada arena y comprueba que su puerta se abre y que el jefe se activa; por último
 * golpea cada palanca y comprueba que abre sus compuertas, se sienta en cada banco, rompe cada
 * pared falsa a tajos y recoge cada fragmento de máscara. Sirve para cualquier mapa con avisos.
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
		Palancas,
		Bancos,
		Secretos,
		Hecho
	};

	void Preparar(ASAEVUSPersonaje* Jugador);
	void ActualizarAvisos(ASAEVUSPersonaje* Jugador);
	void ActualizarDiana(ASAEVUSPersonaje* Jugador);
	void ActualizarArenas(ASAEVUSPersonaje* Jugador);
	void ActualizarPalancas(ASAEVUSPersonaje* Jugador);
	void ActualizarBancos(ASAEVUSPersonaje* Jugador);
	void ActualizarSecretos(ASAEVUSPersonaje* Jugador);
	/** Suelo bajo Actor (la parte de abajo de sus límites) más AlturaLlevar: donde dejar al jugador. */
	static float AlturaJunto(const AActor* Actor);
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
	/** Puertas de arena (con enemigos); las compuertas sin enemigos son de las palancas. */
	TArray<TWeakObjectPtr<ASAEVUSPuertaArena>> Puertas;
	TArray<TWeakObjectPtr<ASAEVUSPalanca>> Palancas;
	TArray<TWeakObjectPtr<ASAEVUSBanco>> Bancos;
	TArray<TWeakObjectPtr<ASAEVUSParedRompible>> Paredes;
	TArray<TWeakObjectPtr<ASAEVUSFragmentoMascara>> Fragmentos;
	int32 TajosDados = 0;
	int32 FragmentosAntes = 0;
	int32 Fallos = 0;

	/** Rendimiento durante el recorrido (desde que aparece el jugador): fotogramas, tiempos y peor fotograma. */
	int32 Fotogramas = 0;
	double SumaSegundos = 0.0;
	float PeorSegundos = 0.f;
	double SumaMsJuego = 0.0;
	double SumaMsGPU = 0.0;
};
