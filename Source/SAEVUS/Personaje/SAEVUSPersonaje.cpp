// Copyright SAEVUS. Todos los derechos reservados.

#include "Personaje/SAEVUSPersonaje.h"
#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Armas/SAEVUSComponenteArmas.h"
#include "Audio/SAEVUSComponenteSonido.h"
#include "Components/PoseableMeshComponent.h"
#include "Camara/SAEVUSComponenteCamara.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteGuardia.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Efectos/SAEVUSComponenteEfectosCombate.h"
#include "Efectos/SAEVUSComponenteEstelaDash.h"
#include "Efectos/SAEVUSComponentePolvoMovimiento.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"
#include "Mundo/SAEVUSInteractuable.h"
#include "Nucleo/SAEVUSControladorJugador.h"
#include "Nucleo/SAEVUSGuardado.h"
#include "Nucleo/SAEVUSModoJuego.h"
#include "Personaje/SAEVUSConfigEntrada.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "SAEVUS.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"

namespace SAEVUSPersonaje
{
	// Aviso al recoger un fragmento de máscara (s).
	constexpr float DuracionAvisoFragmento = 3.f;

	// Cápsula a la medida del cuerpo provisional.
	constexpr float RadioCapsula = 42.f;
	constexpr float MediaAlturaCapsula = 96.f;

	// Las mallas esqueléticas miran a +Y; el actor mira a +X.
	const FRotator CorreccionOrientacionMalla(0.f, -90.f, 0.f);

	// Perfil de colisión del motor para ragdolls (BaseEngine.ini).
	const FName PerfilRagdoll(TEXT("Ragdoll"));

	// Invulnerabilidad del jugador tras recibir un golpe (los enemigos no la tienen).
	constexpr float InvulnerabilidadTrasGolpe = 1.f;
}

ASAEVUSPersonaje::ASAEVUSPersonaje(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USAEVUSComponenteMovimiento>(CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(SAEVUSPersonaje::RadioCapsula, SAEVUSPersonaje::MediaAlturaCapsula);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -SAEVUSPersonaje::MediaAlturaCapsula), SAEVUSPersonaje::CorreccionOrientacionMalla);

	// La rotación del controlador solo mueve la cámara; el cuerpo gira hacia donde se mueve.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	Combate = CreateDefaultSubobject<USAEVUSComponenteCombate>(TEXT("Combate"));
	Armas = CreateDefaultSubobject<USAEVUSComponenteArmas>(TEXT("Armas"));
	Vida = CreateDefaultSubobject<USAEVUSComponenteVida>(TEXT("Vida"));
	Vida->FijarInvulnerabilidadTrasGolpe(SAEVUSPersonaje::InvulnerabilidadTrasGolpe);
	Alma = CreateDefaultSubobject<USAEVUSComponenteAlma>(TEXT("Alma"));
	Guardia = CreateDefaultSubobject<USAEVUSComponenteGuardia>(TEXT("Guardia"));

	EstelaDash = CreateDefaultSubobject<USAEVUSComponenteEstelaDash>(TEXT("EstelaDash"));
	EstelaDash->SetupAttachment(RootComponent);
	PolvoMovimiento = CreateDefaultSubobject<USAEVUSComponentePolvoMovimiento>(TEXT("PolvoMovimiento"));
	PolvoMovimiento->SetupAttachment(RootComponent);
	EfectosCombate = CreateDefaultSubobject<USAEVUSComponenteEfectosCombate>(TEXT("EfectosCombate"));
	EfectosCombate->SetupAttachment(RootComponent);
	Sonido = CreateDefaultSubobject<USAEVUSComponenteSonido>(TEXT("Sonido"));

	// Cuerpo visible: hereda de la malla del personaje el desplazamiento a los pies y la corrección de -90°.
	Cuerpo = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("Cuerpo"));
	Cuerpo->SetupAttachment(GetMesh());
	Cuerpo->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Animacion = CreateDefaultSubobject<USAEVUSComponenteAnimacionProcedural>(TEXT("Animacion"));

	BrazoCamara = CreateDefaultSubobject<USpringArmComponent>(TEXT("BrazoCamara"));
	BrazoCamara->SetupAttachment(RootComponent);
	BrazoCamara->bUsePawnControlRotation = true;

	Camara = CreateDefaultSubobject<USAEVUSComponenteCamara>(TEXT("Camara"));
	Camara->SetupAttachment(BrazoCamara, USpringArmComponent::SocketName);
	Camara->bUsePawnControlRotation = false;
}

