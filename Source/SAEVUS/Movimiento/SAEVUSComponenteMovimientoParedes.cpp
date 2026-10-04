// Copyright SAEVUS. Todos los derechos reservados.

// Movimiento en paredes de USAEVUSComponenteMovimiento: detección de paredes, salto de pared,
// carrera por pared y escalada. Separado de SAEVUSComponenteMovimiento.cpp (paso, dash,
// asistencias de salto, movimientos guardados).

#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

namespace SAEVUSMovimientoParedes
{
	// Sonda del salto de pared: ocho barridos horizontales de cápsula.
	constexpr int32 DireccionesSondaSaltoPared = 8;

	// Las cápsulas de sonda se encogen un poco para que una cápsula que ya toca la pared no
	// empiece el barrido penetrándola.
	constexpr float MargenSonda = 2.f;

	// En una pared la cápsula se acerca cada actualización hasta este hueco, así mantiene el
	// contacto sin rozar contra ella.
	constexpr float HuecoPegadoPared = 1.f;

	// Una carrera cuya normal se parece tanto a la anterior es la misma pared.
	constexpr float ProductoMismaPared = 0.9f;

	// Avanzar menos que esta fracción de lo esperado por la pared o hacia arriba significa que algo bloquea.
	constexpr float FraccionAvanceBloqueado = 0.5f;
}

bool USAEVUSComponenteMovimiento::EstaEnCarreraPared() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ESAEVUSModoMovimientoPersonalizado::CarreraPared);
}

bool USAEVUSComponenteMovimiento::EstaEscalando() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ESAEVUSModoMovimientoPersonalizado::EscaladaPared);
}

int32 USAEVUSComponenteMovimiento::SaltosParedRestantes() const
{
	return FMath::Max(0, ObtenerAjusteSaltoPared().MaxSaltosPared - SaltosParedUsados);
}

int32 USAEVUSComponenteMovimiento::EscaladasRestantes() const
{
	return FMath::Max(0, ObtenerAjusteEscalada().MaxEscaladas - EscaladasUsadas);
}

// --- Detección de paredes -----------------------------------------------------------

bool USAEVUSComponenteMovimiento::BuscarParedCercana(TConstArrayView<FVector> Direcciones, float Distancia, float MaxNormalZPared, FHitResult& OutGolpe) const
{
	using namespace SAEVUSMovimientoParedes;
	if (!HasValidData())
	{
		return false;
	}
	const FVector Inicio = UpdatedComponent->GetComponentLocation();
	const FCollisionShape Sonda = GetPawnCapsuleCollisionShape(SHRINK_RadiusCustom, MargenSonda);
	const float DistanciaSonda = Distancia + MargenSonda;

	FCollisionQueryParams ParametrosConsulta(SCENE_QUERY_STAT(SAEVUSSondaPared), false, CharacterOwner);
	FCollisionResponseParams ParametrosRespuesta;
	InitCollisionParams(ParametrosConsulta, ParametrosRespuesta);
	const ECollisionChannel Canal = UpdatedComponent->GetCollisionObjectType();

	// Los contactos en la semiesfera inferior son cornisas a la altura de los pies (p. ej. el borde
	// del que se acaba de bajar), no paredes: una pared tiene que tocar el lado recto de la cápsula.
	const float ContactoParedMasBajo = Inicio.Z - Sonda.GetCapsuleHalfHeight() + Sonda.GetCapsuleRadius();

	bool bEncontrada = false;
	for (const FVector& Direccion : Direcciones)
	{
		FHitResult Golpe;
		if (!GetWorld()->SweepSingleByChannel(Golpe, Inicio, Inicio + Direccion * DistanciaSonda, FQuat::Identity, Canal, Sonda, ParametrosConsulta, ParametrosRespuesta)
			|| Golpe.bStartPenetrating || FMath::Abs(Golpe.ImpactNormal.Z) > MaxNormalZPared || Golpe.ImpactPoint.Z < ContactoParedMasBajo)
		{
			continue;
		}
		if (!bEncontrada || Golpe.Distance < OutGolpe.Distance)
		{
			OutGolpe = Golpe;
			bEncontrada = true;
		}
	}
	return bEncontrada;
}

