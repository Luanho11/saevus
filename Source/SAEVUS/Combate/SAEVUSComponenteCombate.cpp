// Copyright SAEVUS. Todos los derechos reservados.

#include "Combate/SAEVUSComponenteCombate.h"
#include "CollisionQueryParams.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Nucleo/SAEVUSCanalesColision.h"

namespace SAEVUSCombate
{
	const FSAEVUSRepertorioMelee RepertorioBase;

	// Dibujo de depuración de la esfera de golpe (SAEVUS.Depuracion.Combate).
	constexpr int32 SegmentosEsferaDepuracion = 12;

	/** Golpe de verdad: algo con vida (enemigo, jugador) o un blanco de práctica; no el escenario que recibe daño. */
	bool CuentaComoGolpe(const AActor* Objetivo)
	{
		return Objetivo->FindComponentByClass<USAEVUSComponenteVida>() || Objetivo->ActorHasTag(EtiquetaBlancoPractica);
	}
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
	// El ataque en curso es del repertorio anterior.
	TerminarAtaque();
	DatosRepertorio = NuevoRepertorio;
}

ACharacter* USAEVUSComponenteCombate::ObtenerPersonaje() const
{
	return Cast<ACharacter>(GetOwner());
}

USAEVUSComponenteMovimiento* USAEVUSComponenteCombate::ObtenerMovimiento() const
{
	return SAEVUSMovimiento::DelActor(GetOwner());
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

void USAEVUSComponenteCombate::FijarAtaqueMantenido(bool bMantenido)
{
	const bool bListo = EstaCargado();
	bAtaqueMantenido = bMantenido;
	if (bMantenido)
	{
		return;
	}
	TiempoCargando = 0.f;
	// Soltar con la carga lista: el ataque cargado, si se puede atacar (o encadenar) ahora.
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	const bool bLibre = !bAtacando || TiempoAtaque >= AtaqueEnCurso.EncadenarDesde;
	if (bListo && bLibre && PuedeAtacarAhora())
	{
		FijarContexto(0, Movimiento && Movimiento->IsFalling(), ESAEVUSDireccionAtaque::Frontal, false, true);
		PrepararAtaque(ObtenerRepertorio().AtaqueCargado);
	}
}

void USAEVUSComponenteCombate::CancelarCarga()
{
	bAtaqueMantenido = false;
	TiempoCargando = 0.f;
}

float USAEVUSComponenteCombate::ObtenerProgresoCarga() const
{
	return bAtaqueMantenido ? FMath::Clamp(TiempoCargando / ObtenerRepertorio().TiempoCarga, 0.f, 1.f) : 0.f;
}

ESAEVUSDireccionAtaque USAEVUSComponenteCombate::DireccionSegunInclinacion(float InclinacionGrados) const
{
	const FSAEVUSRepertorioMelee& Repertorio = ObtenerRepertorio();
	if (InclinacionGrados >= Repertorio.UmbralInclinacionArriba)
	{
		return ESAEVUSDireccionAtaque::Arriba;
	}
	return ESAEVUSDireccionAtaque::Frontal;
}

void USAEVUSComponenteCombate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ParadaGolpe.Terminar();
	Super::EndPlay(EndPlayReason);
}

bool USAEVUSComponenteCombate::PuedeCancelarPor(ESAEVUSAccionCancelacion Accion) const
{
	if (!bAtacando)
	{
		return true;
	}
	const float CancelarDesde = AtaqueEnCurso.ObtenerCancelacionDesde(Accion);
	return CancelarDesde >= 0.f && TiempoAtaque >= CancelarDesde;
}

bool USAEVUSComponenteCombate::IntentarCancelarPor(ESAEVUSAccionCancelacion Accion)
{
	if (!PuedeCancelarPor(Accion))
	{
		return false;
	}
	if (bAtacando)
	{
		TerminarAtaque();
	}
	return true;
}

