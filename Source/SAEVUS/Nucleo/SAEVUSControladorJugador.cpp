// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSControladorJugador.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Interfaz/SAEVUSWidgetPausa.h"
#include "Nucleo/SAEVUSModoJuego.h"

ASAEVUSControladorJugador::ASAEVUSControladorJugador()
{
	ClaseMenuPausa = USAEVUSWidgetPausa::StaticClass();
}

bool ASAEVUSControladorJugador::EstaEnMenuPausa() const
{
	return MenuPausa && MenuPausa->IsInViewport();
}

void ASAEVUSControladorJugador::AlternarPausa()
{
	if (EstaEnMenuPausa())
	{
		MenuPausa->RemoveFromParent();
		SetPause(false);
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
		return;
	}
	if (!MenuPausa && ClaseMenuPausa)
	{
		MenuPausa = CreateWidget<USAEVUSWidgetPausa>(this, ClaseMenuPausa);
	}
	if (!MenuPausa)
	{
		return;
	}
	MenuPausa->AddToViewport(1);
	SetPause(true);
	SetShowMouseCursor(true);
	FInputModeGameAndUI Modo;
	Modo.SetWidgetToFocus(MenuPausa->TakeWidget());
	Modo.SetHideCursorDuringCapture(false);
	SetInputMode(Modo);
}

void ASAEVUSControladorJugador::ReiniciarEnPuntoControl()
{
	if (EstaEnMenuPausa())
	{
		AlternarPausa();
	}
	if (ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>())
	{
		ModoJuego->ReiniciarJugador(this);
	}
}

void ASAEVUSControladorJugador::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsistema = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (const UInputMappingContext* Contexto : ContextosEntrada)
		{
			if (Contexto)
			{
				Subsistema->AddMappingContext(Contexto, 0);
			}
		}
	}

	UEnhancedInputComponent* Entrada = Cast<UEnhancedInputComponent>(InputComponent);
	if (Entrada && AccionAlternarHUD)
	{
		Entrada->BindAction(AccionAlternarHUD, ETriggerEvent::Started, this, &ThisClass::AlternarHUDDepuracion);
	}
	if (Entrada && AccionPausa)
	{
		Entrada->BindAction(AccionPausa, ETriggerEvent::Started, this, &ThisClass::AlternarPausa);
	}
}

void ASAEVUSControladorJugador::AlternarHUDDepuracion()
{
	SAEVUSDepuracion::ActivarHUD(!SAEVUSDepuracion::HUDActivo());
}