USAEVUSComponenteMovimiento* ASAEVUSPersonaje::ObtenerMovimiento() const
{
	return CastChecked<USAEVUSComponenteMovimiento>(GetCharacterMovement());
}

void ASAEVUSPersonaje::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// Armas y combate se actualizan primero, así una estocada, un cambio de velocidad o un giro
	// que fijen se aplica en el movimiento del mismo fotograma.
	GetCharacterMovement()->PrimaryComponentTick.AddPrerequisite(Combate, Combate->PrimaryComponentTick);
	GetCharacterMovement()->PrimaryComponentTick.AddPrerequisite(Armas, Armas->PrimaryComponentTick);
	GetCharacterMovement()->PrimaryComponentTick.AddPrerequisite(Alma, Alma->PrimaryComponentTick);
	GetCharacterMovement()->PrimaryComponentTick.AddPrerequisite(Guardia, Guardia->PrimaryComponentTick);
	// La guardia pregunta al combate si puede cortar el ataque: va después de él.
	Guardia->PrimaryComponentTick.AddPrerequisite(Combate, Combate->PrimaryComponentTick);

	Vida->AlMorir.AddUniqueDynamic(this, &ThisClass::ManejarMuerte);
	Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::ManejarCambioVida);
	ObtenerMovimiento()->AlIniciarDash.AddUObject(this, &ThisClass::AlIniciarDash);

	if (ConfigEntrada)
	{
		DetectorDobleToque.Ventana = ConfigEntrada->VentanaDobleToque;
		DetectorDobleToque.Umbral = ConfigEntrada->UmbralDobleToque;
	}
}

void ASAEVUSPersonaje::AlIniciarDash(const FVector& Direccion)
{
	Vida->HacerInvulnerable(ObtenerMovimiento()->ObtenerAjusteDash().DuracionInvulnerable);
}

bool ASAEVUSPersonaje::CamaraBloqueada() const
{
	const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	return Movimiento->EstaEnDash() && Movimiento->ObtenerAjusteDash().bBloquearCamaraEnDash;
}

FRotator ASAEVUSPersonaje::ObtenerRotacionEntrada() const
{
	// Sin controlador (tests, pawns sin dueño) se usan los ejes del mundo: el actor gira al moverse
	// y la base de la entrada cambiaría entre pulsaciones.
	return Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
}

void ASAEVUSPersonaje::ManejarCambioVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador)
{
	if (Delta >= 0.f || !Instigador || Instigador == this || ComponenteVida->EstaMuerto())
	{
		return;
	}
	FVector Alejarse = (GetActorLocation() - Instigador->GetActorLocation()).GetSafeNormal2D();
	if (Alejarse.IsNearlyZero())
	{
		Alejarse = -GetActorForwardVector();
	}
	ObtenerMovimiento()->CancelarDash();
	LaunchCharacter(Alejarse * GolpeRecibido.VelocidadHorizontal + FVector::UpVector * GolpeRecibido.VelocidadVertical, true, true);
}

