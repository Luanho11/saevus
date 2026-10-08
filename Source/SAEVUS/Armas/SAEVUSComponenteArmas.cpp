// Copyright SAEVUS. Todos los derechos reservados.

#include "Armas/SAEVUSComponenteArmas.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Nucleo/SAEVUSCanalesColision.h"

namespace SAEVUSArmas
{
	// Dibujo de depuración de los disparos (SAEVUS.Depuracion.Combate).
	constexpr float DuracionDisparoDepuracion = 1.f;
}

USAEVUSComponenteArmas::USAEVUSComponenteArmas()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USAEVUSComponenteArmas::BeginPlay()
{
	Super::BeginPlay();
	// Cada partida dispersa distinto (sin semilla, todas repetían los mismos tiros).
	FlujoDispersion.Initialize(static_cast<int32>(FPlatformTime::Cycles()));
	RellenarMunicion();
	if (USAEVUSComponenteCombate* Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>())
	{
		Combate->AlGolpear.AddUniqueDynamic(this, &ThisClass::AlGolpearCuerpoACuerpo);
	}
	if (Arsenal.IsValidIndex(RanuraInicial))
	{
		Equipar(RanuraInicial);
	}
}

void USAEVUSComponenteArmas::FijarArsenal(const TArray<const USAEVUSDefinicionArma*>& NuevoArsenal, int32 RanuraInicio)
{
	Arsenal.Reset();
	for (const USAEVUSDefinicionArma* Arma : NuevoArsenal)
	{
		Arsenal.Add(Arma);
	}
	RanuraActual = RanuraPendiente = INDEX_NONE;
	TiempoCambioRestante = TiempoRecargaRestante = EnfriamientoDisparo = 0.f;
	bDisparoPedido = false;
	RellenarMunicion();
	if (Arsenal.IsValidIndex(RanuraInicio))
	{
		Equipar(RanuraInicio);
	}
}

void USAEVUSComponenteArmas::RellenarMunicion()
{
	Cargadores.SetNumZeroed(Arsenal.Num());
	Reservas.SetNumZeroed(Arsenal.Num());
	for (int32 Ranura = 0; Ranura < Arsenal.Num(); ++Ranura)
	{
		const USAEVUSDefinicionArma* Arma = Arsenal[Ranura];
		const bool bFuego = Arma && Arma->EsArmaFuego();
		Cargadores[Ranura] = bFuego ? Arma->ArmaFuego.TamanoCargador : 0;
		Reservas[Ranura] = !bFuego ? 0 : (Arma->ArmaFuego.TieneReservaInfinita() ? INDEX_NONE : Arma->ArmaFuego.MunicionReserva);
	}
}

void USAEVUSComponenteArmas::AlGolpearCuerpoACuerpo(AActor* ActorGolpeado, float Danio)
{
	for (int32 Ranura = 0; Ranura < Arsenal.Num(); ++Ranura)
	{
		const USAEVUSDefinicionArma* Arma = Arsenal[Ranura];
		if (Arma && Arma->EsArmaFuego() && Reservas[Ranura] != INDEX_NONE)
		{
			Reservas[Ranura] = FMath::Min(Reservas[Ranura] + Arma->ArmaFuego.ReservaPorGolpeCuerpoACuerpo, Arma->ArmaFuego.MunicionReserva);
		}
	}
}

void USAEVUSComponenteArmas::CompletarRecarga()
{
	const USAEVUSDefinicionArma* Arma = ObtenerArmaActual();
	if (!Arma || !Arma->EsArmaFuego())
	{
		return;
	}
	const int32 Faltan = FMath::Max(Arma->ArmaFuego.TamanoCargador - Cargadores[RanuraActual], 0);
	const bool bInfinita = Reservas[RanuraActual] == INDEX_NONE;
	const int32 Pasan = bInfinita ? Faltan : FMath::Min(Faltan, Reservas[RanuraActual]);
	Cargadores[RanuraActual] += Pasan;
	if (!bInfinita)
	{
		Reservas[RanuraActual] -= Pasan;
	}
}

const USAEVUSDefinicionArma* USAEVUSComponenteArmas::ObtenerArmaActual() const
{
	return Arsenal.IsValidIndex(RanuraActual) ? Arsenal[RanuraActual].Get() : nullptr;
}

bool USAEVUSComponenteArmas::TieneArmaFuegoEquipada() const
{
	const USAEVUSDefinicionArma* Arma = ObtenerArmaActual();
	return Arma && Arma->EsArmaFuego();
}

