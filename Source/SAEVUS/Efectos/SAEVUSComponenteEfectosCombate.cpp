// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponenteEfectosCombate.h"
#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteGuardia.h"

namespace SAEVUSEfectosCombate
{
	// Partículas finas y breves: chispas, estela y alma (el polvo del dash usa otro emisor).
	constexpr float TamanoParticula = 0.09f;
	constexpr float VidaParticula = 0.3f;
	constexpr float ArrastreParticula = 6.f;
	constexpr int32 MaxParticulas = 512;
}

USAEVUSComponenteEfectosCombate::USAEVUSComponenteEfectosCombate()
{
	Ajuste.TamanoInicial = SAEVUSEfectosCombate::TamanoParticula;
	Ajuste.Vida = SAEVUSEfectosCombate::VidaParticula;
	Ajuste.Arrastre = SAEVUSEfectosCombate::ArrastreParticula;
	Ajuste.AceleracionVertical = 0.f;
	// El arco del tajo deja un par de cientos de partículas por golpe.
	Ajuste.MaxParticulas = SAEVUSEfectosCombate::MaxParticulas;
}

void USAEVUSComponenteEfectosCombate::BeginPlay()
{
	Super::BeginPlay();
	Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>();
	if (Combate)
	{
		Combate->AlGolpear.AddUniqueDynamic(this, &ThisClass::AlGolpear);
		ManejadorEscenario = Combate->AlImpactarEscenario.AddUObject(this, &ThisClass::AlImpactarEscenario);
	}
	Animacion = GetOwner()->FindComponentByClass<USAEVUSComponenteAnimacionProcedural>();
	Alma = GetOwner()->FindComponentByClass<USAEVUSComponenteAlma>();
	if (Alma)
	{
		ManejadorCurar = Alma->AlCurar.AddUObject(this, &ThisClass::AlCurar);
	}
	Guardia = GetOwner()->FindComponentByClass<USAEVUSComponenteGuardia>();
	if (Guardia)
	{
		ManejadorBloqueo = Guardia->AlBloquear.AddUObject(this, &ThisClass::AlBloquear);
	}
}

void USAEVUSComponenteEfectosCombate::AlBloquear(AActor* Atacante, const FVector& PuntoContacto)
{
	// Chispas que saltan hacia el atacante.
	const FVector HaciaAtacante = (PuntoContacto - GetOwner()->GetActorLocation()).GetSafeNormal();
	EmitirRafaga(PuntoContacto, HaciaAtacante * AjusteEfectos.VelocidadBloqueo, AjusteEfectos.ParticulasBloqueo, AjusteEfectos.Dispersion);
}

void USAEVUSComponenteEfectosCombate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Combate)
	{
		Combate->AlImpactarEscenario.Remove(ManejadorEscenario);
	}
	if (Alma)
	{
		Alma->AlCurar.Remove(ManejadorCurar);
	}
	if (Guardia)
	{
		Guardia->AlBloquear.Remove(ManejadorBloqueo);
	}
	Super::EndPlay(EndPlayReason);
}

