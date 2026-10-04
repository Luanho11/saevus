// Copyright SAEVUS. Todos los derechos reservados.

#include "Audio/SAEVUSComponenteSonido.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Sound/SoundWaveProcedural.h"

FSAEVUSAjusteSonidos::FSAEVUSAjusteSonidos()
{
	// Primera pasada de diseño sonoro: todo cambiable en el editor.
	// Tajo: siseo de aire.
	Tajo.Duracion = 0.16f;
	Tajo.MezclaRuido = 0.9f;
	Tajo.FiltroRuido = 0.35f;
	Tajo.FrecuenciaInicio = 900.f;
	Tajo.FrecuenciaFin = 500.f;
	Tajo.Ataque = 0.03f;
	Tajo.Caida = 0.05f;
	Tajo.Volumen = 0.45f;
	Tajo.Semilla = 11;

	// Golpe: impacto sordo y seco.
	Golpe.Duracion = 0.2f;
	Golpe.FrecuenciaInicio = 180.f;
	Golpe.FrecuenciaFin = 55.f;
	Golpe.Aspereza = 0.35f;
	Golpe.MezclaRuido = 0.5f;
	Golpe.FiltroRuido = 0.15f;
	Golpe.Caida = 0.06f;
	Golpe.Volumen = 0.8f;
	Golpe.Semilla = 23;

	// Disparo: chasquido con cuerpo grave.
	Disparo.Duracion = 0.22f;
	Disparo.FrecuenciaInicio = 140.f;
	Disparo.FrecuenciaFin = 50.f;
	Disparo.Aspereza = 0.6f;
	Disparo.MezclaRuido = 0.75f;
	Disparo.FiltroRuido = 0.85f;
	Disparo.Caida = 0.035f;
	Disparo.Volumen = 0.7f;
	Disparo.Semilla = 37;

	// Daño recibido: zumbido áspero que baja.
	Danio.Duracion = 0.28f;
	Danio.FrecuenciaInicio = 320.f;
	Danio.FrecuenciaFin = 110.f;
	Danio.Aspereza = 0.8f;
	Danio.MezclaRuido = 0.25f;
	Danio.Caida = 0.1f;
	Danio.Volumen = 0.55f;
	Danio.Semilla = 41;

	// Curación: campanilla que sube.
	Curacion.Duracion = 0.6f;
	Curacion.FrecuenciaInicio = 420.f;
	Curacion.FrecuenciaFin = 940.f;
	Curacion.Ataque = 0.02f;
	Curacion.Caida = 0.25f;
	Curacion.Volumen = 0.4f;
	Curacion.Semilla = 53;

	// Dash: soplo más largo que el tajo.
	Dash.Duracion = 0.3f;
	Dash.MezclaRuido = 1.f;
	Dash.FiltroRuido = 0.5f;
	Dash.Ataque = 0.01f;
	Dash.Caida = 0.11f;
	Dash.Volumen = 0.5f;
	Dash.Semilla = 61;

	// Doble salto: bocanada suave con un poco de tono.
	DobleSalto.Duracion = 0.22f;
	DobleSalto.FrecuenciaInicio = 300.f;
	DobleSalto.FrecuenciaFin = 520.f;
	DobleSalto.MezclaRuido = 0.75f;
	DobleSalto.FiltroRuido = 0.2f;
	DobleSalto.Caida = 0.07f;
	DobleSalto.Volumen = 0.45f;
	DobleSalto.Semilla = 71;
}

void USAEVUSComponenteSonido::Preparar(EEfecto Efecto, const FSAEVUSSonidoSintetizado& Sonido)
{
	const TArray<int16> Muestras = SAEVUSSintesis::Sintetizar(Sonido);
	FEfecto& Destino = Efectos[static_cast<int32>(Efecto)];
	Destino.Datos.SetNumUninitialized(Muestras.Num() * sizeof(int16));
	FMemory::Memcpy(Destino.Datos.GetData(), Muestras.GetData(), Destino.Datos.Num());
	Destino.Duracion = Sonido.Duracion;
}

