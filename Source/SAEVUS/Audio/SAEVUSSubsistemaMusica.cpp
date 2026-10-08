// Copyright SAEVUS. Todos los derechos reservados.

#include "Audio/SAEVUSSubsistemaMusica.h"
#include "Async/Async.h"
#include "AudioDevice.h"
#include "Components/AudioComponent.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ScopeLock.h"
#include "Sound/SoundWaveProcedural.h"

namespace SAEVUSSubsistemaMusica
{
	constexpr int32 NumPistas = static_cast<int32>(ESAEVUSPistaMusica::Total);
	// Un bucle de margen en la cola: la onda nunca se queda sin muestras.
	constexpr int32 BuclesEnCola = 2;
	// Volumen con el que arrancan las pistas: no cero, para que el motor no las virtualice (EA-14).
	constexpr float VolumenInicial = 0.001f;

	// Las pistas son siempre las mismas: se generan una vez por proceso y los mundos siguientes las reutilizan,
	// sin silencio ni CPU repetida al cambiar de mapa (EA-13).
	FCriticalSection CerrojoCache;
	TArray<TSharedPtr<const TArray<uint8>>> CachePistas;

	bool EstaPeleando(ESAEVUSEstadoEnemigo Estado)
	{
		return Estado == ESAEVUSEstadoEnemigo::Perseguir || Estado == ESAEVUSEstadoEnemigo::Telegrafiar || Estado == ESAEVUSEstadoEnemigo::Atacar
			|| Estado == ESAEVUSEstadoEnemigo::Recuperarse || Estado == ESAEVUSEstadoEnemigo::Aturdido;
	}
}

bool USAEVUSSubsistemaMusica::ShouldCreateSubsystem(UObject* Externo) const
{
	const UWorld* Mundo = Cast<UWorld>(Externo);
	return Mundo && (Mundo->WorldType == EWorldType::Game || Mundo->WorldType == EWorldType::PIE) && Super::ShouldCreateSubsystem(Externo);
}

void USAEVUSSubsistemaMusica::OnWorldBeginPlay(UWorld& Mundo)
{
	using namespace SAEVUSSubsistemaMusica;
	Super::OnWorldBeginPlay(Mundo);
	// Sin dispositivo de audio (tests, -NoSound, servidor) no hay nada que generar.
	if (!Mundo.GetAudioDeviceRaw() || Mundo.GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	{
		FScopeLock Bloqueo(&CerrojoCache);
		if (CachePistas.Num() == NumPistas)
		{
			EmpezarASonar(CachePistas);
			return;
		}
	}
	bGenerando = true;
	Generacion = Async(EAsyncExecution::ThreadPool, []
	{
		TArray<TSharedPtr<const TArray<uint8>>> Pistas;
		for (int32 Pista = 0; Pista < NumPistas; ++Pista)
		{
			const TArray<int16> Muestras = SAEVUSMusica::GenerarPista(static_cast<ESAEVUSPistaMusica>(Pista));
			TArray<uint8> Bytes;
			Bytes.SetNumUninitialized(Muestras.Num() * sizeof(int16));
			FMemory::Memcpy(Bytes.GetData(), Muestras.GetData(), Bytes.Num());
			Pistas.Add(MakeShared<const TArray<uint8>>(MoveTemp(Bytes)));
		}
		FScopeLock Bloqueo(&CerrojoCache);
		CachePistas = Pistas;
		return Pistas;
	});
}

void USAEVUSSubsistemaMusica::Deinitialize()
{
	if (Generacion.IsValid())
	{
		// No dejar el hilo escribiendo en un subsistema que ya no existe.
		Generacion.Wait();
	}
	for (UAudioComponent* Componente : Componentes)
	{
		if (Componente)
		{
			Componente->Stop();
		}
	}
	// El delegado de las ondas no se desenlaza: Stop() es asíncrono y el hilo de audio puede estar dentro de la
	// lambda; esta solo captura el búfer compartido, así que es seguro dejar que muera con su onda (EA-14).
	Super::Deinitialize();
}

TStatId USAEVUSSubsistemaMusica::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USAEVUSSubsistemaMusica, STATGROUP_Tickables);
}

void USAEVUSSubsistemaMusica::EmpezarASonar(const TArray<TSharedPtr<const TArray<uint8>>>& Pistas)
{
	using namespace SAEVUSSubsistemaMusica;
	for (int32 Pista = 0; Pista < Pistas.Num(); ++Pista)
	{
		const TSharedPtr<const TArray<uint8>> Bucle = Pistas[Pista];
		Bucles.Add(Bucle);
		USoundWaveProcedural* Onda = NewObject<USoundWaveProcedural>(this);
		Onda->SetSampleRate(SAEVUSMusica::Muestreo);
		Onda->NumChannels = 1;
		Onda->Duration = INDEFINITELY_LOOPING_DURATION;
		Onda->bLooping = false;
		// Música: suena en los menús (con el juego en pausa) y no se atenúa por distancia.
		Onda->SoundGroup = SOUNDGROUP_Music;
		for (int32 Vuelta = 0; Vuelta < BuclesEnCola; ++Vuelta)
		{
			Onda->QueueAudio(Bucle->GetData(), Bucle->Num());
		}
		// El hilo de audio pide más cuando se le acaba: otra vuelta del bucle.
		Onda->OnSoundWaveProceduralUnderflow.BindLambda([Bucle](USoundWaveProcedural* Procedural, int32 MuestrasPedidas)
		{
			Procedural->QueueAudio(Bucle->GetData(), Bucle->Num());
		});
		Ondas.Add(Onda);
		UAudioComponent* Componente = UGameplayStatics::CreateSound2D(GetWorld(), Onda, VolumenInicial, 1.f, 0.f, nullptr, false, false);
		if (Componente)
		{
			Componente->bIsUISound = true;
			Componente->Play();
		}
		Componentes.Add(Componente);
	}
}

