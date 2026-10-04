// Copyright SAEVUS. Todos los derechos reservados.

#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/CapsuleComponent.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Enemigos/SAEVUSProyectil.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

namespace SAEVUSIAEnemigo
{
	const FSAEVUSFaseEnemigo FaseUnica;
	// La patrulla va a media velocidad: se lee como "no te ha visto".
	constexpr float EscalaVelocidadPatrulla = 0.5f;
	constexpr float DistanciaSinObjetivo = TNumericLimits<float>::Max();
}

USAEVUSComponenteIAEnemigo::USAEVUSComponenteIAEnemigo()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void USAEVUSComponenteIAEnemigo::BeginPlay()
{
	Super::BeginPlay();
	Origen = GetOwner()->GetActorLocation();
	if (USAEVUSComponenteVida* Vida = GetOwner()->FindComponentByClass<USAEVUSComponenteVida>())
	{
		Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::AlCambiarVida);
	}
}

ASAEVUSEnemigo* USAEVUSComponenteIAEnemigo::ObtenerEnemigo() const
{
	return Cast<ASAEVUSEnemigo>(GetOwner());
}

const FSAEVUSAjusteEnemigo& USAEVUSComponenteIAEnemigo::ObtenerAjuste() const
{
	return ObtenerEnemigo()->ObtenerAjuste();
}

const FSAEVUSFaseEnemigo& USAEVUSComponenteIAEnemigo::ObtenerFaseActual() const
{
	const TArray<FSAEVUSFaseEnemigo>& Fases = ObtenerAjuste().Fases;
	return Fases.IsValidIndex(Fase) ? Fases[Fase] : SAEVUSIAEnemigo::FaseUnica;
}

APawn* USAEVUSComponenteIAEnemigo::ObtenerObjetivo() const
{
	return Objetivo.IsValid() ? Objetivo.Get() : UGameplayStatics::GetPlayerPawn(this, 0);
}

bool USAEVUSComponenteIAEnemigo::ObjetivoVivo() const
{
	const APawn* Pawn = ObtenerObjetivo();
	if (!Pawn)
	{
		return false;
	}
	const USAEVUSComponenteVida* Vida = Pawn->FindComponentByClass<USAEVUSComponenteVida>();
	return !Vida || !Vida->EstaMuerto();
}

float USAEVUSComponenteIAEnemigo::DistanciaObjetivo() const
{
	return ObjetivoVivo() ? static_cast<float>(FVector::Dist2D(ObtenerObjetivo()->GetActorLocation(), GetOwner()->GetActorLocation()))
		: SAEVUSIAEnemigo::DistanciaSinObjetivo;
}

FVector USAEVUSComponenteIAEnemigo::DireccionObjetivo() const
{
	const APawn* Pawn = ObtenerObjetivo();
	return Pawn ? (Pawn->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal2D() : GetOwner()->GetActorForwardVector();
}

const FSAEVUSAtaqueEnemigo* USAEVUSComponenteIAEnemigo::ObtenerAtaqueActual() const
{
	const TArray<FSAEVUSAtaqueEnemigo>& Ataques = ObtenerAjuste().Ataques;
	return Ataques.IsValidIndex(IndiceAtaque) ? &Ataques[IndiceAtaque] : nullptr;
}

float USAEVUSComponenteIAEnemigo::ObtenerProgresoAviso() const
{
	const FSAEVUSAtaqueEnemigo* Ataque = ObtenerAtaqueActual();
	if (Estado != ESAEVUSEstadoEnemigo::Telegrafiar || !Ataque)
	{
		return 0.f;
	}
	const float Duracion = Ataque->TiempoTelegrafiar * ObtenerFaseActual().EscalaTiempos;
	return Duracion > 0.f ? FMath::Clamp(TiempoEstado / Duracion, 0.f, 1.f) : 1.f;
}

void USAEVUSComponenteIAEnemigo::CambiarEstado(ESAEVUSEstadoEnemigo NuevoEstado)
{
	const bool bCambia = Estado != NuevoEstado;
	Estado = NuevoEstado;
	TiempoEstado = 0.f;
	// Fuera del reposo lejano, la IA y el movimiento vuelven a actualizarse cada fotograma.
	FijarReposoLejano(false);
	if (bCambia)
	{
		AlCambiarEstado.Broadcast(Estado);
	}
}

void USAEVUSComponenteIAEnemigo::Aturdir()
{
	if (Estado == ESAEVUSEstadoEnemigo::Muerto || ObtenerAjuste().bEsJefe || ObtenerAjuste().TiempoAturdido <= 0.f)
	{
		return;
	}
	IndiceAtaque = INDEX_NONE;
	CambiarEstado(ESAEVUSEstadoEnemigo::Aturdido);
}

void USAEVUSComponenteIAEnemigo::Morir()
{
	IndiceAtaque = INDEX_NONE;
	CambiarEstado(ESAEVUSEstadoEnemigo::Muerto);
}

void USAEVUSComponenteIAEnemigo::AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador)
{
	ActualizarFase();
	if (Delta < 0.f && !ComponenteVida->EstaMuerto())
	{
		Aturdir();
	}
}

