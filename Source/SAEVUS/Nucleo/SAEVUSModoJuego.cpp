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
#include "Personaje/SAEVUSPersonaje.h"
#include "TimerManager.h"

namespace SAEVUSModoJuego
{
	// Opción de la URL con la que UE elige el inicio de llegada (InitNewPlayer → FindPlayerStart).
	const TCHAR* const OpcionPaso = TEXT("portal");
}

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
	}
}

bool ASAEVUSModoJuego::DebeMostrarMenuPrincipal() const
{
	// Al llegar por un paso desde otro mapa ya se está jugando.
	if (LlegoPorPaso())
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
	FTimerHandle SinUso;
	GetWorldTimerManager().SetTimer(SinUso, FTimerDelegate::CreateUObject(this, &ThisClass::ReaparecerJugador, TWeakObjectPtr<AController>(Controlador)),
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
		Controlador->UnPossess();
		PawnMuerto->SetLifeSpan(FMath::Max(DuracionCadaver, UE_KINDA_SMALL_NUMBER));
	}
	if (bTienePuntoControl)
	{
		RestartPlayerAtTransform(Controlador.Get(), PuntoControl);
	}
	else
	{
		RestartPlayer(Controlador.Get());
	}
}

void ASAEVUSModoJuego::ReiniciarJugador(AController* Controlador)
{
	if (!Controlador)
	{
		return;
	}
	if (APawn* PawnActual = Controlador->GetPawn())
	{
		Controlador->UnPossess();
		PawnActual->Destroy();
	}
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
	PuntoControl = Punto;
	bTienePuntoControl = true;
	if (USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this))
	{
		Guardado->GuardarPuntoControl(GetWorld(), Punto);
	}
}
