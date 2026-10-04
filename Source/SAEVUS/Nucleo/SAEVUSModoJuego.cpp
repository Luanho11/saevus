// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSModoJuego.h"
#include "Depuracion/SAEVUSHUDDepuracion.h"
#include "GameFramework/Controller.h"
#include "Nucleo/SAEVUSControladorJugador.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "TimerManager.h"

ASAEVUSModoJuego::ASAEVUSModoJuego()
{
	DefaultPawnClass = ASAEVUSPersonaje::StaticClass();
	PlayerControllerClass = ASAEVUSControladorJugador::StaticClass();
	HUDClass = ASAEVUSHUDDepuracion::StaticClass();
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

void ASAEVUSModoJuego::FijarPuntoControl(const FTransform& Punto)
{
	PuntoControl = Punto;
	bTienePuntoControl = true;
}
