// Copyright SAEVUS. Todos los derechos reservados.

#include "Combate/SAEVUSComponenteCombate.h"
#include "CollisionQueryParams.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"

namespace SAEVUSCombate
{
	const FSAEVUSRepertorioMelee RepertorioBase;

	// Dibujo de depuración de la esfera de golpe (SAEVUS.Depuracion.Combate).
	constexpr int32 SegmentosEsferaDepuracion = 12;
}

USAEVUSComponenteCombate::USAEVUSComponenteCombate()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

const FSAEVUSRepertorioMelee& USAEVUSComponenteCombate::ObtenerRepertorio() const
{
	return DatosRepertorio ? DatosRepertorio->Repertorio : SAEVUSCombate::RepertorioBase;
}

void USAEVUSComponenteCombate::FijarRepertorio(const USAEVUSDatosRepertorioMelee* NuevoRepertorio)
{
	// AtaqueActual apunta al repertorio anterior.
	TerminarAtaque();
	DatosRepertorio = NuevoRepertorio;
}

ACharacter* USAEVUSComponenteCombate::ObtenerPersonaje() const
{
	return Cast<ACharacter>(GetOwner());
}

USAEVUSComponenteMovimiento* USAEVUSComponenteCombate::ObtenerMovimiento() const
{
	const ACharacter* Personaje = ObtenerPersonaje();
	return Personaje ? Cast<USAEVUSComponenteMovimiento>(Personaje->GetCharacterMovement()) : nullptr;
}

void USAEVUSComponenteCombate::PedirAtaque()
{
	TiempoBufferAtaqueRestante = ObtenerRepertorio().TiempoBufferEntrada;
	// Empieza en el acto si se puede, así una pulsación no espera un fotograma.
	ProcesarAtaqueEnBuffer();
}

bool USAEVUSComponenteCombate::IntentarCancelarPor(ESAEVUSAccionCancelacion Accion)
{
	if (!AtaqueActual)
	{
		return true;
	}
	const float CancelarDesde = AtaqueActual->ObtenerCancelacionDesde(Accion);
	if (CancelarDesde < 0.f || TiempoAtaque < CancelarDesde)
	{
		return false;
	}
	TerminarAtaque();
	return true;
}

ESAEVUSFaseAtaque USAEVUSComponenteCombate::ObtenerFaseAtaque() const
{
	return AtaqueActual ? AtaqueActual->ObtenerFaseEn(TiempoAtaque) : ESAEVUSFaseAtaque::Ninguna;
}

void USAEVUSComponenteCombate::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AtaqueActual)
	{
		const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
		if (Movimiento && (Movimiento->EstaEnCarreraPared() || Movimiento->EstaEscalando()))
		{
			// El movimiento en pared se queda con el cuerpo; el ataque se descarta.
			TerminarAtaque();
		}
	}

	if (AtaqueActual)
	{
		TiempoAtaque += DeltaTime;
		if (AtaqueActual->ObtenerFaseEn(TiempoAtaque) == ESAEVUSFaseAtaque::Activa)
		{
			DetectarGolpes(*AtaqueActual);
		}
		if (TiempoAtaque >= AtaqueActual->ObtenerTiempoTotal())
		{
			TerminarAtaque();
		}
	}

	ProcesarAtaqueEnBuffer();
	TiempoBufferAtaqueRestante = FMath::Max(0.f, TiempoBufferAtaqueRestante - DeltaTime);
}

void USAEVUSComponenteCombate::ProcesarAtaqueEnBuffer()
{
	if (TiempoBufferAtaqueRestante <= 0.f || !PuedeAtacarAhora())
	{
		return;
	}
	if (!AtaqueActual)
	{
		const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
		IniciarAtaque(Movimiento && Movimiento->IsFalling(), 0);
		return;
	}
	// Cadena: el siguiente ataque de la misma cadena, en cuanto el actual lo permite.
	const TArray<FSAEVUSDefinicionAtaque>& Cadena = bCadenaAerea ? ObtenerRepertorio().ComboAire : ObtenerRepertorio().ComboSuelo;
	if (TiempoAtaque >= AtaqueActual->EncadenarDesde && Cadena.IsValidIndex(IndiceCombo + 1))
	{
		IniciarAtaque(bCadenaAerea, IndiceCombo + 1);
	}
}

