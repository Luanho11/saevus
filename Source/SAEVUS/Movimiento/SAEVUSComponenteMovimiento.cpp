// Copyright SAEVUS. Todos los derechos reservados.

#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "GameFramework/Character.h"

namespace SAEVUSMovimiento
{
	// Las peticiones de paso y dash viajan en los bits de banderas comprimidas reservados para juegos.
	constexpr uint8 BanderaEsprint = FSavedMove_Character::FLAG_Custom_0;
	constexpr uint8 BanderaCaminar = FSavedMove_Character::FLAG_Custom_1;
	constexpr uint8 BanderaDash = FSavedMove_Character::FLAG_Custom_2;

	const FSAEVUSAjusteMovimientoSuelo AjusteSueloBase;
	const FSAEVUSAjusteMovimientoAire AjusteAireBase;
	const FSAEVUSAjusteDash AjusteDashBase;
	const FSAEVUSAjusteDash AjusteDashArmaFuegoBase = SAEVUSDash::CrearVoltereta();
	const FSAEVUSAjusteSaltoPared AjusteSaltoParedBase;
	const FSAEVUSAjusteCarreraPared AjusteCarreraParedBase;
	const FSAEVUSAjusteEscaladaPared AjusteEscaladaBase;
}

/**
 * Movimiento guardado con las peticiones de paso y dash, más el estado de dash y pared al
 * empezar el movimiento, para que se repita bien durante las correcciones del cliente.
 */
class FSAEVUSMovimientoGuardado : public FSavedMove_Character
{
public:
	using Super = FSavedMove_Character;

	virtual void Clear() override
	{
		Super::Clear();
		bQuiereEsprintGuardado = false;
		bQuiereCaminarGuardado = false;
		bQuiereDashGuardado = false;
		DireccionDashPedidaGuardada = FVector::ZeroVector;
		DireccionDashGuardada = FVector::ForwardVector;
		TiempoDashRestanteGuardado = 0.f;
		EnfriamientoDashRestanteGuardado = 0.f;
		DashesAereosUsadosGuardados = 0;
		SaltosParedUsadosGuardados = 0;
		SaltosAereosUsadosGuardados = 0;
		NormalParedGuardada = FVector::ZeroVector;
		DireccionCarreraParedGuardada = FVector::ZeroVector;
		TiempoParedRestanteGuardado = 0.f;
		NormalUltimaCarreraParedGuardada = FVector::ZeroVector;
		EscaladasUsadasGuardadas = 0;
	}

