// Copyright SAEVUS. Todos los derechos reservados.

#include "Depuracion/SAEVUSSubsistemaRecorrido.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "SAEVUS.h"
#include "Tutorial/SAEVUSPuertaArena.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"
#include "Tutorial/SAEVUSVolumenTutorial.h"
#include "UnrealClient.h"

namespace SAEVUSRecorrido
{
	const TCHAR* const Parametro = TEXT("SAEVUSRecorrido");
	constexpr float LimiteJugador = 120.f;
	constexpr float TiempoAsentar = 1.5f;
	constexpr float TiempoPorAviso = 0.6f;
	// Tras vaciar una arena: tiempo para que la puerta se abra y el jefe reaccione.
	constexpr float TiempoArena = 2.f;
	// A mitad de la espera en la arena, una captura mirando atrás: los enemigos que vienen a por el jugador.
	constexpr float FraccionCapturaEnemigos = 0.6f;
	const FRotator VistaHaciaAtras(-15.f, 180.f, 0.f);
	constexpr float AlturaLlevar = 150.f;
	constexpr float InvulnerableTodoElRecorrido = 100000.f;
	constexpr float DanioLetal = 100000.f;
	// El jugador se coloca delante de cada puerta, en el lado de la arena.
	constexpr float DistanciaAntesPuerta = 600.f;
}

bool USAEVUSSubsistemaRecorrido::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	const UWorld* Mundo = Cast<UWorld>(Outer);
	return Mundo && Mundo->IsGameWorld() && FParse::Param(FCommandLine::Get(), SAEVUSRecorrido::Parametro);
#endif
}

TStatId USAEVUSSubsistemaRecorrido::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USAEVUSSubsistemaRecorrido, STATGROUP_Tickables);
}

void USAEVUSSubsistemaRecorrido::Comprobar(const FString& Descripcion, bool bBien)
{
	Fallos += bBien ? 0 : 1;
	UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS recorrido] %s %s"), bBien ? TEXT("ok   ") : TEXT("FALLO"), *Descripcion);
}

void USAEVUSSubsistemaRecorrido::Terminar()
{
	UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS recorrido] %s"), Fallos == 0 ? TEXT("SUPERADO") : TEXT("FALLIDO"));
	Paso = EPaso::Hecho;
	FPlatformMisc::RequestExit(false, TEXT("Recorrido SAEVUS"));
}

void USAEVUSSubsistemaRecorrido::Llevar(ASAEVUSPersonaje* Jugador, const FVector& Posicion)
{
	Jugador->GetCharacterMovement()->StopMovementImmediately();
	Jugador->SetActorLocation(Posicion, false, nullptr, ETeleportType::TeleportPhysics);
}

void USAEVUSSubsistemaRecorrido::Preparar(ASAEVUSPersonaje* Jugador)
{
	// Invulnerable: el recorrido comprueba el mapa, no el combate.
	Jugador->ObtenerVida()->HacerInvulnerable(SAEVUSRecorrido::InvulnerableTodoElRecorrido);
	for (TActorIterator<ASAEVUSVolumenTutorial> Volumen(GetWorld()); Volumen; ++Volumen)
	{
		Avisos.Add(*Volumen);
	}
	Avisos.Sort([](const TWeakObjectPtr<ASAEVUSVolumenTutorial>& A, const TWeakObjectPtr<ASAEVUSVolumenTutorial>& B)
	{
		return A->GetActorLocation().X < B->GetActorLocation().X;
	});
	for (TActorIterator<ASAEVUSPuertaArena> Puerta(GetWorld()); Puerta; ++Puerta)
	{
		Puertas.Add(*Puerta);
	}
	Puertas.Sort([](const TWeakObjectPtr<ASAEVUSPuertaArena>& A, const TWeakObjectPtr<ASAEVUSPuertaArena>& B)
	{
		return A->GetActorLocation().X < B->GetActorLocation().X;
	});
	UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS recorrido] %d avisos y %d puertas en el mapa"), Avisos.Num(), Puertas.Num());
	Comprobar(TEXT("el mapa tiene avisos de tutorial"), !Avisos.IsEmpty());
}

void USAEVUSSubsistemaRecorrido::Tick(float DeltaTime)
{
	using namespace SAEVUSRecorrido;
	if (Paso == EPaso::Hecho)
	{
		return;
	}
	TiempoPaso += DeltaTime;
	TiempoTotal += DeltaTime;
	const APlayerController* Controlador = GetWorld()->GetFirstPlayerController();
	ASAEVUSPersonaje* Jugador = Controlador ? Cast<ASAEVUSPersonaje>(Controlador->GetPawn()) : nullptr;

	switch (Paso)
	{
	case EPaso::EsperarJugador:
		if (Jugador && TiempoPaso >= TiempoAsentar)
		{
			Preparar(Jugador);
			Paso = EPaso::Avisos;
			TiempoPaso = 0.f;
		}
		else if (TiempoTotal > LimiteJugador)
		{
			Comprobar(TEXT("aparece el jugador"), false);
			Terminar();
		}
		break;
	case EPaso::Avisos:
		if (Jugador)
		{
			ActualizarAvisos(Jugador);
		}
		break;
	case EPaso::Arenas:
		if (Jugador)
		{
			ActualizarArenas(Jugador);
		}
		break;
	default:
		break;
	}
}