FVector USAEVUSComponenteMovimiento::BuscarNormalSaltoPared() const
{
	TArray<FVector, TInlineAllocator<SAEVUSMovimientoParedes::DireccionesSondaSaltoPared>> Direcciones;
	for (int32 Indice = 0; Indice < SAEVUSMovimientoParedes::DireccionesSondaSaltoPared; ++Indice)
	{
		Direcciones.Add(FRotator(0.f, 360.f * Indice / SAEVUSMovimientoParedes::DireccionesSondaSaltoPared, 0.f).Vector());
	}
	const FSAEVUSAjusteSaltoPared& Ajuste = ObtenerAjusteSaltoPared();
	FHitResult Golpe;
	return BuscarParedCercana(Direcciones, Ajuste.DistanciaDeteccion, Ajuste.MaxNormalZPared, Golpe) ? Golpe.ImpactNormal.GetSafeNormal2D() : FVector::ZeroVector;
}

bool USAEVUSComponenteMovimiento::SondearParedActual(float DistanciaDeteccion, float MaxNormalZPared, FHitResult& OutGolpePared)
{
	const FVector HaciaPared[] = { -NormalPared };
	if (!BuscarParedCercana(HaciaPared, DistanciaDeteccion, MaxNormalZPared, OutGolpePared))
	{
		return false;
	}
	NormalPared = OutGolpePared.ImpactNormal.GetSafeNormal2D();
	return true;
}

// --- Salto de pared -------------------------------------------------------------

bool USAEVUSComponenteMovimiento::IntentarSaltoPared()
{
	// Solo cuenta la primera actualización de una pulsación de salto (Jump() pone JumpKeyHoldTime a
	// cero y crece tras cada actualización), y no si esa pulsación ya hizo un salto normal o coyote.
	const bool bSaltoRecienPulsado = CharacterOwner && CharacterOwner->bPressedJump && CharacterOwner->JumpKeyHoldTime <= 0.f
		&& !CharacterOwner->bWasJumping;
	const bool bEnPared = EstaEnCarreraPared() || EstaEscalando();
	if (!bSaltoRecienPulsado || !(IsFalling() || bEnPared) || SaltosParedRestantes() <= 0)
	{
		return false;
	}
	// Saltar desde una carrera o escalada empuja desde la pared en la que está el personaje.
	const FVector NormalEmpuje = bEnPared ? NormalPared : BuscarNormalSaltoPared();
	if (NormalEmpuje.IsNearlyZero())
	{
		return false;
	}
	if (bEnPared)
	{
		SetMovementMode(MOVE_Falling);
	}
	ImpulsarDesdePared(NormalEmpuje);
	if (!CharacterOwner->bClientUpdating)
	{
		AlSaltoPared.Broadcast();
	}
	return true;
}

void USAEVUSComponenteMovimiento::ImpulsarDesdePared(const FVector& NormalEmpuje)
{
	const FSAEVUSAjusteSaltoPared& Ajuste = ObtenerAjusteSaltoPared();
	if (EstaEnDash())
	{
		TerminarDash();
	}

	const FVector Horizontal(Velocity.X, Velocity.Y, 0.f);
	const FVector ParalelaPared = Horizontal - NormalEmpuje * FVector::DotProduct(Horizontal, NormalEmpuje);
	Velocity = NormalEmpuje * Ajuste.VelocidadAlejarse + ParalelaPared * Ajuste.FraccionVelocidadTangencial;
	Velocity.Z = Ajuste.VelocidadSubida;

	++SaltosParedUsados;
	if (Ajuste.bRecargaDashesAereos)
	{
		DashesAereosUsados = 0;
	}
	if (Ajuste.bRecargaSaltosAereos)
	{
		SaltosAereosUsados = 0;
	}
	// La pulsación se usó aquí; no debe ejecutarse también como salto en buffer al aterrizar.
	TiempoBufferSaltoRestante = 0.f;
	TiempoCoyoteRestante = 0.f;

	if (Ajuste.bGirarHaciaSalto)
	{
		MoveUpdatedComponent(FVector::ZeroVector, OrientacionCuerpo(Velocity.GetSafeNormal2D()), /*bSweep*/ false);
	}
}

// --- Piezas comunes de los modos de pared -----------------------------------------------

