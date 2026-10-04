// Copyright SAEVUS. Todos los derechos reservados.

#include "Depuracion/SAEVUSHUDDepuracion.h"
#include "Blueprint/UserWidget.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteGuardia.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"

ASAEVUSHUDDepuracion::ASAEVUSHUDDepuracion()
{
	ClaseInterfaz = USAEVUSWidgetInterfaz::StaticClass();
}

void ASAEVUSHUDDepuracion::BeginPlay()
{
	Super::BeginPlay();
	if (ClaseInterfaz && PlayerOwner)
	{
		Interfaz = CreateWidget<USAEVUSWidgetInterfaz>(PlayerOwner, ClaseInterfaz);
		Interfaz->AddToViewport();
	}
}

void ASAEVUSHUDDepuracion::DrawHUD()
{
	Super::DrawHUD();
	if (SAEVUSDepuracion::HUDActivo())
	{
		DibujarDepuracion();
	}
}

void ASAEVUSHUDDepuracion::DibujarDepuracion()
{

	const ACharacter* Personaje = Cast<ACharacter>(GetOwningPawn());
	const USAEVUSComponenteMovimiento* Movimiento = Personaje ? Cast<USAEVUSComponenteMovimiento>(Personaje->GetCharacterMovement()) : nullptr;
	if (!Movimiento)
	{
		return;
	}

	TArray<FString> Lineas = {
		TEXT("SAEVUS DEPURACIÓN  (F6 / SAEVUS.Depuracion.HUD)"),
		FString::Printf(TEXT("Paso: %s"), *UEnum::GetDisplayValueAsText(Movimiento->ObtenerPasoActual()).ToString()),
		FString::Printf(TEXT("Velocidad: %.0f / %.0f cm/s"), Movimiento->Velocity.Size2D(), Movimiento->GetMaxSpeed()),
		FString::Printf(TEXT("Velocidad vertical: %.0f cm/s"), Movimiento->Velocity.Z),
		FString::Printf(TEXT("Modo: %s"), *Movimiento->GetMovementName()),
		FString::Printf(TEXT("Dash: %s, dashes aéreos %d"), Movimiento->EstaEnDash() ? TEXT("activo") : (Movimiento->PuedeDash() ? TEXT("listo") : TEXT("bloqueado")),
			Movimiento->DashesAereosRestantes()),
		FString::Printf(TEXT("Dobles saltos %d, saltos de pared %d, escaladas %d, en pared %.2f s"), Movimiento->SaltosAereosRestantes(), Movimiento->SaltosParedRestantes(),
			Movimiento->EscaladasRestantes(), Movimiento->TiempoEnParedRestante()),
		FString::Printf(TEXT("Posición: %s"), *Personaje->GetActorLocation().ToCompactString()),
	};

	FString LineaAtaque = TEXT("Ataque: ninguno");
	if (const USAEVUSComponenteCombate* Combate = Personaje->FindComponentByClass<USAEVUSComponenteCombate>())
	{
		if (const FSAEVUSDefinicionAtaque* Ataque = Combate->ObtenerAtaqueActual())
		{
			LineaAtaque = FString::Printf(TEXT("Ataque: %s #%d %s %.2f s"), *Ataque->Nombre.ToString(), Combate->ObtenerIndiceCombo() + 1,
				*UEnum::GetDisplayValueAsText(Combate->ObtenerFaseAtaque()).ToString(), Combate->ObtenerTiempoAtaque());
		}
		if (Combate->TieneAtaqueEnBuffer())
		{
			LineaAtaque += TEXT(" (en buffer)");
		}
	}
	Lineas.Add(LineaAtaque);

	FString LineaArma = TEXT("Arma: ninguna");
	if (const USAEVUSComponenteArmas* Armas = Personaje->FindComponentByClass<USAEVUSComponenteArmas>())
	{
		if (const USAEVUSDefinicionArma* Arma = Armas->ObtenerArmaActual())
		{
			LineaArma = FString::Printf(TEXT("Arma: %s [%d/%d]"), *Arma->NombreVisible.ToString(), Armas->ObtenerRanuraActual() + 1, Armas->ObtenerNumeroRanuras());
			if (Arma->EsArmaFuego())
			{
				LineaArma += FString::Printf(TEXT(" munición %d/%d"), Armas->ObtenerMunicionActual(), Arma->ArmaFuego.TamanoCargador);
			}
		}
		if (Armas->EstaCambiando())
		{
			LineaArma += FString::Printf(TEXT(" (cambiando a %d)"), Armas->ObtenerRanuraPendiente() + 1);
		}
		if (Armas->EstaRecargando())
		{
			LineaArma += TEXT(" (recargando)");
		}
	}
	Lineas.Add(LineaArma);

	if (const USAEVUSComponenteVida* Vida = Personaje->FindComponentByClass<USAEVUSComponenteVida>())
	{
		Lineas.Add(FString::Printf(TEXT("Vida: %.0f/%.0f%s%s"), Vida->ObtenerVida(), Vida->ObtenerVidaMaxima(), Vida->EstaMuerto() ? TEXT(" MUERTO") : TEXT(""),
			Vida->EstaInvulnerable() ? TEXT(" (invulnerable)") : TEXT("")));
	}
	if (const USAEVUSComponenteGuardia* Guardia = Personaje->FindComponentByClass<USAEVUSComponenteGuardia>())
	{
		Lineas.Add(FString::Printf(TEXT("Guardia: %s, bloqueos %d"), Guardia->EstaEnGuardia() ? TEXT("activa") : (Guardia->EstaPedida() ? TEXT("pedida") : TEXT("no")),
			Guardia->ObtenerBloqueos()));
	}
	if (const USAEVUSComponenteAlma* Alma = Personaje->FindComponentByClass<USAEVUSComponenteAlma>())
	{
		Lineas.Add(Alma->EstaCurando()
			? FString::Printf(TEXT("Alma: %.0f/%.0f (curando %.0f %%)"), Alma->ObtenerAlma(), Alma->ObtenerAlmaMaxima(), Alma->ObtenerProgresoCuracion() * 100.f)
			: FString::Printf(TEXT("Alma: %.0f/%.0f"), Alma->ObtenerAlma(), Alma->ObtenerAlmaMaxima()));
	}

	FVector2D Cursor = OrigenPantalla;
	for (const FString& Linea : Lineas)
	{
		DrawText(Linea, ColorTexto, Cursor.X, Cursor.Y, GEngine->GetSmallFont(), EscalaTexto);
		Cursor.Y += AltoLinea;
	}
}
