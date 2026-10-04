// Copyright SAEVUS. Todos los derechos reservados.

#include "Depuracion/SAEVUSSubsistemaNumerosDanio.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "Engine/World.h"

void USAEVUSSubsistemaNumerosDanio::Registrar(const UObject* Contexto, const FVector& Posicion, float Danio, bool bAlJugador)
{
	if (!SAEVUSDepuracion::NumerosDanioActivos() || Danio <= 0.f || !Contexto)
	{
		return;
	}
	const UWorld* Mundo = Contexto->GetWorld();
	USAEVUSSubsistemaNumerosDanio* Subsistema = Mundo ? Mundo->GetSubsystem<USAEVUSSubsistemaNumerosDanio>() : nullptr;
	if (!Subsistema)
	{
		return;
	}
	if (Subsistema->Numeros.Num() >= SAEVUSNumerosDanio::Maximo)
	{
		Subsistema->Numeros.RemoveAt(0);
	}
	Subsistema->Numeros.Add(FSAEVUSNumeroDanio{Posicion, Danio, Mundo->GetTimeSeconds(), bAlJugador});
}

const TArray<FSAEVUSNumeroDanio>& USAEVUSSubsistemaNumerosDanio::ObtenerNumeros()
{
	Numeros.RemoveAll([this](const FSAEVUSNumeroDanio& Numero) { return ObtenerEdad(Numero) >= 1.f; });
	return Numeros;
}

float USAEVUSSubsistemaNumerosDanio::ObtenerEdad(const FSAEVUSNumeroDanio& Numero) const
{
	return FMath::Clamp(static_cast<float>(GetWorld()->GetTimeSeconds() - Numero.Tiempo) / SAEVUSNumerosDanio::Duracion, 0.f, 1.f);
}