	virtual uint8 GetCompressedFlags() const override
	{
		uint8 Banderas = Super::GetCompressedFlags();
		if (bQuiereEsprintGuardado)
		{
			Banderas |= SAEVUSMovimiento::BanderaEsprint;
		}
		if (bQuiereCaminarGuardado)
		{
			Banderas |= SAEVUSMovimiento::BanderaCaminar;
		}
		if (bQuiereDashGuardado)
		{
			Banderas |= SAEVUSMovimiento::BanderaDash;
		}
		return Banderas;
	}

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override
	{
		const FSAEVUSMovimientoGuardado* Otro = static_cast<const FSAEVUSMovimientoGuardado*>(NewMove.Get());
		if (bQuiereEsprintGuardado != Otro->bQuiereEsprintGuardado || bQuiereCaminarGuardado != Otro->bQuiereCaminarGuardado)
		{
			return false;
		}
		// Nunca se combinan una pulsación de dash ni un movimiento que empiece o termine un dash o un uso de pared.
		if (bQuiereDashGuardado || Otro->bQuiereDashGuardado || DashesAereosUsadosGuardados != Otro->DashesAereosUsadosGuardados
			|| SaltosParedUsadosGuardados != Otro->SaltosParedUsadosGuardados || SaltosAereosUsadosGuardados != Otro->SaltosAereosUsadosGuardados
			|| !NormalUltimaCarreraParedGuardada.Equals(Otro->NormalUltimaCarreraParedGuardada)
			|| EscaladasUsadasGuardadas != Otro->EscaladasUsadasGuardadas
			|| (TiempoDashRestanteGuardado > 0.f) != (Otro->TiempoDashRestanteGuardado > 0.f))
		{
			return false;
		}
		return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
	}

	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override
	{
		Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
		if (const USAEVUSComponenteMovimiento* Movimiento = Cast<USAEVUSComponenteMovimiento>(Character->GetCharacterMovement()))
		{
			bQuiereEsprintGuardado = Movimiento->bQuiereEsprint;
			bQuiereCaminarGuardado = Movimiento->bQuiereCaminar;
			bQuiereDashGuardado = Movimiento->bQuiereDash;
			DireccionDashPedidaGuardada = Movimiento->DireccionDashPedida;
			DireccionDashGuardada = Movimiento->DireccionDash;
			TiempoDashRestanteGuardado = Movimiento->TiempoDashRestante;
			EnfriamientoDashRestanteGuardado = Movimiento->EnfriamientoDashRestante;
			DashesAereosUsadosGuardados = Movimiento->DashesAereosUsados;
			SaltosParedUsadosGuardados = Movimiento->SaltosParedUsados;
			SaltosAereosUsadosGuardados = Movimiento->SaltosAereosUsados;
			NormalParedGuardada = Movimiento->NormalPared;
			DireccionCarreraParedGuardada = Movimiento->DireccionCarreraPared;
			TiempoParedRestanteGuardado = Movimiento->TiempoParedRestante;
			NormalUltimaCarreraParedGuardada = Movimiento->NormalUltimaCarreraPared;
			EscaladasUsadasGuardadas = Movimiento->EscaladasUsadas;
		}
	}

	virtual void PrepMoveFor(ACharacter* Character) override
	{
		Super::PrepMoveFor(Character);
		if (USAEVUSComponenteMovimiento* Movimiento = Cast<USAEVUSComponenteMovimiento>(Character->GetCharacterMovement()))
		{
			Movimiento->bQuiereEsprint = bQuiereEsprintGuardado;
			Movimiento->bQuiereCaminar = bQuiereCaminarGuardado;
			Movimiento->bQuiereDash = bQuiereDashGuardado;
			Movimiento->DireccionDashPedida = DireccionDashPedidaGuardada;
			Movimiento->DireccionDash = DireccionDashGuardada;
			Movimiento->TiempoDashRestante = TiempoDashRestanteGuardado;
			Movimiento->EnfriamientoDashRestante = EnfriamientoDashRestanteGuardado;
			Movimiento->DashesAereosUsados = DashesAereosUsadosGuardados;
			Movimiento->SaltosParedUsados = SaltosParedUsadosGuardados;
			Movimiento->SaltosAereosUsados = SaltosAereosUsadosGuardados;
			Movimiento->NormalPared = NormalParedGuardada;
			Movimiento->DireccionCarreraPared = DireccionCarreraParedGuardada;
			Movimiento->TiempoParedRestante = TiempoParedRestanteGuardado;
			Movimiento->NormalUltimaCarreraPared = NormalUltimaCarreraParedGuardada;
			Movimiento->EscaladasUsadas = EscaladasUsadasGuardadas;
		}
	}

private:
	bool bQuiereEsprintGuardado = false;
	bool bQuiereCaminarGuardado = false;
	bool bQuiereDashGuardado = false;
	// Solo en el cliente para sus repeticiones; enviarla al servidor necesita los datos de movimiento propios (fase de red).
	FVector DireccionDashPedidaGuardada = FVector::ZeroVector;
	FVector DireccionDashGuardada = FVector::ForwardVector;
	float TiempoDashRestanteGuardado = 0.f;
	float EnfriamientoDashRestanteGuardado = 0.f;
	int32 DashesAereosUsadosGuardados = 0;
	int32 SaltosParedUsadosGuardados = 0;
	int32 SaltosAereosUsadosGuardados = 0;
	FVector NormalParedGuardada = FVector::ZeroVector;
	FVector DireccionCarreraParedGuardada = FVector::ZeroVector;
	float TiempoParedRestanteGuardado = 0.f;
	FVector NormalUltimaCarreraParedGuardada = FVector::ZeroVector;
	int32 EscaladasUsadasGuardadas = 0;
};

