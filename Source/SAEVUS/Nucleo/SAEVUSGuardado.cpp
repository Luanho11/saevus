// Copyright SAEVUS. Todos los derechos reservados.

#include "Nucleo/SAEVUSGuardado.h"
#include "AudioDevice.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "SAEVUS.h"

namespace SAEVUSGuardado
{
#if WITH_EDITOR
	// Jugar desde el editor no debe pisar la partida del ejecutable (GAMEPLAY-31).
	const TCHAR* const RanuraPorDefecto = TEXT("SAEVUS_Editor");
#else
	const TCHAR* const RanuraPorDefecto = TEXT("SAEVUS");
#endif
	const TCHAR* const SufijoCopia = TEXT("_bak");
	const TCHAR* const ParametroSinGuardar = TEXT("SAEVUSSinGuardar");
	constexpr int32 IndiceUsuario = 0;

	FName ClaveMundo(FName Mapa, FName Actor)
	{
		return FName(*FString::Printf(TEXT("%s.%s"), *Mapa.ToString(), *Actor.ToString()));
	}

	bool QuitarEstadoMapa(USAEVUSPartidaGuardada* Partida, FName Mapa)
	{
		if (!Partida)
		{
			return false;
		}
		const FString Prefijo = Mapa.ToString() + TEXT(".");
		bool bQuitado = false;
		for (auto Iterador = Partida->EstadoMundo.CreateIterator(); Iterador; ++Iterador)
		{
			if (Iterador->ToString().StartsWith(Prefijo))
			{
				Iterador.RemoveCurrent();
				bQuitado = true;
			}
		}
		return bQuitado;
	}

	void Migrar(USAEVUSPartidaGuardada* Partida)
	{
		if (!Partida || Partida->Version >= VersionActual)
		{
			return;
		}
		// Versión 0: un fragmento sin identificador se guardaba como None y ahora el mismo fragmento tiene uno
		// propio, así que contaría dos veces (GAMEPLAY-26).
		Partida->FragmentosMascara.RemoveAll([](const FName& Id) { return Id.IsNone(); });
		Partida->Sensibilidad = FMath::Clamp(Partida->Sensibilidad, 0.1f, 5.f);
		// Versión 1 → 2: aparece EstadoMundo (vacío en las partidas anteriores).
		Partida->Version = VersionActual;
	}

	/**
	 * Cierto si el archivo de la ranura empieza como una partida de UE («GVAS»). Cargar bytes basura revienta el motor
	 * (lee un nombre de millones de caracteres y aborta), así que un archivo que no lo parece ni se intenta leer.
	 */
	bool TieneCabeceraValida(const FString& Ranura)
	{
		const FString Ruta = FPaths::ProjectSavedDir() / TEXT("SaveGames") / (Ranura + TEXT(".sav"));
		TUniquePtr<FArchive> Lector(IFileManager::Get().CreateFileReader(*Ruta));
		if (!Lector || Lector->TotalSize() < 16)
		{
			return false;
		}
		uint8 Cabecera[4] = { 0, 0, 0, 0 };
		Lector->Serialize(Cabecera, sizeof(Cabecera));
		return Cabecera[0] == 'G' && Cabecera[1] == 'V' && Cabecera[2] == 'A' && Cabecera[3] == 'S';
	}

	USAEVUSPartidaGuardada* Cargar(const FString& Ranura)
	{
		const FString RanuraCopia = Ranura + SufijoCopia;
		for (const FString* Candidata : { &Ranura, &RanuraCopia })
		{
			if (!UGameplayStatics::DoesSaveGameExist(*Candidata, IndiceUsuario))
			{
				continue;
			}
			if (!TieneCabeceraValida(*Candidata))
			{
				UE_LOG(LogSAEVUS, Warning, TEXT("La partida guardada '%s' está dañada (cabecera inválida)"), **Candidata);
				continue;
			}
			if (USAEVUSPartidaGuardada* Partida = Cast<USAEVUSPartidaGuardada>(UGameplayStatics::LoadGameFromSlot(*Candidata, IndiceUsuario)))
			{
				if (Candidata == &RanuraCopia)
				{
					UE_LOG(LogSAEVUS, Warning, TEXT("La partida '%s' estaba dañada; se recupera la copia de seguridad"), *Ranura);
				}
				Migrar(Partida);
				return Partida;
			}
			UE_LOG(LogSAEVUS, Warning, TEXT("La partida guardada '%s' no se pudo leer"), **Candidata);
		}
		USAEVUSPartidaGuardada* Nueva = Cast<USAEVUSPartidaGuardada>(UGameplayStatics::CreateSaveGameObject(USAEVUSPartidaGuardada::StaticClass()));
		if (Nueva)
		{
			Nueva->Version = VersionActual;
		}
		return Nueva;
	}