int32 USAEVUSComponenteArmas::ObtenerMunicion(int32 Ranura) const
{
	return Cargadores.IsValidIndex(Ranura) ? Cargadores[Ranura] : 0;
}

int32 USAEVUSComponenteArmas::ObtenerReserva(int32 Ranura) const
{
	return Reservas.IsValidIndex(Ranura) ? Reservas[Ranura] : 0;
}

bool USAEVUSComponenteArmas::PuedeCambiarA(int32 Ranura) const
{
	return Arsenal.IsValidIndex(Ranura) && Arsenal[Ranura] && Ranura != RanuraPendiente && (Ranura != RanuraActual || EstaCambiando());
}

bool USAEVUSComponenteArmas::PedirCambio(int32 Ranura)
{
	if (!PuedeCambiarA(Ranura))
	{
		return false;
	}
	// Cambiar descarta una recarga en curso y cualquier disparo pendiente.
	TiempoRecargaRestante = 0.f;
	bDisparoPedido = false;
	RanuraPendiente = Ranura;
	TiempoCambioRestante = Arsenal[Ranura]->TiempoEquipar;
	if (TiempoCambioRestante <= 0.f)
	{
		Equipar(Ranura);
	}
	return true;
}

int32 USAEVUSComponenteArmas::ObtenerSiguienteRanura() const
{
	if (Arsenal.Num() < 2)
	{
		return INDEX_NONE;
	}
	const int32 Desde = EstaCambiando() ? RanuraPendiente : RanuraActual;
	return (FMath::Max(Desde, 0) + 1) % Arsenal.Num();
}

void USAEVUSComponenteArmas::Equipar(int32 Ranura)
{
	RanuraActual = Ranura;
	RanuraPendiente = INDEX_NONE;
	TiempoCambioRestante = 0.f;

	const USAEVUSDefinicionArma* Arma = ObtenerArmaActual();
	if (Arma && !Arma->EsArmaFuego())
	{
		if (USAEVUSComponenteCombate* Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>())
		{
			Combate->FijarRepertorio(Arma->Repertorio);
		}
	}
}

void USAEVUSComponenteArmas::FijarGatillo(bool bPresionado)
{
	bGatilloPresionado = bPresionado;
	if (bPresionado)
	{
		bDisparoPedido = true;
	}
}

void USAEVUSComponenteArmas::PedirRecarga()
{
	const USAEVUSDefinicionArma* Arma = ObtenerArmaActual();
	if (!Arma || !Arma->EsArmaFuego() || EstaCambiando() || EstaRecargando() || ObtenerMunicionActual() >= Arma->ArmaFuego.TamanoCargador
		|| ObtenerReservaActual() == 0)
	{
		return;
	}
	TiempoRecargaRestante = Arma->ArmaFuego.TiempoRecarga;
	bDisparoPedido = false;
	if (TiempoRecargaRestante <= 0.f)
	{
		CompletarRecarga();
	}
}

void USAEVUSComponenteArmas::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (EstaCambiando())
	{
		TiempoCambioRestante -= DeltaTime;
		if (TiempoCambioRestante <= 0.f)
		{
			Equipar(RanuraPendiente);
		}
	}
	if (EstaRecargando())
	{
		TiempoRecargaRestante -= DeltaTime;
		if (TiempoRecargaRestante <= 0.f)
		{
			TiempoRecargaRestante = 0.f;
			CompletarRecarga();
		}
	}
	IntentarDisparar(DeltaTime);
}

void USAEVUSComponenteArmas::IntentarDisparar(float DeltaTime)
{
	// Se arrastra como mucho un fotograma de exceso, así el fuego automático mantiene su cadencia sin acumular disparos.
	EnfriamientoDisparo = FMath::Max(EnfriamientoDisparo - DeltaTime, -DeltaTime);

	const USAEVUSDefinicionArma* Arma = ObtenerArmaActual();
	if (!Arma || !Arma->EsArmaFuego() || EstaCambiando() || EstaRecargando())
	{
		return;
	}
	const FSAEVUSAjusteArmaFuego& Ajuste = Arma->ArmaFuego;
	const bool bQuiereDisparar = bDisparoPedido || (bGatilloPresionado && Ajuste.bAutomatica);
	if (!bQuiereDisparar || EnfriamientoDisparo > 0.f)
	{
		return;
	}
	bDisparoPedido = false;
	if (Cargadores[RanuraActual] <= 0)
	{
		// Gatillo en seco: recarga en su lugar, o el chasquido si no queda reserva.
		if (ObtenerReservaActual() == 0)
		{
			EnfriamientoDisparo += Ajuste.IntervaloDisparo;
			AlGatilloEnSeco.Broadcast();
			return;
		}
		PedirRecarga();
		return;
	}
	--Cargadores[RanuraActual];
	EnfriamientoDisparo += Ajuste.IntervaloDisparo;
	Disparar(Ajuste);
}