class FSAEVUSDatosPrediccionCliente : public FNetworkPredictionData_Client_Character
{
public:
	explicit FSAEVUSDatosPrediccionCliente(const UCharacterMovementComponent& MovimientoCliente)
		: FNetworkPredictionData_Client_Character(MovimientoCliente)
	{
	}

	virtual FSavedMovePtr AllocateNewMove() override
	{
		return FSavedMovePtr(new FSAEVUSMovimientoGuardado());
	}
};

USAEVUSComponenteMovimiento::USAEVUSComponenteMovimiento()
{
	bWantsInitializeComponent = true;

	// Acción en tercera persona: el personaje gira hacia donde se mueve; la cámara es libre.
	bOrientRotationToMovement = true;
}

void USAEVUSComponenteMovimiento::InitializeComponent()
{
	Super::InitializeComponent();
	AplicarAjustes();
}

ESAEVUSPaso USAEVUSComponenteMovimiento::ObtenerPasoActual() const
{
	if (bQuiereEsprint)
	{
		return ESAEVUSPaso::Esprint;
	}
	return bQuiereCaminar ? ESAEVUSPaso::Caminar : ESAEVUSPaso::Correr;
}

float USAEVUSComponenteMovimiento::ObtenerVelocidadPaso(ESAEVUSPaso Paso) const
{
	const FSAEVUSAjusteMovimientoSuelo& Ajuste = ObtenerAjusteSuelo();
	switch (Paso)
	{
	case ESAEVUSPaso::Caminar:
		return Ajuste.VelocidadCaminar;
	case ESAEVUSPaso::Esprint:
		return Ajuste.VelocidadEsprint;
	case ESAEVUSPaso::Correr:
	default:
		return Ajuste.VelocidadCorrer;
	}
}

const FSAEVUSAjusteMovimientoSuelo& USAEVUSComponenteMovimiento::ObtenerAjusteSuelo() const
{
	return AjustesMovimiento ? AjustesMovimiento->Suelo : SAEVUSMovimiento::AjusteSueloBase;
}

const FSAEVUSAjusteMovimientoAire& USAEVUSComponenteMovimiento::ObtenerAjusteAire() const
{
	return AjustesMovimiento ? AjustesMovimiento->Aire : SAEVUSMovimiento::AjusteAireBase;
}

const FSAEVUSAjusteDash& USAEVUSComponenteMovimiento::ObtenerAjusteDash() const
{
	if (bDashArmaFuego)
	{
		return AjustesMovimiento ? AjustesMovimiento->DashArmaFuego : SAEVUSMovimiento::AjusteDashArmaFuegoBase;
	}
	return AjustesMovimiento ? AjustesMovimiento->Dash : SAEVUSMovimiento::AjusteDashBase;
}

const FSAEVUSAjusteSaltoPared& USAEVUSComponenteMovimiento::ObtenerAjusteSaltoPared() const
{
	return AjustesMovimiento ? AjustesMovimiento->SaltoPared : SAEVUSMovimiento::AjusteSaltoParedBase;
}

const FSAEVUSAjusteCarreraPared& USAEVUSComponenteMovimiento::ObtenerAjusteCarreraPared() const
{
	return AjustesMovimiento ? AjustesMovimiento->CarreraPared : SAEVUSMovimiento::AjusteCarreraParedBase;
}

const FSAEVUSAjusteEscaladaPared& USAEVUSComponenteMovimiento::ObtenerAjusteEscalada() const
{
	return AjustesMovimiento ? AjustesMovimiento->Escalada : SAEVUSMovimiento::AjusteEscaladaBase;
}

