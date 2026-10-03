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
	PedirAtaqueEnDireccion(ESAEVUSDireccionAtaque::Frontal);
}

void USAEVUSComponenteCombate::PedirAtaqueEnDireccion(ESAEVUSDireccionAtaque Direccion)
{
	DireccionPedida = Direccion;
	TiempoBufferAtaqueRestante = ObtenerRepertorio().TiempoBufferEntrada;
	// Empieza en el acto si se puede, así una pulsación no espera un fotograma.
	ProcesarAtaqueEnBuffer();
}

ESAEVUSDireccionAtaque USAEVUSComponenteCombate::DireccionSegunInclinacion(float InclinacionGrados) const
{
	const FSAEVUSRepertorioMelee& Repertorio = ObtenerRepertorio();
	if (InclinacionGrados >= Repertorio.UmbralInclinacionArriba)
	{
		return ESAEVUSDireccionAtaque::Arriba;
	}
	return InclinacionGrados <= Repertorio.UmbralInclinacionAbajo ? ESAEVUSDireccionAtaque::Abajo : ESAEVUSDireccionAtaque::Frontal;
}

void USAEVUSComponenteCombate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ParadaGolpe.Terminar();
	Super::EndPlay(EndPlayReason);
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

	// La parada se cuenta en tiempo real: DeltaTime ya viene dilatado por la parada del dueño.
	ParadaGolpe.Actualizar(DeltaTime / FMath::Max(GetOwner()->CustomTimeDilation, UE_KINDA_SMALL_NUMBER));

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
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	const bool bEnAire = Movimiento && Movimiento->IsFalling();
	// Abajo solo existe en el aire.
	const ESAEVUSDireccionAtaque Direccion = (DireccionPedida == ESAEVUSDireccionAtaque::Abajo && !bEnAire) ? ESAEVUSDireccionAtaque::Frontal : DireccionPedida;
	if (!AtaqueActual)
	{
		if (Direccion == ESAEVUSDireccionAtaque::Frontal)
		{
			IniciarAtaque(bEnAire, 0);
		}
		else
		{
			IniciarAtaqueDireccional(bEnAire, Direccion);
		}
		return;
	}
	if (TiempoAtaque < AtaqueActual->EncadenarDesde)
	{
		return;
	}
	// Un tajo direccional sale desde la ventana de encadenar de cualquier ataque, y tras él cualquier otro.
	if (Direccion != ESAEVUSDireccionAtaque::Frontal)
	{
		IniciarAtaqueDireccional(bEnAire, Direccion);
		return;
	}
	if (DireccionAtaque != ESAEVUSDireccionAtaque::Frontal)
	{
		IniciarAtaque(bEnAire, 0);
		return;
	}
	// Cadena: el siguiente ataque de la misma cadena, en cuanto el actual lo permite.
	const TArray<FSAEVUSDefinicionAtaque>& Cadena = bCadenaAerea ? ObtenerRepertorio().ComboAire : ObtenerRepertorio().ComboSuelo;
	if (Cadena.IsValidIndex(IndiceCombo + 1))
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
	IndiceCombo = Indice;
	bCadenaAerea = bEnAire;
	DireccionAtaque = ESAEVUSDireccionAtaque::Frontal;
	PrepararAtaque(Cadena[Indice]);
}

void USAEVUSComponenteCombate::IniciarAtaqueDireccional(bool bEnAire, ESAEVUSDireccionAtaque Direccion)
{
	if (!ObtenerMovimiento())
	{
		return;
	}
	IndiceCombo = 0;
	bCadenaAerea = bEnAire;
	DireccionAtaque = Direccion;
	PrepararAtaque(Direccion == ESAEVUSDireccionAtaque::Arriba ? ObtenerRepertorio().TajoArriba : ObtenerRepertorio().TajoAbajo);
}

void USAEVUSComponenteCombate::PrepararAtaque(const FSAEVUSDefinicionAtaque& Ataque)
{
	USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	AtaqueActual = &Ataque;
	TiempoAtaque = 0.f;
	TiempoBufferAtaqueRestante = 0.f;
	DireccionPedida = ESAEVUSDireccionAtaque::Frontal;
	bImpactoAplicado = false;
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
	DireccionAtaque = ESAEVUSDireccionAtaque::Frontal;
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

	TArray<AActor*, TInlineAllocator<4>> Nuevos;
	for (const FOverlapResult& Solape : Solapes)
	{
		AActor* Objetivo = Solape.GetActor();
		if (!Objetivo || Objetivo == Duenio || ActoresGolpeados.Contains(Objetivo) || Nuevos.Contains(Objetivo))
		{
			continue;
		}
		Nuevos.Add(Objetivo);
	}
	for (AActor* Objetivo : Nuevos)
	{
		ActoresGolpeados.Add(Objetivo);
		UGameplayStatics::ApplyDamage(Objetivo, Ataque.Danio, Duenio->GetInstigatorController(), Duenio, UDamageType::StaticClass());
		AlGolpear.Broadcast(Objetivo, Ataque.Danio);
	}
	if (!Nuevos.IsEmpty())
	{
		AplicarImpacto(Ataque, Nuevos);
	}
}

FVector USAEVUSComponenteCombate::DireccionEmpuje(const AActor* Objetivo) const
{
	switch (DireccionAtaque)
	{
	case ESAEVUSDireccionAtaque::Arriba:
		return FVector::UpVector;
	case ESAEVUSDireccionAtaque::Abajo:
		return FVector::DownVector;
	case ESAEVUSDireccionAtaque::Frontal:
	default:
	{
		const FVector HaciaObjetivo = (Objetivo->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D();
		return HaciaObjetivo.IsNearlyZero() ? GetOwner()->GetActorForwardVector().GetSafeNormal2D() : HaciaObjetivo;
	}
	}
}

void USAEVUSComponenteCombate::AplicarImpacto(const FSAEVUSDefinicionAtaque& Ataque, TConstArrayView<AActor*> Golpeados)
{
	for (AActor* Objetivo : Golpeados)
	{
		if (ACharacter* PersonajeGolpeado = Cast<ACharacter>(Objetivo); PersonajeGolpeado && Ataque.EmpujeObjetivo > 0.f)
		{
			PersonajeGolpeado->LaunchCharacter(DireccionEmpuje(Objetivo) * Ataque.EmpujeObjetivo, true, DireccionAtaque != ESAEVUSDireccionAtaque::Frontal);
		}
	}
	if (bImpactoAplicado)
	{
		return;
	}
	bImpactoAplicado = true;

	ACharacter* Personaje = ObtenerPersonaje();
	USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	if (Personaje && Movimiento)
	{
		if (Ataque.VelocidadRebote > 0.f && Movimiento->IsFalling())
		{
			Movimiento->Rebotar(Ataque.VelocidadRebote);
		}
		else if (Ataque.RetrocesoAtacante > 0.f)
		{
			// El golpe frena la estocada y echa al atacante hacia atrás: se siente el contacto.
			Movimiento->TerminarEstocada();
			Personaje->LaunchCharacter(-Personaje->GetActorForwardVector().GetSafeNormal2D() * Ataque.RetrocesoAtacante, true, false);
		}
	}

	TArray<AActor*, TInlineAllocator<5>> Parados(Golpeados);
	Parados.Add(GetOwner());
	ParadaGolpe.Iniciar(Parados, ObtenerRepertorio().EscalaTiempoParada, Ataque.DuracionParadaGolpe);
}