void USAEVUSComponenteMovimiento::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (EstaEnCarreraPared())
	{
		FisicaCarreraPared(DeltaTime, Iterations);
		return;
	}
	if (EstaEscalando())
	{
		FisicaEscalada(DeltaTime, Iterations);
		return;
	}
	Super::PhysCustom(DeltaTime, Iterations);
}

void USAEVUSComponenteMovimiento::SoltarPared(float TiempoRestante, int32 Iteraciones)
{
	TiempoParedRestante = 0.f;
	// La velocidad que haya fijado quien llama continúa en la caída.
	SetMovementMode(MOVE_Falling);
	if (TiempoRestante >= MIN_TICK_TIME)
	{
		StartNewPhysics(TiempoRestante, Iteraciones);
	}
}

FVector USAEVUSComponenteMovimiento::MoverJuntoAPared(float DeltaTime, int32 Iteraciones, const FHitResult& GolpePared, const FQuat& Orientacion)
{
	using namespace SAEVUSMovimientoParedes;
	const FVector PosicionAnterior = UpdatedComponent->GetComponentLocation();
	const FVector Pegado = -NormalPared * FMath::Max(0.f, static_cast<float>(GolpePared.Distance) - MargenSonda - HuecoPegadoPared);
	const FVector Desplazamiento = Velocity * DeltaTime + Pegado;
	FHitResult Golpe(1.f);
	SafeMoveUpdatedComponent(Desplazamiento, Orientacion, true, Golpe);
	if (Golpe.Time < 1.f)
	{
		if (Velocity.Z <= 0.f && IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Golpe))
		{
			ProcessLanded(Golpe, DeltaTime * (1.f - Golpe.Time), Iteraciones);
			return UpdatedComponent->GetComponentLocation() - PosicionAnterior;
		}
		HandleImpact(Golpe, DeltaTime, Desplazamiento);
		SlideAlongSurface(Desplazamiento, 1.f - Golpe.Time, Golpe.Normal, Golpe, true);
	}
	return UpdatedComponent->GetComponentLocation() - PosicionAnterior;
}

// --- Carrera por pared --------------------------------------------------------------

bool USAEVUSComponenteMovimiento::IntentarIniciarCarreraPared()
{
	const FSAEVUSAjusteCarreraPared& Ajuste = ObtenerAjusteCarreraPared();
	if (Ajuste.DuracionMaxima <= 0.f || !IsFalling() || EstaEnDash() || Velocity.Z < -Ajuste.MaxVelocidadCaidaEntrada)
	{
		return false;
	}
	// El jugador tiene que dirigir: sin entrada, no hay carrera por pared.
	const FVector DireccionEntrada = Acceleration.GetSafeNormal2D();
	const FVector DireccionMovimiento = FVector(Velocity.X, Velocity.Y, 0.f).GetSafeNormal();
	if (DireccionEntrada.IsNearlyZero() || DireccionMovimiento.IsNearlyZero())
	{
		return false;
	}

	// Paredes al lado del movimiento, o en diagonal por delante.
	const FVector Lado = FVector::CrossProduct(FVector::UpVector, DireccionMovimiento);
	const FVector Direcciones[] = { Lado, -Lado, (DireccionMovimiento + Lado).GetSafeNormal(), (DireccionMovimiento - Lado).GetSafeNormal() };
	FHitResult GolpePared;
	if (!BuscarParedCercana(Direcciones, Ajuste.DistanciaDeteccion, Ajuste.MaxNormalZPared, GolpePared))
	{
		return false;
	}
	const FVector Normal = GolpePared.ImpactNormal.GetSafeNormal2D();
	// No se vuelve a recorrer la misma pared antes de aterrizar o de recorrer otra.
	if (!NormalUltimaCarreraPared.IsNearlyZero() && FVector::DotProduct(Normal, NormalUltimaCarreraPared) > SAEVUSMovimientoParedes::ProductoMismaPared)
	{
		return false;
	}
	if (FVector::DotProduct(DireccionEntrada, Normal) > Ajuste.UmbralSoltarEntrada)
	{
		return false;
	}
	const FVector Horizontal(Velocity.X, Velocity.Y, 0.f);
	const FVector ParalelaPared = Horizontal - Normal * FVector::DotProduct(Horizontal, Normal);
	if (ParalelaPared.Size() < Ajuste.VelocidadMinimaEntrada)
	{
		return false;
	}

	NormalPared = Normal;
	DireccionCarreraPared = ParalelaPared.GetSafeNormal();
	TiempoParedRestante = Ajuste.DuracionMaxima;
	const float VelocidadVerticalEntrada = FMath::Clamp(static_cast<float>(Velocity.Z), 0.f, Ajuste.MaxVelocidadSubidaEntrada);
	Velocity = DireccionCarreraPared * Ajuste.Velocidad;
	Velocity.Z = VelocidadVerticalEntrada;
	if (Ajuste.bRecargaDashesAereos)
	{
		DashesAereosUsados = 0;
	}
	SetMovementMode(MOVE_Custom, static_cast<uint8>(ESAEVUSModoMovimientoPersonalizado::CarreraPared));
	MoveUpdatedComponent(FVector::ZeroVector, OrientacionCuerpo(DireccionCarreraPared), /*bSweep*/ false);
	return true;
}