bool USAEVUSComponenteCombate::GuardarAccionPendiente(ESAEVUSAccionCancelacion Accion, TFunction<void()> Ejecutar)
{
	if (!bAtacando)
	{
		return false;
	}
	const float CancelarDesde = AtaqueEnCurso.ObtenerCancelacionDesde(Accion);
	const float Buffer = ObtenerRepertorio().TiempoBufferAccion;
	// Sin ventana (el ataque compromete) o demasiado lejos: la pulsación se pierde, como debe.
	if (CancelarDesde < 0.f || CancelarDesde - TiempoAtaque > Buffer)
	{
		return false;
	}
	EjecutarAccionPendiente = MoveTemp(Ejecutar);
	AccionPendiente = Accion;
	TiempoAccionPendienteRestante = Buffer;
	return true;
}

ESAEVUSFaseAtaque USAEVUSComponenteCombate::ObtenerFaseAtaque() const
{
	return bAtacando ? AtaqueEnCurso.ObtenerFaseEn(TiempoAtaque) : ESAEVUSFaseAtaque::Ninguna;
}

void USAEVUSComponenteCombate::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// La parada se cuenta en tiempo real: DeltaTime ya viene dilatado por la parada del dueño. Con escala 0 (congelado)
	// DeltaTime es 0 y se usa el tiempo del mundo, así la parada también termina.
	const float Dilatacion = GetOwner()->CustomTimeDilation;
	ParadaGolpe.Actualizar(Dilatacion > UE_KINDA_SMALL_NUMBER ? DeltaTime / Dilatacion : GetWorld()->GetDeltaSeconds());

	if (bAtacando)
	{
		const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
		if (Movimiento && (Movimiento->EstaEnCarreraPared() || Movimiento->EstaEscalando()))
		{
			// El movimiento en pared se queda con el cuerpo; el ataque se descarta.
			TerminarAtaque();
		}
	}

	if (bAtacando)
	{
		const float Antes = TiempoAtaque;
		TiempoAtaque += DeltaTime;
		// Golpea si este paso de tiempo toca la ventana activa, aunque un fotograma largo la salte entera.
		const float InicioActivo = AtaqueEnCurso.TiempoPreparacion;
		const float FinActivo = InicioActivo + AtaqueEnCurso.TiempoActivo;
		if (TiempoAtaque >= InicioActivo && Antes < FinActivo)
		{
			DetectarGolpes(AtaqueEnCurso);
		}
		if (TiempoAtaque >= AtaqueEnCurso.ObtenerTiempoTotal())
		{
			TerminarAtaque();
		}
	}

	if (EjecutarAccionPendiente)
	{
		TiempoAccionPendienteRestante -= DeltaTime;
		if (TiempoAccionPendienteRestante <= 0.f)
		{
			EjecutarAccionPendiente = nullptr;
		}
		else if (PuedeCancelarPor(AccionPendiente))
		{
			// Se vacía antes de ejecutar: la acción vuelve a pasar por IntentarCancelarPor y no debe reencolarse.
			const TFunction<void()> Ejecutar = MoveTemp(EjecutarAccionPendiente);
			EjecutarAccionPendiente = nullptr;
			Ejecutar();
		}
	}

	ProcesarAtaqueEnBuffer();
	TiempoBufferAtaqueRestante = FMath::Max(0.f, TiempoBufferAtaqueRestante - DeltaTime);

	// La carga crece mientras se mantiene el botón y no hay ataque en curso (o está en su recuperación).
	const bool bPuedeCargar = !bAtacando || AtaqueEnCurso.ObtenerFaseEn(TiempoAtaque) == ESAEVUSFaseAtaque::Recuperacion;
	if (bAtaqueMantenido && bPuedeCargar)
	{
		TiempoCargando += DeltaTime;
	}
}