void USAEVUSComponenteMovimiento::FijarAjustesMovimiento(const USAEVUSAjustesMovimiento* NuevosAjustes)
{
	AjustesMovimiento = NuevosAjustes;
	AplicarAjustes();
}

ESAEVUSPaso USAEVUSComponenteMovimiento::ObtenerPasoAire() const
{
	return ObtenerAjusteAire().bBloquearPasoEnAire ? PasoAire : ObtenerPasoActual();
}

bool USAEVUSComponenteMovimiento::EnVentanaCoyote() const
{
	return IsFalling() && TiempoCoyoteRestante > 0.f;
}

void USAEVUSComponenteMovimiento::GuardarSaltoEnBuffer()
{
	if (IsFalling())
	{
		TiempoBufferSaltoRestante = ObtenerAjusteAire().TiempoBufferSalto;
	}
}

int32 USAEVUSComponenteMovimiento::DashesAereosRestantes() const
{
	return FMath::Max(0, ObtenerAjusteDash().MaxDashesAereos - DashesAereosUsados);
}

bool USAEVUSComponenteMovimiento::PuedeDash() const
{
	if (EstaEnDash() || EnfriamientoDashRestante > 0.f || ObtenerAjusteDash().Duracion <= 0.f || IsCrouching())
	{
		return false;
	}
	if (IsMovingOnGround())
	{
		return true;
	}
	return IsFalling() && DashesAereosRestantes() > 0;
}

void USAEVUSComponenteMovimiento::IniciarDash()
{
	const FSAEVUSAjusteDash& Ajuste = ObtenerAjusteDash();

	// Prioridad: dirección pedida (doble toque); si no, la entrada (Acceleration, que viaja con el
	// movimiento, así servidor y cliente coinciden); si no, hacia delante.
	const FVector DireccionEntrada = Acceleration.GetSafeNormal2D();
	if (!DireccionDashPedida.IsNearlyZero())
	{
		DireccionDash = DireccionDashPedida;
	}
	else
	{
		DireccionDash = DireccionEntrada.IsNearlyZero() ? UpdatedComponent->GetForwardVector().GetSafeNormal2D() : DireccionEntrada;
	}
	DireccionDashPedida = FVector::ZeroVector;
	TiempoDashRestante = Ajuste.Duracion;

	float VelocidadVertical = Velocity.Z;
	if (IsFalling())
	{
		++DashesAereosUsados;
		VelocidadVertical = Ajuste.VelocidadVerticalDashAereo;
	}
	Velocity = DireccionDash * Ajuste.Velocidad;
	Velocity.Z = VelocidadVertical;

	if (Ajuste.bGirarHaciaDash)
	{
		MoveUpdatedComponent(FVector::ZeroVector, DireccionDash.ToOrientationQuat(), /*bSweep*/ false);
	}

	// Las repeticiones tras una corrección vuelven a simular el dash: sin efectos repetidos.
	if (!(CharacterOwner && CharacterOwner->bClientUpdating))
	{
		AlIniciarDash.Broadcast(DireccionDash);
	}
}

void USAEVUSComponenteMovimiento::CancelarDash()
{
	if (EstaEnDash())
	{
		TerminarDash();
	}
}

void USAEVUSComponenteMovimiento::IniciarEstocada(const FVector& Direccion, float Velocidad, float Duracion)
{
	const FVector Horizontal = Direccion.GetSafeNormal2D();
	if (Velocidad <= 0.f || Duracion <= 0.f || Horizontal.IsNearlyZero())
	{
		return;
	}
	DireccionEstocada = Horizontal;
	VelocidadEstocada = Velocidad;
	TiempoEstocadaRestante = Duracion;
}

void USAEVUSComponenteMovimiento::GirarHacia(const FVector& Direccion)
{
	const FVector Horizontal = Direccion.GetSafeNormal2D();
	if (HasValidData() && !Horizontal.IsNearlyZero())
	{
		MoveUpdatedComponent(FVector::ZeroVector, Horizontal.ToOrientationQuat(), false);
	}
}

