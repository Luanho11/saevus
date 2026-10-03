// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponenteEfectosCombate.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"

void USAEVUSComponenteEfectosCombate::BeginPlay()
{
	Super::BeginPlay();
	if (USAEVUSComponenteCombate* Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>())
	{
		Combate->AlGolpear.AddUniqueDynamic(this, &ThisClass::AlGolpear);
	}
	Alma = GetOwner()->FindComponentByClass<USAEVUSComponenteAlma>();
	if (Alma)
	{
		ManejadorCurar = Alma->AlCurar.AddUObject(this, &ThisClass::AlCurar);
	}
}

void USAEVUSComponenteEfectosCombate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Alma)
	{
		Alma->AlCurar.Remove(ManejadorCurar);
	}
	Super::EndPlay(EndPlayReason);
}

void USAEVUSComponenteEfectosCombate::AlGolpear(AActor* ActorGolpeado, float Danio)
{
	if (!ActorGolpeado)
	{
		return;
	}
	// Las chispas salen del golpeado alejándose del atacante.
	const FVector Alejandose = (ActorGolpeado->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
	EmitirRafaga(ActorGolpeado->GetActorLocation(), Alejandose * AjusteEfectos.VelocidadGolpe, AjusteEfectos.ParticulasGolpe, AjusteEfectos.Dispersion);
}

void USAEVUSComponenteEfectosCombate::AlCurar(float VidaRecuperada)
{
	EmitirRafaga(GetOwner()->GetActorLocation(), FVector::UpVector * AjusteEfectos.VelocidadCuracion, AjusteEfectos.ParticulasCuracion, AjusteEfectos.Dispersion);
}

void USAEVUSComponenteEfectosCombate::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (Alma && Alma->EstaCurando())
	{
		AcumuladoEmision += AjusteEfectos.ParticulasPorSegundoCurando * DeltaTime;
		const int32 Cantidad = FMath::FloorToInt32(AcumuladoEmision);
		AcumuladoEmision -= Cantidad;
		const FVector Centro = GetOwner()->GetActorLocation();
		for (int32 Indice = 0; Indice < Cantidad; ++Indice)
		{
			// Desde un punto al azar alrededor del cuerpo, hacia el centro: el alma se concentra.
			const FVector Desde = Centro + FMath::VRand() * AjusteEfectos.RadioConvergencia;
			Emitir(Desde, (Centro - Desde) / FMath::Max(ObtenerAjuste().Vida, UE_KINDA_SMALL_NUMBER));
		}
	}
	else
	{
		AcumuladoEmision = 0.f;
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
