// Copyright SAEVUS. Todos los derechos reservados.

#include "Enemigos/SAEVUSComponenteIAEnemigo.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Enemigos/SAEVUSDirectorCombate.h"
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
	// Ajuste de respaldo si el componente está en un actor que no es un enemigo.
	const FSAEVUSAjusteEnemigo AjusteVacio;
	// La patrulla va a media velocidad: se lee como "no te ha visto".
	constexpr float EscalaVelocidadPatrulla = 0.5f;
	constexpr float DistanciaSinObjetivo = TNumericLimits<float>::Max();
	// Voladores: cuánto pesa la diferencia de altura frente a avanzar (más = corrige la altura antes).
	constexpr float GananciaAltura = 2.f;
	// Tras recibir daño, el enemigo sabe dónde está el agresor y no lo pierde de vista por distancia durante este tiempo.
	constexpr float DuracionAlertaPorDanio = 6.f;
	// Cada cuánto se mira si hay pared entre el enemigo y el objetivo.
	constexpr float IntervaloVista = 0.2f;
	// Patrulla: por debajo de esta velocidad (cm/s) durante este tiempo (s) está contra una pared y da la vuelta.
	constexpr float VelocidadBloqueado = 15.f;
	constexpr float TiempoParaDarLaVuelta = 0.4f;
	// Los quietos también vuelven a su puesto si algo los saca de él más de esto (cm).
	constexpr float RadioPuestoMinimo = 150.f;
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
	AleatorioAtaques.Initialize(static_cast<int32>(GetOwner()->GetUniqueID()));
	EjePatrulla = GetOwner()->GetActorForwardVector().GetSafeNormal2D();
	if (EjePatrulla.IsNearlyZero())
	{
		EjePatrulla = FVector::ForwardVector;
	}
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
	const ASAEVUSEnemigo* Enemigo = ObtenerEnemigo();
	return Enemigo ? Enemigo->ObtenerAjuste() : SAEVUSIAEnemigo::AjusteVacio;
}

const FSAEVUSFaseEnemigo& USAEVUSComponenteIAEnemigo::ObtenerFaseActual() const
{
	const TArray<FSAEVUSFaseEnemigo>& Fases = ObtenerAjuste().Fases;
	return Fases.IsValidIndex(Fase) ? Fases[Fase] : SAEVUSIAEnemigo::FaseUnica;
}

APawn* USAEVUSComponenteIAEnemigo::ObtenerObjetivo() const
{
	if (Objetivo.IsValid())
	{
		return Objetivo.Get();
	}
	// El pawn del jugador se busca una vez por fotograma, no en cada consulta de la IA.
	if (FotogramaObjetivo != GFrameCounter)
	{
		FotogramaObjetivo = GFrameCounter;
		ObjetivoJugador = UGameplayStatics::GetPlayerPawn(this, 0);
	}
	return ObjetivoJugador.Get();
}