void USAEVUSComponenteMovimiento::TerminarDash()
{
	const FSAEVUSAjusteDash& Ajuste = ObtenerAjusteDash();
	TiempoDashRestante = 0.f;
	EnfriamientoDashRestante = Ajuste.Enfriamiento;

	const FVector Horizontal = FVector(Velocity.X, Velocity.Y, 0.f).GetClampedToMaxSize(Ajuste.VelocidadSalida);
	Velocity.X = Horizontal.X;
	Velocity.Y = Horizontal.Y;
}

float USAEVUSComponenteMovimiento::GetMaxSpeed() const
{
	if (EstaEnDash())
	{
		return ObtenerAjusteDash().Velocidad;
	}
	if (EstaEnEstocada())
	{
		return VelocidadEstocada;
	}
	if (EstaEnCarreraPared())
	{
		return ObtenerAjusteCarreraPared().Velocidad;
	}
	if (EstaEscalando())
	{
		return ObtenerAjusteEscalada().Velocidad;
	}
	if (IsCrouching())
	{
		return Super::GetMaxSpeed();
	}
	if (IsMovingOnGround())
	{
		return ObtenerVelocidadPaso(ObtenerPasoActual()) * EscalaVelocidadAccion;
	}
	// En el aire también hay tope por paso, así el control aéreo nunca baja un salto de esprint a velocidad de carrera.
	if (IsFalling())
	{
		return ObtenerVelocidadPaso(ObtenerPasoAire());
	}
	return Super::GetMaxSpeed();
}

float USAEVUSComponenteMovimiento::GetGravityZ() const
{
	if (EstaEnDash() && IsFalling() && ObtenerAjusteDash().bSuspenderGravedadEnDash)
	{
		return 0.f;
	}
	return Super::GetGravityZ() * EscalaGravedadAccion;
}

void USAEVUSComponenteMovimiento::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	// Un dash mantiene velocidad y dirección: sin dirigir con la entrada, sin fricción ni frenado.
	// Quien llama pasa aquí la velocidad horizontal (caminar y caer gestionan Z por su cuenta).
	if (EstaEnDash() && HasValidData() && !HasAnimRootMotion())
	{
		const float VelocidadVertical = Velocity.Z;
		Velocity = DireccionDash * ObtenerAjusteDash().Velocidad;
		Velocity.Z = VelocidadVertical;
		return;
	}
	if (EstaEnEstocada() && HasValidData() && !HasAnimRootMotion())
	{
		const float VelocidadVertical = Velocity.Z;
		Velocity = DireccionEstocada * VelocidadEstocada;
		Velocity.Z = VelocidadVertical;
		return;
	}
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

int32 USAEVUSComponenteMovimiento::SaltosAereosRestantes() const
{
	return FMath::Max(0, ObtenerAjusteAire().MaxSaltosAereos - SaltosAereosUsados);
}

void USAEVUSComponenteMovimiento::RecargarRecursosAereos()
{
	DashesAereosUsados = 0;
	SaltosAereosUsados = 0;
}

void USAEVUSComponenteMovimiento::Rebotar(float VelocidadVertical)
{
	if (EstaEnDash())
	{
		TerminarDash();
	}
	TerminarEstocada();
	Velocity.Z = VelocidadVertical;
	RecargarRecursosAereos();
}

bool USAEVUSComponenteMovimiento::IntentarSaltoAereo()
{
	// Misma regla de pulsación que el salto de pared: solo la primera actualización de una pulsación
	// nueva que no haya hecho ya un salto de suelo o coyote.
	const bool bSaltoRecienPulsado = CharacterOwner && CharacterOwner->bPressedJump && CharacterOwner->JumpKeyHoldTime <= 0.f
		&& !CharacterOwner->bWasJumping;
	if (!bSaltoRecienPulsado || !IsFalling() || EnVentanaCoyote() || SaltosAereosRestantes() <= 0)
	{
		return false;
	}
	// Justo antes de aterrizar gana el buffer: sale un salto de suelo completo y el doble salto se conserva.
	if (SueloDentroDelBuffer())
	{
		return false;
	}
	if (EstaEnDash())
	{
		TerminarDash();
	}
	Velocity.Z = ObtenerAjusteAire().VelocidadDobleSalto;
	++SaltosAereosUsados;
	// La pulsación se usó aquí; no debe ejecutarse también como salto en buffer al aterrizar.
	TiempoBufferSaltoRestante = 0.f;
	if (!CharacterOwner->bClientUpdating)
	{
		AlSaltoAereo.Broadcast();
	}
	return true;
}