void USAEVUSSubsistemaRecorrido::ActualizarAvisos(ASAEVUSPersonaje* Jugador)
{
	using namespace SAEVUSRecorrido;
	if (!Avisos.IsValidIndex(Indice))
	{
		Paso = EPaso::Arenas;
		Indice = 0;
		TiempoPaso = 0.f;
		bLlevado = false;
		return;
	}
	ASAEVUSVolumenTutorial* Volumen = Avisos[Indice].Get();
	if (!bLlevado)
	{
		Llevar(Jugador, FVector(Volumen->GetActorLocation().X, Volumen->GetActorLocation().Y, AlturaLlevar));
		bLlevado = true;
		TiempoPaso = 0.f;
		return;
	}
	if (TiempoPaso < TiempoPorAviso)
	{
		return;
	}
	const USAEVUSSubsistemaTutorial* Tutorial = GetWorld()->GetSubsystem<USAEVUSSubsistemaTutorial>();
	const FSAEVUSMensajeTutorial* Mensaje = Tutorial ? Tutorial->ObtenerMensajeActivo() : nullptr;
	const bool bAparece = Mensaje && Mensaje->Titulo.EqualTo(Volumen->ObtenerTitulo());
	Comprobar(FString::Printf(TEXT("aviso \"%s\" en X = %.0f"), *Volumen->ObtenerTitulo().ToString(), Volumen->GetActorLocation().X), bAparece);
	FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("SAEVUS_Recorrido_%02d.png"), Indice), true, false);
	++Indice;
	bLlevado = false;
}

void USAEVUSSubsistemaRecorrido::ActualizarArenas(ASAEVUSPersonaje* Jugador)
{
	using namespace SAEVUSRecorrido;
	if (!Puertas.IsValidIndex(Indice))
	{
		Terminar();
		return;
	}
	ASAEVUSPuertaArena* Puerta = Puertas[Indice].Get();
	if (!bLlevado)
	{
		// Dentro de la arena, delante de su puerta: los enemigos (y el jefe) reaccionan.
		Llevar(Jugador, FVector(Puerta->GetActorLocation().X - DistanciaAntesPuerta, 0.f, AlturaLlevar));
		bLlevado = true;
		TiempoPaso = 0.f;
		return;
	}
	if (!bCapturaEnemigos && TiempoPaso >= TiempoArena * FraccionCapturaEnemigos)
	{
		bCapturaEnemigos = true;
		if (AController* Controlador = Jugador->GetController())
		{
			Controlador->SetControlRotation(VistaHaciaAtras);
		}
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("SAEVUS_Recorrido_Arena%d_Enemigos.png"), Indice), true, false);
	}
	if (TiempoPaso < TiempoArena)
	{
		return;
	}
	// Una vez por arena: comprobar el jefe activo y vaciarla (enemigos entre la puerta anterior y esta).
	if (!bVaciada)
	{
		const float XFin = static_cast<float>(Puerta->GetActorLocation().X);
		const float XInicio = Puertas.IsValidIndex(Indice - 1) ? static_cast<float>(Puertas[Indice - 1]->GetActorLocation().X) : TNumericLimits<float>::Lowest();
		int32 Enemigos = 0;
		for (TActorIterator<ASAEVUSEnemigo> Enemigo(GetWorld()); Enemigo; ++Enemigo)
		{
			const float X = static_cast<float>(Enemigo->GetActorLocation().X);
			if (X > XInicio && X < XFin && !Enemigo->ObtenerVida()->EstaMuerto())
			{
				if (Enemigo->ObtenerAjuste().bEsJefe)
				{
					Comprobar(FString::Printf(TEXT("el jefe %s se activa al entrar"), *Enemigo->ObtenerAjuste().NombreVisible.ToString()),
						Enemigo->ObtenerIA()->ObtenerEstado() != ESAEVUSEstadoEnemigo::Reposo);
				}
				Enemigo->ObtenerVida()->AplicarDanio(DanioLetal, Jugador);
				++Enemigos;
			}
		}
		UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS recorrido] arena %d: %d enemigos derrotados"), Indice, Enemigos);
		if (AController* Controlador = Jugador->GetController())
		{
			Controlador->SetControlRotation(FRotator::ZeroRotator);
		}
		bVaciada = true;
		TiempoPaso = 0.f;
		return;
	}
	// Con la puerta ya hundida: comprobar y pasar a la siguiente arena.
	Comprobar(FString::Printf(TEXT("la puerta de la arena %d se abre al vaciarla"), Indice), Puerta->EstaAbierta());
	FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("SAEVUS_Recorrido_Arena%d.png"), Indice), true, false);
	++Indice;
	bLlevado = false;
	bVaciada = false;
	bCapturaEnemigos = false;
}
