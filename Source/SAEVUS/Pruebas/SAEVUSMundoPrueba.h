// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Efectos/SAEVUSComponenteEstelaDash.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Personaje/SAEVUSPersonaje.h"

/** Fixture compartido por los tests que simulan un ASAEVUSPersonaje real. */
namespace SAEVUSPruebas
{
	constexpr EAutomationTestFlags BanderasPrueba = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;

	constexpr float PasoSimulacion = 1.f / 60.f;

	// En el editor, CMC se niega a empezar a caer durante el primer segundo de tiempo de mundo
	// (UCharacterMovementComponent::StartFalling), así que los mundos de prueba empiezan después.
	constexpr double TiempoInicioMundo = 10.0;

	// Cubo del motor: 100 cm, pivote en el centro.
	const TCHAR* const RutaMallaCubo = TEXT("/Engine/BasicShapes/Cube.Cube");
	constexpr float TamanoCubo = 100.f;

	inline int32 PasosPara(float Segundos)
	{
		return FMath::CeilToInt32(Segundos / PasoSimulacion);
	}

	/** Mundo de juego desechable (sin mapa, sin modo de juego, sin controlador) para simular un personaje. */
	class FMundoPrueba
	{
	public:
		FMundoPrueba()
		{
			FWorldContext& Contexto = GEngine->CreateNewWorldContext(EWorldType::Game);
			// Nombre único: puede que un mundo de prueba anterior aún no haya pasado por el recolector.
			const FName NombreMundo = MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("SAEVUSMundoPrueba"));
			Mundo = UWorld::CreateWorld(EWorldType::Game, false, NombreMundo, GetTransientPackage());
			Contexto.SetCurrentWorld(Mundo);
			Mundo->InitializeActorsForPlay(FURL());
			Mundo->BeginPlay();
			// Sin modo de juego nadie despacha BeginPlay, así los actores creados no empezarían a jugar.
			if (!Mundo->GetBegunPlay())
			{
				Mundo->GetWorldSettings()->NotifyBeginPlay();
			}
			Mundo->TimeSeconds = Mundo->UnpausedTimeSeconds = Mundo->RealTimeSeconds = TiempoInicioMundo;
		}

		~FMundoPrueba()
		{
			for (AActor* Actor : FActorRange(Mundo))
			{
				Actor->RouteEndPlay(EEndPlayReason::LevelTransition);
			}
			GEngine->ShutdownWorldNetDriver(Mundo);
			Mundo->DestroyWorld(true);
			Mundo->SetPhysicsScene(nullptr);
			GEngine->DestroyWorldContext(Mundo);
			Mundo->RemoveFromRoot();
		}

		FMundoPrueba(const FMundoPrueba&) = delete;
		FMundoPrueba& operator=(const FMundoPrueba&) = delete;

		UWorld* ObtenerMundo() const { return Mundo; }

		/** Caja sólida del tamaño dado (cm), centrada en Centro. */
		AStaticMeshActor* CrearBloque(const FVector& Centro, const FVector& Tamano)
		{
			AStaticMeshActor* Bloque = Mundo->SpawnActor<AStaticMeshActor>(Centro, FRotator::ZeroRotator);
			UStaticMeshComponent* Malla = Bloque->GetStaticMeshComponent();
			// La movilidad estática rechaza cambios de malla después de BeginPlay.
			Malla->SetMobility(EComponentMobility::Movable);
			Malla->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, RutaMallaCubo));
			Bloque->SetActorScale3D(Tamano / TamanoCubo);
			return Bloque;
		}

		/** Crea cualquier actor (blancos, objetos) en Posicion. */
		template <typename TActor>
		TActor* CrearActor(const FVector& Posicion, const FRotator& Rotacion = FRotator::ZeroRotator)
		{
			FActorSpawnParameters Parametros;
			Parametros.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			return Mundo->SpawnActor<TActor>(Posicion, Rotacion, Parametros);
		}

		/**
		 * Crea un personaje sin controlador que aun así simula, cayendo desde Posicion.
		 * Usa AsentarEnSuelo() cuando debe empezar de pie.
		 */
		ASAEVUSPersonaje* CrearPersonaje(const FVector& Posicion, const USAEVUSAjustesMovimiento* Ajustes = nullptr)
		{
			FActorSpawnParameters Parametros;
			Parametros.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ASAEVUSPersonaje* Personaje = Mundo->SpawnActor<ASAEVUSPersonaje>(Posicion, FRotator::ZeroRotator, Parametros);

			USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
			Movimiento->bRunPhysicsWithNoController = true;
			Movimiento->FijarAjustesMovimiento(Ajustes);
			// El motor solo elige un modo por defecto al aparecer si la bandera de arriba ya estaba activa.
			Movimiento->SetMovementMode(MOVE_Falling);
			Movimiento->Velocity = FVector::ZeroVector;
			MovimientoSimulado = Movimiento;
			CombateSimulado = Personaje->ObtenerCombate();
			ArmasSimuladas = Personaje->ObtenerArmas();
			EstelaSimulada = Personaje->ObtenerEstelaDash();
			AnimacionSimulada = Personaje->ObtenerAnimacion();
			return Personaje;
		}

		/** Avanza hasta que el personaje pisa algo; falso si no aterriza en MaxSegundos. */
		bool AsentarEnSuelo(float MaxSegundos = 2.f)
		{
			for (int32 Indice = 0; Indice < PasosPara(MaxSegundos); ++Indice)
			{
				if (MovimientoSimulado->IsMovingOnGround())
				{
					return true;
				}
				Paso();
			}
			return MovimientoSimulado->IsMovingOnGround();
		}

		/**
		 * Avanza el personaje un paso fijo: armas y combate, luego movimiento, como los ordenan los
		 * prerrequisitos de tick en el juego. Los componentes se actualizan a mano: un mundo sin modo
		 * de juego ni viewport no ejecuta sus grupos de tick de forma fiable, y así es determinista.
		 */
		void Paso()
		{
			check(MovimientoSimulado);
			Mundo->TimeSeconds += PasoSimulacion;
			Mundo->UnpausedTimeSeconds += PasoSimulacion;
			Mundo->RealTimeSeconds += PasoSimulacion;
			if (ArmasSimuladas)
			{
				ArmasSimuladas->TickComponent(PasoSimulacion, LEVELTICK_All, &ArmasSimuladas->PrimaryComponentTick);
			}
			if (CombateSimulado)
			{
				CombateSimulado->TickComponent(PasoSimulacion, LEVELTICK_All, &CombateSimulado->PrimaryComponentTick);
			}
			MovimientoSimulado->TickComponent(PasoSimulacion, LEVELTICK_All, &MovimientoSimulado->PrimaryComponentTick);
			if (EstelaSimulada)
			{
				EstelaSimulada->TickComponent(PasoSimulacion, LEVELTICK_All, &EstelaSimulada->PrimaryComponentTick);
			}
			if (AnimacionSimulada)
			{
				AnimacionSimulada->Actualizar(PasoSimulacion);
			}
		}

	private:
		UWorld* Mundo = nullptr;
		USAEVUSComponenteMovimiento* MovimientoSimulado = nullptr;
		USAEVUSComponenteCombate* CombateSimulado = nullptr;
		USAEVUSComponenteArmas* ArmasSimuladas = nullptr;
		USAEVUSComponenteEstelaDash* EstelaSimulada = nullptr;
		USAEVUSComponenteAnimacionProcedural* AnimacionSimulada = nullptr;
	};
}

#endif // WITH_DEV_AUTOMATION_TESTS