void USAEVUSComponenteCombate::ProcesarAtaqueEnBuffer()
{
	if (TiempoBufferAtaqueRestante <= 0.f || !PuedeAtacarAhora())
	{
		return;
	}
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	const bool bEnAire = Movimiento && Movimiento->IsFalling();
	const ESAEVUSDireccionAtaque Direccion = DireccionPedida;
	if (!bAtacando)
	{
		// Atacar durante un dash: tajo en carrera en su dirección (GunZ).
		if (Direccion == ESAEVUSDireccionAtaque::Frontal && Movimiento && Movimiento->EstaEnDash())
		{
			IniciarTajoEnCarrera(bEnAire, Movimiento->ObtenerDireccionDash());
		}
		else if (Direccion == ESAEVUSDireccionAtaque::Frontal)
		{
			IniciarAtaque(bEnAire, 0);
		}
		else
		{
			IniciarAtaqueDireccional(bEnAire, Direccion);
		}
		return;
	}
	if (TiempoAtaque < AtaqueEnCurso.EncadenarDesde)
	{
		return;
	}
	// Un tajo direccional sale desde la ventana de encadenar de cualquier ataque, y tras él cualquier otro.
	if (Direccion != ESAEVUSDireccionAtaque::Frontal)
	{
		IniciarAtaqueDireccional(bEnAire, Direccion);
		return;
	}
	if (DireccionAtaque != ESAEVUSDireccionAtaque::Frontal || bTajoEnCarrera || bAtaqueCargado)
	{
		IniciarAtaque(bEnAire, 0);
		return;
	}
	// Si se aterrizó (o se salió de un borde) a mitad de la cadena, esta se reinicia en el medio actual (GAMEPLAY-19).
	if (bEnAire != bCadenaAerea)
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

void USAEVUSComponenteCombate::FijarContexto(int32 Indice, bool bEnAire, ESAEVUSDireccionAtaque Direccion, bool bCarrera, bool bCargado)
{
	IndiceCombo = Indice;
	bCadenaAerea = bEnAire;
	DireccionAtaque = Direccion;
	bTajoEnCarrera = bCarrera;
	bAtaqueCargado = bCargado;
}

void USAEVUSComponenteCombate::IniciarAtaque(bool bEnAire, int32 Indice)
{
	const TArray<FSAEVUSDefinicionAtaque>& Cadena = bEnAire ? ObtenerRepertorio().ComboAire : ObtenerRepertorio().ComboSuelo;
	if (!Cadena.IsValidIndex(Indice) || !ObtenerMovimiento())
	{
		return;
	}
	FijarContexto(Indice, bEnAire, ESAEVUSDireccionAtaque::Frontal, false, false);
	PrepararAtaque(Cadena[Indice]);
}

void USAEVUSComponenteCombate::IniciarTajoEnCarrera(bool bEnAire, const FVector& DireccionDash)
{
	FijarContexto(0, bEnAire, ESAEVUSDireccionAtaque::Frontal, true, false);
	// Siguiendo a la cámara, el tajo en carrera también sale de frente.
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	const FVector Direccion = Movimiento && Movimiento->MiraHaciaCamara() ? FVector::ZeroVector : DireccionDash.GetSafeNormal2D();
	PrepararAtaque(ObtenerRepertorio().TajoEnCarrera, Direccion.IsNearlyZero() ? nullptr : &Direccion);
}

void USAEVUSComponenteCombate::IniciarAtaqueDireccional(bool bEnAire, ESAEVUSDireccionAtaque Direccion)
{
	if (!ObtenerMovimiento())
	{
		return;
	}
	FijarContexto(0, bEnAire, Direccion, false, false);
	PrepararAtaque(ObtenerRepertorio().TajoArriba);
}

void USAEVUSComponenteCombate::PrepararAtaque(const FSAEVUSDefinicionAtaque& Ataque, const FVector* DireccionForzada)
{
	USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	// Copia: el ataque en curso no depende de que el repertorio (un Data Asset) siga igual mientras dura.
	AtaqueEnCurso = Ataque;
	bAtacando = true;
	TiempoAtaque = 0.f;
	TiempoBufferAtaqueRestante = 0.f;
	DireccionPedida = ESAEVUSDireccionAtaque::Frontal;
	bImpactoAplicado = false;
	ActoresGolpeados.Reset();

	// Ataca hacia la entrada de movimiento si la hay; si no, de frente. Siguiendo a la cámara (GunZ),
	// siempre de frente: hacia donde se mira.
	const FVector DireccionEntrada = Movimiento->MiraHaciaCamara() ? FVector::ZeroVector : Movimiento->GetCurrentAcceleration().GetSafeNormal2D();
	const FVector Direccion = DireccionForzada ? *DireccionForzada
		: (DireccionEntrada.IsNearlyZero() ? GetOwner()->GetActorForwardVector().GetSafeNormal2D() : DireccionEntrada);

	Movimiento->CancelarDash();
	Movimiento->GirarHacia(Direccion);
	Movimiento->IniciarEstocada(Direccion, Ataque.VelocidadEstocada, Ataque.DuracionEstocada);
	Movimiento->FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad::Ataque, Ataque.EscalaVelocidadSuelo);
	if (Movimiento->IsFalling())
	{
		// Tajo aéreo: frena la caída y flota mientras dura (GunZ).
		Movimiento->FijarEscalaGravedadAccion(Ataque.EscalaGravedadEnAire);
		if (Movimiento->Velocity.Z < 0.f)
		{
			Movimiento->Velocity.Z *= Ataque.FraccionCaidaConservada;
		}
	}
	else
	{
		// En el suelo la gravedad es la normal: no se arrastra la flotación de un tajo aéreo anterior.
		Movimiento->FijarEscalaGravedadAccion(1.f);
	}
	AlIniciarAtaque.Broadcast();
}

