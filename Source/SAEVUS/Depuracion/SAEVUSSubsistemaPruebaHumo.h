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
		Disparar,
		Reaparecer,
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
	float DuracionAtaque = 0.f;
	bool bAtaqueEmpezo = false;
	bool bGatilloPulsado = false;
	int32 DisparosRealizados = 0;
	float VidaTrasGolpe = 0.f;
	bool bMurio = false;
	bool bReaparecio = false;
	float VidaAlReaparecer = 0.f;
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