void ASAEVUSPersonaje::ManejarMuerte(USAEVUSComponenteVida* ComponenteVida, AActor* Asesino)
{
	UE_LOG(LogSAEVUS, Log, TEXT("%s muerto por %s"), *GetName(), *GetNameSafe(Asesino));
	Armas->FijarGatillo(false);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GetMesh()->GetSkeletalMeshAsset() && GetMesh()->GetPhysicsAsset())
	{
		GetMesh()->SetCollisionProfileName(SAEVUSPersonaje::PerfilRagdoll);
		GetMesh()->SetSimulatePhysics(true);
	}

	AController* ControladorDuenio = GetController();
	if (APlayerController* ControladorJugador = Cast<APlayerController>(ControladorDuenio))
	{
		DisableInput(ControladorJugador);
	}
	if (ASAEVUSModoJuego* ModoJuego = GetWorld()->GetAuthGameMode<ASAEVUSModoJuego>())
	{
		ModoJuego->NotificarMuerteJugador(ControladorDuenio);
	}
}

void ASAEVUSPersonaje::BeginPlay()
{
	Super::BeginPlay();
	// Máscaras extra de los fragmentos guardados.
	VidaMaximaBase = Vida->ObtenerVidaMaxima();
	if (const USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this))
	{
		FragmentosMascara = Guardado->ObtenerNumeroFragmentos();
		const int32 Extra = FragmentosMascara / FMath::Max(Progreso.FragmentosPorMascara, 1);
		if (Extra > 0)
		{
			Vida->FijarVidaMaxima(VidaMaximaBase + Extra * Progreso.VidaPorMascara);
		}
	}
}

void ASAEVUSPersonaje::RecogerFragmentoMascara(FName Identificador)
{
	++FragmentosMascara;
	if (USAEVUSSubsistemaGuardado* Guardado = USAEVUSSubsistemaGuardado::Obtener(this))
	{
		Guardado->GuardarFragmento(Identificador);
	}
	const int32 PorMascara = FMath::Max(Progreso.FragmentosPorMascara, 1);
	const bool bMascaraEntera = FragmentosMascara % PorMascara == 0;
	if (bMascaraEntera)
	{
		// Una máscara más, con la vida llena.
		Vida->FijarVidaMaxima(Vida->ObtenerVidaMaxima() + Progreso.VidaPorMascara);
	}
	if (USAEVUSSubsistemaTutorial* Tutorial = GetWorld()->GetSubsystem<USAEVUSSubsistemaTutorial>())
	{
		Tutorial->MostrarMensaje(NSLOCTEXT("SAEVUSPersonaje", "Fragmento", "Fragmento de máscara"),
			bMascaraEntera ? NSLOCTEXT("SAEVUSPersonaje", "MascaraEntera", "La máscara está entera: una máscara de vida más.")
				: FText::Format(NSLOCTEXT("SAEVUSPersonaje", "FragmentosFaltan", "{0} de {1}."), FragmentosMascara % PorMascara, PorMascara),
			SAEVUSPersonaje::DuracionAvisoFragmento);
	}
}

void ASAEVUSPersonaje::PedirInteraccion()
{
	if (bSentado)
	{
		Levantarse();
		return;
	}
	if (ISAEVUSInteractuable* Objeto = Cast<ISAEVUSInteractuable>(Interactuable.Get()))
	{
		Objeto->Interactuar(this);
	}
}

void ASAEVUSPersonaje::FijarInteractuable(AActor* Actor)
{
	if (Cast<ISAEVUSInteractuable>(Actor))
	{
		Interactuable = Actor;
	}
}

void ASAEVUSPersonaje::QuitarInteractuable(AActor* Actor)
{
	if (Interactuable.Get() == Actor)
	{
		Interactuable.Reset();
	}
}

void ASAEVUSPersonaje::Sentarse(const FTransform& Asiento)
{
	bSentado = true;
	Alma->CancelarCuracion();
	FijarGuardia(false);
	ObtenerMovimiento()->StopMovementImmediately();
	SetActorLocationAndRotation(FVector(Asiento.GetLocation().X, Asiento.GetLocation().Y, GetActorLocation().Z), Asiento.GetRotation().Rotator());
}

