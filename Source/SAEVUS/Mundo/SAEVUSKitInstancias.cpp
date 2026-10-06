// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSKitInstancias.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

ASAEVUSKitInstancias::ASAEVUSKitInstancias()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));
	RootComponent->SetMobility(EComponentMobility::Static);
}

void ASAEVUSKitInstancias::AnadirPieza(UStaticMesh* Malla, const FTransform& Transformacion, const TArray<UMaterialInterface*>& Materiales)
{
	if (!Malla)
	{
		return;
	}
	UInstancedStaticMeshComponent* Componente = nullptr;
	for (UInstancedStaticMeshComponent* Existente : Instancias)
	{
		if (Existente && Existente->GetStaticMesh() == Malla)
		{
			Componente = Existente;
			break;
		}
	}
	if (!Componente)
	{
		// Componente de instancia (se guarda con el nivel), estático y sin colisión.
		Componente = NewObject<UInstancedStaticMeshComponent>(this, NAME_None, RF_Transactional);
		Componente->SetMobility(EComponentMobility::Static);
		Componente->SetupAttachment(RootComponent);
		Componente->SetStaticMesh(Malla);
		Componente->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Componente->SetGenerateOverlapEvents(false);
		for (int32 Ranura = 0; Ranura < Materiales.Num(); ++Ranura)
		{
			Componente->SetMaterial(Ranura, Materiales[Ranura]);
		}
		Componente->CreationMethod = EComponentCreationMethod::Instance;
		AddInstanceComponent(Componente);
		Componente->RegisterComponent();
		Instancias.Add(Componente);
	}
	Componente->AddInstance(Transformacion, true);
}

void ASAEVUSKitInstancias::Vaciar()
{
	for (UInstancedStaticMeshComponent* Componente : Instancias)
	{
		if (Componente)
		{
			Componente->DestroyComponent();
		}
	}
	Instancias.Reset();
}

int32 ASAEVUSKitInstancias::NumeroPiezas() const
{
	int32 Total = 0;
	for (const UInstancedStaticMeshComponent* Componente : Instancias)
	{
		Total += Componente ? Componente->GetInstanceCount() : 0;
	}
	return Total;
}