bool USAEVUSSubsistemaMusica::TieneLineaDeVision(const AActor* Enemigo, const AActor* Jugador) const
{
	FCollisionQueryParams Parametros(SCENE_QUERY_STAT(MusicaVision), false);
	Parametros.AddIgnoredActor(Enemigo);
	Parametros.AddIgnoredActor(Jugador);
	return !GetWorld()->LineTraceTestByChannel(Enemigo->GetActorLocation(), Jugador->GetActorLocation(), ECC_Visibility, Parametros);
}

ESAEVUSPistaMusica USAEVUSSubsistemaMusica::DecidirPista(float DeltaTime)
{
	using namespace SAEVUSSubsistemaMusica;
	const APawn* Jugador = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	bool bPelea = false;
	bool bJefe = false;
	if (Jugador)
	{
		const double RadioCuadrado = FMath::Square(static_cast<double>(Ajuste.RadioPelea));
		for (TActorIterator<ASAEVUSEnemigo> Enemigo(GetWorld()); Enemigo; ++Enemigo)
		{
			const ESAEVUSEstadoEnemigo Estado = Enemigo->ObtenerIA()->ObtenerEstado();
			if (!EstaPeleando(Estado) || FVector::DistSquared(Enemigo->GetActorLocation(), Jugador->GetActorLocation()) > RadioCuadrado)
			{
				continue;
			}
			// Uno que solo persigue al otro lado de un muro no es una pelea visible: no enciende la música (EA-10).
			if (Estado == ESAEVUSEstadoEnemigo::Perseguir && !TieneLineaDeVision(*Enemigo, Jugador))
			{
				continue;
			}
			bPelea = true;
			bJefe |= Enemigo->ObtenerAjuste().bEsJefe;
		}
	}
	TiempoDesdePelea = bPelea ? 0.f : TiempoDesdePelea + DeltaTime;
	if (bJefe)
	{
		PistaActual = ESAEVUSPistaMusica::Jefe;
	}
	else if (bPelea || (PistaActual != ESAEVUSPistaMusica::Exploracion && TiempoDesdePelea < Ajuste.EsperaFinPelea))
	{
		// Tras el jefe, si sigue habiendo pelea, la de siempre; si no, espera un poco antes de calmarse.
		PistaActual = bPelea || PistaActual == ESAEVUSPistaMusica::Combate ? ESAEVUSPistaMusica::Combate : PistaActual;
	}
	else
	{
		PistaActual = ESAEVUSPistaMusica::Exploracion;
	}
	return PistaActual;
}

void USAEVUSSubsistemaMusica::Tick(float DeltaTime)
{
	using namespace SAEVUSSubsistemaMusica;
	if (bGenerando && Generacion.IsReady())
	{
		const TArray<TSharedPtr<const TArray<uint8>>> Pistas = Generacion.Get();
		Generacion.Reset();
		bGenerando = false;
		EmpezarASonar(Pistas);
	}
	TiempoDecision -= DeltaTime;
	if (TiempoDecision <= 0.f)
	{
		DecidirPista(Ajuste.IntervaloDecision - TiempoDecision);
		TiempoDecision = Ajuste.IntervaloDecision;
	}
	if (Componentes.IsEmpty())
	{
		return;
	}
	// Fundidos: la pista que manda sube; la calma queda de fondo en la pelea; el resto baja.
	for (int32 Pista = 0; Pista < Componentes.Num(); ++Pista)
	{
		const ESAEVUSPistaMusica Esta = static_cast<ESAEVUSPistaMusica>(Pista);
		float Objetivo = Esta == PistaActual ? 1.f : 0.f;
		// La calma queda de fondo en una pelea normal; con el jefe se apaga, porque sus acordes chocan con los del jefe (EA-11).
		if (Esta == ESAEVUSPistaMusica::Exploracion && PistaActual == ESAEVUSPistaMusica::Combate)
		{
			Objetivo = Ajuste.CalmaEnPelea;
		}
		const float Rapidez = Objetivo > Volumenes[Pista] ? Ajuste.FundidoEntrada : Ajuste.FundidoSalida;
		Volumenes[Pista] = FMath::FInterpConstantTo(Volumenes[Pista], Objetivo, DeltaTime, Rapidez);
		if (UAudioComponent* Componente = Componentes[Pista])
		{
			Componente->SetVolumeMultiplier(FMath::Max(Volumenes[Pista] * Ajuste.Volumen, 0.001f));
		}
	}
}
