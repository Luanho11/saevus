// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponenteParticulas.h"
#include "Nucleo/SAEVUSFormasMotor.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSParticulas
{
	// Malla por defecto: el cubo del motor (12 triángulos). Se puede cambiar en el Blueprint.
	const TCHAR* const RutaMallaPorDefecto = SAEVUSFormas::Cubo;
	// Velocidad a la que una partícula orientada mide 1 + Estiramiento veces su tamaño.
	constexpr float VelocidadEstiramiento = 1000.f;
}

USAEVUSComponenteParticulas::USAEVUSComponenteParticulas()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	CastShadow = false;
	// Las instancias se colocan en el mundo; el componente no debe arrastrarlas al moverse el dueño.
	SetUsingAbsoluteLocation(true);
	SetUsingAbsoluteRotation(true);
	SetUsingAbsoluteScale(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Malla(SAEVUSParticulas::RutaMallaPorDefecto);
	if (Malla.Succeeded())
	{
		SetStaticMesh(Malla.Object);
	}
}

void USAEVUSComponenteParticulas::BeginPlay()
{
	Super::BeginPlay();
	SetWorldLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	Aleatorio.Initialize(GetUniqueID());
	PrepararPool();
}

void USAEVUSComponenteParticulas::PrepararPool()
{
	if (Particulas.Num() == Ajuste.MaxParticulas)
	{
		return;
	}
	ClearInstances();
	Particulas.SetNum(Ajuste.MaxParticulas);
	Transformaciones.Init(FTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector), Ajuste.MaxParticulas);
	AddInstances(Transformaciones, false, true);
	SiguienteHueco = 0;
}

void USAEVUSComponenteParticulas::Emitir(const FVector& Posicion, const FVector& Velocidad)
{
	PrepararPool();
	// Buffer circular: si todo está vivo, la nueva reemplaza a la más vieja.
	FParticula& Particula = Particulas[SiguienteHueco];
	SiguienteHueco = (SiguienteHueco + 1) % Particulas.Num();

	Particula.Posicion = Posicion;
	Particula.Velocidad = Velocidad;
	Particula.Rotacion = FRotator(Aleatorio.FRandRange(0.f, 360.f), Aleatorio.FRandRange(0.f, 360.f), Aleatorio.FRandRange(0.f, 360.f));
	Particula.Giro = FRotator(Aleatorio.FRandRange(-1.f, 1.f), Aleatorio.FRandRange(-1.f, 1.f), Aleatorio.FRandRange(-1.f, 1.f)) * Ajuste.GiroMaximo;
	NumVivas += Particula.bViva ? 0 : 1;
	Particula.Edad = 0.f;
	Particula.bViva = true;
	bInstanciasSucias = true;
}

void USAEVUSComponenteParticulas::EmitirRafaga(const FVector& Posicion, const FVector& VelocidadBase, int32 Cantidad, float Dispersion)
{
	for (int32 Indice = 0; Indice < Cantidad; ++Indice)
	{
		Emitir(Posicion, VelocidadBase + Aleatorio.GetUnitVector() * Aleatorio.FRandRange(0.f, Dispersion));
	}
}

int32 USAEVUSComponenteParticulas::ObtenerParticulasActivas() const
{
	return NumVivas;
}

void USAEVUSComponenteParticulas::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Sin nada vivo ni que esconder no hay trabajo (el emisor sigue escuchando: emitir lo despierta).
	if (NumVivas == 0 && !bInstanciasSucias)
	{
		return;
	}

	// Exponencial: frena igual a 15 que a 144 fotogramas por segundo (1 − k·dt frenaba el doble o el triple a pocos fps).
	const float Frenado = FMath::Exp(-FMath::Max(Ajuste.Arrastre, 0.f) * DeltaTime);
	for (FParticula& Particula : Particulas)
	{
		if (!Particula.bViva)
		{
			continue;
		}
		Particula.Edad += DeltaTime;
		if (Particula.Edad >= Ajuste.Vida)
		{
			Particula.bViva = false;
			--NumVivas;
			bInstanciasSucias = true;
			continue;
		}
		Particula.Velocidad *= Frenado;
		Particula.Velocidad.Z += Ajuste.AceleracionVertical * DeltaTime;
		Particula.Posicion += Particula.Velocidad * DeltaTime;
		Particula.Rotacion += Particula.Giro * DeltaTime;
		bInstanciasSucias = true;
	}
	if (bInstanciasSucias)
	{
		ActualizarInstancias();
	}
}

void USAEVUSComponenteParticulas::ActualizarInstancias()
{
	bInstanciasSucias = false;
	if (Transformaciones.Num() != Particulas.Num() || GetInstanceCount() != Particulas.Num())
	{
		return;
	}
	for (int32 Indice = 0; Indice < Particulas.Num(); ++Indice)
	{
		const FParticula& Particula = Particulas[Indice];
		const float Alfa = Particula.bViva ? FMath::Clamp(Particula.Edad / Ajuste.Vida, 0.f, 1.f) : 1.f;
		const float Escala = Particula.bViva ? FMath::Lerp(Ajuste.TamanoInicial, Ajuste.TamanoFinal, Alfa) : 0.f;
		if (Ajuste.bOrientarVelocidad && !Particula.Velocidad.IsNearlyZero())
		{
			// Alargada en la dirección de la velocidad: una raya que se acorta al frenar.
			const float Largo = 1.f + Ajuste.Estiramiento * static_cast<float>(Particula.Velocidad.Size()) / SAEVUSParticulas::VelocidadEstiramiento;
			Transformaciones[Indice] = FTransform(Particula.Velocidad.ToOrientationQuat(), Particula.Posicion, FVector(Escala * Largo, Escala, Escala));
			continue;
		}
		Transformaciones[Indice] = FTransform(Particula.Rotacion, Particula.Posicion, FVector(Escala));
	}
	BatchUpdateInstancesTransforms(0, Transformaciones, /*bWorldSpace*/ true, /*bMarkRenderStateDirty*/ true, /*bTeleport*/ true);
}
