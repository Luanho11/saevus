// Copyright SAEVUS. Todos los derechos reservados.

#include "Depuracion/SAEVUSSubsistemaRecorrido.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "DynamicRHI.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Mundo/SAEVUSBanco.h"
#include "Mundo/SAEVUSFragmentoMascara.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"
#include "Mundo/SAEVUSPalanca.h"
#include "Mundo/SAEVUSParedRompible.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "RenderTimer.h"
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
	// Diana: un muñeco por encima de esta altura; el jugador dispara desde esta distancia por delante.
	constexpr float AlturaMinimaDiana = 300.f;
	constexpr float DistanciaDiana = 700.f;
	// Tiempo para equipar el arma, para apuntar (la cámara sigue a la rotación) y para que llegue el golpe.
	constexpr float TiempoEquiparDiana = 0.8f;
	constexpr float TiempoApuntarDiana = 0.4f;
	constexpr float TiempoComprobarDiana = 0.4f;
	// Palancas: el jugador se pone delante (en -X) mirando a +X y tira un tajo.
	constexpr float DistanciaPalanca = 110.f;
	constexpr float TiempoAntesTajoPalanca = 0.3f;
	constexpr float TiempoComprobarPalanca = 0.6f;
	// Bancos: tiempo para que el solape avise al jugador y para ver la pose sentada.
	constexpr float TiempoAnteBanco = 0.3f;
	constexpr float TiempoSentado = 0.6f;
	// Paredes falsas: tajos espaciados (el combo encadena) y un máximo por si no se rompe.
	constexpr float TiempoEntreTajos = 0.55f;
	constexpr int32 TajosMaximosPared = 8;
	// Fragmentos: tiempo para que el solape lo recoja.
	constexpr float TiempoRecogida = 0.4f;
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
	// Informativo (depende de la máquina): no cuenta como fallo.
	if (Fotogramas > 0)
	{
		const double Media = SumaSegundos / Fotogramas;
		UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS recorrido] rendimiento: %d fotogramas, media %.2f ms (%.1f fps), peor %.2f ms | juego %.2f ms, GPU %.2f ms"),
			Fotogramas, Media * 1000.0, Media > 0.0 ? 1.0 / Media : 0.0, PeorSegundos * 1000.f, SumaMsJuego / Fotogramas, SumaMsGPU / Fotogramas);
	}
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
		if (Puerta->TieneEnemigos())
		{
			Puertas.Add(*Puerta);
		}
	}
	for (TActorIterator<ASAEVUSPalanca> Palanca(GetWorld()); Palanca; ++Palanca)
	{
		Palancas.Add(*Palanca);
	}
	Palancas.Sort([](const TWeakObjectPtr<ASAEVUSPalanca>& A, const TWeakObjectPtr<ASAEVUSPalanca>& B)
	{
		return A->GetActorLocation().X < B->GetActorLocation().X;
	});
	for (TActorIterator<ASAEVUSBanco> Banco(GetWorld()); Banco; ++Banco)
	{
		Bancos.Add(*Banco);
	}
	for (TActorIterator<ASAEVUSParedRompible> Pared(GetWorld()); Pared; ++Pared)
	{
		Paredes.Add(*Pared);
	}
	for (TActorIterator<ASAEVUSFragmentoMascara> Fragmento(GetWorld()); Fragmento; ++Fragmento)
	{
		Fragmentos.Add(*Fragmento);
	}
	Bancos.Sort([](const TWeakObjectPtr<ASAEVUSBanco>& A, const TWeakObjectPtr<ASAEVUSBanco>& B)
	{
		return A->GetActorLocation().X < B->GetActorLocation().X;
	});
	Puertas.Sort([](const TWeakObjectPtr<ASAEVUSPuertaArena>& A, const TWeakObjectPtr<ASAEVUSPuertaArena>& B)
	{
		return A->GetActorLocation().X < B->GetActorLocation().X;
	});
	UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS recorrido] %d avisos, %d puertas de arena, %d palancas, %d bancos, %d paredes falsas y %d fragmentos en el mapa"),
		Avisos.Num(), Puertas.Num(), Palancas.Num(), Bancos.Num(), Paredes.Num(), Fragmentos.Num());
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
	if (Paso != EPaso::EsperarJugador)
	{
		++Fotogramas;
		SumaSegundos += DeltaTime;
		PeorSegundos = FMath::Max(PeorSegundos, DeltaTime);
		SumaMsJuego += FPlatformTime::ToMilliseconds(GGameThreadTime);
		SumaMsGPU += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	}
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
	case EPaso::Diana:
		if (Jugador)
		{
			ActualizarDiana(Jugador);
		}
		break;
	case EPaso::Arenas:
		if (Jugador)
		{
			ActualizarArenas(Jugador);
		}
		break;
	case EPaso::Palancas:
		if (Jugador)
		{
			ActualizarPalancas(Jugador);
		}
		break;
	case EPaso::Bancos:
		if (Jugador)
		{
			ActualizarBancos(Jugador);
		}
		break;
	case EPaso::Secretos:
		if (Jugador)
		{
			ActualizarSecretos(Jugador);
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
		Paso = EPaso::Diana;
		SubpasoDiana = 0;
		TiempoPaso = 0.f;
		return;
	}
	ASAEVUSVolumenTutorial* Volumen = Avisos[Indice].Get();
	if (!bLlevado)
	{
		// El centro del volumen queda a la altura de un jugador de pie sobre el suelo del aviso.
		Llevar(Jugador, Volumen->GetActorLocation());
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

void USAEVUSSubsistemaRecorrido::ActualizarDiana(ASAEVUSPersonaje* Jugador)
{
	using namespace SAEVUSRecorrido;
	USAEVUSComponenteArmas* Armas = Jugador->ObtenerArmas();
	APlayerController* Controlador = Cast<APlayerController>(Jugador->GetController());
	auto PasarAArenas = [this, Armas]()
	{
		Armas->PedirCambio(0);
		Paso = EPaso::Arenas;
		Indice = 0;
		TiempoPaso = 0.f;
		bLlevado = false;
	};
	switch (SubpasoDiana)
	{
	case 0:
	{
		// La diana más alta del mapa y la primera ranura con arma de fuego.
		for (TActorIterator<ASAEVUSMunecoEntrenamiento> Muneco(GetWorld()); Muneco; ++Muneco)
		{
			if (Muneco->GetActorLocation().Z > AlturaMinimaDiana && (!Diana.IsValid() || Muneco->GetActorLocation().Z > Diana->GetActorLocation().Z))
			{
				Diana = *Muneco;
			}
		}
		int32 RanuraFuego = INDEX_NONE;
		for (int32 Ranura = 0; Ranura < Armas->ObtenerNumeroRanuras() && RanuraFuego == INDEX_NONE; ++Ranura)
		{
			const USAEVUSDefinicionArma* Arma = Armas->ObtenerArma(Ranura);
			RanuraFuego = Arma && Arma->EsArmaFuego() ? Ranura : INDEX_NONE;
		}
		if (!Diana.IsValid())
		{
			UE_LOG(LogSAEVUS, Display, TEXT("[SAEVUS recorrido] sin diana alta en el mapa: no se prueban las armas"));
			PasarAArenas();
			return;
		}
		Comprobar(TEXT("el arsenal tiene un arma de fuego"), RanuraFuego != INDEX_NONE);
		if (RanuraFuego == INDEX_NONE || !Controlador)
		{
			PasarAArenas();
			return;
		}
		const FVector PosicionDiana = Diana->GetActorLocation();
		Llevar(Jugador, FVector(PosicionDiana.X - DistanciaDiana, PosicionDiana.Y * 0.5f, AlturaLlevar));
		Armas->PedirCambio(RanuraFuego);
		SubpasoDiana = 1;
		TiempoPaso = 0.f;
		break;
	}
	case 1:
	{
		// Apunta desde la cámara cada fotograma (la cámara se mueve al girar) y dispara al acabar.
		FVector PosicionVista;
		FRotator RotacionVista;
		Controlador->GetPlayerViewPoint(PosicionVista, RotacionVista);
		Controlador->SetControlRotation((Diana->GetActorLocation() - PosicionVista).Rotation());
		if (TiempoPaso >= TiempoEquiparDiana + TiempoApuntarDiana && !Armas->EstaCambiando())
		{
			Armas->FijarGatillo(true);
			Armas->FijarGatillo(false);
			SubpasoDiana = 2;
			TiempoPaso = 0.f;
		}
		break;
	}
	default:
		if (TiempoPaso >= TiempoComprobarDiana)
		{
			Comprobar(FString::Printf(TEXT("un disparo de %s alcanza la diana alta (%d golpes)"),
				Armas->ObtenerArmaActual() ? *Armas->ObtenerArmaActual()->NombreVisible.ToString() : TEXT("?"), Diana->ObtenerGolpes()), Diana->ObtenerGolpes() > 0);
			FScreenshotRequest::RequestScreenshot(TEXT("SAEVUS_Recorrido_Diana.png"), true, false);
			Controlador->SetControlRotation(FRotator::ZeroRotator);
			PasarAArenas();
		}
		break;
	}
}

void USAEVUSSubsistemaRecorrido::ActualizarBancos(ASAEVUSPersonaje* Jugador)
{
	using namespace SAEVUSRecorrido;
	if (!Bancos.IsValidIndex(Indice))
	{
		Paso = EPaso::Secretos;
		Indice = 0;
		TiempoPaso = 0.f;
		bLlevado = false;
		TajosDados = 0;
		return;
	}
	ASAEVUSBanco* Banco = Bancos[Indice].Get();
	if (!bLlevado)
	{
		// En el banco (su origen está a la altura del centro de un jugador de pie).
		Llevar(Jugador, Banco->GetActorLocation());
		bLlevado = true;
		bVaciada = false;
		TiempoPaso = 0.f;
		return;
	}
	if (!bVaciada && TiempoPaso >= TiempoAnteBanco)
	{
		Comprobar(FString::Printf(TEXT("el banco %d se anuncia al acercarse"), Indice), Jugador->ObtenerInteractuable() == Banco);
		Jugador->PedirInteraccion();
		Comprobar(FString::Printf(TEXT("sentarse en el banco %d (vida completa)"), Indice),
			Jugador->EstaSentado() && Banco->ObtenerUsos() > 0 && Jugador->ObtenerVida()->ObtenerVida() >= Jugador->ObtenerVida()->ObtenerVidaMaxima());
		bVaciada = true;
		TiempoPaso = 0.f;
		return;
	}
	// Captura con el jugador aún sentado (la captura sale al final del fotograma).
	if (bVaciada && !bCapturaEnemigos && TiempoPaso >= TiempoSentado * 0.5f)
	{
		bCapturaEnemigos = true;
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("SAEVUS_Recorrido_Banco%d.png"), Indice), true, false);
	}
	if (!bVaciada || TiempoPaso < TiempoSentado)
	{
		return;
	}
	bCapturaEnemigos = false;
	Jugador->PedirInteraccion();
	Comprobar(FString::Printf(TEXT("levantarse del banco %d"), Indice), !Jugador->EstaSentado());
	++Indice;
	bLlevado = false;
	bVaciada = false;
}

