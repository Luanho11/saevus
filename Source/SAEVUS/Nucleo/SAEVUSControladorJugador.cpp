// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSControladorJugador.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Interfaz/SAEVUSWidgetPausa.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Nucleo/SAEVUSModoJuego.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

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
	// El menú principal no se cierra con la tecla de pausa: hay que elegir.
	if (EstaEnMenuPrincipal())
	{
		return;
	}
	if (EstaEnMenuPausa())
	{
		CerrarMenu(MenuPausa);
		return;
	}
	if (!MenuPausa && ClaseMenuPausa)
	{
		MenuPausa = CreateWidget<USAEVUSWidgetPausa>(this, ClaseMenuPausa);
	}
	AbrirMenu(MenuPausa);
}

void ASAEVUSControladorJugador::AbrirMenu(USAEVUSWidgetPausa* Menu)
{
	if (!Menu)
	{
		return;
	}
	Menu->AddToViewport(1);
	SetPause(true);
	SetShowMouseCursor(true);
	// El foco en el primer botón: así se navega con la cruceta o las flechas y se confirma con el botón inferior o Intro.
	UWidget* PrimerBoton = Menu->ObtenerPrimerBoton();
	FInputModeGameAndUI Modo;
	Modo.SetWidgetToFocus(PrimerBoton ? PrimerBoton->TakeWidget() : Menu->TakeWidget());
	Modo.SetHideCursorDuringCapture(false);
	SetInputMode(Modo);
	if (PrimerBoton)
	{
		PrimerBoton->SetUserFocus(this);
	}
}

void ASAEVUSControladorJugador::CerrarMenu(USAEVUSWidgetPausa* Menu)
{
	if (Menu)
	{
		Menu->RemoveFromParent();
	}
	SetPause(false);
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}

bool ASAEVUSControladorJugador::MenuTieneFoco() const
{
	const USAEVUSWidgetPausa* Menu = EstaEnMenuPrincipal() ? MenuPrincipal.Get() : (EstaEnMenuPausa() ? MenuPausa.Get() : nullptr);
	return Menu && Menu->TieneFocoAlgunBoton();
}

bool ASAEVUSControladorJugador::EstaEnMenuPrincipal() const
{
	return MenuPrincipal && MenuPrincipal->IsInViewport();
}

void ASAEVUSControladorJugador::MostrarMenuPrincipal()
{
	if (EstaEnMenuPausa())
	{
		CerrarMenu(MenuPausa);
	}
	if (!MenuPrincipal && ClaseMenuPausa)
	{
		MenuPrincipal = CreateWidget<USAEVUSWidgetPausa>(this, ClaseMenuPausa);
		MenuPrincipal->FijarModo(ESAEVUSModoMenu::Principal);
	}
	if (!EstaEnMenuPrincipal())
	{
		AbrirMenu(MenuPrincipal);
	}
}

void ASAEVUSControladorJugador::CerrarMenuPrincipal()
{
	if (EstaEnMenuPrincipal())
	{
		CerrarMenu(MenuPrincipal);
	}
}

void ASAEVUSControladorJugador::IrAMapa(const FString& Ruta)
{
	if (Ruta.IsEmpty())
	{
		return;
	}
	CerrarMenuPrincipal();
	if (EstaEnMenuPausa())
	{
		CerrarMenu(MenuPausa);
	}
	UGameplayStatics::OpenLevel(this, FName(*Ruta));
}

void ASAEVUSControladorJugador::JugarEn(const FString& Ruta)
{
	if (SAEVUSGuardado::NombreMapa(GetWorld()) == FName(*FPackageName::GetShortName(Ruta)))
	{
		CerrarMenuPrincipal();
		return;
	}
	IrAMapa(Ruta);
}

void ASAEVUSControladorJugador::ContinuarPartidaGuardada()
{
	CerrarMenuPrincipal();
	FTransform Punto;
	const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this);
	ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>();
	if (Guardado && ModoJuego && Guardado->ObtenerPuntoControl(GetWorld(), Punto))
	{
		ModoJuego->FijarPuntoControl(Punto);
		ModoJuego->ReiniciarJugador(this);
	}
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

void ASAEVUSControladorJugador::BeginPlay()
{
	Super::BeginPlay();
	// Opciones de la sesión anterior.
	if (const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this))
	{
		Sensibilidad = Guardado->ObtenerSensibilidad();
		Volumen = Guardado->ObtenerVolumen();
	}
	if (IsLocalPlayerController())
	{
		SAEVUSGuardado::AplicarVolumen(GetWorld(), Volumen);
	}
	const ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>();
	if (IsLocalPlayerController() && ModoJuego && ModoJuego->DebeMostrarMenuPrincipal())
	{
		MostrarMenuPrincipal();
	}
}

void ASAEVUSControladorJugador::FijarVolumen(float Nuevo)
{
	Volumen = FMath::Clamp(Nuevo, 0.f, 1.f);
	SAEVUSGuardado::AplicarVolumen(GetWorld(), Volumen);
	if (USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this))
	{
		Guardado->GuardarVolumen(Volumen);
	}
}

void ASAEVUSControladorJugador::FijarSensibilidad(float Nueva)
{
	Sensibilidad = Nueva;
	if (USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this))
	{
		Guardado->GuardarSensibilidad(Nueva);
	}
}
