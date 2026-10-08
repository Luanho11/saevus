// Copyright SAEVUS. Todos los derechos reservados.

#include "Audio/SAEVUSComponenteSonidoEnemigo.h"
#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Enemigos/SAEVUSEnemigo.h"
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

	// Aleteo: soplo corto y suave, casi solo ruido grave.
	Aleteo.Duracion = 0.12f;
	Aleteo.FrecuenciaInicio = 140.f;
	Aleteo.FrecuenciaFin = 90.f;
	Aleteo.MezclaRuido = 0.9f;
	Aleteo.FiltroRuido = 0.15f;
	Aleteo.Ataque = 0.02f;
	Aleteo.Caida = 0.05f;
	Aleteo.Volumen = 0.25f;
	Aleteo.Semilla = 131;
}

USAEVUSComponenteSonidoEnemigo::USAEVUSComponenteSonidoEnemigo()
{
	// Solo los voladores lo encienden (aleteo).
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USAEVUSComponenteSonidoEnemigo::BeginPlay()
{
	Super::BeginPlay();
	DatosAviso = SAEVUSSintesis::SintetizarBytes(Ajuste.Aviso);
	DatosAtaque = SAEVUSSintesis::SintetizarBytes(Ajuste.Ataque);
	DatosMuerte = SAEVUSSintesis::SintetizarBytes(Ajuste.Muerte);
	ActualizarTipo();
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

void USAEVUSComponenteSonidoEnemigo::ActualizarTipo()
{
	const ASAEVUSEnemigo* Enemigo = Cast<ASAEVUSEnemigo>(GetOwner());
	const bool bVolador = Enemigo && Enemigo->ObtenerAjuste().bVolador;
	if (bVolador && DatosAleteo.IsEmpty())
	{
		DatosAleteo = SAEVUSSintesis::SintetizarBytes(Ajuste.Aleteo);
	}
	SetComponentTickEnabled(bVolador);
}

void USAEVUSComponenteSonidoEnemigo::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const ASAEVUSEnemigo* Enemigo = Cast<ASAEVUSEnemigo>(GetOwner());
	const USAEVUSComponenteIAEnemigo* IA = Enemigo ? Enemigo->ObtenerIA() : nullptr;
	if (!IA || IA->ObtenerEstado() == ESAEVUSEstadoEnemigo::Muerto)
	{
		return;
	}
	// Un batido por ciclo de aleteo (el mismo ritmo que el balanceo del cuerpo).
	const float Periodo = 1.f / FMath::Max(Enemigo->ObtenerAjuste().FrecuenciaAleteo, KINDA_SMALL_NUMBER);
	TiempoAleteo += DeltaTime;
	if (TiempoAleteo < Periodo)
	{
		return;
	}
	TiempoAleteo -= Periodo;
	const APawn* Objetivo = IA->ObtenerObjetivo();
	if (Objetivo && FVector::Dist(Objetivo->GetActorLocation(), Enemigo->GetActorLocation()) <= Ajuste.DistanciaAleteo)
	{
		Reproducir(DatosAleteo, Ajuste.Aleteo.Duracion);
	}
}

void USAEVUSComponenteSonidoEnemigo::Reproducir(const TArray<uint8>& Datos, float Duracion)
{
	if (Datos.IsEmpty())
	{
		return;
	}
	++Reproducciones;
	UGameplayStatics::PlaySoundAtLocation(this, SAEVUSSintesis::CrearOnda(this, Datos, Duracion), GetOwner()->GetActorLocation(), Ajuste.Volumen, 1.f, 0.f, SAEVUSSintesis::ObtenerAtenuacion());
}
