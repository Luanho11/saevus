// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSModoJuego.h"
#include "Depuracion/SAEVUSHUDDepuracion.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Nucleo/SAEVUSControladorJugador.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "TimerManager.h"

namespace SAEVUSModoJuego
{
	// Opción de la URL con la que UE elige el inicio de llegada (InitNewPlayer → FindPlayerStart).
	const TCHAR* const OpcionPaso = TEXT("portal");
}

const TCHAR* const ASAEVUSModoJuego::OpcionContinuar = TEXT("continuar");

ASAEVUSModoJuego::ASAEVUSModoJuego()
{
	DefaultPawnClass = ASAEVUSPersonaje::StaticClass();
	PlayerControllerClass = ASAEVUSControladorJugador::StaticClass();
	HUDClass = ASAEVUSHUDDepuracion::StaticClass();
	MapasConMenuPrincipal = { TEXT("L_Tutorial") };
}

bool ASAEVUSModoJuego::LlegoPorPaso() const
{
	return UGameplayStatics::HasOption(OptionsString, SAEVUSModoJuego::OpcionPaso);
}

AActor* ASAEVUSModoJuego::ChoosePlayerStart_Implementation(AController* Player)
{
	for (TActorIterator<APlayerStart> Inicio(GetWorld()); Inicio; ++Inicio)
	{
		if (Inicio->PlayerStartTag.IsNone())
		{
			return *Inicio;
		}
	}
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ASAEVUSModoJuego::HandleStartingNewPlayer_Implementation(APlayerController* NuevoJugador)
{
	Super::HandleStartingNewPlayer_Implementation(NuevoJugador);
	if (NuevoJugador && LlegoPorPaso() && NuevoJugador->StartSpot.IsValid())
	{
		FijarPuntoControl(NuevoJugador->StartSpot->GetActorTransform());
		return;
	}
	// "Continuar" desde el menú de otro mapa: se aparece en el punto de control guardado de este.
	FTransform Punto;
	const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this);
	if (NuevoJugador && UGameplayStatics::HasOption(OptionsString, OpcionContinuar) && Guardado && Guardado->ObtenerPuntoControl(GetWorld(), Punto))
	{
		FijarPuntoControl(Punto);
		ReiniciarJugador(NuevoJugador);
	}
}

bool ASAEVUSModoJuego::DebeMostrarMenuPrincipal() const
{
	// Al llegar por un paso desde otro mapa, o al continuar la partida, ya se está jugando.
	if (LlegoPorPaso() || UGameplayStatics::HasOption(OptionsString, OpcionContinuar))
	{
		return false;
	}
	for (const TCHAR* Parametro : { TEXT("SAEVUSPruebaHumo"), TEXT("SAEVUSRecorrido"), TEXT("SAEVUSSinMenu") })
	{
		if (FParse::Param(FCommandLine::Get(), Parametro))
		{
			return false;
		}
	}
	return MapasConMenuPrincipal.Contains(SAEVUSGuardado::NombreMapa(GetWorld()));
}

void ASAEVUSModoJuego::NotificarMuerteJugador(AController* Controlador)
{
	if (!Controlador)
	{
		return;
	}
	GetWorldTimerManager().SetTimer(TemporizadorReaparicion, FTimerDelegate::CreateUObject(this, &ThisClass::ReaparecerJugador, TWeakObjectPtr<AController>(Controlador)),
		FMath::Max(RetardoReaparicion, UE_KINDA_SMALL_NUMBER), false);
}

void ASAEVUSModoJuego::ReaparecerJugador(TWeakObjectPtr<AController> Controlador)
{
	if (!Controlador.IsValid())
	{
		return;
	}
	if (APawn* PawnMuerto = Controlador->GetPawn())
	{
		// Si ya reapareció por otro camino (volver al punto de control desde la pausa), este pawn está vivo: no se toca.
		const ASAEVUSPersonaje* Personaje = Cast<ASAEVUSPersonaje>(PawnMuerto);
		if (Personaje && !Personaje->ObtenerVida()->EstaMuerto())
		{
			return;
		}
		Controlador->UnPossess();
		PawnMuerto->SetLifeSpan(FMath::Max(DuracionCadaver, UE_KINDA_SMALL_NUMBER));
	}
	ReaparecerEnPuntoControl(Controlador.Get());
}

void ASAEVUSModoJuego::ReiniciarJugador(AController* Controlador)
{
	if (!Controlador)
	{
		return;
	}
	// La reaparición tras morir que estuviera esperando ya no hace falta.
	GetWorldTimerManager().ClearTimer(TemporizadorReaparicion);
	if (APawn* PawnActual = Controlador->GetPawn())
	{
		Controlador->UnPossess();
		PawnActual->Destroy();
	}
	ReaparecerEnPuntoControl(Controlador);
}

void ASAEVUSModoJuego::ReaparecerEnPuntoControl(AController* Controlador)
{
	if (bTienePuntoControl)
	{
		RestartPlayerAtTransform(Controlador, PuntoControl);
	}
	else
	{
		RestartPlayer(Controlador);
	}
}

void ASAEVUSModoJuego::FijarPuntoControl(const FTransform& Punto)
{
	// Solo posición y giro sobre el eje vertical: un volumen escalado no debe escalar ni inclinar al personaje (GAMEPLAY-13).
	PuntoControl = FTransform(FRotator(0.f, Punto.Rotator().Yaw, 0.f), Punto.GetLocation(), FVector::OneVector);
	bTienePuntoControl = true;
	if (USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this))
	{
		Guardado->GuardarPuntoControl(GetWorld(), PuntoControl);
	}
}
