// Copyright SAEVUS. Todos los derechos reservados.

#include "Audio/SAEVUSComponenteSonidoEnemigo.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWaveProcedural.h"

FSAEVUSAjusteSonidosEnemigo::FSAEVUSAjusteSonidosEnemigo()
{
	// Aviso: gruñido grave y áspero que sube.
	Aviso.Duracion = 0.45f;
	Aviso.FrecuenciaInicio = 70.f;
	Aviso.FrecuenciaFin = 160.f;
	Aviso.Aspereza = 0.7f;
	Aviso.MezclaRuido = 0.3f;
	Aviso.FiltroRuido = 0.1f;
	Aviso.Ataque = 0.15f;
	Aviso.Caida = 0.2f;
	Aviso.Volumen = 0.55f;
	Aviso.Semilla = 101;

	// Ataque: arrastre de aire con cuerpo.
	Ataque.Duracion = 0.3f;
	Ataque.FrecuenciaInicio = 220.f;
	Ataque.FrecuenciaFin = 90.f;
	Ataque.MezclaRuido = 0.8f;
	Ataque.FiltroRuido = 0.3f;
	Ataque.Ataque = 0.02f;
	Ataque.Caida = 0.1f;
	Ataque.Volumen = 0.55f;
	Ataque.Semilla = 113;

	// Muerte: chillido que cae y se apaga.
	Muerte.Duracion = 0.6f;
	Muerte.FrecuenciaInicio = 600.f;
	Muerte.FrecuenciaFin = 80.f;
	Muerte.Aspereza = 0.4f;
	Muerte.MezclaRuido = 0.4f;
	Muerte.FiltroRuido = 0.4f;
	Muerte.Caida = 0.2f;
	Muerte.Volumen = 0.6f;
	Muerte.Semilla = 127;
}

void USAEVUSComponenteSonidoEnemigo::BeginPlay()
{
	Super::BeginPlay();
	DatosAviso = SAEVUSSintesis::SintetizarBytes(Ajuste.Aviso);
	DatosAtaque = SAEVUSSintesis::SintetizarBytes(Ajuste.Ataque);
	DatosMuerte = SAEVUSSintesis::SintetizarBytes(Ajuste.Muerte);
	if (USAEVUSComponenteIAEnemigo* IA = GetOwner()->FindComponentByClass<USAEVUSComponenteIAEnemigo>())
	{
		Manejador = IA->AlCambiarEstado.AddUObject(this, &ThisClass::AlCambiarEstado);
	}
}

void USAEVUSComponenteSonidoEnemigo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USAEVUSComponenteIAEnemigo* IA = GetOwner()->FindComponentByClass<USAEVUSComponenteIAEnemigo>())
	{
		IA->AlCambiarEstado.Remove(Manejador);
	}
	Super::EndPlay(EndPlayReason);
}

void USAEVUSComponenteSonidoEnemigo::AlCambiarEstado(ESAEVUSEstadoEnemigo NuevoEstado)
{
	switch (NuevoEstado)
	{
	case ESAEVUSEstadoEnemigo::Telegrafiar:
		Reproducir(DatosAviso, Ajuste.Aviso.Duracion);
		break;
	case ESAEVUSEstadoEnemigo::Atacar:
		Reproducir(DatosAtaque, Ajuste.Ataque.Duracion);
		break;
	case ESAEVUSEstadoEnemigo::Muerto:
		Reproducir(DatosMuerte, Ajuste.Muerte.Duracion);
		break;
	default:
		break;
	}
}

void USAEVUSComponenteSonidoEnemigo::Reproducir(const TArray<uint8>& Datos, float Duracion)
{
	if (Datos.IsEmpty())
	{
		return;
	}
	++Reproducciones;
	UGameplayStatics::PlaySoundAtLocation(this, SAEVUSSintesis::CrearOnda(this, Datos, Duracion), GetOwner()->GetActorLocation(), Ajuste.Volumen);
}
