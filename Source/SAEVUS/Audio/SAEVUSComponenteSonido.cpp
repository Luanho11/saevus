// Copyright SAEVUS. Todos los derechos reservados.

#include "Audio/SAEVUSComponenteSonido.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteGuardia.h"
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

	// Gatillo en seco: clic metálico corto y flojo.
	GatilloEnSeco.Duracion = 0.06f;
	GatilloEnSeco.FrecuenciaInicio = 2400.f;
	GatilloEnSeco.FrecuenciaFin = 1800.f;
	GatilloEnSeco.Aspereza = 0.3f;
	GatilloEnSeco.MezclaRuido = 0.5f;
	GatilloEnSeco.FiltroRuido = 0.9f;
	GatilloEnSeco.Caida = 0.012f;
	GatilloEnSeco.Volumen = 0.35f;
	GatilloEnSeco.Semilla = 41;

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

	// Bloqueo: choque metálico corto y brillante.
	Bloqueo.Duracion = 0.25f;
	Bloqueo.FrecuenciaInicio = 1300.f;
	Bloqueo.FrecuenciaFin = 900.f;
	Bloqueo.Aspereza = 0.5f;
	Bloqueo.MezclaRuido = 0.35f;
	Bloqueo.FiltroRuido = 0.9f;
	Bloqueo.Caida = 0.06f;
	Bloqueo.Volumen = 0.55f;
	Bloqueo.Semilla = 83;

	// Contraataque: campanada alta y limpia que sube.
	Contraataque.Duracion = 0.45f;
	Contraataque.FrecuenciaInicio = 1500.f;
	Contraataque.FrecuenciaFin = 2100.f;
	Contraataque.Aspereza = 0.15f;
	Contraataque.MezclaRuido = 0.1f;
	Contraataque.FiltroRuido = 0.9f;
	Contraataque.Caida = 0.15f;
	Contraataque.Volumen = 0.6f;
	Contraataque.Semilla = 89;

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

	// Pisada: golpe sordo y corto de bota en piedra.
	Paso.Duracion = 0.09f;
	Paso.FrecuenciaInicio = 160.f;
	Paso.FrecuenciaFin = 90.f;
	Paso.MezclaRuido = 0.85f;
	Paso.FiltroRuido = 0.12f;
	Paso.Ataque = 0.003f;
	Paso.Caida = 0.025f;
	Paso.Volumen = 0.3f;
	Paso.Semilla = 31;

	// Aterrizaje: el mismo golpe, más largo y grave.
	Aterrizaje.Duracion = 0.22f;
	Aterrizaje.FrecuenciaInicio = 110.f;
	Aterrizaje.FrecuenciaFin = 50.f;
	Aterrizaje.MezclaRuido = 0.7f;
	Aterrizaje.FiltroRuido = 0.08f;
	Aterrizaje.Ataque = 0.004f;
	Aterrizaje.Caida = 0.07f;
	Aterrizaje.Volumen = 0.6f;
	Aterrizaje.Semilla = 37;
}

USAEVUSComponenteSonido::USAEVUSComponenteSonido()
{
	// Tick para contar las pisadas.
	PrimaryComponentTick.bCanEverTick = true;
}

void USAEVUSComponenteSonido::Preparar(EEfecto Efecto, const FSAEVUSSonidoSintetizado& Sonido)
{
	FEfecto& Destino = Efectos[static_cast<int32>(Efecto)];
	Destino.Datos = SAEVUSSintesis::SintetizarBytes(Sonido);
	Destino.Duracion = Sonido.Duracion;
}