void USAEVUSComponenteIAEnemigo::ActualizarFase()
{
	const USAEVUSComponenteVida* Vida = GetOwner()->FindComponentByClass<USAEVUSComponenteVida>();
	const TArray<FSAEVUSFaseEnemigo>& Fases = ObtenerAjuste().Fases;
	if (!Vida || Fases.IsEmpty())
	{
		return;
	}
	const float Fraccion = Vida->ObtenerVida() / Vida->ObtenerVidaMaxima();
	int32 NuevaFase = 0;
	for (int32 Indice = 0; Indice < Fases.Num(); ++Indice)
	{
		if (Fraccion <= Fases[Indice].FraccionVida)
		{
			NuevaFase = Indice;
		}
	}
	// Las fases solo avanzan.
	if (NuevaFase > Fase)
	{
		Fase = NuevaFase;
		AlCambiarFase.Broadcast(Fase);
	}
}

void USAEVUSComponenteIAEnemigo::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Actualizar(DeltaTime);
}

void USAEVUSComponenteIAEnemigo::Actualizar(float DeltaTime)
{
	if (Estado == ESAEVUSEstadoEnemigo::Muerto || !ObtenerEnemigo())
	{
		return;
	}
	TiempoEstado += DeltaTime;
	const FSAEVUSFaseEnemigo& FaseActual = ObtenerFaseActual();
	const FSAEVUSAtaqueEnemigo* Ataque = ObtenerAtaqueActual();
	// La fase acelera también el paso.
	ObtenerEnemigo()->GetCharacterMovement()->MaxWalkSpeed = ObtenerAjuste().VelocidadMovimiento * FaseActual.EscalaVelocidad;

	switch (Estado)
	{
	case ESAEVUSEstadoEnemigo::Reposo:
		ActualizarReposo();
		break;
	case ESAEVUSEstadoEnemigo::Perseguir:
		ActualizarPerseguir();
		break;
	case ESAEVUSEstadoEnemigo::Telegrafiar:
		ActualizarTelegrafiar();
		break;
	case ESAEVUSEstadoEnemigo::Atacar:
		ActualizarAtacar();
		break;
	case ESAEVUSEstadoEnemigo::Recuperarse:
		if (!Ataque || TiempoEstado >= Ataque->TiempoRecuperacion * FaseActual.EscalaTiempos)
		{
			IndiceAtaque = INDEX_NONE;
			CambiarEstado(ESAEVUSEstadoEnemigo::Perseguir);
		}
		break;
	case ESAEVUSEstadoEnemigo::Aturdido:
		if (TiempoEstado >= ObtenerAjuste().TiempoAturdido)
		{
			CambiarEstado(ESAEVUSEstadoEnemigo::Perseguir);
		}
		break;
	default:
		break;
	}
	ComprobarContacto();
}