bool USAEVUSComponenteMovimiento::SueloDentroDelBuffer() const
{
	const float Distancia = -Velocity.Z * ObtenerAjusteAire().TiempoBufferSalto;
	if (Distancia <= 0.f || !HasValidData())
	{
		return false;
	}
	FCollisionQueryParams ParametrosConsulta(SCENE_QUERY_STAT(SAEVUSSueloBuffer), false, CharacterOwner);
	FCollisionResponseParams ParametrosRespuesta;
	InitCollisionParams(ParametrosConsulta, ParametrosRespuesta);
	const FVector Inicio = UpdatedComponent->GetComponentLocation();
	FHitResult Golpe;
	return GetWorld()->SweepSingleByChannel(Golpe, Inicio, Inicio - FVector(0.f, 0.f, Distancia), FQuat::Identity, UpdatedComponent->GetCollisionObjectType(),
		GetPawnCapsuleCollisionShape(SHRINK_None), ParametrosConsulta, ParametrosRespuesta) && IsWalkable(Golpe);
}

bool USAEVUSComponenteMovimiento::DoJump(bool bReplayingMoves, float DeltaTime)
{
	const bool bSalto = Super::DoJump(bReplayingMoves, DeltaTime);
	if (bSalto)
	{
		TiempoCoyoteRestante = 0.f;
		TiempoBufferSaltoRestante = 0.f;
		// Saltar durante un dash de suelo lo cancela; el salto sale con la velocidad de salida del dash.
		if (EstaEnDash())
		{
			TerminarDash();
		}
	}
	return bSalto;
}

void USAEVUSComponenteMovimiento::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	// Super avisa al personaje, que reinicia su estado de salto al aterrizar; los saltos en buffer van después.
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	const bool bDejoSuelo = IsFalling() && (PreviousMovementMode == MOVE_Walking || PreviousMovementMode == MOVE_NavWalking);
	if (bDejoSuelo)
	{
		PasoAire = ObtenerPasoActual();
		// Un despegue con salto ya usó el salto de suelo; solo salir caminando de un borde abre la ventana.
		const bool bSaltoAlSalir = CharacterOwner && CharacterOwner->bPressedJump;
		TiempoCoyoteRestante = bSaltoAlSalir ? 0.f : ObtenerAjusteAire().TiempoCoyote;
	}
	else if (!IsFalling())
	{
		TiempoCoyoteRestante = 0.f;
	}

	const bool bEstabaEnCarreraPared = PreviousMovementMode == MOVE_Custom
		&& PreviousCustomMode == static_cast<uint8>(ESAEVUSModoMovimientoPersonalizado::CarreraPared);
	if (bEstabaEnCarreraPared && !EstaEnCarreraPared())
	{
		NormalUltimaCarreraPared = NormalPared;
	}
	if (!EstaEnCarreraPared() && !EstaEscalando())
	{
		TiempoParedRestante = 0.f;
	}

	if (IsMovingOnGround())
	{
		RecargarRecursosAereos();
		SaltosParedUsados = 0;
		NormalUltimaCarreraPared = FVector::ZeroVector;
		EscaladasUsadas = 0;
	}
	else if (!IsFalling())
	{
		// Dash y estocada solo existen al caminar y al caer; cualquier otro modo los termina.
		CancelarDash();
		TiempoEstocadaRestante = 0.f;
	}

	if (IsMovingOnGround() && TiempoBufferSaltoRestante > 0.f && CharacterOwner)
	{
		TiempoBufferSaltoRestante = 0.f;
		CharacterOwner->Jump();
	}
}