void USAEVUSSubsistemaRecorrido::ActualizarSecretos(ASAEVUSPersonaje* Jugador)
{
	using namespace SAEVUSRecorrido;
	// Primero las paredes falsas (a tajos de verdad), después los fragmentos (tocándolos).
	if (Paredes.IsValidIndex(Indice))
	{
		ASAEVUSParedRompible* Pared = Paredes[Indice].Get();
		if (!bLlevado)
		{
			const FVector Posicion = Pared->GetActorLocation();
			const float MitadAncho = static_cast<float>(Pared->GetComponentsBoundingBox().GetExtent().X);
			Llevar(Jugador, FVector(Posicion.X - MitadAncho - DistanciaPalanca, Posicion.Y, AlturaJunto(Pared)));
			Jugador->SetActorRotation(FRotator::ZeroRotator);
			if (AController* Controlador = Jugador->GetController())
			{
				Controlador->SetControlRotation(FRotator::ZeroRotator);
			}
			bLlevado = true;
			TajosDados = 0;
			TiempoPaso = 0.f;
			return;
		}
		if (!Pared->EstaRota() && TajosDados < TajosMaximosPared)
		{
			if (TiempoPaso >= TiempoEntreTajos)
			{
				Jugador->PedirAtaque();
				Jugador->SoltarAtaque();
				++TajosDados;
				TiempoPaso = 0.f;
			}
			return;
		}
		Comprobar(FString::Printf(TEXT("la pared falsa %d cae a tajos (%d)"), Indice, TajosDados), Pared->EstaRota());
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("SAEVUS_Recorrido_Pared%d.png"), Indice), true, false);
		++Indice;
		bLlevado = false;
		return;
	}
	const int32 IndiceFragmento = Indice - Paredes.Num();
	if (!Fragmentos.IsValidIndex(IndiceFragmento))
	{
		Terminar();
		return;
	}
	if (!bLlevado)
	{
		FragmentosAntes = Jugador->ObtenerFragmentosMascara();
		if (Fragmentos[IndiceFragmento].IsValid())
		{
			Llevar(Jugador, Fragmentos[IndiceFragmento]->GetActorLocation());
		}
		bLlevado = true;
		TiempoPaso = 0.f;
		return;
	}
	if (TiempoPaso < TiempoRecogida)
	{
		return;
	}
	Comprobar(FString::Printf(TEXT("el fragmento de máscara %d se recoge al tocarlo"), IndiceFragmento),
		!Fragmentos[IndiceFragmento].IsValid() && Jugador->ObtenerFragmentosMascara() == FragmentosAntes + 1);
	++Indice;
	bLlevado = false;
}