bool USAEVUSComponenteCombate::PuedeAtacarAhora() const
{
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	if (!Movimiento || !(Movimiento->IsMovingOnGround() || Movimiento->IsFalling()))
	{
		return false;
	}
	return !Movimiento->EstaEnDash() || ObtenerRepertorio().bAtaqueTerminaDash;
}

void USAEVUSComponenteCombate::IniciarAtaque(bool bEnAire, int32 Indice)
{
	const TArray<FSAEVUSDefinicionAtaque>& Cadena = bEnAire ? ObtenerRepertorio().ComboAire : ObtenerRepertorio().ComboSuelo;
	USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	if (!Cadena.IsValidIndex(Indice) || !Movimiento)
	{
		return;
	}
	const FSAEVUSDefinicionAtaque& Ataque = Cadena[Indice];

	AtaqueActual = &Ataque;
	IndiceCombo = Indice;
	bCadenaAerea = bEnAire;
	TiempoAtaque = 0.f;
	TiempoBufferAtaqueRestante = 0.f;
	ActoresGolpeados.Reset();

	// Ataca hacia la entrada de movimiento si la hay; si no, de frente.
	const FVector DireccionEntrada = Movimiento->GetCurrentAcceleration().GetSafeNormal2D();
	const FVector Direccion = DireccionEntrada.IsNearlyZero() ? GetOwner()->GetActorForwardVector().GetSafeNormal2D() : DireccionEntrada;

	Movimiento->CancelarDash();
	Movimiento->GirarHacia(Direccion);
	Movimiento->IniciarEstocada(Direccion, Ataque.VelocidadEstocada, Ataque.DuracionEstocada);
	Movimiento->FijarEscalaVelocidadAccion(Ataque.EscalaVelocidadSuelo);
}

void USAEVUSComponenteCombate::TerminarAtaque()
{
	AtaqueActual = nullptr;
	IndiceCombo = INDEX_NONE;
	TiempoAtaque = 0.f;
	ActoresGolpeados.Reset();
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		Movimiento->FijarEscalaVelocidadAccion(1.f);
	}
}

void USAEVUSComponenteCombate::DetectarGolpes(const FSAEVUSDefinicionAtaque& Ataque)
{
	AActor* Duenio = GetOwner();
	UWorld* Mundo = GetWorld();
	if (!Duenio || !Mundo)
	{
		return;
	}
	const FVector Centro = Duenio->GetActorLocation() + Duenio->GetActorForwardVector() * Ataque.DesplazamientoFrontalGolpe + FVector::UpVector * Ataque.AlturaGolpe;

	FCollisionObjectQueryParams TiposObjeto;
	TiposObjeto.AddObjectTypesToQuery(ECC_Pawn);
	TiposObjeto.AddObjectTypesToQuery(ECC_PhysicsBody);
	TiposObjeto.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams ParametrosConsulta(SCENE_QUERY_STAT(SAEVUSAtaque), false, Duenio);

	TArray<FOverlapResult> Solapes;
	Mundo->OverlapMultiByObjectType(Solapes, Centro, FQuat::Identity, TiposObjeto, FCollisionShape::MakeSphere(Ataque.RadioGolpe), ParametrosConsulta);

	if (SAEVUSDepuracion::DibujoCombateActivo())
	{
		DrawDebugSphere(Mundo, Centro, Ataque.RadioGolpe, SAEVUSCombate::SegmentosEsferaDepuracion, Solapes.IsEmpty() ? FColor::Yellow : FColor::Red);
	}

	for (const FOverlapResult& Solape : Solapes)
	{
		AActor* Objetivo = Solape.GetActor();
		if (!Objetivo || Objetivo == Duenio || ActoresGolpeados.Contains(Objetivo))
		{
			continue;
		}
		ActoresGolpeados.Add(Objetivo);
		UGameplayStatics::ApplyDamage(Objetivo, Ataque.Danio, Duenio->GetInstigatorController(), Duenio, UDamageType::StaticClass());
		AlGolpear.Broadcast(Objetivo, Ataque.Danio);
	}
}
