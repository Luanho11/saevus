// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponenteEstelaDash.h"
#include "GameFramework/Character.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"

USAEVUSComponenteMovimiento* USAEVUSComponenteEstelaDash::ObtenerMovimiento() const
{
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	return Personaje ? Cast<USAEVUSComponenteMovimiento>(Personaje->GetCharacterMovement()) : nullptr;
}

void USAEVUSComponenteEstelaDash::BeginPlay()
{
	Super::BeginPlay();
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		ManejadorDash = Movimiento->AlIniciarDash.AddUObject(this, &ThisClass::AlIniciarDash);
	}
}

void USAEVUSComponenteEstelaDash::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		Movimiento->AlIniciarDash.Remove(ManejadorDash);
	}
	Super::EndPlay(EndPlayReason);
}

FVector USAEVUSComponenteEstelaDash::ObtenerPuntoEmision() const
{
	return GetOwner()->GetActorLocation() + FVector(0.f, 0.f, AjusteEstela.AlturaEmision);
}

void USAEVUSComponenteEstelaDash::AlIniciarDash(const FVector& Direccion)
{
	AcumuladoEmision = 0.f;
	EmitirRafaga(ObtenerPuntoEmision(), -Direccion * AjusteEstela.VelocidadRetroceso, AjusteEstela.ParticulasRafaga, AjusteEstela.Dispersion);
}

void USAEVUSComponenteEstelaDash::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	if (Movimiento && Movimiento->EstaEnDash())
	{
		AcumuladoEmision += AjusteEstela.ParticulasPorSegundo * DeltaTime;
		const int32 Cantidad = FMath::FloorToInt32(AcumuladoEmision);
		AcumuladoEmision -= Cantidad;
		if (Cantidad > 0)
		{
			EmitirRafaga(ObtenerPuntoEmision(), -Movimiento->ObtenerDireccionDash() * AjusteEstela.VelocidadRetroceso, Cantidad, AjusteEstela.Dispersion);
		}
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