void USAEVUSComponenteMovimiento::FisicaCarreraPared(float DeltaTime, int32 Iteraciones)
{
	using namespace SAEVUSMovimientoParedes;
	if (DeltaTime < MIN_TICK_TIME || !HasValidData())
	{
		return;
	}
	const FSAEVUSAjusteCarreraPared& Ajuste = ObtenerAjusteCarreraPared();

	TiempoParedRestante -= DeltaTime;
	if (TiempoParedRestante <= 0.f || FVector::DotProduct(Acceleration.GetSafeNormal2D(), NormalPared) > Ajuste.UmbralSoltarEntrada)
	{
		// Sin tiempo, o dirigiendo lejos de la pared: cae conservando la velocidad de la carrera.
		SoltarPared(DeltaTime, Iteraciones);
		return;
	}

	// La pared tiene que seguir ahí; seguir su normal permite correr por paredes curvas.
	FHitResult GolpePared;
	if (!SondearParedActual(Ajuste.DistanciaDeteccion, Ajuste.MaxNormalZPared, GolpePared))
	{
		SoltarPared(DeltaTime, Iteraciones);
		return;
	}
	DireccionCarreraPared = (DireccionCarreraPared - NormalPared * FVector::DotProduct(DireccionCarreraPared, NormalPared)).GetSafeNormal();
	if (DireccionCarreraPared.IsNearlyZero())
	{
		SoltarPared(DeltaTime, Iteraciones);
		return;
	}

	const float VelocidadVertical = Velocity.Z + GetGravityZ() * Ajuste.EscalaGravedad * DeltaTime;
	Velocity = DireccionCarreraPared * Ajuste.Velocidad;
	Velocity.Z = VelocidadVertical;

	const FVector Movido = MoverJuntoAPared(DeltaTime, Iteraciones, GolpePared, OrientacionCuerpo(DireccionCarreraPared));
	if (!EstaEnCarreraPared())
	{
		return; // aterrizó durante el movimiento
	}

	// Bajar hasta un suelo termina la carrera en un aterrizaje.
	if (Velocity.Z <= 0.f)
	{
		FFindFloorResult Suelo;
		FindFloor(UpdatedComponent->GetComponentLocation(), Suelo, false);
		if (Suelo.IsWalkableFloor() && Suelo.GetDistanceToFloor() <= MAX_FLOOR_DIST)
		{
			ProcessLanded(Suelo.HitResult, 0.f, Iteraciones);
			return;
		}
	}

	// Algo por delante en la pared detuvo la carrera.
	if (FVector::DotProduct(Movido, DireccionCarreraPared) < Ajuste.Velocidad * DeltaTime * FraccionAvanceBloqueado)
	{
		SoltarPared(0.f, Iteraciones);
	}
}

// --- Escalada ---------------------------------------------------------------------