void USAEVUSComponenteCombate::TerminarAtaque()
{
	bAtacando = false;
	TiempoAtaque = 0.f;
	FijarContexto(INDEX_NONE, false, ESAEVUSDireccionAtaque::Frontal, false, false);
	ActoresGolpeados.Reset();
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		Movimiento->FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad::Ataque, 1.f);
		Movimiento->FijarEscalaGravedadAccion(1.f);
	}
}

bool USAEVUSComponenteCombate::DentroDelAbanico(const FSAEVUSDefinicionAtaque& Ataque, const FVector& Origen, const FVector& Frente, const FVector& Punto)
{
	const FVector Delta = Punto - Origen;
	if (FMath::Abs(Delta.Z) > Ataque.MitadAlturaAbanico)
	{
		return false;
	}
	const FVector Horizontal(Delta.X, Delta.Y, 0.f);
	const double Distancia = Horizontal.Size();
	if (Distancia > Ataque.AlcanceAbanico)
	{
		return false;
	}
	// Pegado al cuerpo cuenta en cualquier dirección.
	if (Distancia < UE_KINDA_SMALL_NUMBER || Ataque.AnguloAbanico >= 180.f)
	{
		return true;
	}
	const double Coseno = FVector::DotProduct(Horizontal / Distancia, Frente.GetSafeNormal2D());
	return Coseno >= FMath::Cos(FMath::DegreesToRadians(Ataque.AnguloAbanico));
}

bool USAEVUSComponenteCombate::ObtenerAbanicoActivo(FVector& OutOrigen, FVector& OutFrente, float& OutAvance, float& OutSentido) const
{
	const AActor* Duenio = GetOwner();
	if (!bAtacando || !Duenio || AtaqueEnCurso.AnguloAbanico <= 0.f || ObtenerFaseAtaque() != ESAEVUSFaseAtaque::Activa)
	{
		return false;
	}
	OutOrigen = Duenio->GetActorLocation() + FVector::UpVector * AtaqueEnCurso.AlturaGolpe;
	OutFrente = Duenio->GetActorForwardVector().GetSafeNormal2D();
	OutAvance = FMath::Clamp((TiempoAtaque - AtaqueEnCurso.TiempoPreparacion) / FMath::Max(AtaqueEnCurso.TiempoActivo, UE_KINDA_SMALL_NUMBER), 0.f, 1.f);
	// Como la hoja del gesto: el tajo va de derecha a izquierda, el revés al contrario.
	OutSentido = SAEVUSCombate::SentidoBarrido(AtaqueEnCurso.Gesto);
	return true;
}