bool USAEVUSComponenteIAEnemigo::ObjetivoVivo() const
{
	const APawn* Pawn = ObtenerObjetivo();
	if (!Pawn)
	{
		return false;
	}
	// La vida del objetivo se recuerda mientras sea el mismo pawn (el jugador reaparece como otro).
	if (PawnVidaObjetivo.Get() != Pawn)
	{
		PawnVidaObjetivo = Pawn;
		VidaObjetivo = Pawn->FindComponentByClass<USAEVUSComponenteVida>();
	}
	return !VidaObjetivo.IsValid() || !VidaObjetivo->EstaMuerto();
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
	// La ficha de ataque se devuelve al dejar de avisar, atacar o recuperarse.
	if (bTieneFicha && (NuevoEstado == ESAEVUSEstadoEnemigo::Reposo || NuevoEstado == ESAEVUSEstadoEnemigo::Perseguir
		|| NuevoEstado == ESAEVUSEstadoEnemigo::Aturdido || NuevoEstado == ESAEVUSEstadoEnemigo::Muerto))
	{
		bTieneFicha = false;
		if (USAEVUSDirectorCombate* Director = GetWorld()->GetSubsystem<USAEVUSDirectorCombate>())
		{
			Director->SoltarFicha(this);
		}
	}
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

void USAEVUSComponenteIAEnemigo::Reiniciar()
{
	if (Estado == ESAEVUSEstadoEnemigo::Muerto)
	{
		return;
	}
	Fase = 0;
	IndiceAtaque = INDEX_NONE;
	TiempoAlertaRestante = 0.f;
	TiempoTransicionRestante = 0.f;
	bGolpeAplicado = false;
	CambiarEstado(ESAEVUSEstadoEnemigo::Reposo);
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
		// Un enemigo (o un jefe) al que hieren desde lejos o sin haber detectado a nadie va a por quien lo hirió (EA-2).
		if (Instigador && Instigador->IsA<APawn>())
		{
			TiempoAlertaRestante = SAEVUSIAEnemigo::DuracionAlertaPorDanio;
			if (Estado == ESAEVUSEstadoEnemigo::Reposo)
			{
				CambiarEstado(ESAEVUSEstadoEnemigo::Perseguir);
			}
		}
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
		const float Transicion = ObtenerFaseActual().DuracionTransicion;
		if (Transicion > 0.f && Estado != ESAEVUSEstadoEnemigo::Muerto && !Vida->EstaMuerto())
		{
			// Rugido: corta lo que hacía, queda quieto e intocable hasta que acaba.
			TiempoTransicionRestante = Transicion;
			IndiceAtaque = INDEX_NONE;
			if (Estado == ESAEVUSEstadoEnemigo::Telegrafiar || Estado == ESAEVUSEstadoEnemigo::Atacar || Estado == ESAEVUSEstadoEnemigo::Recuperarse)
			{
				CambiarEstado(ESAEVUSEstadoEnemigo::Perseguir);
			}
			if (USAEVUSComponenteVida* VidaMutable = GetOwner()->FindComponentByClass<USAEVUSComponenteVida>())
			{
				VidaMutable->HacerInvulnerable(Transicion);
			}
		}
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
	DeltaActual = DeltaTime;
	TiempoAlertaRestante = FMath::Max(0.f, TiempoAlertaRestante - DeltaTime);
	if (TiempoTransicionRestante > 0.f)
	{
		// Ruge quieto mirando al objetivo: sin movimiento, sin ataques y sin contacto.
		TiempoTransicionRestante = FMath::Max(0.f, TiempoTransicionRestante - DeltaTime);
		if (UCharacterMovementComponent* MovimientoQuieto = ObtenerEnemigo()->GetCharacterMovement())
		{
			MovimientoQuieto->Velocity.X = 0.0;
			MovimientoQuieto->Velocity.Y = 0.0;
		}
		Encarar(DireccionObjetivo());
		return;
	}
	const FSAEVUSFaseEnemigo& FaseActual = ObtenerFaseActual();
	const FSAEVUSAtaqueEnemigo* Ataque = ObtenerAtaqueActual();
	// La fase acelera también el paso.
	UCharacterMovementComponent* Movimiento = ObtenerEnemigo()->GetCharacterMovement();
	Movimiento->MaxWalkSpeed = Movimiento->MaxFlySpeed = ObtenerAjuste().VelocidadMovimiento * FaseActual.EscalaVelocidad;
	MantenerVuelo();

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
	if (Distancia <= Ajuste.RadioDeteccion && PuedeDetectarObjetivo())
	{
		CambiarEstado(ESAEVUSEstadoEnemigo::Perseguir);
		return;
	}
	if (Distancia > Ajuste.RadioDeteccion)
	{
		// Fuera de su radio no mira: al entrar en él, la primera comprobación de vista es inmediata.
		TiempoSinMirar = SAEVUSIAEnemigo::IntervaloVista;
	}
	const bool bLejos = Distancia > Ajuste.RadioDeteccion * Ajuste.FactorDistanciaReposoLejos;
	FijarReposoLejano(bLejos);
	if (bLejos)
	{
		return;
	}
	const FVector Posicion = GetOwner()->GetActorLocation();
	const FVector Desvio = FVector(Posicion.X - Origen.X, Posicion.Y - Origen.Y, 0.f);
	// Tras perseguir se vuelve al puesto: lejos de su zona, regresa al origen en lugar de patrullar donde esté.
	const float RadioZona = FMath::Max(Ajuste.RadioPatrulla, SAEVUSIAEnemigo::RadioPuestoMinimo);
	if (Desvio.Size() > RadioZona * 1.5f)
	{
		MoverHacia(ConAltura(-Desvio.GetSafeNormal(), static_cast<float>(Origen.Z)), SAEVUSIAEnemigo::EscalaVelocidadPatrulla);
		return;
	}
	if (Ajuste.RadioPatrulla <= 0.f)
	{
		return;
	}
	// Ida y vuelta por el eje del enemigo al aparecer (los voladores, a su altura de origen). Da la vuelta al llegar al
	// radio o al toparse con una pared (si no avanza, antes se quedaba andando contra ella).
	const float Desplazamiento = static_cast<float>(FVector::DotProduct(Desvio, EjePatrulla));
	if (Desplazamiento * SentidoPatrulla >= Ajuste.RadioPatrulla)
	{
		SentidoPatrulla = -SentidoPatrulla;
		TiempoBloqueado = 0.f;
	}
	const ACharacter* Personaje = ObtenerEnemigo();
	const float VelocidadHorizontal = Personaje ? static_cast<float>(Personaje->GetVelocity().Size2D()) : 0.f;
	TiempoBloqueado = VelocidadHorizontal < SAEVUSIAEnemigo::VelocidadBloqueado ? TiempoBloqueado + DeltaActual : 0.f;
	if (TiempoBloqueado >= SAEVUSIAEnemigo::TiempoParaDarLaVuelta)
	{
		SentidoPatrulla = -SentidoPatrulla;
		TiempoBloqueado = 0.f;
	}
	MoverHacia(ConAltura(EjePatrulla * SentidoPatrulla, static_cast<float>(Origen.Z)), SAEVUSIAEnemigo::EscalaVelocidadPatrulla);
}

bool USAEVUSComponenteIAEnemigo::PuedeDetectarObjetivo()
{
	const APawn* Pawn = ObtenerObjetivo();
	const FSAEVUSAjusteEnemigo& Ajuste = ObtenerAjuste();
	if (!Pawn)
	{
		return false;
	}
	// A distinta altura (cornisa, otro piso) no se ve; los voladores flotan a su altura de vuelo.
	const float DiferenciaAltura = FMath::Abs(static_cast<float>(Pawn->GetActorLocation().Z - GetOwner()->GetActorLocation().Z));
	const float AlturaPermitida = Ajuste.AlturaMaximaDeteccion + (Ajuste.bVolador ? Ajuste.AlturaVuelo : 0.f);
	if (DiferenciaAltura > AlturaPermitida)
	{
		return false;
	}
	if (!Ajuste.bExigirLineaDeVision)
	{
		return true;
	}
	// Una pared por medio no se mira en cada fotograma: unas pocas veces por segundo basta.
	TiempoSinMirar += DeltaActual;
	if (TiempoSinMirar >= SAEVUSIAEnemigo::IntervaloVista)
	{
		TiempoSinMirar = 0.f;
		FCollisionQueryParams Parametros(SCENE_QUERY_STAT(IAVision), false);
		Parametros.AddIgnoredActor(GetOwner());
		Parametros.AddIgnoredActor(Pawn);
		bVeObjetivo = !GetWorld()->LineTraceTestByChannel(GetOwner()->GetActorLocation(), Pawn->GetActorLocation(), ECC_Visibility, Parametros);
	}
	return bVeObjetivo;
}

void USAEVUSComponenteIAEnemigo::EsperarTurno(float Distancia)
{
	// Sin ficha de ataque: encara al jugador y se desplaza de lado a su ritmo, sin acercarse más.
	Encarar(DireccionObjetivo());
	const FVector Hacia = DireccionObjetivo();
	const float Lado = (GetOwner()->GetUniqueID() & 1u) ? 1.f : -1.f;
	FVector Direccion = FVector::CrossProduct(FVector::UpVector, Hacia) * Lado;
	// Si está demasiado encima, retrocede un poco mientras rodea.
	if (Distancia < ObtenerAjuste().DistanciaEspera)
	{
		Direccion -= Hacia * 0.5f;
	}
	MoverHacia(ConAltura(Direccion.GetSafeNormal(), static_cast<float>(Origen.Z)), SAEVUSIAEnemigo::EscalaVelocidadPatrulla);
}

FVector USAEVUSComponenteIAEnemigo::ConAltura(const FVector& DireccionHorizontal, float AlturaObjetivo) const
{
	const FSAEVUSAjusteEnemigo& Ajuste = ObtenerAjuste();
	if (!Ajuste.bVolador)
	{
		return DireccionHorizontal;
	}
	// Sube o baja con más fuerza cuanto más lejos está de su altura; a AlturaVuelo de distancia, a tope.
	const float Diferencia = AlturaObjetivo - static_cast<float>(GetOwner()->GetActorLocation().Z);
	const float Vertical = FMath::Clamp(Diferencia / FMath::Max(Ajuste.AlturaVuelo, 1.f) * SAEVUSIAEnemigo::GananciaAltura, -1.f, 1.f);
	return FVector(DireccionHorizontal.X, DireccionHorizontal.Y, Vertical).GetClampedToMaxSize(1.f);
}

void USAEVUSComponenteIAEnemigo::MantenerVuelo()
{
	UCharacterMovementComponent* Movimiento = ObtenerEnemigo()->GetCharacterMovement();
	if (ObtenerAjuste().bVolador && Estado != ESAEVUSEstadoEnemigo::Aturdido && !Movimiento->IsFlying())
	{
		Movimiento->SetMovementMode(MOVE_Flying);
	}
}

void USAEVUSComponenteIAEnemigo::FijarReposoLejano(bool bLejos)
{
	const float Intervalo = bLejos ? ObtenerAjuste().IntervaloReposoLejos : 0.f;
	SetComponentTickInterval(Intervalo);
	// El movimiento también descansa, pero solo de pie o volando: uno que cae sigue simulando hasta aterrizar.
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	if (UCharacterMovementComponent* Movimiento = Personaje ? Personaje->GetCharacterMovement() : nullptr)
	{
		Movimiento->SetComponentTickInterval(bLejos && (Movimiento->IsMovingOnGround() || Movimiento->IsFlying()) ? Intervalo : 0.f);
	}
}

void USAEVUSComponenteIAEnemigo::ActualizarPerseguir()
{
	const float Distancia = DistanciaObjetivo();
	if (Distancia > ObtenerAjuste().RadioPerdida && TiempoAlertaRestante <= 0.f)
	{
		CambiarEstado(ESAEVUSEstadoEnemigo::Reposo);
		return;
	}
	SiguienteAtaqueAntes = SiguienteAtaque;
	if (ElegirAtaque(Distancia))
	{
		// Los jefes atacan cuando quieren; el resto pide turno al director (como mucho dos avisan o atacan a la vez).
		USAEVUSDirectorCombate* Director = ObtenerAjuste().bEsJefe ? nullptr : GetWorld()->GetSubsystem<USAEVUSDirectorCombate>();
		if (Director && !Director->PedirFicha(this))
		{
			IndiceAtaque = INDEX_NONE;
			SiguienteAtaque = SiguienteAtaqueAntes;
			EsperarTurno(Distancia);
			return;
		}
		bTieneFicha = Director != nullptr;
		Encarar(DireccionObjetivo());
		CambiarEstado(ESAEVUSEstadoEnemigo::Telegrafiar);
		return;
	}
	// Los de distancia retroceden si el objetivo se les echa encima; los voladores se ponen a su altura de vuelo.
	const bool bRetirarse = Distancia < ObtenerAjuste().DistanciaRetirada;
	const APawn* Pawn = ObtenerObjetivo();
	const float AlturaObjetivo = Pawn ? static_cast<float>(Pawn->GetActorLocation().Z) + ObtenerAjuste().AlturaVuelo : static_cast<float>(Origen.Z);
	MoverHacia(ConAltura(bRetirarse ? -DireccionObjetivo() : DireccionObjetivo(), AlturaObjetivo), 1.f);
}

void USAEVUSComponenteIAEnemigo::ActualizarTelegrafiar()
{
	// Quieto y mirando al objetivo: el aviso es la ventana del jugador para leer el ataque. Pasada la fracción de
	// seguimiento deja de apuntar, así el jugador que se mueve a tiempo (o rodea) lo esquiva.
	const FSAEVUSAtaqueEnemigo* Ataque = ObtenerAtaqueActual();
	const float DuracionAviso = Ataque ? Ataque->TiempoTelegrafiar * ObtenerFaseActual().EscalaTiempos : 0.f;
	if (TiempoEstado <= DuracionAviso * ObtenerAjuste().FraccionSeguimientoAviso)
	{
		Encarar(DireccionObjetivo());
	}
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
	bOndaAplicada = false;
	CambiarEstado(ESAEVUSEstadoEnemigo::Atacar);
	if (Ataque->Tipo == ESAEVUSTipoAtaqueEnemigo::Picado)
	{
		// El picado va en línea recta en 3D hacia donde está el objetivo ahora.
		if (const APawn* Pawn = ObtenerObjetivo())
		{
			const FVector Direccion3D = (Pawn->GetActorLocation() - Enemigo->GetActorLocation()).GetSafeNormal();
			DireccionAtaque = Direccion3D.IsNearlyZero() ? DireccionAtaque : Direccion3D;
		}
	}
	if (Ataque->Tipo == ESAEVUSTipoAtaqueEnemigo::Salto)
	{
		const float Escala = ObtenerFaseActual().EscalaVelocidad;
		Enemigo->LanzarPropio(DireccionAtaque * Ataque->Velocidad * Escala + FVector::UpVector * Ataque->VelocidadVertical);
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
	case ESAEVUSTipoAtaqueEnemigo::Picado:
		if (!bTermino)
		{
			const float Velocidad = Ataque->Velocidad * ObtenerFaseActual().EscalaVelocidad;
			Movimiento->MaxFlySpeed = Velocidad;
			Movimiento->Velocity = DireccionAtaque * Velocidad;
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
		// Mazazo al caer: al aterrizar, una onda hace daño a quien esté cerca (no solo el roce de los cuerpos).
		if (bTermino && bDejoSuelo && !bOndaAplicada && Ataque->RadioOnda > 0.f)
		{
			bOndaAplicada = true;
			const APawn* Pawn = ObtenerObjetivo();
			if (Pawn && ObjetivoVivo() && FVector::Dist2D(Pawn->GetActorLocation(), Enemigo->GetActorLocation()) <= Ataque->RadioOnda
				&& FMath::Abs(Pawn->GetActorLocation().Z - Enemigo->GetActorLocation().Z) <= Ataque->AlcanceVertical)
			{
				GolpearObjetivo(Ataque->Danio);
			}
		}
		break;
	case ESAEVUSTipoAtaqueEnemigo::Proyectil:
		// Sale cuando el brazo llega al final del barrido; queda quieto la duración del ataque.
		if (!bGolpeAplicado && TiempoEstado >= FMath::Min(Ataque->TiempoImpacto, Ataque->Duracion))
		{
			bGolpeAplicado = true;
			LanzarProyectil(*Ataque);
		}
		break;
	case ESAEVUSTipoAtaqueEnemigo::Golpe:
		if (!bGolpeAplicado && TiempoEstado >= FMath::Min(Ataque->TiempoImpacto, Ataque->Duracion))
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
	// En abanico: repartidos por igual en AperturaAbanico, girando en horizontal la dirección al objetivo.
	const int32 Numero = FMath::Max(Ataque.NumeroProyectiles, 1);
	const float Paso = Numero > 1 ? Ataque.AperturaAbanico / static_cast<float>(Numero - 1) : 0.f;
	for (int32 Indice = 0; Indice < Numero; ++Indice)
	{
		const float Angulo = Numero > 1 ? -Ataque.AperturaAbanico * 0.5f + Paso * static_cast<float>(Indice) : 0.f;
		const FVector Direccion = (Destino - Boca).RotateAngleAxis(Angulo, FVector::UpVector);
		if (ASAEVUSProyectil* Proyectil = GetWorld()->SpawnActor<ASAEVUSProyectil>(Boca, FRotator::ZeroRotator, Parametros))
		{
			Proyectil->Lanzar(Direccion, Ataque.Velocidad * ObtenerFaseActual().EscalaVelocidad, Ataque.Danio, Enemigo, Enemigo->ObtenerMaterialProyectil());
		}
	}
}

bool USAEVUSComponenteIAEnemigo::FaseAdmiteAtaque(int32 Indice) const
{
	const TArray<int32>& Permitidos = ObtenerFaseActual().Ataques;
	return Permitidos.IsEmpty() || Permitidos.Contains(Indice);
}

bool USAEVUSComponenteIAEnemigo::AlcanzaEnAltura(const FSAEVUSAtaqueEnemigo& Ataque) const
{
	// Los golpes y embestidas son de suelo: con el objetivo en una cornisa por encima no se golpea al aire.
	if (Ataque.Tipo != ESAEVUSTipoAtaqueEnemigo::Golpe && Ataque.Tipo != ESAEVUSTipoAtaqueEnemigo::Embestida)
	{
		return true;
	}
	const APawn* Pawn = ObtenerObjetivo();
	if (!Pawn || ObtenerAjuste().bVolador)
	{
		return true;
	}
	return FMath::Abs(static_cast<float>(Pawn->GetActorLocation().Z - GetOwner()->GetActorLocation().Z)) <= Ataque.AlcanceVertical;
}

bool USAEVUSComponenteIAEnemigo::ElegirAtaque(float Distancia)
{
	const TArray<FSAEVUSAtaqueEnemigo>& Ataques = ObtenerAjuste().Ataques;
	if (ObtenerAjuste().bAtaquesAleatorios)
	{
		TArray<int32> Validos;
		for (int32 Indice = 0; Indice < Ataques.Num(); ++Indice)
		{
			const FSAEVUSAtaqueEnemigo& Ataque = Ataques[Indice];
			if (FaseAdmiteAtaque(Indice) && Distancia >= Ataque.DistanciaMinima && Distancia <= Ataque.DistanciaMaxima && AlcanzaEnAltura(Ataque))
			{
				Validos.Add(Indice);
			}
		}
		if (Validos.IsEmpty())
		{
			return false;
		}
		// No se repite el último si hay otra opción (a menos que sea la única).
		if (Validos.Num() > 1)
		{
			Validos.Remove(UltimoAtaque);
		}
		int32 Elegido = Validos[AleatorioAtaques.RandRange(0, Validos.Num() - 1)];
		// Al que se cura se le castiga con el ataque de aviso más corto.
		const APawn* Pawn = ObtenerObjetivo();
		const USAEVUSComponenteAlma* Alma = Pawn ? Pawn->FindComponentByClass<USAEVUSComponenteAlma>() : nullptr;
		if (Alma && Alma->EstaCurando())
		{
			for (const int32 Indice : Validos)
			{
				if (Ataques[Indice].TiempoTelegrafiar < Ataques[Elegido].TiempoTelegrafiar)
				{
					Elegido = Indice;
				}
			}
		}
		IndiceAtaque = Elegido;
		UltimoAtaque = Elegido;
		return true;
	}
	// En rotación desde el siguiente al último usado: variedad predecible (y comprobable).
	for (int32 Paso = 0; Paso < Ataques.Num(); ++Paso)
	{
		const int32 Indice = (SiguienteAtaque + Paso) % Ataques.Num();
		const FSAEVUSAtaqueEnemigo& Ataque = Ataques[Indice];
		if (FaseAdmiteAtaque(Indice) && Distancia >= Ataque.DistanciaMinima && Distancia <= Ataque.DistanciaMaxima && AlcanzaEnAltura(Ataque))
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
		const FRotator Meta = Direccion.ToOrientationRotator();
		const float Velocidad = ObtenerAjuste().VelocidadGiro;
		if (Velocidad <= 0.f || DeltaActual <= 0.f)
		{
			GetOwner()->SetActorRotation(Meta);
			return;
		}
		GetOwner()->SetActorRotation(FMath::RInterpConstantTo(GetOwner()->GetActorRotation(), Meta, DeltaActual, Velocidad));
	}
}