float USAEVUSSubsistemaRecorrido::AlturaJunto(const AActor* Actor)
{
	return static_cast<float>(Actor->GetComponentsBoundingBox(true).Min.Z) + SAEVUSRecorrido::AlturaLlevar;
}

void USAEVUSSubsistemaRecorrido::ActualizarPalancas(ASAEVUSPersonaje* Jugador)
{
	using namespace SAEVUSRecorrido;
	if (!Palancas.IsValidIndex(Indice))
	{
		Paso = EPaso::Bancos;
		Indice = 0;
		TiempoPaso = 0.f;
		bLlevado = false;
		bVaciada = false;
		return;
	}
	ASAEVUSPalanca* Palanca = Palancas[Indice].Get();
	if (!bLlevado)
	{
		// Delante de la palanca, mirando hacia ella.
		const FVector Posicion = Palanca->GetActorLocation();
		Llevar(Jugador, FVector(Posicion.X - DistanciaPalanca, Posicion.Y, AlturaJunto(Palanca)));
		Jugador->SetActorRotation(FRotator::ZeroRotator);
		if (AController* Controlador = Jugador->GetController())
		{
			Controlador->SetControlRotation(FRotator::ZeroRotator);
		}
		bLlevado = true;
		bVaciada = false;
		TiempoPaso = 0.f;
		return;
	}
	if (!bVaciada && TiempoPaso >= TiempoAntesTajoPalanca)
	{
		// Un tajo de verdad: comprueba que la palanca se puede golpear.
		Jugador->PedirAtaque();
		Jugador->SoltarAtaque();
		bVaciada = true;
		TiempoPaso = 0.f;
		return;
	}
	if (!bVaciada || TiempoPaso < TiempoComprobarPalanca)
	{
		return;
	}
	bool bPuertasAbiertas = !Palanca->ObtenerPuertas().IsEmpty();
	for (const ASAEVUSPuertaArena* Compuerta : Palanca->ObtenerPuertas())
	{
		bPuertasAbiertas &= Compuerta && Compuerta->EstaAbierta();
	}
	Comprobar(FString::Printf(TEXT("un tajo acciona la palanca %d en X = %.0f"), Indice, Palanca->GetActorLocation().X), Palanca->EstaAccionada());
	Comprobar(FString::Printf(TEXT("la palanca %d abre sus compuertas"), Indice), bPuertasAbiertas);
	FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("SAEVUS_Recorrido_Palanca%d.png"), Indice), true, false);
	++Indice;
	bLlevado = false;
	bVaciada = false;
}

void USAEVUSSubsistemaRecorrido::ActualizarArenas(ASAEVUSPersonaje* Jugador)
{
	using namespace SAEVUSRecorrido;
	if (!Puertas.IsValidIndex(Indice))
	{
		Paso = EPaso::Palancas;
		Indice = 0;
		TiempoPaso = 0.f;
		bLlevado = false;
		return;
	}
	ASAEVUSPuertaArena* Puerta = Puertas[Indice].Get();
	if (!bLlevado)
	{
		// Dentro de la arena, delante de su puerta: los enemigos (y el jefe) reaccionan.
		Llevar(Jugador, FVector(Puerta->GetActorLocation().X - DistanciaAntesPuerta, 0.f, AlturaJunto(Puerta)));
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