void ASAEVUSPersonaje::Levantarse()
{
	bSentado = false;
}

void ASAEVUSPersonaje::PedirSalto()
{
	if (bSentado)
	{
		// Saltar desde el banco solo levanta.
		Levantarse();
		return;
	}
	if (!Combate->IntentarCancelarPor(ESAEVUSAccionCancelacion::Salto))
	{
		return;
	}
	Alma->CancelarCuracion();
	USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento();
	if (Movimiento->IsFalling() && !Movimiento->EnVentanaCoyote())
	{
		Movimiento->GuardarSaltoEnBuffer();
	}
	Jump();
}

bool ASAEVUSPersonaje::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	return GetLocalRole() == ROLE_Authority && CanBeDamaged() && Damage != 0.f;
}

bool ASAEVUSPersonaje::CanJumpWhileFalling() const
{
	// Tiempo coyote: el primer salto (de suelo) sigue disponible un momento tras salir caminando de un borde.
	return ObtenerMovimiento()->EnVentanaCoyote();
}

void ASAEVUSPersonaje::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Entrada = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Entrada)
	{
		UE_LOG(LogSAEVUS, Error, TEXT("%s: el componente de entrada no es EnhancedInputComponent; revisa DefaultInput.ini."), *GetNameSafe(this));
		return;
	}
	if (!ConfigEntrada)
	{
		UE_LOG(LogSAEVUS, Error, TEXT("%s: sin ConfigEntrada asignada; el personaje no responderá a la entrada."), *GetNameSafe(this));
		return;
	}

	if (ConfigEntrada->AccionMover)
	{
		Entrada->BindAction(ConfigEntrada->AccionMover, ETriggerEvent::Triggered, this, &ThisClass::ManejarMover);
		// Al soltar, el detector de doble toque necesita ver el eje a cero.
		Entrada->BindAction(ConfigEntrada->AccionMover, ETriggerEvent::Completed, this, &ThisClass::ManejarMoverSoltar);
	}
	if (ConfigEntrada->AccionMirar)
	{
		Entrada->BindAction(ConfigEntrada->AccionMirar, ETriggerEvent::Triggered, this, &ThisClass::ManejarMirar);
	}
	if (ConfigEntrada->AccionMirarRaton)
	{
		Entrada->BindAction(ConfigEntrada->AccionMirarRaton, ETriggerEvent::Triggered, this, &ThisClass::ManejarMirar);
	}
	if (ConfigEntrada->AccionSaltar)
	{
		Entrada->BindAction(ConfigEntrada->AccionSaltar, ETriggerEvent::Started, this, &ThisClass::PedirSalto);
		Entrada->BindAction(ConfigEntrada->AccionSaltar, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	// Acciones mantenidas: Triggered cada fotograma mientras se mantienen (cierto), Completed al soltar (falso).
	if (ConfigEntrada->AccionEsprint)
	{
		Entrada->BindAction(ConfigEntrada->AccionEsprint, ETriggerEvent::Triggered, this, &ThisClass::ManejarEsprint);
		Entrada->BindAction(ConfigEntrada->AccionEsprint, ETriggerEvent::Completed, this, &ThisClass::ManejarEsprint);
	}
	if (ConfigEntrada->AccionCaminar)
	{
		Entrada->BindAction(ConfigEntrada->AccionCaminar, ETriggerEvent::Triggered, this, &ThisClass::ManejarCaminar);
		Entrada->BindAction(ConfigEntrada->AccionCaminar, ETriggerEvent::Completed, this, &ThisClass::ManejarCaminar);
	}
	if (ConfigEntrada->AccionDash)
	{
		Entrada->BindAction(ConfigEntrada->AccionDash, ETriggerEvent::Started, this, &ThisClass::PedirDash);
	}
	if (ConfigEntrada->AccionAtacar)
	{
		Entrada->BindAction(ConfigEntrada->AccionAtacar, ETriggerEvent::Started, this, &ThisClass::PedirAtaque);
		Entrada->BindAction(ConfigEntrada->AccionAtacar, ETriggerEvent::Completed, this, &ThisClass::SoltarAtaque);
	}
	if (ConfigEntrada->AccionGuardia)
	{
		Entrada->BindAction(ConfigEntrada->AccionGuardia, ETriggerEvent::Started, this, &ThisClass::ManejarGuardia);
		Entrada->BindAction(ConfigEntrada->AccionGuardia, ETriggerEvent::Completed, this, &ThisClass::ManejarGuardia);
	}
	if (ConfigEntrada->AccionInteractuar)
	{
		Entrada->BindAction(ConfigEntrada->AccionInteractuar, ETriggerEvent::Started, this, &ThisClass::PedirInteraccion);
	}
	if (ConfigEntrada->AccionCurar)
	{
		Entrada->BindAction(ConfigEntrada->AccionCurar, ETriggerEvent::Started, this, &ThisClass::ManejarCurar);
		Entrada->BindAction(ConfigEntrada->AccionCurar, ETriggerEvent::Completed, this, &ThisClass::ManejarCurar);
	}
	if (ConfigEntrada->AccionRecargar)
	{
		Entrada->BindAction(ConfigEntrada->AccionRecargar, ETriggerEvent::Started, this, &ThisClass::PedirRecarga);
	}
	if (ConfigEntrada->AccionSiguienteArma)
	{
		Entrada->BindAction(ConfigEntrada->AccionSiguienteArma, ETriggerEvent::Started, this, &ThisClass::PedirSiguienteArma);
	}
	for (int32 Ranura = 0; Ranura < ConfigEntrada->AccionesRanuraArma.Num(); ++Ranura)
	{
		if (ConfigEntrada->AccionesRanuraArma[Ranura])
		{
			Entrada->BindAction(ConfigEntrada->AccionesRanuraArma[Ranura], ETriggerEvent::Started, this, &ThisClass::PedirRanuraArma, Ranura);
		}
	}
}

void ASAEVUSPersonaje::ManejarMover(const FInputActionValue& Valor)
{
	AplicarEntradaMovimiento(Valor.Get<FVector2D>());
}

void ASAEVUSPersonaje::ManejarMoverSoltar(const FInputActionValue& Valor)
{
	AplicarEntradaMovimiento(FVector2D::ZeroVector);
}

void ASAEVUSPersonaje::AplicarEntradaMovimiento(const FVector2D& Eje)
{
	if (bSentado)
	{
		// Sentado no se mueve; empujar en cualquier dirección levanta.
		if (Eje.IsNearlyZero())
		{
			return;
		}
		Levantarse();
	}
	const FRotationMatrix MatrizGuinada(FRotator(0.f, ObtenerRotacionEntrada().Yaw, 0.f));
	const FVector Adelante = MatrizGuinada.GetUnitAxis(EAxis::X);
	const FVector Derecha = MatrizGuinada.GetUnitAxis(EAxis::Y);
	if (!Eje.IsZero())
	{
		AddMovementInput(Adelante, Eje.Y);
		AddMovementInput(Derecha, Eje.X);
	}

	if (ConfigEntrada && !ConfigEntrada->bDashDobleToque)
	{
		return;
	}
	const FVector2D Toque = DetectorDobleToque.Actualizar(Eje, GetWorld()->GetTimeSeconds());
	if (!Toque.IsZero())
	{
		PedirDashHacia(Adelante * Toque.Y + Derecha * Toque.X);
	}
}

void ASAEVUSPersonaje::ManejarMirar(const FInputActionValue& Valor)
{
	if (CamaraBloqueada())
	{
		return;
	}
	const ASAEVUSControladorJugador* Controlador = Cast<ASAEVUSControladorJugador>(Controller);
	const FVector2D Eje = Valor.Get<FVector2D>() * (Controlador ? Controlador->ObtenerSensibilidad() : 1.f);
	AddControllerYawInput(Eje.X);
	AddControllerPitchInput(Eje.Y);
}

void ASAEVUSPersonaje::ManejarEsprint(const FInputActionValue& Valor)
{
	ObtenerMovimiento()->PedirEsprint(Valor.Get<bool>());
}

void ASAEVUSPersonaje::ManejarCaminar(const FInputActionValue& Valor)
{
	ObtenerMovimiento()->PedirCaminar(Valor.Get<bool>());
}

void ASAEVUSPersonaje::PedirDash()
{
	Levantarse();
	if (Combate->IntentarCancelarPor(ESAEVUSAccionCancelacion::Dash))
	{
		Alma->CancelarCuracion();
		ObtenerMovimiento()->FijarDashArmaFuego(Armas->TieneArmaFuegoEquipada());
		ObtenerMovimiento()->PedirDash();
	}
}

void ASAEVUSPersonaje::PedirDashHacia(const FVector& Direccion)
{
	if (Combate->IntentarCancelarPor(ESAEVUSAccionCancelacion::Dash))
	{
		Alma->CancelarCuracion();
		ObtenerMovimiento()->FijarDashArmaFuego(Armas->TieneArmaFuegoEquipada());
		ObtenerMovimiento()->PedirDashEnDireccion(Direccion);
	}
}

void ASAEVUSPersonaje::FijarCuracion(bool bPulsada)
{
	// En guardia no se cura.
	Alma->FijarCuracionPedida(bPulsada && !Guardia->EstaPedida());
}

void ASAEVUSPersonaje::FijarGuardia(bool bPulsada)
{
	if (bPulsada)
	{
		Alma->CancelarCuracion();
	}
	Guardia->FijarGuardiaPedida(bPulsada);
}

void ASAEVUSPersonaje::ManejarGuardia(const FInputActionValue& Valor)
{
	FijarGuardia(Valor.Get<bool>());
}

void ASAEVUSPersonaje::ManejarCurar(const FInputActionValue& Valor)
{
	FijarCuracion(Valor.Get<bool>());
}

void ASAEVUSPersonaje::PedirAtaque()
{
	Levantarse();
	if (Armas->TieneArmaFuegoEquipada())
	{
		Armas->FijarGatillo(true);
	}
	else if (!Armas->EstaCambiando())
	{
		Alma->CancelarCuracion();
		// Atacar suelta la guardia; si sigue pulsada vuelve en la ventana de cancelación del tajo.
		Guardia->Interrumpir();
		Combate->FijarAtaqueMantenido(true);
		// Arma cuerpo a cuerpo, o sin arsenal (repertorio por defecto del combate). La inclinación de la
		// cámara elige el tajo: arriba, de frente o abajo (en el aire).
		const float Inclinacion = Controller ? static_cast<float>(FRotator::NormalizeAxis(GetControlRotation().Pitch)) : 0.f;
		Combate->PedirAtaqueEnDireccion(Combate->DireccionSegunInclinacion(Inclinacion));
	}
}

void ASAEVUSPersonaje::SoltarAtaque()
{
	Armas->FijarGatillo(false);
	Combate->FijarAtaqueMantenido(false);
}

void ASAEVUSPersonaje::PedirRanuraArma(int32 Ranura)
{
	if (Armas->PuedeCambiarA(Ranura) && Combate->IntentarCancelarPor(ESAEVUSAccionCancelacion::CambioArma))
	{
		Armas->PedirCambio(Ranura);
	}
}

void ASAEVUSPersonaje::PedirSiguienteArma()
{
	PedirRanuraArma(Armas->ObtenerSiguienteRanura());
}

void ASAEVUSPersonaje::PedirRecarga()
{
	Armas->PedirRecarga();
}