void USAEVUSComponenteMovimiento::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	TiempoCoyoteRestante = FMath::Max(0.f, TiempoCoyoteRestante - DeltaSeconds);
	TiempoBufferSaltoRestante = FMath::Max(0.f, TiempoBufferSaltoRestante - DeltaSeconds);

	TiempoEstocadaRestante = FMath::Max(0.f, TiempoEstocadaRestante - DeltaSeconds);

	if (EstaEnDash())
	{
		TiempoDashRestante = FMath::Max(0.f, TiempoDashRestante - DeltaSeconds);
		if (!EstaEnDash())
		{
			TerminarDash();
		}
	}
	else
	{
		EnfriamientoDashRestante = FMath::Max(0.f, EnfriamientoDashRestante - DeltaSeconds);
	}
}

void USAEVUSComponenteMovimiento::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	const bool bSaltoPared = IntentarSaltoPared();
	const bool bSaltoAereo = !bSaltoPared && IntentarSaltoAereo();

	// La petición se consume siempre: un dash pulsado durante el enfriamiento no se guarda.
	if (bQuiereDash)
	{
		bQuiereDash = false;
		if (PuedeDash())
		{
			IniciarDash();
		}
	}

	// Llegar de frente escala; llegar en ángulo corre por la pared.
	if (!bSaltoPared && !bSaltoAereo && !IntentarIniciarEscalada())
	{
		IntentarIniciarCarreraPared();
	}
}

FString USAEVUSComponenteMovimiento::GetMovementName() const
{
	if (EstaEnCarreraPared())
	{
		return TEXT("Carrera por pared");
	}
	return EstaEscalando() ? TEXT("Escalada") : Super::GetMovementName();
}

void USAEVUSComponenteMovimiento::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bQuiereEsprint = (Flags & SAEVUSMovimiento::BanderaEsprint) != 0;
	bQuiereCaminar = (Flags & SAEVUSMovimiento::BanderaCaminar) != 0;
	bQuiereDash = (Flags & SAEVUSMovimiento::BanderaDash) != 0;
}

FNetworkPredictionData_Client* USAEVUSComponenteMovimiento::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		USAEVUSComponenteMovimiento* EsteMutable = const_cast<USAEVUSComponenteMovimiento*>(this);
		EsteMutable->ClientPredictionData = new FSAEVUSDatosPrediccionCliente(*this);
	}
	return ClientPredictionData;
}

void USAEVUSComponenteMovimiento::AplicarAjustes()
{
	const FSAEVUSAjusteMovimientoSuelo& Suelo = ObtenerAjusteSuelo();
	// Los sistemas que ignoran el paso (navegación, IA) leen MaxWalkSpeed, así que refleja el paso por defecto.
	MaxWalkSpeed = Suelo.VelocidadCorrer;
	MaxAcceleration = Suelo.AceleracionMaxima;
	BrakingDecelerationWalking = Suelo.DeceleracionFrenado;
	GroundFriction = Suelo.FriccionSuelo;
	RotationRate = FRotator(0.f, Suelo.VelocidadGiro, 0.f);

	const FSAEVUSAjusteMovimientoAire& Aire = ObtenerAjusteAire();
	JumpZVelocity = Aire.VelocidadSalto;
	GravityScale = Aire.EscalaGravedad;
	AirControl = Aire.ControlAire;
	AirControlBoostMultiplier = Aire.MultiplicadorImpulsoControlAire;
	AirControlBoostVelocityThreshold = Aire.UmbralImpulsoControlAire;
	FallingLateralFriction = Aire.FriccionLateralCaida;
	BrakingDecelerationFalling = Aire.DeceleracionFrenadoCaida;

	if (CharacterOwner)
	{
		CharacterOwner->JumpMaxHoldTime = Aire.TiempoMaximoMantenerSalto;
	}
}