void USAEVUSComponenteIAEnemigo::ActualizarReposo()
{
	const FSAEVUSAjusteEnemigo& Ajuste = ObtenerAjuste();
	const float Distancia = DistanciaObjetivo();
	if (Distancia <= Ajuste.RadioDeteccion)
	{
		CambiarEstado(ESAEVUSEstadoEnemigo::Perseguir);
		return;
	}
	const bool bLejos = Distancia > Ajuste.RadioDeteccion * Ajuste.FactorDistanciaReposoLejos;
	FijarReposoLejano(bLejos);
	if (bLejos)
	{
		return;
	}
	if (Ajuste.RadioPatrulla <= 0.f)
	{
		return;
	}
	// Ida y vuelta por el eje X del origen.
	const float Desplazamiento = static_cast<float>(GetOwner()->GetActorLocation().X - Origen.X);
	if (Desplazamiento * SentidoPatrulla >= Ajuste.RadioPatrulla)
	{
		SentidoPatrulla = -SentidoPatrulla;
	}
	MoverHacia(FVector(SentidoPatrulla, 0.f, 0.f), SAEVUSIAEnemigo::EscalaVelocidadPatrulla);
}

void USAEVUSComponenteIAEnemigo::FijarReposoLejano(bool bLejos)
{
	const float Intervalo = bLejos ? ObtenerAjuste().IntervaloReposoLejos : 0.f;
	SetComponentTickInterval(Intervalo);
	// El movimiento también descansa, pero solo de pie: uno que cae sigue simulando hasta aterrizar.
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	if (UCharacterMovementComponent* Movimiento = Personaje ? Personaje->GetCharacterMovement() : nullptr)
	{
		Movimiento->SetComponentTickInterval(bLejos && Movimiento->IsMovingOnGround() ? Intervalo : 0.f);
	}
}

void USAEVUSComponenteIAEnemigo::ActualizarPerseguir()
{
	const float Distancia = DistanciaObjetivo();
	if (Distancia > ObtenerAjuste().RadioPerdida)
	{
		CambiarEstado(ESAEVUSEstadoEnemigo::Reposo);
		return;
	}
	if (ElegirAtaque(Distancia))
	{
		Encarar(DireccionObjetivo());
		CambiarEstado(ESAEVUSEstadoEnemigo::Telegrafiar);
		return;
	}
	// Los de distancia retroceden si el objetivo se les echa encima.
	const bool bRetirarse = Distancia < ObtenerAjuste().DistanciaRetirada;
	MoverHacia(bRetirarse ? -DireccionObjetivo() : DireccionObjetivo(), 1.f);
}

void USAEVUSComponenteIAEnemigo::ActualizarTelegrafiar()
{
	// Quieto y mirando al objetivo: el aviso es la ventana del jugador para leer el ataque.
	Encarar(DireccionObjetivo());
	const FSAEVUSAtaqueEnemigo* Ataque = ObtenerAtaqueActual();
	if (!Ataque || TiempoEstado >= Ataque->TiempoTelegrafiar * ObtenerFaseActual().EscalaTiempos)
	{
		EmpezarAtaque();
	}
}

void USAEVUSComponenteIAEnemigo::EmpezarAtaque()
{
	const FSAEVUSAtaqueEnemigo* Ataque = ObtenerAtaqueActual();
	ASAEVUSEnemigo* Enemigo = ObtenerEnemigo();
	if (!Ataque)
	{
		CambiarEstado(ESAEVUSEstadoEnemigo::Perseguir);
		return;
	}
	// La dirección se fija al acabar el aviso: el jugador que se mueve a tiempo lo esquiva.
	DireccionAtaque = DireccionObjetivo();
	bGolpeAplicado = false;
	bDejoSuelo = false;
	CambiarEstado(ESAEVUSEstadoEnemigo::Atacar);
	if (Ataque->Tipo == ESAEVUSTipoAtaqueEnemigo::Proyectil)
	{
		LanzarProyectil(*Ataque);
	}
	if (Ataque->Tipo == ESAEVUSTipoAtaqueEnemigo::Salto)
	{
		const float Escala = ObtenerFaseActual().EscalaVelocidad;
		Enemigo->LaunchCharacter(DireccionAtaque * Ataque->Velocidad * Escala + FVector::UpVector * Ataque->VelocidadVertical, true, true);
	}
}