bool USAEVUSComponenteMovimiento::IntentarIniciarEscalada()
{
	const FSAEVUSAjusteEscaladaPared& Ajuste = ObtenerAjusteEscalada();
	if (Ajuste.DuracionMaxima <= 0.f || !IsFalling() || EstaEnDash() || EscaladasRestantes() <= 0
		|| Velocity.Z < -Ajuste.MaxVelocidadCaidaEntrada)
	{
		return false;
	}
	const FVector DireccionEntrada = Acceleration.GetSafeNormal2D();
	const FVector Horizontal(Velocity.X, Velocity.Y, 0.f);
	const FVector DireccionMovimiento = Horizontal.GetSafeNormal();
	if (DireccionEntrada.IsNearlyZero() || DireccionMovimiento.IsNearlyZero())
	{
		return false;
	}

	// Solo una pared justo delante del movimiento, a la que se llega de frente y hacia la que se dirige.
	const FVector Delante[] = { DireccionMovimiento };
	FHitResult GolpePared;
	if (!BuscarParedCercana(Delante, Ajuste.DistanciaDeteccion, Ajuste.MaxNormalZPared, GolpePared))
	{
		return false;
	}
	const FVector Normal = GolpePared.ImpactNormal.GetSafeNormal2D();
	const FVector HaciaPared = -Normal;
	if (FVector::DotProduct(DireccionMovimiento, HaciaPared) < Ajuste.UmbralFrontal || FVector::DotProduct(DireccionEntrada, HaciaPared) < Ajuste.UmbralFrontal
		|| FVector::DotProduct(Horizontal, HaciaPared) < Ajuste.VelocidadMinimaEntrada)
	{
		return false;
	}

	NormalPared = Normal;
	TiempoParedRestante = Ajuste.DuracionMaxima;
	++EscaladasUsadas;
	if (Ajuste.bRecargaDashesAereos)
	{
		DashesAereosUsados = 0;
	}
	Velocity = FVector(0.f, 0.f, Ajuste.Velocidad);
	SetMovementMode(MOVE_Custom, static_cast<uint8>(ESAEVUSModoMovimientoPersonalizado::EscaladaPared));
	MoveUpdatedComponent(FVector::ZeroVector, OrientacionCuerpo(HaciaPared), /*bSweep*/ false);
	return true;
}

void USAEVUSComponenteMovimiento::FisicaEscalada(float DeltaTime, int32 Iteraciones)
{
	using namespace SAEVUSMovimientoParedes;
	if (DeltaTime < MIN_TICK_TIME || !HasValidData())
	{
		return;
	}
	const FSAEVUSAjusteEscaladaPared& Ajuste = ObtenerAjusteEscalada();

	TiempoParedRestante -= DeltaTime;
	if (TiempoParedRestante <= 0.f)
	{
		// Sin tiempo: voltereta lejos de la pared.
		Velocity = NormalPared * Ajuste.VelocidadVolteretaAlejarse;
		Velocity.Z = Ajuste.VelocidadVolteretaSubida;
		MoveUpdatedComponent(FVector::ZeroVector, OrientacionCuerpo(NormalPared), /*bSweep*/ false);
		SoltarPared(DeltaTime, Iteraciones);
		return;
	}
	if (FVector::DotProduct(Acceleration.GetSafeNormal2D(), NormalPared) > Ajuste.UmbralSoltarEntrada)
	{
		// Se suelta: cae en vertical.
		Velocity = FVector::ZeroVector;
		SoltarPared(DeltaTime, Iteraciones);
		return;
	}

	FHitResult GolpePared;
	if (!SondearParedActual(Ajuste.DistanciaDeteccion, Ajuste.MaxNormalZPared, GolpePared))
	{
		// Pasó el borde superior: sigue subiendo y avanza para subirse encima.
		Velocity = -NormalPared * Ajuste.VelocidadAvanceCornisa;
		Velocity.Z = Ajuste.Velocidad;
		SoltarPared(DeltaTime, Iteraciones);
		return;
	}

	Velocity = FVector(0.f, 0.f, Ajuste.Velocidad);
	const FVector Movido = MoverJuntoAPared(DeltaTime, Iteraciones, GolpePared, OrientacionCuerpo(-NormalPared));
	if (EstaEscalando() && Movido.Z < Ajuste.Velocidad * DeltaTime * FraccionAvanceBloqueado)
	{
		// Algo por encima detiene la escalada.
		Velocity = FVector::ZeroVector;
		SoltarPared(0.f, Iteraciones);
	}
}
