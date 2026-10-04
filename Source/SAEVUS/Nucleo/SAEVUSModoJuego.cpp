// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSModoJuego.h"
#include "Depuracion/SAEVUSHUDDepuracion.h"
#include "GameFramework/Controller.h"
#include "Misc/CommandLine.h"
#include "Nucleo/SAEVUSControladorJugador.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "TimerManager.h"

ASAEVUSModoJuego::ASAEVUSModoJuego()
{
	DefaultPawnClass = ASAEVUSPersonaje::StaticClass();
	PlayerControllerClass = ASAEVUSControladorJugador::StaticClass();
	HUDClass = ASAEVUSHUDDepuracion::StaticClass();
	MapasConMenuPrincipal = { TEXT("L_Tutorial") };
}

bool ASAEVUSModoJuego::DebeMostrarMenuPrincipal() const
{
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
