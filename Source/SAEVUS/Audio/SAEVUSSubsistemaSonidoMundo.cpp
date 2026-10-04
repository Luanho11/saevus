// Copyright SAEVUS. Todos los derechos reservados.

#include "Audio/SAEVUSSubsistemaSonidoMundo.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

FSAEVUSAjusteSonidosMundo::FSAEVUSAjusteSonidosMundo()
{
	// Palanca: golpe metálico seco que baja un poco.
	Palanca.Duracion = 0.3f;
	Palanca.FrecuenciaInicio = 900.f;
	Palanca.FrecuenciaFin = 520.f;
	Palanca.Aspereza = 0.55f;
	Palanca.MezclaRuido = 0.3f;
	Palanca.FiltroRuido = 0.85f;
	Palanca.Caida = 0.08f;
	Palanca.Volumen = 0.6f;
	Palanca.Semilla = 211;

	// Puerta: retumbo largo y grave de piedra que se hunde.
	Puerta.Duracion = 1.1f;
	Puerta.FrecuenciaInicio = 70.f;
	Puerta.FrecuenciaFin = 45.f;
	Puerta.Aspereza = 0.5f;
	Puerta.MezclaRuido = 0.85f;
	Puerta.FiltroRuido = 0.08f;
	Puerta.Ataque = 0.1f;
	Puerta.Caida = 0.5f;
	Puerta.Volumen = 0.7f;
	Puerta.Semilla = 223;

	// Banco: campanada suave que sube un poco y se queda sonando.
	Banco.Duracion = 0.9f;
	Banco.FrecuenciaInicio = 520.f;
	Banco.FrecuenciaFin = 660.f;
	Banco.Aspereza = 0.05f;
	Banco.MezclaRuido = 0.05f;
	Banco.FiltroRuido = 0.5f;
	Banco.Ataque = 0.03f;
	Banco.Caida = 0.4f;
	Banco.Volumen = 0.45f;
	Banco.Semilla = 227;

	// Escudo: choque metálico seco y brillante.
	Escudo.Duracion = 0.22f;
	Escudo.FrecuenciaInicio = 1100.f;
	Escudo.FrecuenciaFin = 700.f;
	Escudo.Aspereza = 0.6f;
	Escudo.MezclaRuido = 0.45f;
	Escudo.FiltroRuido = 0.9f;
	Escudo.Caida = 0.05f;
	Escudo.Volumen = 0.6f;
	Escudo.Semilla = 229;
}

const FSAEVUSSonidoSintetizado& FSAEVUSAjusteSonidosMundo::Obtener(ESAEVUSSonidoMundo Sonido) const
{
	switch (Sonido)
	{
	case ESAEVUSSonidoMundo::Palanca:
		return Palanca;
	case ESAEVUSSonidoMundo::Banco:
		return Banco;
	case ESAEVUSSonidoMundo::Escudo:
		return Escudo;
	default:
		return Puerta;
	}
}

void USAEVUSSubsistemaSonidoMundo::Reproducir(const UObject* Contexto, ESAEVUSSonidoMundo Sonido, const FVector& Posicion)
{
	const UWorld* Mundo = Contexto ? Contexto->GetWorld() : nullptr;
	if (USAEVUSSubsistemaSonidoMundo* Subsistema = Mundo ? Mundo->GetSubsystem<USAEVUSSubsistemaSonidoMundo>() : nullptr)
	{
		Subsistema->ReproducirEn(Sonido, Posicion);
	}
}

void USAEVUSSubsistemaSonidoMundo::ReproducirEn(ESAEVUSSonidoMundo Sonido, const FVector& Posicion)
{
	const int32 Indice = static_cast<int32>(Sonido);
	if (Indice < 0 || Indice >= static_cast<int32>(ESAEVUSSonidoMundo::Total))
	{
		return;
	}
	const FSAEVUSSonidoSintetizado& Definicion = Ajuste.Obtener(Sonido);
	if (Datos[Indice].IsEmpty())
	{
		Datos[Indice] = SAEVUSSintesis::SintetizarBytes(Definicion);
	}
	++Reproducciones;
	UGameplayStatics::PlaySoundAtLocation(this, SAEVUSSintesis::CrearOnda(this, Datos[Indice], Definicion.Duracion), Posicion, Ajuste.Volumen);
}