void USAEVUSComponenteCombate::DetectarGolpes(const FSAEVUSDefinicionAtaque& Ataque)
{
	AActor* Duenio = GetOwner();
	UWorld* Mundo = GetWorld();
	if (!Duenio || !Mundo)
	{
		return;
	}
	const bool bAbanico = Ataque.AnguloAbanico > 0.f;
	const FVector Origen = Duenio->GetActorLocation() + FVector::UpVector * Ataque.AlturaGolpe;
	const FVector Frente = Duenio->GetActorForwardVector().GetSafeNormal2D();
	// Abanico: la consulta es la esfera que lo contiene y luego se filtra por el punto más cercano de cada uno.
	const FVector Centro = bAbanico ? Origen
		: Duenio->GetActorLocation() + Duenio->GetActorForwardVector() * Ataque.DesplazamientoFrontalGolpe + FVector::UpVector * Ataque.AlturaGolpe;
	const float Radio = bAbanico ? FMath::Sqrt(FMath::Square(Ataque.AlcanceAbanico) + FMath::Square(Ataque.MitadAlturaAbanico)) : Ataque.RadioGolpe;

	FCollisionObjectQueryParams TiposObjeto;
	TiposObjeto.AddObjectTypesToQuery(ECC_Pawn);
	TiposObjeto.AddObjectTypesToQuery(ECC_PhysicsBody);
	TiposObjeto.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams ParametrosConsulta(SCENE_QUERY_STAT(SAEVUSAtaque), false, Duenio);

	TArray<FOverlapResult> Solapes;
	Mundo->OverlapMultiByObjectType(Solapes, Centro, FQuat::Identity, TiposObjeto, FCollisionShape::MakeSphere(Radio), ParametrosConsulta);

	if (SAEVUSDepuracion::DibujoCombateActivo())
	{
		if (bAbanico)
		{
			const float Angulo = FMath::DegreesToRadians(Ataque.AnguloAbanico);
			DrawDebugCone(Mundo, Origen, Frente, Ataque.AlcanceAbanico, Angulo, FMath::DegreesToRadians(1.f), SAEVUSCombate::SegmentosEsferaDepuracion,
				Solapes.IsEmpty() ? FColor::Yellow : FColor::Red);
		}
		else
		{
			DrawDebugSphere(Mundo, Centro, Ataque.RadioGolpe, SAEVUSCombate::SegmentosEsferaDepuracion, Solapes.IsEmpty() ? FColor::Yellow : FColor::Red);
		}
	}

	TArray<AActor*, TInlineAllocator<4>> Nuevos;
	for (const FOverlapResult& Solape : Solapes)
	{
		AActor* Objetivo = Solape.GetActor();
		if (!Objetivo || Objetivo == Duenio || ActoresGolpeados.Contains(Objetivo) || Nuevos.Contains(Objetivo))
		{
			continue;
		}
		// Solo lo que la hoja puede tocar: las zonas de aviso, de banco o de paso (que solo solapan) no se golpean.
		const UPrimitiveComponent* Componente = Solape.GetComponent();
		if (!Componente || Componente->GetCollisionResponseToChannel(SAEVUSColision::TrazaArma) != ECR_Block)
		{
			continue;
		}
		if (bAbanico)
		{
			// El punto del objetivo más cercano al origen tiene que caer en el abanico.
			FVector Punto = Objetivo->GetActorLocation();
			FVector Cercano;
			if (Componente->GetClosestPointOnCollision(Origen, Cercano) >= 0.f)
			{
				Punto = Cercano;
			}
			if (!DentroDelAbanico(Ataque, Origen, Frente, Punto))
			{
				continue;
			}
		}
		Nuevos.Add(Objetivo);
	}
	for (AActor* Objetivo : Nuevos)
	{
		ActoresGolpeados.Add(Objetivo);
		// Lo que no pierde vida (pinchos, escenario, un escudo de frente) cuenta para el impacto y el rebote,
		// pero no es un golpe: no da alma ni chispas.
		const float Hecho = UGameplayStatics::ApplyDamage(Objetivo, Ataque.Danio, Duenio->GetInstigatorController(), Duenio, UDamageType::StaticClass());
		if (Hecho > 0.f && SAEVUSCombate::CuentaComoGolpe(Objetivo))
		{
			AlGolpear.Broadcast(Objetivo, Hecho);
		}
		else if (!SAEVUSCombate::CuentaComoGolpe(Objetivo))
		{
			AlImpactarEscenario.Broadcast(Objetivo);
		}
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
		if (Ataque.RetrocesoAtacante > 0.f)
		{
			// El golpe frena la estocada y echa al atacante hacia atrás: se siente el contacto.
			Movimiento->TerminarEstocada();
			Personaje->LaunchCharacter(-Personaje->GetActorForwardVector().GetSafeNormal2D() * Ataque.RetrocesoAtacante, true, false);
		}
	}

	TArray<AActor*, TInlineAllocator<5>> Parados(Golpeados);
	Parados.Add(GetOwner());
	ParadaGolpe.Iniciar(Parados, ObtenerRepertorio().EscalaTiempoParada, Ataque.DuracionParadaGolpe);
	AlImpactar.Broadcast(Ataque);
}
