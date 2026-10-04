// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponenteEfectosCombate.h"
#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteGuardia.h"

namespace SAEVUSEfectosCombate
{
	// Partículas finas y breves: chispas, estela y alma (el polvo del dash usa otro emisor).
	constexpr float TamanoParticula = 0.07f;
	constexpr float VidaParticula = 0.3f;
	constexpr float ArrastreParticula = 6.f;
}

USAEVUSComponenteEfectosCombate::USAEVUSComponenteEfectosCombate()
{
	Ajuste.TamanoInicial = SAEVUSEfectosCombate::TamanoParticula;
	Ajuste.Vida = SAEVUSEfectosCombate::VidaParticula;
	Ajuste.Arrastre = SAEVUSEfectosCombate::ArrastreParticula;
	Ajuste.AceleracionVertical = 0.f;
}

void USAEVUSComponenteEfectosCombate::BeginPlay()
{
	Super::BeginPlay();
	Combate = GetOwner()->FindComponentByClass<USAEVUSComponenteCombate>();
	if (Combate)
	{
		Combate->AlGolpear.AddUniqueDynamic(this, &ThisClass::AlGolpear);
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
	// Las chispas salen del golpeado alejándose del atacante.
	const FVector Alejandose = (ActorGolpeado->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
	EmitirRafaga(ActorGolpeado->GetActorLocation(), Alejandose * AjusteEfectos.VelocidadGolpe, AjusteEfectos.ParticulasGolpe, AjusteEfectos.Dispersion);
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

void USAEVUSComponenteEfectosCombate::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	EmitirEstelaArma();
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