void USAEVUSComponenteSonido::BeginPlay()
{
	Super::BeginPlay();
	Preparar(EEfecto::Tajo, Ajuste.Tajo);
	Preparar(EEfecto::Golpe, Ajuste.Golpe);
	Preparar(EEfecto::Disparo, Ajuste.Disparo);
	Preparar(EEfecto::Danio, Ajuste.Danio);
	Preparar(EEfecto::Curacion, Ajuste.Curacion);
	Preparar(EEfecto::Dash, Ajuste.Dash);
	Preparar(EEfecto::DobleSalto, Ajuste.DobleSalto);

	AActor* Duenio = GetOwner();
	if (USAEVUSComponenteCombate* Combate = Duenio->FindComponentByClass<USAEVUSComponenteCombate>())
	{
		Manejadores.Emplace(Combate, Combate->AlIniciarAtaque.AddUObject(this, &ThisClass::AlIniciarAtaque));
		Combate->AlGolpear.AddUniqueDynamic(this, &ThisClass::AlGolpear);
	}
	if (USAEVUSComponenteArmas* Armas = Duenio->FindComponentByClass<USAEVUSComponenteArmas>())
	{
		Armas->AlDisparar.AddUniqueDynamic(this, &ThisClass::AlDisparar);
	}
	if (USAEVUSComponenteVida* Vida = Duenio->FindComponentByClass<USAEVUSComponenteVida>())
	{
		Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::AlCambiarVida);
	}
	if (USAEVUSComponenteAlma* Alma = Duenio->FindComponentByClass<USAEVUSComponenteAlma>())
	{
		Manejadores.Emplace(Alma, Alma->AlCurar.AddUObject(this, &ThisClass::AlCurar));
	}
	if (const ACharacter* Personaje = Cast<ACharacter>(Duenio))
	{
		if (USAEVUSComponenteMovimiento* Movimiento = Cast<USAEVUSComponenteMovimiento>(Personaje->GetCharacterMovement()))
		{
			Manejadores.Emplace(Movimiento, Movimiento->AlIniciarDash.AddUObject(this, &ThisClass::AlIniciarDash));
			Manejadores.Emplace(Movimiento, Movimiento->AlSaltoAereo.AddUObject(this, &ThisClass::AlSaltoAereo));
		}
	}
}

void USAEVUSComponenteSonido::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Los eventos nativos guardan el manejador; se quitan sabiendo de qué componente es cada uno.
	for (const TPair<TWeakObjectPtr<UObject>, FDelegateHandle>& Par : Manejadores)
	{
		if (USAEVUSComponenteCombate* Combate = Cast<USAEVUSComponenteCombate>(Par.Key.Get()))
		{
			Combate->AlIniciarAtaque.Remove(Par.Value);
		}
		else if (USAEVUSComponenteAlma* Alma = Cast<USAEVUSComponenteAlma>(Par.Key.Get()))
		{
			Alma->AlCurar.Remove(Par.Value);
		}
		else if (USAEVUSComponenteMovimiento* Movimiento = Cast<USAEVUSComponenteMovimiento>(Par.Key.Get()))
		{
			Movimiento->AlIniciarDash.Remove(Par.Value);
			Movimiento->AlSaltoAereo.Remove(Par.Value);
		}
	}
	Manejadores.Reset();
	Super::EndPlay(EndPlayReason);
}

void USAEVUSComponenteSonido::Reproducir(EEfecto Efecto, float VolumenCategoria)
{
	const FEfecto& Datos = Efectos[static_cast<int32>(Efecto)];
	if (Datos.Datos.IsEmpty())
	{
		return;
	}
	++Reproducciones;
	// Una onda procedural nueva por reproducción: se consume al sonar.
	USoundWaveProcedural* Onda = NewObject<USoundWaveProcedural>(this);
	Onda->SetSampleRate(SAEVUSSintesis::FrecuenciaMuestreo);
	Onda->NumChannels = 1;
	Onda->Duration = Datos.Duracion;
	Onda->bLooping = false;
	Onda->QueueAudio(Datos.Datos.GetData(), Datos.Datos.Num());
	UGameplayStatics::PlaySound2D(this, Onda, Ajuste.VolumenGeneral * VolumenCategoria);
}

void USAEVUSComponenteSonido::AlIniciarAtaque()
{
	Reproducir(EEfecto::Tajo, Ajuste.VolumenCombate);
}

void USAEVUSComponenteSonido::AlGolpear(AActor* ActorGolpeado, float Cantidad)
{
	Reproducir(EEfecto::Golpe, Ajuste.VolumenCombate);
}

void USAEVUSComponenteSonido::AlDisparar(FVector Boca, FVector Impacto, AActor* ActorGolpeado)
{
	Reproducir(EEfecto::Disparo, Ajuste.VolumenCombate);
}

void USAEVUSComponenteSonido::AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador)
{
	if (Delta < 0.f)
	{
		Reproducir(EEfecto::Danio, Ajuste.VolumenCombate);
	}
}

void USAEVUSComponenteSonido::AlCurar(float VidaRecuperada)
{
	Reproducir(EEfecto::Curacion, Ajuste.VolumenCombate);
}

void USAEVUSComponenteSonido::AlIniciarDash(const FVector& Direccion)
{
	Reproducir(EEfecto::Dash, Ajuste.VolumenMovimiento);
}

void USAEVUSComponenteSonido::AlSaltoAereo()
{
	Reproducir(EEfecto::DobleSalto, Ajuste.VolumenMovimiento);
}