void USAEVUSComponenteEfectosCombate::AlGolpear(AActor* ActorGolpeado, float Danio)
{
	if (!ActorGolpeado)
	{
		return;
	}
	// Las chispas salen del golpeado alejándose del atacante, desde el punto de la hoja más cercano a él (donde se ve el
	// contacto) y no desde el centro de su cápsula; más cuanto más daño (ANIMVFX-5).
	const FVector Alejandose = (ActorGolpeado->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
	FVector Punto = ActorGolpeado->GetActorLocation();
	FVector Base;
	FVector Punta;
	if (Animacion && Animacion->ObtenerHoja(Base, Punta))
	{
		const FVector EnLaHoja = FMath::ClosestPointOnSegment(Punto, Base, Punta);
		if (FVector::DistSquared(EnLaHoja, Punto) <= FMath::Square(AjusteEfectos.DistanciaMaximaContacto))
		{
			Punto = EnLaHoja;
		}
	}
	const float FuerzaGolpe = FMath::Clamp(Danio / FMath::Max(AjusteEfectos.DanioReferenciaGolpe, UE_KINDA_SMALL_NUMBER), 0.6f, 2.f);
	EmitirRafaga(Punto, Alejandose * AjusteEfectos.VelocidadGolpe, FMath::Max(1, FMath::RoundToInt(AjusteEfectos.ParticulasGolpe * FuerzaGolpe)), AjusteEfectos.Dispersion);
}

void USAEVUSComponenteEfectosCombate::AlImpactarEscenario(AActor* Escenario)
{
	if (!Escenario)
	{
		return;
	}
	// Pocas chispas desde la punta de la hoja hacia atrás: el filo raspa la piedra.
	FVector Base;
	FVector Punta;
	const bool bHoja = Animacion && Animacion->ObtenerHoja(Base, Punta);
	const FVector Punto = bHoja ? FMath::ClosestPointOnSegment(Escenario->GetActorLocation(), Base, Punta) : Escenario->GetActorLocation();
	const FVector Atras = (GetOwner()->GetActorLocation() - Escenario->GetActorLocation()).GetSafeNormal();
	EmitirRafaga(Punto, Atras * AjusteEfectos.VelocidadBloqueo, AjusteEfectos.ParticulasBloqueo, AjusteEfectos.Dispersion);
}

void USAEVUSComponenteEfectosCombate::AlCurar(float VidaRecuperada)
{
	EmitirRafaga(GetOwner()->GetActorLocation(), FVector::UpVector * AjusteEfectos.VelocidadCuracion, AjusteEfectos.ParticulasCuracion, AjusteEfectos.Dispersion);
}

void USAEVUSComponenteEfectosCombate::EmitirEstelaArma()
{
	FVector Base;
	FVector Punta;
	if (!Combate || !Animacion || Combate->ObtenerFaseAtaque() != ESAEVUSFaseAtaque::Activa || !Animacion->ObtenerHoja(Base, Punta))
	{
		return;
	}
	for (int32 Indice = 0; Indice < AjusteEfectos.ParticulasEstelaPorFotograma; ++Indice)
	{
		// Más densa hacia la punta, que es la que barre el arco.
		const float Fraccion = FMath::Sqrt(Aleatorio.FRand());
		Emitir(FMath::Lerp(Base, Punta, Fraccion), Aleatorio.VRand() * AjusteEfectos.DispersionEstela);
	}
}

void USAEVUSComponenteEfectosCombate::EmitirArcoTajo()
{
	FVector Origen;
	FVector Frente;
	float Avance = 0.f;
	float Sentido = 1.f;
	const FSAEVUSDefinicionAtaque* Ataque = Combate ? Combate->ObtenerAtaqueActual() : nullptr;
	if (!Ataque || !Combate->ObtenerAbanicoActivo(Origen, Frente, Avance, Sentido))
	{
		bArcoActivo = false;
		return;
	}
	// El barrido va de un borde del abanico al otro durante la fase activa (positivo = a la derecha).
	const float Angulo = Sentido * Ataque->AnguloAbanico * (1.f - 2.f * Avance);
	const float Desde = bArcoActivo ? AnguloArcoAnterior : Sentido * Ataque->AnguloAbanico;
	AnguloArcoAnterior = Angulo;
	bArcoActivo = true;
	const FVector Derecha = FVector::CrossProduct(FVector::UpVector, Frente);
	const int32 Pasos = FMath::Max(1, FMath::CeilToInt32(FMath::Abs(Angulo - Desde) * AjusteEfectos.PasosArcoPorGrado));
	for (int32 Paso = 1; Paso <= Pasos; ++Paso)
	{
		const float Radianes = FMath::DegreesToRadians(FMath::Lerp(Desde, Angulo, static_cast<float>(Paso) / Pasos));
		const FVector Radial = Frente * FMath::Cos(Radianes) + Derecha * FMath::Sin(Radianes);
		for (int32 Indice = 0; Indice < AjusteEfectos.ParticulasArcoPorPaso; ++Indice)
		{
			// Más densas hacia el borde: el filo del arco es el alcance.
			const float Fraccion = FMath::Lerp(AjusteEfectos.FraccionInteriorArco, 1.f, FMath::Sqrt(Aleatorio.FRand()));
			Emitir(Origen + Radial * Ataque->AlcanceAbanico * Fraccion, Radial * AjusteEfectos.DispersionEstela);
		}
	}
}

void USAEVUSComponenteEfectosCombate::EmitirCarga(float DeltaTime)
{
	FVector Base;
	FVector Punta;
	const float Progreso = Combate ? Combate->ObtenerProgresoCarga() : 0.f;
	if (Progreso <= 0.f || !Animacion || !Animacion->ObtenerHoja(Base, Punta))
	{
		AcumuladoCarga = 0.f;
		return;
	}
	// Más partículas cuanto más cargado; lista, al doble.
	const int32 Cantidad = ConsumirEmision(AcumuladoCarga, AjusteEfectos.ParticulasPorSegundoCarga * (Combate->EstaCargado() ? 2.f : Progreso), DeltaTime);
	for (int32 Indice = 0; Indice < Cantidad; ++Indice)
	{
		const FVector Destino = FMath::Lerp(Base, Punta, Aleatorio.FRand());
		const FVector Desde = Destino + Aleatorio.GetUnitVector() * AjusteEfectos.RadioConvergencia * 0.5f;
		Emitir(Desde, (Destino - Desde) / FMath::Max(ObtenerAjuste().Vida, UE_KINDA_SMALL_NUMBER));
	}
}

void USAEVUSComponenteEfectosCombate::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	EmitirEstelaArma();
	EmitirArcoTajo();
	EmitirCarga(DeltaTime);
	if (Alma && Alma->EstaCurando())
	{
		const int32 Cantidad = ConsumirEmision(AcumuladoEmision, AjusteEfectos.ParticulasPorSegundoCurando, DeltaTime);
		const FVector Centro = GetOwner()->GetActorLocation();
		for (int32 Indice = 0; Indice < Cantidad; ++Indice)
		{
			// Desde un punto al azar alrededor del cuerpo, hacia el centro: el alma se concentra.
			const FVector Desde = Centro + Aleatorio.GetUnitVector() * AjusteEfectos.RadioConvergencia;
			Emitir(Desde, (Centro - Desde) / FMath::Max(ObtenerAjuste().Vida, UE_KINDA_SMALL_NUMBER));
		}
	}
	else
	{
		AcumuladoEmision = 0.f;
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
