// Copyright SAEVUS. Todos los derechos reservados.

#include "Camara/SAEVUSComponenteCamara.h"
#include "Combate/SAEVUSComponenteCombate.h"
#include "Combate/SAEVUSComponenteGuardia.h"
#include "Combate/SAEVUSComponenteVida.h"
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
	ActualizarSacudida(DeltaTime);
	if (Trauma > 0.f)
	{
		DesiredView.Location += DesiredView.Rotation.RotateVector(DesplazamientoSacudida);
		DesiredView.Rotation += GiroSacudida;
	}
}

void USAEVUSComponenteCamara::BeginPlay()
{
	Super::BeginPlay();
	// La cámara escucha los impactos de su dueño: golpes dados, recibidos y bloqueos.
	AActor* Duenio = GetOwner();
	if (USAEVUSComponenteCombate* Combate = Duenio->FindComponentByClass<USAEVUSComponenteCombate>())
	{
		Combate->AlGolpear.AddUniqueDynamic(this, &ThisClass::AlGolpear);
	}
	if (USAEVUSComponenteVida* Vida = Duenio->FindComponentByClass<USAEVUSComponenteVida>())
	{
		Vida->AlCambiarVida.AddUniqueDynamic(this, &ThisClass::AlCambiarVida);
	}
	if (USAEVUSComponenteGuardia* Guardia = Duenio->FindComponentByClass<USAEVUSComponenteGuardia>())
	{
		Guardia->AlBloquear.AddUObject(this, &ThisClass::AlBloquear);
	}
}

void USAEVUSComponenteCamara::AlGolpear(AActor* ActorGolpeado, float Danio)
{
	Sacudir(ObtenerAjuste().SacudidaGolpeDado);
}

void USAEVUSComponenteCamara::AlCambiarVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador)
{
	if (Delta < 0.f)
	{
		Sacudir(ObtenerAjuste().SacudidaGolpeRecibido);
	}
}

void USAEVUSComponenteCamara::AlBloquear(AActor* Atacante, const FVector& PuntoContacto)
{
	Sacudir(ObtenerAjuste().SacudidaBloqueo);
}

void USAEVUSComponenteCamara::Sacudir(float NuevoTrauma)
{
	if (ObtenerAjuste().bSacudida)
	{
		Trauma = FMath::Clamp(Trauma + NuevoTrauma, 0.f, 1.f);
	}
}

void USAEVUSComponenteCamara::ActualizarSacudida(float DeltaTime)
{
	const FSAEVUSAjusteCamara& Ajuste = ObtenerAjuste();
	Trauma = FMath::Max(0.f, Trauma - Ajuste.RecuperacionSacudida * DeltaTime);
	if (Trauma <= 0.f)
	{
		DesplazamientoSacudida = FVector::ZeroVector;
		GiroSacudida = FRotator::ZeroRotator;
		return;
	}
	// Ruido suave (Perlin) en vez de aleatorio puro: tiembla sin saltos bruscos. Cada eje con su semilla.
	TiempoSacudida += DeltaTime * Ajuste.FrecuenciaSacudida;
	const float Sacudida = Trauma * Trauma;
	auto Ruido = [this](float Semilla) { return FMath::PerlinNoise1D(TiempoSacudida + Semilla); };
	DesplazamientoSacudida = FVector(0.f, Ruido(11.f), Ruido(29.f)) * Ajuste.DesplazamientoMaxSacudida * Sacudida;
	GiroSacudida = FRotator(Ruido(47.f), Ruido(71.f), Ruido(97.f)) * (Ajuste.GiroMaxSacudida * Sacudida);
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
	if (Ajuste.MaterialDibujado)
	{
		PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(1.f, Ajuste.MaterialDibujado));
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
