// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponenteEstelaEspada.h"
#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSTiposCombate.h"

USAEVUSComponenteEstelaEspada::USAEVUSComponenteEstelaEspada(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	// Después de la animación (también en PostPhysics; ver BeginPlay): la hoja ya está en su pose de este fotograma.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	// En espacio de mundo: la estela se queda donde pasó la hoja aunque el personaje siga moviéndose.
	SetUsingAbsoluteLocation(true);
	SetUsingAbsoluteRotation(true);
	SetUsingAbsoluteScale(true);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCastShadow(false);
	bUseAsyncCooking = false;
}

void USAEVUSComponenteEstelaEspada::BeginPlay()
{
	Super::BeginPlay();
	SetWorldTransform(FTransform::Identity);
	if (AActor* Dueno = GetOwner())
	{
		Combate = Dueno->FindComponentByClass<USAEVUSComponenteCombate>();
		Animacion = Dueno->FindComponentByClass<USAEVUSComponenteAnimacionProcedural>();
		if (Animacion)
		{
			AddTickPrerequisiteComponent(Animacion);
		}
	}
}

void USAEVUSComponenteEstelaEspada::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const int32 TramosAntes = Tramos.Num();
	for (FTramo& Tramo : Tramos)
	{
		Tramo.Edad += DeltaTime;
	}
	Tramos.RemoveAll([this](const FTramo& Tramo) { return Tramo.Edad >= AjusteEstela.VidaTramo; });

	const FSAEVUSDefinicionAtaque* Ataque = Combate ? Combate->ObtenerAtaqueActual() : nullptr;
	FVector Base;
	FVector Punta;
	if (Ataque && Ataque->ObtenerFaseEn(Combate->ObtenerTiempoAtaque()) == ESAEVUSFaseAtaque::Activa && Animacion && Animacion->ObtenerHoja(Base, Punta))
	{
		const float Escala = Combate->EsAtaqueCargado() ? AjusteEstela.EscalaCargado : 1.f;
		// Entre el tramo anterior y este la hoja barre un arco: se intercalan tramos girando la hoja alrededor de la
		// guarda, así la media luna es curva aunque la fase activa dure pocos fotogramas.
		if (Tramos.Num() > 0 && Tramos.Last().Edad <= DeltaTime * 1.5f)
		{
			const FTramo Anterior = Tramos.Last();
			const FVector HojaAnterior = Anterior.Punta - Anterior.Base;
			const FVector HojaActual = Punta - Base;
			const FQuat Giro = FQuat::FindBetweenVectors(HojaAnterior, HojaActual);
			const float Grados = FMath::RadiansToDegrees(Giro.GetAngle());
			const int32 Intermedios = FMath::Min(FMath::FloorToInt32(Grados / AjusteEstela.GradosPorTramo), AjusteEstela.MaxTramos / 2);
			for (int32 Paso = 1; Paso <= Intermedios; ++Paso)
			{
				const float Alfa = static_cast<float>(Paso) / static_cast<float>(Intermedios + 1);
				const FVector BaseIntermedia = FMath::Lerp(Anterior.Base, Base, Alfa);
				const FVector HojaIntermedia = FQuat::Slerp(FQuat::Identity, Giro, Alfa).RotateVector(HojaAnterior);
				Tramos.Add({ BaseIntermedia, BaseIntermedia + HojaIntermedia, FMath::Lerp(Anterior.Edad, 0.f, Alfa), Escala });
			}
		}
		Tramos.Add({ Base, Punta, 0.f, Escala });
		if (Tramos.Num() > AjusteEstela.MaxTramos)
		{
			Tramos.RemoveAt(0, Tramos.Num() - AjusteEstela.MaxTramos);
		}
	}
	if (Tramos.Num() > 0 || TramosAntes > 0)
	{
		Reconstruir();
	}
}

void USAEVUSComponenteEstelaEspada::Reconstruir()
{
	if (Tramos.Num() < 2)
	{
		if (NumTramosSeccion > 0)
		{
			ClearAllMeshSections();
			NumTramosSeccion = 0;
		}
		return;
	}
	Vertices.Reset();
	Triangulos.Reset();
	Colores.Reset();
	for (const FTramo& Tramo : Tramos)
	{
		const FVector Hoja = Tramo.Punta - Tramo.Base;
		const float Vida = 1.f - Tramo.Edad / AjusteEstela.VidaTramo;
		Vertices.Add(Tramo.Base + Hoja * AjusteEstela.FraccionInterior);
		Vertices.Add(Tramo.Base + Hoja * (AjusteEstela.FraccionExterior * Tramo.Escala));
		// Alfa = opacidad (el material la usa como intensidad aditiva): viva en el filo, apagada hacia dentro.
		Colores.Add(FLinearColor(1.f, 1.f, 1.f, Vida * AjusteEstela.OpacidadInterior * Tramo.Escala));
		Colores.Add(FLinearColor(1.f, 1.f, 1.f, Vida * Tramo.Escala));
	}
	for (int32 Indice = 0; Indice + 1 < Tramos.Num(); ++Indice)
	{
		const int32 A = Indice * 2;
		Triangulos.Append({ A, A + 1, A + 2, A + 1, A + 3, A + 2 });
	}
	// Con los mismos tramos solo cambian posiciones y colores: se actualiza la sección en vez de rehacerla.
	if (Tramos.Num() == NumTramosSeccion)
	{
		UpdateMeshSection_LinearColor(0, Vertices, TArray<FVector>(), TArray<FVector2D>(), Colores, TArray<FProcMeshTangent>());
		return;
	}
	CreateMeshSection_LinearColor(0, Vertices, Triangulos, TArray<FVector>(), TArray<FVector2D>(), Colores, TArray<FProcMeshTangent>(), false);
	NumTramosSeccion = Tramos.Num();
}
