// Copyright SAEVUS. Todos los derechos reservados.

#include "Depuracion/SAEVUSSubsistemaPruebaHumo.h"
#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/SkinnedAsset.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteGuardia.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "Efectos/SAEVUSComponenteEstelaDash.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "EngineUtils.h"
#include "DynamicRHI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "RenderTimer.h"
#include "SAEVUS.h"
#include "UnrealClient.h"

namespace SAEVUSPruebaHumo
{
	const TCHAR* const ParametroLineaComandos = TEXT("SAEVUSPruebaHumo");
	// Opcional: captura también un fotograma de ProfileGPU a mitad del esprint (tiempos de GPU por pasada en el log).
	const TCHAR* const ParametroPerfilGPU = TEXT("SAEVUSPruebaHumoPerfilGPU");
	const TCHAR* const NombreCaptura = TEXT("SAEVUS_PruebaHumo.png");
	const TCHAR* const NombreCapturaDash = TEXT("SAEVUS_PruebaHumo_Dash.png");
	const TCHAR* const NombreCapturaCorrer = TEXT("SAEVUS_PruebaHumo_Correr.png");
	const TCHAR* const NombreCapturaAtaque = TEXT("SAEVUS_PruebaHumo_Ataque.png");

	// Doble toque a la derecha: fotogramas pulsado, suelto y la segunda pulsación (dentro de la ventana a cualquier fps razonable).
	constexpr int32 FotogramasPulsado = 3;
	constexpr int32 FotogramasSuelto = 3;
	// Fotogramas tras empezar el dash antes de la captura, para que se vea la estela.
	constexpr int32 FotogramasHastaCapturaDash = 4;

	// Duración de las etapas en segundos. El asentado cubre la aparición, el aterrizaje y los primeros tirones.
	constexpr float DuracionAsentar = 2.f;
	constexpr float DuracionCorrer = 2.f;
	constexpr float DuracionEsprint = 2.f;
	constexpr float DuracionMaxSalto = 2.f;
	// Cubre el dash, su salida y el enfriamiento.
	constexpr float DuracionEtapaDash = 1.f;
	// Más largo que cualquier ataque por defecto.
	constexpr float DuracionMaxEtapaAtaque = 2.f;
	constexpr float ToleranciaDuracionAtaque = 0.15f;
	// Cambia a la segunda ranura del arsenal (un arma de fuego en el arsenal por defecto) y dispara una vez.
	constexpr int32 RanuraArmaFuego = 1;
	constexpr float DuracionEtapaGuardia = 0.5f;
	const TCHAR* const NombreCapturaGuardia = TEXT("SAEVUS_PruebaHumo_Guardia.png");
	constexpr float DuracionEtapaDisparo = 1.5f;
	// Daño, muerte y reaparición por la vía de daño del motor (necesita el modo de juego).
	constexpr float DanioPrueba = 10.f;
	constexpr float DanioLetal = 100000.f;
	constexpr float LimiteReaparicion = 8.f;
	constexpr float RetardoCaptura = 0.5f;
	constexpr float LimitePawn = 120.f;
	// Etapa de enemigos: el jugador aparece a esta distancia (en +Y) del primer enemigo y espera.
	constexpr float DistanciaEnemigos = 700.f;
	constexpr float DuracionEtapaEnemigos = 1.5f;
	const TCHAR* const NombreCapturaEnemigos = TEXT("SAEVUS_PruebaHumo_Enemigos.png");
	/** Radio mínimo (cm) de los límites del cuerpo: un humanoide de pie mide bastante más. */
	constexpr float RadioMinimoCuerpo = 60.f;

	// Valores medidos frente a los ajustes del propio personaje.
	constexpr float ToleranciaVelocidad = 0.05f;
	constexpr float ToleranciaCimaSalto = 0.1f;
}

bool USAEVUSSubsistemaPruebaHumo::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_BUILD_SHIPPING
	return false;
#else
	const UWorld* Mundo = Cast<UWorld>(Outer);
	return Mundo && Mundo->IsGameWorld() && FParse::Param(FCommandLine::Get(), SAEVUSPruebaHumo::ParametroLineaComandos);
#endif
}

TStatId USAEVUSSubsistemaPruebaHumo::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USAEVUSSubsistemaPruebaHumo, STATGROUP_Tickables);
}

