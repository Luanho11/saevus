// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSSubsistemaPruebaHumo.generated.h"

class ASAEVUSPersonaje;

/**
 * Prueba de humo en el juego, sin intervención, activa solo con el parámetro -SAEVUSPruebaHumo
 * (nunca en Shipping). Lleva al personaje del jugador por correr, esprint, salto, dash, ataque,
 * disparo, daño, muerte y reaparición; compara los resultados con sus ajustes; hace una captura con
 * el HUD de depuración; escribe "[SAEVUS humo] SUPERADA/FALLIDA" y una línea de tiempos por fotograma; y sale.
 *
 * Uso: UnrealEditor.exe SAEVUS.uproject -game -windowed -ResX=1280 -ResY=720 -SAEVUSPruebaHumo
 */
UCLASS()
class SAEVUS_API USAEVUSSubsistemaPruebaHumo : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Inicio de la interfaz de USubsystem
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ Fin de la interfaz de USubsystem

	//~ Inicio de la interfaz de FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	/** Sigue avanzando con el juego en pausa (comprueba el menú de pausa). */
	virtual bool IsTickableWhenPaused() const override { return true; }
	//~ Fin de la interfaz de FTickableGameObject

private:
	enum class EEtapa : uint8
	{
		EsperarPawn,
		Asentar,
		Correr,
		Esprint,
		Saltar,
		Dash,
		Atacar,
		Guardia,
		Pausa,
		MenuPrincipal,
		Disparar,
		Morir,
		Reaparecer,
		Enemigos,
		Captura,
		Informe,
		Hecho
	};

	void EntrarEtapa(EEtapa NuevaEtapa);
	void Comprobar(const TCHAR* Descripcion, float Medido, float Esperado, float ToleranciaRelativa);
	void Terminar();

	TWeakObjectPtr<ASAEVUSPersonaje> Personaje;
	EEtapa Etapa = EEtapa::EsperarPawn;
	float TiempoEtapa = 0.f;

	float VelocidadMaxCorrer = 0.f;
	float VelocidadMaxEsprint = 0.f;
	float ZInicioSalto = 0.f;
	float ZCimaSalto = 0.f;
	bool bDejoSuelo = false;
	float VelocidadMaxDash = 0.f;
	int32 FotogramaEtapa = 0;
	bool bDashEmpezo = false;
	bool bCapturaDashHecha = false;
	int32 ParticulasDash = 0;
	bool bCapturaCorrerHecha = false;
	bool bCapturaCalentamientoHecha = false;
	bool bCapturaAtaqueHecha = false;
	/** Radio de los límites del cuerpo animado al asentarse; si colapsa, el caballero no se ve. */
	float RadioCuerpo = 0.f;
	float DuracionAtaque = 0.f;
	bool bGuardiaActiva = false;
	bool bMenuPausaVisto = false;
	bool bMenuConFoco = false;
	bool bMenuPrincipalVisto = false;
	bool bMenuPrincipalCerrado = false;
	bool bCapturaMuerteHecha = false;
	bool bAtaqueEmpezo = false;
	bool bGatilloPulsado = false;
	/** El arsenal tiene arma de fuego (ahora solo la espada): si no, no se comprueba el disparo. */
	bool bHayArmaFuego = false;
	int32 DisparosRealizados = 0;
	float VidaTrasGolpe = 0.f;
	/** Vida tras un golpe letal durante la invulnerabilidad que sigue al primero (debe ignorarse). */
	float VidaTrasGolpeInvulnerable = 0.f;
	bool bMurio = false;
	bool bReaparecio = false;
	float VidaAlReaparecer = 0.f;
	/** Enemigos del mapa que pasaron a perseguir (o más) al acercar al jugador; -1 si el mapa no tiene. */
	int32 EnemigosActivos = -1;
	bool bPerfilGPUHecho = false;

	/** Tiempos por fotograma acumulados solo en las etapas de acción (se excluyen los tirones de aparición y asentado). */
	void MuestrearTiempos(float DeltaTime);

	double SumaTiemposFotograma = 0.0;
	double SumaMsHiloJuego = 0.0;
	double SumaMsHiloRender = 0.0;
	double SumaMsGPU = 0.0;
	float PeorFotograma = 0.f;
	int32 Fotogramas = 0;
	int32 Fallos = 0;
};