	bool Guardar(USAEVUSPartidaGuardada* Partida, const FString& Ranura)
	{
		return Partida && UGameplayStatics::SaveGameToSlot(Partida, Ranura, IndiceUsuario);
	}

	/** Como Guardar, pero la escritura del disco va en otro hilo: la partida no se para al guardar jugando. */
	void GuardarSinEsperar(USAEVUSPartidaGuardada* Partida, const FString& Ranura, TFunction<void(bool)> AlTerminar)
	{
		if (!Partida)
		{
			AlTerminar(false);
			return;
		}
		UGameplayStatics::AsyncSaveGameToSlot(Partida, Ranura, IndiceUsuario, FAsyncSaveGameToSlotDelegate::CreateLambda(
			[AlTerminar](const FString& RanuraGuardada, int32, bool bBien)
			{
				if (!bBien)
				{
					UE_LOG(LogSAEVUS, Warning, TEXT("No se pudo guardar la partida en '%s'"), *RanuraGuardada);
				}
				AlTerminar(bBien);
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
	// Copia de seguridad: lo que se acaba de leer bien queda aparte antes de empezar a escribir encima.
	if (bGuardarActivo && UGameplayStatics::DoesSaveGameExist(SAEVUSGuardado::RanuraPorDefecto, SAEVUSGuardado::IndiceUsuario))
	{
		SAEVUSGuardado::Guardar(Partida, FString(SAEVUSGuardado::RanuraPorDefecto) + SAEVUSGuardado::SufijoCopia);
	}
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
	Partida->Sensibilidad = FMath::Clamp(Sensibilidad, 0.1f, 5.f);
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

bool USAEVUSSubsistemaGuardado::TieneEstadoMundo(const AActor* Actor) const
{
	return Actor && Partida->EstadoMundo.Contains(SAEVUSGuardado::ClaveMundo(SAEVUSGuardado::NombreMapa(Actor->GetWorld()), Actor->GetFName()));
}

void USAEVUSSubsistemaGuardado::GuardarEstadoMundo(const AActor* Actor)
{
	if (!Actor)
	{
		return;
	}
	Partida->EstadoMundo.Add(SAEVUSGuardado::ClaveMundo(SAEVUSGuardado::NombreMapa(Actor->GetWorld()), Actor->GetFName()));
	Guardar();
}

bool USAEVUSSubsistemaGuardado::LimpiarEstadoMapa(FName Mapa)
{
	const bool bHabiaAlgo = SAEVUSGuardado::QuitarEstadoMapa(Partida, Mapa);
	if (bHabiaAlgo)
	{
		Guardar();
	}
	return bHabiaAlgo;
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
	if (!bGuardarActivo)
	{
		return;
	}
	// Dos escrituras asíncronas a la vez sobre el mismo archivo pueden dejarlo truncado: una sola en vuelo.
	if (bEscribiendo)
	{
		bSucio = true;
		return;
	}
	EscribirAhora();
}

void USAEVUSSubsistemaGuardado::EscribirAhora()
{
	bEscribiendo = true;
	bSucio = false;
	SAEVUSGuardado::GuardarSinEsperar(Partida, SAEVUSGuardado::RanuraPorDefecto,
		[Esto = TWeakObjectPtr<USAEVUSSubsistemaGuardado>(this)](bool)
		{
			if (USAEVUSSubsistemaGuardado* Subsistema = Esto.Get())
			{
				Subsistema->bEscribiendo = false;
				if (Subsistema->bSucio)
				{
					Subsistema->EscribirAhora();
				}
			}
		});
}
