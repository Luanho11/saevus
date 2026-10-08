// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSExploracion.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interfaz/SAEVUSMapa.h"
#include "Nucleo/SAEVUSGuardado.h"

namespace SAEVUSExploracion
{
	FIntPoint Celda(const FVector& Posicion)
	{
		return FIntPoint(FMath::FloorToInt32(Posicion.X / TamanoCelda), FMath::FloorToInt32(Posicion.Y / TamanoCelda));
	}

	FBox2D CajaCelda(const FIntPoint& Indice)
	{
		const FVector2D Esquina(Indice.X * TamanoCelda, Indice.Y * TamanoCelda);
		return FBox2D(Esquina, Esquina + FVector2D(TamanoCelda));
	}

	TArray<FSAEVUSBloqueMapa> RecortarBloques(const TArray<FSAEVUSBloqueMapa>& Bloques, const TSet<FIntPoint>& Celdas)
	{
		TArray<FSAEVUSBloqueMapa> Trozos;
		for (const FSAEVUSBloqueMapa& Bloque : Bloques)
		{
			// Solo las celdas que toca el bloque.
			const FIntPoint Desde = Celda(FVector(Bloque.Caja.Min, 0.0));
			const FIntPoint Hasta = Celda(FVector(Bloque.Caja.Max, 0.0));
			for (int32 X = Desde.X; X <= Hasta.X; ++X)
			{
				for (int32 Y = Desde.Y; Y <= Hasta.Y; ++Y)
				{
					if (!Celdas.Contains(FIntPoint(X, Y)))
					{
						continue;
					}
					const FBox2D Celda2D = CajaCelda(FIntPoint(X, Y));
					const FBox2D Recorte(FVector2D::Max(Bloque.Caja.Min, Celda2D.Min), FVector2D::Min(Bloque.Caja.Max, Celda2D.Max));
					if (Recorte.Min.X < Recorte.Max.X && Recorte.Min.Y < Recorte.Max.Y)
					{
						Trozos.Add(FSAEVUSBloqueMapa{ Recorte, Bloque.Altura });
					}
				}
			}
		}
		return Trozos;
	}
}

bool USAEVUSSubsistemaExploracion::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* Mundo = Cast<UWorld>(Outer);
	return Mundo && Mundo->IsGameWorld();
}

void USAEVUSSubsistemaExploracion::OnWorldBeginPlay(UWorld& Mundo)
{
	Super::OnWorldBeginPlay(Mundo);
	if (const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(&Mundo))
	{
		Celdas.Append(Guardado->ObtenerExploracion(SAEVUSGuardado::NombreMapa(&Mundo)));
	}
}

void USAEVUSSubsistemaExploracion::Deinitialize()
{
	Guardar();
	Super::Deinitialize();
}

TStatId USAEVUSSubsistemaExploracion::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(USAEVUSSubsistemaExploracion, STATGROUP_Tickables);
}

bool USAEVUSSubsistemaExploracion::Descubrir(const FVector& Posicion)
{
	using namespace SAEVUSExploracion;
	const FIntPoint Centro = Celda(Posicion);
	bool bNueva = false;
	for (int32 X = -RadioDescubrir; X <= RadioDescubrir; ++X)
	{
		for (int32 Y = -RadioDescubrir; Y <= RadioDescubrir; ++Y)
		{
			bool bYaEstaba = false;
			Celdas.Add(Centro + FIntPoint(X, Y), &bYaEstaba);
			bNueva |= !bYaEstaba;
		}
	}
	bSucio |= bNueva;
	return bNueva;
}

void USAEVUSSubsistemaExploracion::Tick(float DeltaTime)
{
	const APlayerController* Controlador = GetWorld()->GetFirstPlayerController();
	if (const APawn* Jugador = Controlador ? Controlador->GetPawn() : nullptr)
	{
		// Solo al pasar a otra celda: quieto o dentro de la misma no hay nada nuevo que descubrir.
		const FIntPoint Centro = SAEVUSExploracion::Celda(Jugador->GetActorLocation());
		if (!bHayCeldaActual || Centro != CeldaActual)
		{
			bHayCeldaActual = true;
			CeldaActual = Centro;
			Descubrir(Jugador->GetActorLocation());
		}
	}
	TiempoSinGuardar += DeltaTime;
	if (bSucio && TiempoSinGuardar >= SAEVUSExploracion::IntervaloGuardado)
	{
		Guardar();
	}
}

void USAEVUSSubsistemaExploracion::Guardar()
{
	TiempoSinGuardar = 0.f;
	if (!bSucio)
	{
		return;
	}
	bSucio = false;
	if (USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(GetWorld()))
	{
		Guardado->GuardarExploracion(SAEVUSGuardado::NombreMapa(GetWorld()), Celdas.Array());
	}
}
