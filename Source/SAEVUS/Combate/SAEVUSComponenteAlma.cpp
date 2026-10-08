// Copyright SAEVUS. Todos los derechos reservados.

#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "GameFramework/Character.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"

USAEVUSComponenteAlma::USAEVUSComponenteAlma()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USAEVUSComponenteAlma::BeginPlay()
{
	Super::BeginPlay();
	Vida = GetOwner()->FindComponentByClass<USAEVUSComponenteVida>();
	Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>();
	if (Vida)
	{
		Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::AlCambiarVida);
	}
	if (Combate)
	{
		Combate->AlGolpear.AddUniqueDynamic(this, &ThisClass::AlGolpear);
	}
}

USAEVUSComponenteMovimiento* USAEVUSComponenteAlma::ObtenerMovimiento() const
{
	return SAEVUSMovimiento::DelActor(GetOwner());
}

void USAEVUSComponenteAlma::GanarAlma(float Cantidad)
{
	const float Antes = Alma;
	Alma = FMath::Clamp(Alma + Cantidad, 0.f, Ajuste.AlmaMaxima);
	if (Alma > Antes)
	{
		AlGanarAlma.Broadcast(Alma - Antes);
	}
}

void USAEVUSComponenteAlma::AlGolpear(AActor* ActorGolpeado, float Danio)
{
	GanarAlma(Ajuste.AlmaPorGolpe);
}

void USAEVUSComponenteAlma::AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador)
{
	if (Delta < 0.f)
	{
		// Un golpe corta la curación; hay que volver a pulsar.
		CancelarCuracion();
	}
}

void USAEVUSComponenteAlma::FijarCuracionPedida(bool bPedida)
{
	if (bPedida)
	{
		bCuracionPedida = true;
	}
	else
	{
		CancelarCuracion();
	}
}

void USAEVUSComponenteAlma::CancelarCuracion()
{
	// Cortada por una acción o un golpe: no se reanuda hasta una nueva pulsación.
	bCuracionPedida = false;
	PausarCuracion();
}

void USAEVUSComponenteAlma::PausarCuracion()
{
	TiempoCuracion = 0.f;
	FijarCurando(false);
}

float USAEVUSComponenteAlma::ObtenerProgresoCuracion() const
{
	return bCurando ? FMath::Clamp(TiempoCuracion / Ajuste.DuracionCuracion, 0.f, 1.f) : 0.f;
}

bool USAEVUSComponenteAlma::PuedeCurar() const
{
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	return Vida && !Vida->EstaMuerto() && Vida->ObtenerVida() < Vida->ObtenerVidaMaxima() && Alma >= Ajuste.CosteCuracion
		&& Movimiento && Movimiento->IsMovingOnGround() && !Movimiento->EstaEnDash() && !(Combate && Combate->EstaAtacando());
}

void USAEVUSComponenteAlma::FijarCurando(bool bNuevo)
{
	if (bCurando == bNuevo)
	{
		return;
	}
	bCurando = bNuevo;
	// Curar ancla al personaje: sin velocidad en el suelo mientras dura.
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		Movimiento->FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad::Curacion, bCurando ? 0.f : 1.f);
	}
}

void USAEVUSComponenteAlma::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bCuracionPedida || !PuedeCurar())
	{
		// Sin poder curar (en el aire, sin alma...) se pausa; mantenida, empieza en cuanto se pueda.
		PausarCuracion();
		return;
	}
	FijarCurando(true);
	TiempoCuracion += DeltaTime;
	if (TiempoCuracion >= Ajuste.DuracionCuracion)
	{
		TiempoCuracion = 0.f;
		Alma -= Ajuste.CosteCuracion;
		const float Recuperada = Vida->Curar(Ajuste.VidaPorCuracion);
		AlCurar.Broadcast(Recuperada);
	}
}
