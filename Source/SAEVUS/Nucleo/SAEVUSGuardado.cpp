// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSGuardado.h"
#include "AudioDevice.h"
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

	/** Como Guardar, pero la escritura del disco va en otro hilo: la partida no se para al guardar jugando. */
	void GuardarSinEsperar(USAEVUSPartidaGuardada* Partida, const FString& Ranura)
	{
		if (!Partida)
		{
			return;
		}
		UGameplayStatics::AsyncSaveGameToSlot(Partida, Ranura, IndiceUsuario, FAsyncSaveGameToSlotDelegate::CreateLambda(
			[](const FString& RanuraGuardada, int32, bool bBien)
			{
				if (!bBien)
				{
					UE_LOG(LogSAEVUS, Warning, TEXT("No se pudo guardar la partida en '%s'"), *RanuraGuardada);
				}
			}));
	}

	FName NombreMapa(const UWorld* Mundo)
	{
		return Mundo ? FName(*UGameplayStatics::GetCurrentLevelName(Mundo, true)) : NAME_None;
	}

	void AplicarVolumen(const UWorld* Mundo, float Volumen)
	{
		if (FAudioDevice* Dispositivo = Mundo ? Mundo->GetAudioDeviceRaw() : nullptr)
		{
			Dispositivo->SetTransientPrimaryVolume(FMath::Clamp(Volumen, 0.f, 1.f));
		}
	}
}

void USAEVUSSubsistemaGuardado::Initialize(FSubsystemCollectionBase& Coleccion)
{
	Super::Initialize(Coleccion);
	Partida = SAEVUSGuardado::Cargar(SAEVUSGuardado::RanuraPorDefecto);
	if (!Partida)
	{
		// Sin partida no hay nada que leer ni escribir: una vacía, así el resto del subsistema nunca ve un nulo.
		Partida = NewObject<USAEVUSPartidaGuardada>(this);
	}
	bGuardarActivo = !FParse::Param(FCommandLine::Get(), SAEVUSGuardado::ParametroSinGuardar);
}

void USAEVUSSubsistemaGuardado::Deinitialize()
{
	if (bGuardarActivo && !SAEVUSGuardado::Guardar(Partida, SAEVUSGuardado::RanuraPorDefecto))
	{
		UE_LOG(LogSAEVUS, Warning, TEXT("No se pudo guardar la partida en '%s'"), SAEVUSGuardado::RanuraPorDefecto);
	}
	Super::Deinitialize();
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

void USAEVUSSubsistemaGuardado::GuardarVolumen(float Volumen)
{
	Partida->Volumen = Volumen;
	Guardar();
}

TArray<FIntPoint> USAEVUSSubsistemaGuardado::ObtenerExploracion(FName Mapa) const
{
	const FSAEVUSExploracionMapa* Exploracion = Partida->Exploracion.Find(Mapa);
	return Exploracion ? Exploracion->Celdas : TArray<FIntPoint>();
}

void USAEVUSSubsistemaGuardado::GuardarExploracion(FName Mapa, const TArray<FIntPoint>& Celdas)
{
	Partida->Exploracion.FindOrAdd(Mapa).Celdas = Celdas;
	Guardar();
}

void USAEVUSSubsistemaGuardado::GuardarFragmento(FName Identificador)
{
	Partida->FragmentosMascara.AddUnique(Identificador);
	Guardar();
}

void USAEVUSSubsistemaGuardado::GuardarPuntoControl(const UWorld* Mundo, const FTransform& Punto)
{
	const FName Mapa = SAEVUSGuardado::NombreMapa(Mundo);
	Partida->bTienePuntoControl = true;
	Partida->MapaPuntoControl = Mapa;
	Partida->PuntoControl = Punto;
	Partida->PuntosControl.Add(Mapa, Punto);
	Guardar();
}

bool USAEVUSSubsistemaGuardado::ObtenerPuntoControl(const UWorld* Mundo, FTransform& OutPunto) const
{
	const FName Mapa = SAEVUSGuardado::NombreMapa(Mundo);
	if (const FTransform* Punto = Partida->PuntosControl.Find(Mapa))
	{
		OutPunto = *Punto;
		return true;
	}
	// Partidas guardadas antes de tener un punto por mapa.
	if (Partida->bTienePuntoControl && Partida->MapaPuntoControl == Mapa)
	{
		OutPunto = Partida->PuntoControl;
		return true;
	}
	return false;
}

void USAEVUSSubsistemaGuardado::Guardar()
{
	if (bGuardarActivo)
	{
		SAEVUSGuardado::GuardarSinEsperar(Partida, SAEVUSGuardado::RanuraPorDefecto);
	}
}
