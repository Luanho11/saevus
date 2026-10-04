// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSGuardado.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "SAEVUS.h"

namespace SAEVUSGuardado
{
	const TCHAR* const RanuraPorDefecto = TEXT("SAEVUS");
	const TCHAR* const ParametroSinGuardar = TEXT("SAEVUSSinGuardar");
	constexpr int32 IndiceUsuario = 0;

	USAEVUSPartidaGuardada* Cargar(const FString& Ranura)
	{
		if (UGameplayStatics::DoesSaveGameExist(Ranura, IndiceUsuario))
		{
			if (USAEVUSPartidaGuardada* Partida = Cast<USAEVUSPartidaGuardada>(UGameplayStatics::LoadGameFromSlot(Ranura, IndiceUsuario)))
			{
				return Partida;
			}
			UE_LOG(LogSAEVUS, Warning, TEXT("La partida guardada '%s' no se pudo leer; se empieza con una nueva"), *Ranura);
		}
		return Cast<USAEVUSPartidaGuardada>(UGameplayStatics::CreateSaveGameObject(USAEVUSPartidaGuardada::StaticClass()));
	}

	bool Guardar(USAEVUSPartidaGuardada* Partida, const FString& Ranura)
	{
		return Partida && UGameplayStatics::SaveGameToSlot(Partida, Ranura, IndiceUsuario);
	}

	FName NombreMapa(const UWorld* Mundo)
	{
		return Mundo ? FName(*UGameplayStatics::GetCurrentLevelName(Mundo, true)) : NAME_None;
	}
}

void USAEVUSSubsistemaGuardado::Initialize(FSubsystemCollectionBase& Coleccion)
{
	Super::Initialize(Coleccion);
	Partida = SAEVUSGuardado::Cargar(SAEVUSGuardado::RanuraPorDefecto);
	bGuardarActivo = !FParse::Param(FCommandLine::Get(), SAEVUSGuardado::ParametroSinGuardar);
}

USAEVUSSubsistemaGuardado* USAEVUSSubsistemaGuardado::Obtener(const UObject* Contexto)
{
	const UWorld* Mundo = Contexto ? Contexto->GetWorld() : nullptr;
	const UGameInstance* Instancia = Mundo ? Mundo->GetGameInstance() : nullptr;
	return Instancia ? Instancia->GetSubsystem<USAEVUSSubsistemaGuardado>() : nullptr;
}

void USAEVUSSubsistemaGuardado::GuardarSensibilidad(float Sensibilidad)
{
	Partida->Sensibilidad = Sensibilidad;
	Guardar();
}

void USAEVUSSubsistemaGuardado::GuardarPuntoControl(const UWorld* Mundo, const FTransform& Punto)
{
	Partida->bTienePuntoControl = true;
	Partida->MapaPuntoControl = SAEVUSGuardado::NombreMapa(Mundo);
	Partida->PuntoControl = Punto;
	Guardar();
}

bool USAEVUSSubsistemaGuardado::ObtenerPuntoControl(const UWorld* Mundo, FTransform& OutPunto) const
{
	if (!Partida->bTienePuntoControl || Partida->MapaPuntoControl != SAEVUSGuardado::NombreMapa(Mundo))
	{
		return false;
	}
	OutPunto = Partida->PuntoControl;
	return true;
}

void USAEVUSSubsistemaGuardado::Guardar()
{
	if (bGuardarActivo && !SAEVUSGuardado::Guardar(Partida, SAEVUSGuardado::RanuraPorDefecto))
	{
		UE_LOG(LogSAEVUS, Warning, TEXT("No se pudo guardar la partida en '%s'"), SAEVUSGuardado::RanuraPorDefecto);
	}
}