void USAEVUSComponenteArmas::ObtenerPuntoVista(FVector& OutPosicion, FRotator& OutRotacion) const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->GetController())
	{
		Pawn->GetController()->GetPlayerViewPoint(OutPosicion, OutRotacion);
		return;
	}
	GetOwner()->GetActorEyesViewPoint(OutPosicion, OutRotacion);
}

void USAEVUSComponenteArmas::Disparar(const FSAEVUSAjusteArmaFuego& Ajuste)
{
	AActor* Duenio = GetOwner();
	UWorld* Mundo = GetWorld();
	if (!Duenio || !Mundo)
	{
		return;
	}
	++DisparosRealizados;

	FVector PosicionVista;
	FRotator RotacionVista;
	ObtenerPuntoVista(PosicionVista, RotacionVista);
	const FVector DireccionApunte = RotacionVista.Vector();
	const FVector Boca = Duenio->GetActorTransform().TransformPosition(DesplazamientoBoca);

	// El cuerpo gira hacia donde va el disparo.
	if (const ACharacter* Personaje = Cast<ACharacter>(Duenio))
	{
		if (USAEVUSComponenteMovimiento* Movimiento = SAEVUSMovimiento::DelActor(Personaje))
		{
			Movimiento->GirarHacia(DireccionApunte);
		}
	}

	FCollisionQueryParams ParametrosConsulta(SCENE_QUERY_STAT(SAEVUSDisparo), true, Duenio);
	const float DispersionRadianes = FMath::DegreesToRadians(Ajuste.DispersionGrados);
	// En tercera persona el rayo de vista empieza detrás del personaje, así que es más largo que el alcance.
	const float LongitudRayoVista = Ajuste.Alcance + FVector::Dist(PosicionVista, Boca);

	for (int32 Perdigon = 0; Perdigon < Ajuste.PerdigonesPorDisparo; ++Perdigon)
	{
		const FVector Direccion = DispersionRadianes > 0.f ? FlujoDispersion.VRandCone(DireccionApunte, DispersionRadianes) : DireccionApunte;

		// 1) A qué apunta el rayo de vista: el punto de mira.
		FHitResult GolpeVista;
		const FVector FinVista = PosicionVista + Direccion * LongitudRayoVista;
		const FVector PuntoMira = Mundo->LineTraceSingleByChannel(GolpeVista, PosicionVista, FinVista, SAEVUSColision::TrazaArma, ParametrosConsulta) ? GolpeVista.ImpactPoint : FinVista;

		// 2) El disparo en sí, desde la boca hacia el punto de mira, así cuenta la cobertura delante del personaje.
		FVector DireccionDisparo = (PuntoMira - Boca).GetSafeNormal();
		if (FVector::DotProduct(DireccionDisparo, DireccionApunte) <= 0.f)
		{
			// Punto de mira detrás de la boca (algo entre cámara y personaje): se dispara según la vista.
			DireccionDisparo = Direccion;
		}
		const FVector FinDisparo = Boca + DireccionDisparo * Ajuste.Alcance;
		FHitResult GolpeDisparo;
		const bool bGolpe = Mundo->LineTraceSingleByChannel(GolpeDisparo, Boca, FinDisparo, SAEVUSColision::TrazaArma, ParametrosConsulta);
		AActor* ActorGolpeado = bGolpe ? GolpeDisparo.GetActor() : nullptr;
		const FVector Impacto = bGolpe ? GolpeDisparo.ImpactPoint : FinDisparo;

		if (ActorGolpeado)
		{
			UGameplayStatics::ApplyPointDamage(ActorGolpeado, Ajuste.Danio, DireccionDisparo, GolpeDisparo, Duenio->GetInstigatorController(), Duenio, UDamageType::StaticClass());
		}
		if (SAEVUSDepuracion::DibujoCombateActivo())
		{
			DrawDebugLine(Mundo, Boca, Impacto, ActorGolpeado ? FColor::Red : FColor::Yellow, false, SAEVUSArmas::DuracionDisparoDepuracion);
		}
		AlDisparar.Broadcast(Boca, Impacto, ActorGolpeado);
	}
}
