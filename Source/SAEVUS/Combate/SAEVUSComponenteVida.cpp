// Copyright SAEVUS. Todos los derechos reservados.

#include "Combate/SAEVUSComponenteVida.h"
#include "Depuracion/SAEVUSSubsistemaNumerosDanio.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

USAEVUSComponenteVida::USAEVUSComponenteVida()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void USAEVUSComponenteVida::InitializeComponent()
{
	Super::InitializeComponent();
	// Válida antes de BeginPlay, así otros componentes la pueden leer al inicializarse.
	Vida = VidaMaxima;
}

void USAEVUSComponenteVida::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* Duenio = GetOwner())
	{
		Duenio->OnTakeAnyDamage.AddUniqueDynamic(this, &ThisClass::ManejarDanioRecibido);
	}
}

void USAEVUSComponenteVida::ManejarDanioRecibido(AActor* ActorDaniado, float Danio, const UDamageType* TipoDanio, AController* InstigadoPor, AActor* Causante)
{
	if (bIgnorarDanioPropio && Causante == GetOwner())
	{
		return;
	}
	AplicarDanio(Danio, Causante);
}

float USAEVUSComponenteVida::AplicarDanio(float Cantidad, AActor* Causante)
{
	if (bMuerto || Cantidad <= 0.f || EstaInvulnerable())
	{
		return 0.f;
	}
	if (FiltroDanio.IsBound())
	{
		Cantidad = FiltroDanio.Execute(Cantidad, Causante);
		if (Cantidad <= 0.f)
		{
			return 0.f;
		}
	}
	const float Quitada = FMath::Min(Cantidad, Vida);
	Vida -= Quitada;
	if (Vida > 0.f && DuracionInvulnerableTrasGolpe > 0.f)
	{
		HacerInvulnerable(DuracionInvulnerableTrasGolpe);
		InvulnerableTrasGolpeHasta = GetWorld()->GetTimeSeconds() + DuracionInvulnerableTrasGolpe;
	}
	if (const AActor* Duenio = GetOwner())
	{
		const APawn* Pawn = Cast<APawn>(Duenio);
		USAEVUSSubsistemaNumerosDanio::Registrar(this, Duenio->GetActorLocation() + FVector::UpVector * Duenio->GetSimpleCollisionHalfHeight(),
			Quitada, Pawn && Pawn->IsPlayerControlled());
	}
	AlCambiarVida.Broadcast(this, -Quitada, Causante);
	if (Vida <= 0.f)
	{
		Vida = 0.f;
		bMuerto = true;
		AlMorir.Broadcast(this, Causante);
	}
	return Quitada;
}

float USAEVUSComponenteVida::Curar(float Cantidad)
{
	if (bMuerto || Cantidad <= 0.f)
	{
		return 0.f;
	}
	const float Sumada = FMath::Min(Cantidad, VidaMaxima - Vida);
	if (Sumada > 0.f)
	{
		Vida += Sumada;
		AlCambiarVida.Broadcast(this, Sumada, nullptr);
	}
	return Sumada;
}

void USAEVUSComponenteVida::HacerInvulnerable(float Duracion)
{
	if (const UWorld* Mundo = GetWorld())
	{
		InvulnerableHasta = FMath::Max(InvulnerableHasta, Mundo->GetTimeSeconds() + Duracion);
	}
}

bool USAEVUSComponenteVida::EstaInvulnerable() const
{
	const UWorld* Mundo = GetWorld();
	return Mundo && Mundo->GetTimeSeconds() < InvulnerableHasta;
}

void USAEVUSComponenteVida::FijarVidaMaxima(float NuevaVidaMaxima)
{
	VidaMaxima = FMath::Max(1.f, NuevaVidaMaxima);
	const float Delta = VidaMaxima - Vida;
	Vida = VidaMaxima;
	bMuerto = false;
	if (Delta != 0.f)
	{
		AlCambiarVida.Broadcast(this, Delta, nullptr);
	}
}

float USAEVUSComponenteVida::MedirDanio(TFunctionRef<void()> Aplicar)
{
	const float Antes = Vida;
	Aplicar();
	return FMath::Max(0.f, Antes - Vida);
}

bool USAEVUSComponenteVida::PuedeRecibirDanio(const AActor* Duenio, float Danio)
{
	return Duenio && Duenio->GetLocalRole() == ROLE_Authority && Duenio->CanBeDamaged() && Danio != 0.f;
}

bool USAEVUSComponenteVida::EstaEnInvulnerabilidadTrasGolpe() const
{
	const UWorld* Mundo = GetWorld();
	return Mundo && Mundo->GetTimeSeconds() < InvulnerableTrasGolpeHasta;
}

void USAEVUSComponenteVida::RestablecerVida()
{
	const float Delta = VidaMaxima - Vida;
	Vida = VidaMaxima;
	bMuerto = false;
	if (Delta != 0.f)
	{
		AlCambiarVida.Broadcast(this, Delta, nullptr);
	}
}