void USAEVUSSubsistemaPruebaHumo::EntrarEtapa(EEtapa NuevaEtapa)
{
	Etapa = NuevaEtapa;
	TiempoEtapa = 0.f;
}

void USAEVUSSubsistemaPruebaHumo::Comprobar(const TCHAR* Descripcion, float Medido, float Esperado, float ToleranciaRelativa)
{
	const bool bBien = FMath::IsNearlyEqual(Medido, Esperado, FMath::Abs(Esperado) * ToleranciaRelativa);
	Fallos += bBien ? 0 : 1;
	UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS humo] %s %s: %.1f (esperado %.1f +-%.0f%%)"),
		bBien ? TEXT("ok   ") : TEXT("FALLO"), Descripcion, Medido, Esperado, ToleranciaRelativa * 100.f);
}

void USAEVUSSubsistemaPruebaHumo::Tick(float DeltaTime)
{
	using namespace SAEVUSPruebaHumo;
	TiempoEtapa += DeltaTime;

	if (Etapa == EEtapa::Correr || Etapa == EEtapa::Esprint || Etapa == EEtapa::Saltar || Etapa == EEtapa::Dash
		|| Etapa == EEtapa::Atacar || Etapa == EEtapa::Disparar)
	{
		MuestrearTiempos(DeltaTime);
	}

	ASAEVUSPersonaje* Pawn = Personaje.Get();
	USAEVUSComponenteMovimiento* Movimiento = Pawn ? Pawn->ObtenerMovimiento() : nullptr;
	if (Etapa != EEtapa::EsperarPawn && Etapa != EEtapa::Hecho && !Movimiento)
	{
		UE_LOG(LogSAEVUS, Error, TEXT("[SAEVUS humo] FALLIDA: el personaje del jugador desapareció"));
		++Fallos;
		Terminar();
		return;
	}

	switch (Etapa)
	{
	case EEtapa::EsperarPawn:
		if (const APlayerController* Controlador = GetWorld()->GetFirstPlayerController())
		{
			Personaje = Cast<ASAEVUSPersonaje>(Controlador->GetPawn());
		}
		if (Personaje.IsValid())
		{
			UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS humo] pawn %s apareció en %s"), *Personaje->GetName(), *Personaje->GetActorLocation().ToCompactString());
			SAEVUSDepuracion::ActivarHUD(true);
			EntrarEtapa(EEtapa::Asentar);
		}
		else if (TiempoEtapa > LimitePawn)
		{
			UE_LOG(LogSAEVUS, Error, TEXT("[SAEVUS humo] FALLIDA: ningún ASAEVUSPersonaje poseído tras %.0f s"), LimitePawn);
			++Fallos;
			Terminar();
		}
		break;

	case EEtapa::Asentar:
		if (TiempoEtapa >= DuracionAsentar)
		{
			UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS humo] asentado, modo %s"), *Movimiento->GetMovementName());
			if (UPoseableMeshComponent* Cuerpo = Pawn->ObtenerCuerpo())
			{
				RadioCuerpo = Cuerpo->Bounds.SphereRadius;
				UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS humo] cuerpo: malla %s, esqueleto válido %d, radio de límites %.0f"),
					*GetNameSafe(Cuerpo->GetSkinnedAsset()), Pawn->ObtenerAnimacion()->EsqueletoValido(), RadioCuerpo);
			}
			EntrarEtapa(EEtapa::Correr);
		}
		break;

	case EEtapa::Correr:
		Pawn->AddMovementInput(FVector::ForwardVector, 1.f);
		if (!bCapturaCorrerHecha && TiempoEtapa >= DuracionCorrer * 0.75f)
		{
			bCapturaCorrerHecha = true;
			FScreenshotRequest::RequestScreenshot(NombreCapturaCorrer, true, false);
		}
		VelocidadMaxCorrer = FMath::Max(VelocidadMaxCorrer, static_cast<float>(Movimiento->Velocity.Size2D()));
		if (TiempoEtapa >= DuracionCorrer)
		{
			Movimiento->PedirEsprint(true);
			EntrarEtapa(EEtapa::Esprint);
		}
		break;

	case EEtapa::Esprint:
		Pawn->AddMovementInput(FVector::ForwardVector, 1.f);
		VelocidadMaxEsprint = FMath::Max(VelocidadMaxEsprint, static_cast<float>(Movimiento->Velocity.Size2D()));
		if (!bPerfilGPUHecho && TiempoEtapa >= DuracionEsprint * 0.5f && FParse::Param(FCommandLine::Get(), ParametroPerfilGPU))
		{
			bPerfilGPUHecho = true;
			GEngine->Exec(GetWorld(), TEXT("ProfileGPU"));
		}
		if (TiempoEtapa >= DuracionEsprint)
		{
			Movimiento->PedirEsprint(false);
			ZInicioSalto = ZCimaSalto = Pawn->GetActorLocation().Z;
			Pawn->Jump();
			EntrarEtapa(EEtapa::Saltar);
		}
		break;

	case EEtapa::Saltar:
		Pawn->StopJumping();
		ZCimaSalto = FMath::Max(ZCimaSalto, static_cast<float>(Pawn->GetActorLocation().Z));
		bDejoSuelo |= Movimiento->IsFalling();
		if ((bDejoSuelo && Movimiento->IsMovingOnGround()) || TiempoEtapa >= DuracionMaxSalto)
		{
			FotogramaEtapa = 0;
			EntrarEtapa(EEtapa::Dash);
		}
		break;

	case EEtapa::Dash:
	{
		// Doble toque real por la misma vía que la entrada del jugador.
		const FVector2D Derecha(1.f, 0.f);
		if (FotogramaEtapa < FotogramasPulsado + FotogramasSuelto + 1)
		{
			const bool bPulsado = FotogramaEtapa < FotogramasPulsado || FotogramaEtapa == FotogramasPulsado + FotogramasSuelto;
			Pawn->AplicarEntradaMovimiento(bPulsado ? Derecha : FVector2D::ZeroVector);
		}
		else if (FotogramaEtapa == FotogramasPulsado + FotogramasSuelto + 1)
		{
			Pawn->AplicarEntradaMovimiento(FVector2D::ZeroVector);
		}
		++FotogramaEtapa;
		bDashEmpezo |= Movimiento->EstaEnDash();
		if (bDashEmpezo && !bCapturaDashHecha && FotogramaEtapa >= FotogramasPulsado + FotogramasSuelto + 1 + FotogramasHastaCapturaDash)
		{
			bCapturaDashHecha = true;
			ParticulasDash = Pawn->ObtenerEstelaDash()->ObtenerParticulasActivas();
			FScreenshotRequest::RequestScreenshot(NombreCapturaDash, true, false);
		}
		VelocidadMaxDash = FMath::Max(VelocidadMaxDash, static_cast<float>(Movimiento->Velocity.Size2D()));
		if (TiempoEtapa >= DuracionEtapaDash)
		{
			Pawn->PedirAtaque();
			bAtaqueEmpezo = Pawn->ObtenerCombate()->EstaAtacando();
			EntrarEtapa(EEtapa::Atacar);
		}
		break;
	}

	case EEtapa::Atacar:
		if (!bCapturaAtaqueHecha && Pawn->ObtenerCombate()->ObtenerFaseAtaque() == ESAEVUSFaseAtaque::Activa)
		{
			bCapturaAtaqueHecha = true;
			FScreenshotRequest::RequestScreenshot(NombreCapturaAtaque, true, false);
		}
		if (!Pawn->ObtenerCombate()->EstaAtacando() || TiempoEtapa >= DuracionMaxEtapaAtaque)
		{
			DuracionAtaque = TiempoEtapa;
			Pawn->FijarGuardia(true);
			EntrarEtapa(EEtapa::Guardia);
		}
		break;

	case EEtapa::Guardia:
		if (!bGuardiaActiva && TiempoEtapa >= DuracionEtapaGuardia * 0.6f)
		{
			// La captura sale en el fotograma siguiente: se pide con la guardia aún puesta.
			bGuardiaActiva = Pawn->ObtenerGuardia()->EstaEnGuardia();
			FScreenshotRequest::RequestScreenshot(NombreCapturaGuardia, true, false);
		}
		if (TiempoEtapa >= DuracionEtapaGuardia)
		{
			Pawn->FijarGuardia(false);
			Pawn->PedirRanuraArma(RanuraArmaFuego);
			EntrarEtapa(EEtapa::Disparar);
		}
		break;

	case EEtapa::Disparar:
	{
		const USAEVUSComponenteArmas* Armas = Pawn->ObtenerArmas();
		if (bGatilloPulsado)
		{
			Pawn->SoltarAtaque();
		}
		else if (!Armas->EstaCambiando() && Armas->TieneArmaFuegoEquipada())
		{
			Pawn->PedirAtaque();
			bGatilloPulsado = true;
		}
		if (TiempoEtapa >= DuracionEtapaDisparo)
		{
			// Se apunta ahora: el pawn reaparecido tiene componentes nuevos.
			DisparosRealizados = Armas->ObtenerDisparosRealizados();
			UGameplayStatics::ApplyDamage(Pawn, DanioPrueba, nullptr, nullptr, UDamageType::StaticClass());
			VidaTrasGolpe = Pawn->ObtenerVida()->ObtenerVida();
			// Justo después del golpe el jugador es invulnerable: el golpe letal se ignora.
			UGameplayStatics::ApplyDamage(Pawn, DanioLetal, nullptr, nullptr, UDamageType::StaticClass());
			VidaTrasGolpeInvulnerable = Pawn->ObtenerVida()->ObtenerVida();
			EntrarEtapa(EEtapa::Morir);
		}
		break;
	}

	case EEtapa::Morir:
		if (!Pawn->ObtenerVida()->EstaInvulnerable())
		{
			UGameplayStatics::ApplyDamage(Pawn, DanioLetal, nullptr, nullptr, UDamageType::StaticClass());
			bMurio = Pawn->ObtenerVida()->EstaMuerto();
			EntrarEtapa(EEtapa::Reaparecer);
		}
		else if (TiempoEtapa >= LimiteReaparicion)
		{
			EntrarEtapa(EEtapa::Reaparecer);
		}
		break;

	case EEtapa::Reaparecer:
	{
		// El cuerpo muerto se queda (Personaje sigue apuntándolo); se espera al nuevo pawn del controlador.
		const APlayerController* Controlador = GetWorld()->GetFirstPlayerController();
		ASAEVUSPersonaje* NuevoPawn = Controlador ? Cast<ASAEVUSPersonaje>(Controlador->GetPawn()) : nullptr;
		if (NuevoPawn && NuevoPawn != Pawn)
		{
			bReaparecio = true;
			VidaAlReaparecer = NuevoPawn->ObtenerVida()->ObtenerVida();
			Personaje = NuevoPawn;
			FScreenshotRequest::RequestScreenshot(NombreCaptura, true, false);
			EntrarEtapa(EEtapa::Enemigos);
		}
		else if (TiempoEtapa >= LimiteReaparicion)
		{
			FScreenshotRequest::RequestScreenshot(NombreCaptura, true, false);
			EntrarEtapa(EEtapa::Captura);
		}
		break;
	}

	case EEtapa::Enemigos:
	{
		// Con enemigos en el mapa: el jugador se planta cerca del primero, mirándolo, y espera.
		TActorIterator<ASAEVUSEnemigo> Primero(GetWorld());
		if (!Primero)
		{
			EntrarEtapa(EEtapa::Captura);
			break;
		}
		if (TiempoEtapa <= 0.f || EnemigosActivos < 0)
		{
			const FVector Destino = Primero->GetActorLocation() + FVector(0.f, DistanciaEnemigos, 0.f);
			Pawn->SetActorLocation(FVector(Destino.X, Destino.Y, Pawn->GetActorLocation().Z), false, nullptr, ETeleportType::TeleportPhysics);
			if (APlayerController* Controlador = GetWorld()->GetFirstPlayerController())
			{
				Controlador->SetControlRotation(FRotator(-15.f, -90.f, 0.f));
			}
			EnemigosActivos = 0;
		}
		if (TiempoEtapa >= DuracionEtapaEnemigos)
		{
			for (TActorIterator<ASAEVUSEnemigo> Enemigo(GetWorld()); Enemigo; ++Enemigo)
			{
				EnemigosActivos += Enemigo->ObtenerIA()->ObtenerEstado() != ESAEVUSEstadoEnemigo::Reposo ? 1 : 0;
			}
			UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS humo] enemigos activos tras acercarse: %d"), EnemigosActivos);
			FScreenshotRequest::RequestScreenshot(NombreCapturaEnemigos, true, false);
			EntrarEtapa(EEtapa::Captura);
		}
		break;
	}

	case EEtapa::Captura:
		if (TiempoEtapa >= RetardoCaptura)
		{
			EntrarEtapa(EEtapa::Informe);
		}
		break;

	case EEtapa::Informe:
	{
		Comprobar(TEXT("cuerpo animado con tamaño de personaje"), RadioCuerpo >= RadioMinimoCuerpo ? 1.f : 0.f, 1.f, 0.f);
		Comprobar(TEXT("velocidad al correr"), VelocidadMaxCorrer, Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Correr), ToleranciaVelocidad);
		Comprobar(TEXT("velocidad de esprint"), VelocidadMaxEsprint, Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Esprint), ToleranciaVelocidad);
		const float CimaEsperada = FMath::Square(Movimiento->JumpZVelocity) / (2.f * FMath::Abs(Movimiento->GetGravityZ()));
		Comprobar(TEXT("cima del salto"), ZCimaSalto - ZInicioSalto, CimaEsperada, ToleranciaCimaSalto);
		Comprobar(TEXT("dash por doble toque"), bDashEmpezo ? 1.f : 0.f, 1.f, 0.f);
		Comprobar(TEXT("velocidad del dash"), VelocidadMaxDash, Movimiento->ObtenerAjusteDash().Velocidad, ToleranciaVelocidad);
		Comprobar(TEXT("estela del dash con partículas"), ParticulasDash > 0 ? 1.f : 0.f, 1.f, 0.f);
		Comprobar(TEXT("ataque empezado"), bAtaqueEmpezo ? 1.f : 0.f, 1.f, 0.f);
		Comprobar(TEXT("duración del ataque"), DuracionAtaque, Pawn->ObtenerCombate()->ObtenerRepertorio().ComboSuelo[0].ObtenerTiempoTotal(), ToleranciaDuracionAtaque);
		Comprobar(TEXT("guardia activa"), bGuardiaActiva ? 1.f : 0.f, 1.f, 0.f);
		Comprobar(TEXT("disparos"), static_cast<float>(DisparosRealizados), 1.f, 0.f);
		Comprobar(TEXT("vida tras un golpe"), VidaTrasGolpe, Pawn->ObtenerVida()->ObtenerVidaMaxima() - DanioPrueba, 0.f);
		Comprobar(TEXT("golpe ignorado durante la invulnerabilidad"), VidaTrasGolpeInvulnerable, VidaTrasGolpe, 0.f);
		Comprobar(TEXT("murió"), bMurio ? 1.f : 0.f, 1.f, 0.f);
		if (EnemigosActivos >= 0)
		{
			Comprobar(TEXT("los enemigos cercanos reaccionan al jugador"), EnemigosActivos > 0 ? 1.f : 0.f, 1.f, 0.f);
		}
		Comprobar(TEXT("reapareció"), bReaparecio ? 1.f : 0.f, 1.f, 0.f);
		Comprobar(TEXT("vida al reaparecer"), VidaAlReaparecer, Pawn->ObtenerVida()->ObtenerVidaMaxima(), 0.f);
		Terminar();
		break;
	}

	case EEtapa::Hecho:
	default:
		break;
	}
}

