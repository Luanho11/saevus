// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSControladorJugador.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

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
}

void ASAEVUSControladorJugador::AlternarHUDDepuracion()
{
	SAEVUSDepuracion::ActivarHUD(!SAEVUSDepuracion::HUDActivo());
}
