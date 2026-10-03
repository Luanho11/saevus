// Copyright SAEVUS. Todos los derechos reservados.

#include "Camara/SAEVUSComponenteCamara.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"

namespace SAEVUSCamara
{
	const FSAEVUSAjusteCamara AjusteBase;
}

const FSAEVUSAjusteCamara& USAEVUSComponenteCamara::ObtenerAjuste() const
{
	return AjustesCamara ? AjustesCamara->Camara : SAEVUSCamara::AjusteBase;
}

void USAEVUSComponenteCamara::FijarAjustesCamara(const USAEVUSAjustesCamara* NuevosAjustes)
{
	AjustesCamara = NuevosAjustes;
	AplicarAjustes();
}

void USAEVUSComponenteCamara::OnRegister()
{
	Super::OnRegister();
	AplicarAjustes();
}

void USAEVUSComponenteCamara::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	FieldOfView = FMath::FInterpTo(FieldOfView, ObtenerCampoVisionObjetivo(), DeltaTime, ObtenerAjuste().VelocidadInterpCampoVision);
	Super::GetCameraView(DeltaTime, DesiredView);
}

void USAEVUSComponenteCamara::AplicarAjustes()
{
	const FSAEVUSAjusteCamara& Ajuste = ObtenerAjuste();
	FieldOfView = Ajuste.CampoVisionBase;
	AplicarEstilo(Ajuste);

	USpringArmComponent* Brazo = Cast<USpringArmComponent>(GetAttachParent());
	if (!Brazo)
	{
		return;
	}
	Brazo->TargetArmLength = Ajuste.LongitudBrazo;
	Brazo->SocketOffset = Ajuste.DesplazamientoExtremo;
	Brazo->ProbeSize = Ajuste.TamanoSonda;
	Brazo->bEnableCameraLag = Ajuste.bRetrasoPosicion;
	Brazo->CameraLagSpeed = Ajuste.VelocidadRetrasoPosicion;
	Brazo->CameraLagMaxDistance = Ajuste.DistanciaMaximaRetraso;
}

void USAEVUSComponenteCamara::AplicarEstilo(const FSAEVUSAjusteCamara& Ajuste)
{
	PostProcessBlendWeight = 1.f;
	PostProcessSettings.WeightedBlendables.Array.Reset();
	if (Ajuste.MaterialContorno)
	{
		PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(1.f, Ajuste.MaterialContorno));
	}
	PostProcessSettings.bOverride_VignetteIntensity = true;
	PostProcessSettings.VignetteIntensity = Ajuste.IntensidadVineta;
	PostProcessSettings.bOverride_ColorSaturation = true;
	PostProcessSettings.ColorSaturation = FVector4(Ajuste.Saturacion, Ajuste.Saturacion, Ajuste.Saturacion, 1.f);
}

float USAEVUSComponenteCamara::ObtenerCampoVisionObjetivo() const
{
	const FSAEVUSAjusteCamara& Ajuste = ObtenerAjuste();

	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	const USAEVUSComponenteMovimiento* Movimiento = Personaje ? Cast<USAEVUSComponenteMovimiento>(Personaje->GetCharacterMovement()) : nullptr;
	if (!Movimiento)
	{
		return Ajuste.CampoVisionBase;
	}

	// Lo manda la velocidad real, no la petición de esprint, así nunca se abre contra una pared.
	const float VelocidadCorrer = Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Correr);
	const float VelocidadEsprint = Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Esprint);
	const float AlfaEsprint = FMath::Clamp(FMath::GetRangePct(VelocidadCorrer, VelocidadEsprint, Movimiento->Velocity.Size2D()), 0.f, 1.f);
	return FMath::Lerp(Ajuste.CampoVisionBase, Ajuste.CampoVisionEsprint, AlfaEsprint);
}