void USAEVUSComponenteIAEnemigo::ActualizarAtacar()
{
	const FSAEVUSAtaqueEnemigo* Ataque = ObtenerAtaqueActual();
	ASAEVUSEnemigo* Enemigo = ObtenerEnemigo();
	UCharacterMovementComponent* Movimiento = Enemigo->GetCharacterMovement();
	if (!Ataque)
	{
		CambiarEstado(ESAEVUSEstadoEnemigo::Perseguir);
		return;
	}
	bool bTermino = TiempoEstado >= Ataque->Duracion;
	switch (Ataque->Tipo)
	{
	case ESAEVUSTipoAtaqueEnemigo::Embestida:
		if (!bTermino)
		{
			// Tope de velocidad subido y entrada hacia delante: el movimiento no frena la carrera.
			const float Velocidad = Ataque->Velocidad * ObtenerFaseActual().EscalaVelocidad;
			Movimiento->MaxWalkSpeed = Velocidad;
			const float VelocidadVertical = static_cast<float>(Movimiento->Velocity.Z);
			Movimiento->Velocity = DireccionAtaque * Velocidad;
			Movimiento->Velocity.Z = VelocidadVertical;
			Enemigo->AddMovementInput(DireccionAtaque, 1.f);
		}
		else
		{
			Movimiento->Velocity = FVector::ZeroVector;
		}
		break;
	case ESAEVUSTipoAtaqueEnemigo::Salto:
		bDejoSuelo |= Movimiento->IsFalling();
		bTermino |= bDejoSuelo && Movimiento->IsMovingOnGround();
		break;
	case ESAEVUSTipoAtaqueEnemigo::Proyectil:
		// Lanzado al empezar; queda quieto la duración del ataque.
		break;
	case ESAEVUSTipoAtaqueEnemigo::Golpe:
		if (!bGolpeAplicado)
		{
			bGolpeAplicado = true;
			const APawn* Pawn = ObtenerObjetivo();
			const FVector Centro = Enemigo->GetActorLocation() + DireccionAtaque * Ataque->AlcanceGolpe;
			if (Pawn && ObjetivoVivo() && FVector::Dist(Pawn->GetActorLocation(), Centro) <= Ataque->RadioGolpe)
			{
				GolpearObjetivo(Ataque->Danio);
			}
		}
		break;
	}
	if (bTermino)
	{
		CambiarEstado(ESAEVUSEstadoEnemigo::Recuperarse);
	}
}

void USAEVUSComponenteIAEnemigo::LanzarProyectil(const FSAEVUSAtaqueEnemigo& Ataque)
{
	ASAEVUSEnemigo* Enemigo = ObtenerEnemigo();
	const APawn* Pawn = ObtenerObjetivo();
	// Sale de delante del cuerpo, a la altura de la máscara, apuntando al centro del objetivo.
	const FVector Boca = Enemigo->GetActorLocation() + DireccionAtaque * (Enemigo->ObtenerAjuste().RadioCapsula + Ataque.AlcanceGolpe * 0.25f);
	const FVector Destino = Pawn ? Pawn->GetActorLocation() : Boca + DireccionAtaque;
	FActorSpawnParameters Parametros;
	Parametros.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Parametros.Owner = Enemigo;
	if (ASAEVUSProyectil* Proyectil = GetWorld()->SpawnActor<ASAEVUSProyectil>(Boca, FRotator::ZeroRotator, Parametros))
	{
		Proyectil->Lanzar(Destino - Boca, Ataque.Velocidad * ObtenerFaseActual().EscalaVelocidad, Ataque.Danio, Enemigo, Enemigo->ObtenerMaterialProyectil());
	}
}