void USAEVUSComponenteSonido::BeginPlay()
{
	Super::BeginPlay();
	Preparar(EEfecto::Tajo, Ajuste.Tajo);
	Preparar(EEfecto::Golpe, Ajuste.Golpe);
	Preparar(EEfecto::Disparo, Ajuste.Disparo);
	Preparar(EEfecto::GatilloEnSeco, Ajuste.GatilloEnSeco);
	Preparar(EEfecto::Danio, Ajuste.Danio);
	Preparar(EEfecto::Curacion, Ajuste.Curacion);
	Preparar(EEfecto::Bloqueo, Ajuste.Bloqueo);
	Preparar(EEfecto::Contraataque, Ajuste.Contraataque);
	Preparar(EEfecto::Dash, Ajuste.Dash);
	Preparar(EEfecto::DobleSalto, Ajuste.DobleSalto);
	Preparar(EEfecto::Paso, Ajuste.Paso);
	Preparar(EEfecto::Aterrizaje, Ajuste.Aterrizaje);
	AleatorioPasos.Initialize(static_cast<int32>(GetOwner()->GetUniqueID()));

	AActor* Duenio = GetOwner();
	if (USAEVUSComponenteCombate* Combate = Duenio->FindComponentByClass<USAEVUSComponenteCombate>())
	{
		Manejadores.Emplace(Combate, Combate->AlIniciarAtaque.AddUObject(this, &ThisClass::AlIniciarAtaque));
		Manejadores.Emplace(Combate, Combate->AlImpactarEscenario.AddUObject(this, &ThisClass::AlImpactarEscenario));
		Combate->AlGolpear.AddUniqueDynamic(this, &ThisClass::AlGolpear);
	}
	if (USAEVUSComponenteArmas* Armas = Duenio->FindComponentByClass<USAEVUSComponenteArmas>())
	{
		Armas->AlDisparar.AddUniqueDynamic(this, &ThisClass::AlDisparar);
		Armas->AlGatilloEnSeco.AddUniqueDynamic(this, &ThisClass::AlGatilloEnSeco);
	}
	if (USAEVUSComponenteVida* Vida = Duenio->FindComponentByClass<USAEVUSComponenteVida>())
	{
		Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::AlCambiarVida);
	}
	if (USAEVUSComponenteAlma* Alma = Duenio->FindComponentByClass<USAEVUSComponenteAlma>())
	{
		Manejadores.Emplace(Alma, Alma->AlCurar.AddUObject(this, &ThisClass::AlCurar));
	}
	if (USAEVUSComponenteGuardia* Guardia = Duenio->FindComponentByClass<USAEVUSComponenteGuardia>())
	{
		Manejadores.Emplace(Guardia, Guardia->AlBloquear.AddUObject(this, &ThisClass::AlBloquear));
		Manejadores.Emplace(Guardia, Guardia->AlContraatacar.AddUObject(this, &ThisClass::AlContraatacar));
	}
	if (const ACharacter* Personaje = Cast<ACharacter>(Duenio))
	{
		if (USAEVUSComponenteMovimiento* Movimiento = SAEVUSMovimiento::DelActor(Personaje))
		{
			Manejadores.Emplace(Movimiento, Movimiento->AlIniciarDash.AddUObject(this, &ThisClass::AlIniciarDash));
			Manejadores.Emplace(Movimiento, Movimiento->AlSaltoAereo.AddUObject(this, &ThisClass::AlSaltoAereo));
			Manejadores.Emplace(Movimiento, Movimiento->AlAterrizar.AddUObject(this, &ThisClass::AlAterrizar));
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
			Combate->AlImpactarEscenario.Remove(Par.Value);
		}
		else if (USAEVUSComponenteAlma* Alma = Cast<USAEVUSComponenteAlma>(Par.Key.Get()))
		{
			Alma->AlCurar.Remove(Par.Value);
		}
		else if (USAEVUSComponenteGuardia* Guardia = Cast<USAEVUSComponenteGuardia>(Par.Key.Get()))
		{
			Guardia->AlBloquear.Remove(Par.Value);
			Guardia->AlContraatacar.Remove(Par.Value);
		}
		else if (USAEVUSComponenteMovimiento* Movimiento = Cast<USAEVUSComponenteMovimiento>(Par.Key.Get()))
		{
			Movimiento->AlIniciarDash.Remove(Par.Value);
			Movimiento->AlSaltoAereo.Remove(Par.Value);
			Movimiento->AlAterrizar.Remove(Par.Value);
		}
	}
	Manejadores.Reset();
	Super::EndPlay(EndPlayReason);
}

void USAEVUSComponenteSonido::Reproducir(EEfecto Efecto, float VolumenCategoria, float Tono)
{
	const FEfecto& Datos = Efectos[static_cast<int32>(Efecto)];
	if (Datos.Datos.IsEmpty())
	{
		return;
	}
	++Reproducciones;
	// Una onda procedural nueva por reproducción: se consume al sonar.
	UGameplayStatics::PlaySound2D(this, SAEVUSSintesis::CrearOnda(this, Datos.Datos, Datos.Duracion), Ajuste.VolumenGeneral * VolumenCategoria, Tono);
}

