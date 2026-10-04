// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSEstadoInterfaz.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Armas/SAEVUSTiposArma.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"

#define LOCTEXT_NAMESPACE "SAEVUSInterfaz"

FSAEVUSEstadoInterfaz FSAEVUSEstadoInterfaz::Calcular(const APawn* Pawn, const UWorld* Mundo, float VidaPorMascara, float FundidoAviso)
{
	FSAEVUSEstadoInterfaz Estado;
	const USAEVUSComponenteVida* VidaJugador = Pawn ? Pawn->FindComponentByClass<USAEVUSComponenteVida>() : nullptr;
	Estado.bJugadorMuerto = !Pawn || (VidaJugador && VidaJugador->EstaMuerto());
	if (Pawn)
	{
		if (const USAEVUSComponenteVida* Vida = Pawn->FindComponentByClass<USAEVUSComponenteVida>(); Vida && VidaPorMascara > 0.f)
		{
			const int32 Numero = FMath::CeilToInt32(Vida->ObtenerVidaMaxima() / VidaPorMascara);
			for (int32 Indice = 0; Indice < Numero; ++Indice)
			{
				Estado.Mascaras.Add(FMath::Clamp((Vida->ObtenerVida() - Indice * VidaPorMascara) / VidaPorMascara, 0.f, 1.f));
			}
		}
		if (const USAEVUSComponenteAlma* Alma = Pawn->FindComponentByClass<USAEVUSComponenteAlma>())
		{
			Estado.FraccionAlma = Alma->ObtenerAlma() / Alma->ObtenerAlmaMaxima();
			Estado.bAlmaParaCurar = Alma->ObtenerAlma() >= Alma->ObtenerAjuste().CosteCuracion;
			Estado.bCurando = Alma->EstaCurando();
		}
		if (const USAEVUSComponenteArmas* Armas = Pawn->FindComponentByClass<USAEVUSComponenteArmas>())
		{
			const USAEVUSDefinicionArma* Arma = Armas->ObtenerArmaActual();
			Estado.TextoArma = Arma ? FText::FromName(Arma->NombreVisible) : LOCTEXT("SinArma", "Aguijón");
			if (Armas->TieneArmaFuegoEquipada())
			{
				Estado.TextoMunicion = Armas->EstaRecargando()
					? LOCTEXT("Recargando", "Recargando...")
					: (Armas->ObtenerReservaActual() == INDEX_NONE
						? FText::Format(LOCTEXT("MunicionInfinita", "{0} / ∞"), Armas->ObtenerMunicionActual())
						: FText::Format(LOCTEXT("Municion", "{0} / {1}"), Armas->ObtenerMunicionActual(), Armas->ObtenerReservaActual()));
			}
		}
	}

	if (Mundo)
	{
		if (const USAEVUSSubsistemaTutorial* Tutorial = Mundo->GetSubsystem<USAEVUSSubsistemaTutorial>())
		{
			if (const FSAEVUSMensajeTutorial* Mensaje = Tutorial->ObtenerMensajeActivo())
			{
				Estado.bHayAviso = true;
				Estado.TituloAviso = Mensaje->Titulo;
				Estado.TextoAviso = Mensaje->Texto;
				const float Edad = Tutorial->ObtenerEdadMensaje();
				Estado.OpacidadAviso = FundidoAviso > 0.f ? FMath::Clamp(Edad / FundidoAviso, 0.f, 1.f) : 1.f;
				if (Mensaje->Duracion > 0.f && FundidoAviso > 0.f)
				{
					Estado.OpacidadAviso = FMath::Min(Estado.OpacidadAviso, FMath::Clamp((Mensaje->Duracion - Edad) / FundidoAviso, 0.f, 1.f));
				}
			}
		}

		double MejorDistancia = TNumericLimits<double>::Max();
		for (TActorIterator<ASAEVUSEnemigo> Enemigo(Mundo); Enemigo; ++Enemigo)
		{
			const ESAEVUSEstadoEnemigo EstadoIA = Enemigo->ObtenerIA()->ObtenerEstado();
			if (!Enemigo->ObtenerAjuste().bEsJefe || EstadoIA == ESAEVUSEstadoEnemigo::Reposo || EstadoIA == ESAEVUSEstadoEnemigo::Muerto)
			{
				continue;
			}
			const double Distancia = Pawn ? FVector::DistSquared(Enemigo->GetActorLocation(), Pawn->GetActorLocation()) : 0.0;
			if (Distancia < MejorDistancia)
			{
				MejorDistancia = Distancia;
				const USAEVUSComponenteVida* VidaJefe = Enemigo->ObtenerVida();
				Estado.bJefeVisible = true;
				Estado.NombreJefe = Enemigo->ObtenerAjuste().NombreVisible;
				Estado.FraccionVidaJefe = VidaJefe->ObtenerVida() / VidaJefe->ObtenerVidaMaxima();
			}
		}
	}
	return Estado;
}

#undef LOCTEXT_NAMESPACE