bool USAEVUSComponenteIAEnemigo::FaseAdmiteAtaque(int32 Indice) const
{
	const TArray<int32>& Permitidos = ObtenerFaseActual().Ataques;
	return Permitidos.IsEmpty() || Permitidos.Contains(Indice);
}

bool USAEVUSComponenteIAEnemigo::ElegirAtaque(float Distancia)
{
	const TArray<FSAEVUSAtaqueEnemigo>& Ataques = ObtenerAjuste().Ataques;
	// En rotación desde el siguiente al último usado: variedad predecible (y comprobable).
	for (int32 Paso = 0; Paso < Ataques.Num(); ++Paso)
	{
		const int32 Indice = (SiguienteAtaque + Paso) % Ataques.Num();
		const FSAEVUSAtaqueEnemigo& Ataque = Ataques[Indice];
		if (FaseAdmiteAtaque(Indice) && Distancia >= Ataque.DistanciaMinima && Distancia <= Ataque.DistanciaMaxima)
		{
			IndiceAtaque = Indice;
			SiguienteAtaque = (Indice + 1) % Ataques.Num();
			return true;
		}
	}
	return false;
}

void USAEVUSComponenteIAEnemigo::ComprobarContacto()
{
	APawn* Pawn = ObtenerObjetivo();
	if (!Pawn || !ObjetivoVivo())
	{
		return;
	}
	const UCapsuleComponent* MiCapsula = ObtenerEnemigo()->GetCapsuleComponent();
	const UCapsuleComponent* SuCapsula = Cast<UCapsuleComponent>(Pawn->GetRootComponent());
	if (!SuCapsula)
	{
		return;
	}
	const FVector Delta = Pawn->GetActorLocation() - GetOwner()->GetActorLocation();
	const float SumaRadios = MiCapsula->GetScaledCapsuleRadius() + SuCapsula->GetScaledCapsuleRadius() + ObtenerAjuste().MargenContacto;
	const float SumaAlturas = MiCapsula->GetScaledCapsuleHalfHeight() + SuCapsula->GetScaledCapsuleHalfHeight() + ObtenerAjuste().MargenContacto;
	if (Delta.Size2D() > SumaRadios || FMath::Abs(Delta.Z) > SumaAlturas)
	{
		return;
	}
	// Durante una embestida o un salto el contacto pega con la fuerza del ataque.
	const FSAEVUSAtaqueEnemigo* Ataque = ObtenerAtaqueActual();
	const bool bAtaqueDeCuerpo = Estado == ESAEVUSEstadoEnemigo::Atacar && Ataque && Ataque->Tipo != ESAEVUSTipoAtaqueEnemigo::Golpe;
	GolpearObjetivo(bAtaqueDeCuerpo ? Ataque->Danio : ObtenerAjuste().DanioContacto);
}

void USAEVUSComponenteIAEnemigo::GolpearObjetivo(float Danio)
{
	// La invulnerabilidad del objetivo (tras golpe, dash) evita que el contacto pegue cada fotograma.
	if (APawn* Pawn = ObtenerObjetivo(); Pawn && Danio > 0.f)
	{
		UGameplayStatics::ApplyDamage(Pawn, Danio, nullptr, GetOwner(), UDamageType::StaticClass());
	}
}

void USAEVUSComponenteIAEnemigo::MoverHacia(const FVector& Direccion, float Escala)
{
	ObtenerEnemigo()->AddMovementInput(Direccion, Escala);
}

void USAEVUSComponenteIAEnemigo::Encarar(const FVector& Direccion)
{
	if (!Direccion.IsNearlyZero())
	{
		GetOwner()->SetActorRotation(Direccion.ToOrientationRotator());
	}
}