void USAEVUSSubsistemaPruebaHumo::MuestrearTiempos(float DeltaTime)
{
	SumaTiemposFotograma += DeltaTime;
	PeorFotograma = FMath::Max(PeorFotograma, DeltaTime);
	SumaMsHiloJuego += FPlatformTime::ToMilliseconds(GGameThreadTime);
	SumaMsHiloRender += FPlatformTime::ToMilliseconds(GRenderThreadTime);
	SumaMsGPU += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	++Fotogramas;
}

void USAEVUSSubsistemaPruebaHumo::Terminar()
{
	if (Fotogramas > 0)
	{
		const double TiempoMedio = SumaTiemposFotograma / Fotogramas;
		UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS humo] fotogramas %d, media %.2f ms (%.1f fps), peor %.2f ms | juego %.2f ms, render %.2f ms, GPU %.2f ms"),
			Fotogramas, TiempoMedio * 1000.0, TiempoMedio > 0.0 ? 1.0 / TiempoMedio : 0.0, PeorFotograma * 1000.f,
			SumaMsHiloJuego / Fotogramas, SumaMsHiloRender / Fotogramas, SumaMsGPU / Fotogramas);
	}
	UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS humo] %s"), Fallos == 0 ? TEXT("SUPERADA") : TEXT("FALLIDA"));
	EntrarEtapa(EEtapa::Hecho);
	FPlatformMisc::RequestExit(false, TEXT("Prueba de humo SAEVUS"));
}
