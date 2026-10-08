// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponenteEcoDash.h"
#include "Components/PoseableMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"

namespace SAEVUSEcoDash
{
	const FName ParametroColor(TEXT("Color"));
	const FName ParametroBrillo(TEXT("Brillo"));
}

USAEVUSComponenteEcoDash::USAEVUSComponenteEcoDash()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// Después de la animación: el eco copia la pose de este fotograma.
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void USAEVUSComponenteEcoDash::BeginPlay()
{
	Super::BeginPlay();
	if (!Cuerpo)
	{
		Cuerpo = GetOwner()->FindComponentByClass<UPoseableMeshComponent>();
	}
	if (USAEVUSComponenteMovimiento* Movimiento = SAEVUSMovimiento::DelActor(GetOwner()))
	{
		ManejadorDash = Movimiento->AlIniciarDash.AddUObject(this, &ThisClass::AlIniciarDash);
	}
}

void USAEVUSComponenteEcoDash::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USAEVUSComponenteMovimiento* Movimiento = SAEVUSMovimiento::DelActor(GetOwner()))
	{
		Movimiento->AlIniciarDash.Remove(ManejadorDash);
	}
	for (UPoseableMeshComponent* Malla : Mallas)
	{
		if (Malla)
		{
			Malla->DestroyComponent();
		}
	}
	Mallas.Reset();
	Super::EndPlay(EndPlayReason);
}

int32 USAEVUSComponenteEcoDash::ObtenerEcosVisibles() const
{
	int32 Visibles = 0;
	for (const FEco& Eco : Ecos)
	{
		Visibles += Eco.bActivo ? 1 : 0;
	}
	return Visibles;
}

void USAEVUSComponenteEcoDash::AlIniciarDash(const FVector& Direccion)
{
	if (Ajuste.EcosPorDash <= 0 || !Cuerpo || !Cuerpo->GetSkinnedAsset())
	{
		return;
	}
	// El primero en el acto (la salida), los demás repartidos por el dash.
	EcosPendientes = Ajuste.EcosPorDash;
	TiempoSiguienteEco = 0.f;
	SetComponentTickEnabled(true);
}

void USAEVUSComponenteEcoDash::DejarEco()
{
	using namespace SAEVUSEcoDash;
	// El grupo se crea la primera vez (el cuerpo ya tiene malla) y se reutiliza siempre.
	if (Mallas.IsEmpty())
	{
		for (int32 Indice = 0; Indice < Ajuste.EcosPorDash; ++Indice)
		{
			UPoseableMeshComponent* Malla = NewObject<UPoseableMeshComponent>(GetOwner());
			Malla->SetUsingAbsoluteLocation(true);
			Malla->SetUsingAbsoluteRotation(true);
			Malla->SetUsingAbsoluteScale(true);
			Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Malla->SetCastShadow(false);
			Malla->SetVisibility(false);
			Malla->RegisterComponent();
			Malla->SetSkinnedAssetAndUpdate(Cuerpo->GetSkinnedAsset());
			UMaterialInstanceDynamic* Material = MaterialEco ? UMaterialInstanceDynamic::Create(MaterialEco, this) : nullptr;
			if (Material)
			{
				Material->SetVectorParameterValue(ParametroColor, Ajuste.Color);
				for (int32 Ranura = 0; Ranura < Malla->GetNumMaterials(); ++Ranura)
				{
					Malla->SetMaterial(Ranura, Material);
				}
			}
			Mallas.Add(Malla);
			Materiales.Add(Material);
			Ecos.AddDefaulted();
		}
	}
	UPoseableMeshComponent* Malla = Mallas[Siguiente];
	Malla->SetWorldTransform(Cuerpo->GetComponentTransform());
	if (Malla->BoneSpaceTransforms.Num() == Cuerpo->BoneSpaceTransforms.Num())
	{
		Malla->BoneSpaceTransforms = Cuerpo->BoneSpaceTransforms;
		Malla->MarkRefreshTransformDirty();
		Malla->RefreshBoneTransforms();
	}
	Malla->SetVisibility(true);
	Ecos[Siguiente] = { 0.f, true };
	Siguiente = (Siguiente + 1) % Mallas.Num();
}

void USAEVUSComponenteEcoDash::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	using namespace SAEVUSEcoDash;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TiempoSiguienteEco -= DeltaTime;
	while (EcosPendientes > 0 && TiempoSiguienteEco <= 0.f)
	{
		DejarEco();
		--EcosPendientes;
		TiempoSiguienteEco += Ajuste.Intervalo;
	}
	bool bQueda = EcosPendientes > 0;
	for (int32 Indice = 0; Indice < Ecos.Num(); ++Indice)
	{
		FEco& Eco = Ecos[Indice];
		if (!Eco.bActivo)
		{
			continue;
		}
		Eco.Edad += DeltaTime;
		const float Resto = 1.f - Eco.Edad / Ajuste.Vida;
		if (Resto <= 0.f)
		{
			Eco.bActivo = false;
			Mallas[Indice]->SetVisibility(false);
			continue;
		}
		if (Materiales[Indice])
		{
			Materiales[Indice]->SetScalarParameterValue(ParametroBrillo, Ajuste.Brillo * Resto * Resto);
		}
		bQueda = true;
	}
	if (!bQueda)
	{
		SetComponentTickEnabled(false);
	}
}
