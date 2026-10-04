// Copyright SAEVUS. Todos los derechos reservados.

#include "Combate/SAEVUSComponenteGuardia.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"

USAEVUSComponenteGuardia::USAEVUSComponenteGuardia()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USAEVUSComponenteGuardia::BeginPlay()
{
	Super::BeginPlay();
	Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>();
	Armas = GetOwner()->FindComponentByClass<USAEVUSComponenteArmas>();
	Vida = GetOwner()->FindComponentByClass<USAEVUSComponenteVida>();
	if (Vida)
	{
		Vida->FiltroDanio.BindUObject(this, &ThisClass::FiltrarDanio);
	}
}

void USAEVUSComponenteGuardia::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Vida)
	{
		Vida->FiltroDanio.Unbind();
	}
	Super::EndPlay(EndPlayReason);
}

USAEVUSComponenteMovimiento* USAEVUSComponenteGuardia::ObtenerMovimiento() const
{
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	return Personaje ? Cast<USAEVUSComponenteMovimiento>(Personaje->GetCharacterMovement()) : nullptr;
}

void USAEVUSComponenteGuardia::FijarGuardiaPedida(bool bNueva)
{
	bPedida = bNueva;
	if (!bPedida)
	{
		Salir();
	}
}

void USAEVUSComponenteGuardia::Interrumpir()
{
	Salir();
}

bool USAEVUSComponenteGuardia::PuedeEntrar() const
{
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	const bool bCuerpoACuerpo = !Armas || (!Armas->TieneArmaFuegoEquipada() && !Armas->EstaCambiando());
	const bool bVivo = !Vida || !Vida->EstaMuerto();
	const bool bMovimientoLibre = Movimiento && (Movimiento->IsMovingOnGround() || Movimiento->IsFalling()) && !Movimiento->EstaEnDash();
	return bCuerpoACuerpo && bVivo && bMovimientoLibre;
}

void USAEVUSComponenteGuardia::Entrar()
{
	if (bEnGuardia)
	{
		return;
	}
	bEnGuardia = true;
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		Movimiento->FijarEscalaVelocidadAccion(Ajuste.EscalaVelocidad);
		// En guardia se encara hacia donde mira la cámara, no hacia donde se camina.
		bOrientabaAlMovimiento = Movimiento->bOrientRotationToMovement;
		Movimiento->bOrientRotationToMovement = false;
	}
}

void USAEVUSComponenteGuardia::Salir()
{
	if (!bEnGuardia)
	{
		return;
	}
	bEnGuardia = false;
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		Movimiento->FijarEscalaVelocidadAccion(1.f);
		Movimiento->bOrientRotationToMovement = bOrientabaAlMovimiento;
	}
}

void USAEVUSComponenteGuardia::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnGuardia && !PuedeEntrar())
	{
		Salir();
	}
	// Pedida durante un ataque: entra en cuanto la ventana de cancelación del ataque lo permite.
	if (bPedida && !bEnGuardia && PuedeEntrar() && (!Combate || Combate->IntentarCancelarPor(ESAEVUSAccionCancelacion::Guardia)))
	{
		Entrar();
	}
	if (bEnGuardia)
	{
		const APawn* Pawn = Cast<APawn>(GetOwner());
		const AController* Controlador = Pawn ? Pawn->GetController() : nullptr;
		USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
		if (Controlador && Movimiento)
		{
			Movimiento->GirarHacia(FRotator(0.f, Controlador->GetControlRotation().Yaw, 0.f).Vector());
		}
	}
}

bool USAEVUSComponenteGuardia::CubreDesde(const FVector& Posicion) const
{
	const FVector Hacia = (Posicion - GetOwner()->GetActorLocation()).GetSafeNormal2D();
	if (Hacia.IsNearlyZero())
	{
		return true;
	}
	const float Coseno = FVector::DotProduct(Hacia, GetOwner()->GetActorForwardVector().GetSafeNormal2D());
	return Coseno >= FMath::Cos(FMath::DegreesToRadians(Ajuste.MediaAperturaCobertura));
}

float USAEVUSComponenteGuardia::FiltrarDanio(float Cantidad, AActor* Causante)
{
	// Sin causante (caídas, peligros) o desde fuera del cono, la guardia no sirve.
	if (!bEnGuardia || !Causante || Causante == GetOwner() || !CubreDesde(Causante->GetActorLocation()))
	{
		return Cantidad;
	}
	++Bloqueos;
	ReaccionarBloqueo(Causante);
	return Cantidad * (1.f - Ajuste.FraccionBloqueada);
}

void USAEVUSComponenteGuardia::ReaccionarBloqueo(AActor* Atacante)
{
	const double Ahora = GetWorld()->GetTimeSeconds();
	if (Ahora - UltimaReaccion < Ajuste.TiempoEntreReacciones)
	{
		return;
	}
	UltimaReaccion = Ahora;
	const FVector HaciaAtacante = (Atacante->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D();
	if (ACharacter* Personaje = Cast<ACharacter>(GetOwner()))
	{
		Personaje->LaunchCharacter(-HaciaAtacante * Ajuste.RetrocesoPropio, true, false);
	}
	if (ACharacter* PersonajeAtacante = Cast<ACharacter>(Atacante))
	{
		PersonajeAtacante->LaunchCharacter(HaciaAtacante * Ajuste.EmpujeAtacante, true, false);
	}
	AlBloquear.Broadcast(Atacante, GetOwner()->GetActorLocation() + HaciaAtacante * Ajuste.DistanciaContacto);
}