void USAEVUSComponenteSonido::AlIniciarAtaque()
{
	// Cada golpe con su tono (datos del ataque): el remate suena grave y pesado, la estocada aguda.
	const USAEVUSComponenteCombate* Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>();
	const FSAEVUSDefinicionAtaque* Ataque = Combate ? Combate->ObtenerAtaqueActual() : nullptr;
	Reproducir(EEfecto::Tajo, Ajuste.VolumenCombate, Ataque ? Ataque->TonoSonido : 1.f);
}

void USAEVUSComponenteSonido::AlImpactarEscenario(AActor* Escenario)
{
	// Un golpe seco contra piedra o madera: el sonido metálico del bloqueo, más grave y suave.
	Reproducir(EEfecto::Bloqueo, Ajuste.VolumenCombate * 0.6f, 0.8f);
}

void USAEVUSComponenteSonido::AlGolpear(AActor* ActorGolpeado, float Cantidad)
{
	Reproducir(EEfecto::Golpe, Ajuste.VolumenCombate);
}

void USAEVUSComponenteSonido::AlDisparar(FVector Boca, FVector Impacto, AActor* ActorGolpeado)
{
	Reproducir(EEfecto::Disparo, Ajuste.VolumenCombate);
}

void USAEVUSComponenteSonido::AlGatilloEnSeco()
{
	Reproducir(EEfecto::GatilloEnSeco, Ajuste.VolumenCombate);
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

void USAEVUSComponenteSonido::AlBloquear(AActor* Atacante, const FVector& PuntoContacto)
{
	Reproducir(EEfecto::Bloqueo, Ajuste.VolumenCombate);
}

void USAEVUSComponenteSonido::AlContraatacar(AActor* Atacante, const FVector& PuntoContacto)
{
	Reproducir(EEfecto::Contraataque, Ajuste.VolumenCombate);
}

void USAEVUSComponenteSonido::AlIniciarDash(const FVector& Direccion)
{
	Reproducir(EEfecto::Dash, Ajuste.VolumenMovimiento);
}

void USAEVUSComponenteSonido::AlSaltoAereo()
{
	Reproducir(EEfecto::DobleSalto, Ajuste.VolumenMovimiento);
}

void USAEVUSComponenteSonido::AlAterrizar(float VelocidadCaida)
{
	// Un bajón de nada (escalón, estocada que roza el suelo) no suena como un aterrizaje.
	if (VelocidadCaida < Ajuste.VelocidadMinimaAterrizaje)
	{
		RecorridoPaso = 0.f;
		return;
	}
	const float Fuerza = FMath::Clamp(VelocidadCaida / Ajuste.VelocidadCaidaAterrizaje, 0.2f, 1.f);
	Reproducir(EEfecto::Aterrizaje, Ajuste.VolumenMovimiento * Fuerza, FMath::Lerp(1.15f, 0.85f, Fuerza));
	RecorridoPaso = 0.f;
}

void USAEVUSComponenteSonido::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const USAEVUSComponenteMovimiento* Movimiento = SAEVUSMovimiento::DelActor(GetOwner());
	const USAEVUSComponenteCombate* Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>();
	// Sin pisadas mientras se ataca o se desliza en un dash/estocada: no son pasos.
	if (!Movimiento || !Movimiento->IsMovingOnGround() || Movimiento->EstaEnDash() || Movimiento->EstaEnEstocada() || (Combate && Combate->EstaAtacando()))
	{
		return;
	}
	// Solo hay pisadas si el jugador anda de verdad (entrada), no con la inercia que queda tras un tajo o un dash.
	if (Movimiento->GetCurrentAcceleration().IsNearlyZero())
	{
		return;
	}
	// Una pisada cada DistanciaPaso andada (dos por ciclo de zancada), con el tono algo cambiado.
	RecorridoPaso += static_cast<float>(Movimiento->Velocity.Size2D()) * DeltaTime;
	if (RecorridoPaso >= Ajuste.DistanciaPaso)
	{
		RecorridoPaso = FMath::Fmod(RecorridoPaso, Ajuste.DistanciaPaso);
		Reproducir(EEfecto::Paso, Ajuste.VolumenMovimiento, 1.f + AleatorioPasos.FRandRange(-Ajuste.VariacionTonoPaso, Ajuste.VariacionTonoPaso));
	}
}
