// Copyright SAEVUS. Todos los derechos reservados.

#include "Enemigos/SAEVUSComponenteEscudo.h"
#include "Audio/SAEVUSSubsistemaSonidoMundo.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Efectos/SAEVUSComponenteParticulas.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

namespace SAEVUSEscudo
{
	// Chispas del bloqueo, delante del escudo.
	constexpr int32 ParticulasBloqueo = 14;
	constexpr float VelocidadParticulasBloqueo = 450.f;
	constexpr float DispersionParticulasBloqueo = 200.f;
	constexpr float AdelanteChispas = 50.f;
	// El empuje al atacante lo levanta un poco del suelo, así se nota.
	constexpr float EmpujeVertical = 150.f;
}

void USAEVUSComponenteEscudo::BeginPlay()
{
	Super::BeginPlay();
	if (USAEVUSComponenteVida* Vida = GetOwner()->FindComponentByClass<USAEVUSComponenteVida>())
	{
		Vida->FiltroDanio.BindUObject(this, &ThisClass::Filtrar);
	}
}

void USAEVUSComponenteEscudo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USAEVUSComponenteVida* Vida = GetOwner()->FindComponentByClass<USAEVUSComponenteVida>())
	{
		Vida->FiltroDanio.Unbind();
	}
	Super::EndPlay(EndPlayReason);
}

bool USAEVUSComponenteEscudo::EstaRoto() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < RotoHasta;
}

float USAEVUSComponenteEscudo::Filtrar(float Cantidad, AActor* Causante)
{
	using namespace SAEVUSEscudo;
	const ASAEVUSEnemigo* Enemigo = Cast<ASAEVUSEnemigo>(GetOwner());
	if (!Enemigo || !Causante || EstaRoto())
	{
		return Cantidad;
	}
	const FSAEVUSAjusteEnemigo& Ajuste = Enemigo->ObtenerAjuste();
	if (Ajuste.AnguloEscudo <= 0.f)
	{
		return Cantidad;
	}
	// De frente: dentro de la mitad del ángulo a cada lado. Desde encima (pogo) no hay dirección y pasa.
	const FVector HaciaCausante = (Causante->GetActorLocation() - Enemigo->GetActorLocation()).GetSafeNormal2D();
	const float Coseno = FVector::DotProduct(HaciaCausante, Enemigo->GetActorForwardVector().GetSafeNormal2D());
	if (HaciaCausante.IsNearlyZero() || Coseno < FMath::Cos(FMath::DegreesToRadians(Ajuste.AnguloEscudo * 0.5f)))
	{
		return Cantidad;
	}
	if (Ajuste.DanioRompeEscudo > 0.f && Cantidad >= Ajuste.DanioRompeEscudo)
	{
		// Un golpe fuerte lo rompe y pasa entero.
		RotoHasta = GetWorld()->GetTimeSeconds() + Ajuste.TiempoEscudoRoto;
		USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Escudo, Enemigo->GetActorLocation());
		return Cantidad;
	}
	++Bloqueos;
	if (ACharacter* Atacante = Cast<ACharacter>(Causante))
	{
		Atacante->LaunchCharacter(HaciaCausante * Ajuste.EmpujeEscudo + FVector::UpVector * EmpujeVertical, true, true);
	}
	if (USAEVUSComponenteParticulas* Efectos = Enemigo->FindComponentByClass<USAEVUSComponenteParticulas>())
	{
		Efectos->EmitirRafaga(Enemigo->GetActorLocation() + HaciaCausante * (Ajuste.RadioCapsula + AdelanteChispas),
			HaciaCausante * VelocidadParticulasBloqueo, ParticulasBloqueo, DispersionParticulasBloqueo);
	}
	USAEVUSSubsistemaSonidoMundo::Reproducir(this, ESAEVUSSonidoMundo::Escudo, Enemigo->GetActorLocation());
	return 0.f;
}
