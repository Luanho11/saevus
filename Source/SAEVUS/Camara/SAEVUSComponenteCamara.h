// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Camara/SAEVUSAjustesCamara.h"
#include "Camera/CameraComponent.h"
#include "SAEVUSComponenteCamara.generated.h"

/**
 * Cámara de seguimiento en tercera persona de SAEVUS.
 *
 * Va unida a un USpringArmComponent: aplica a ese brazo el encuadre y el retraso, y abre el
 * campo de visión cuando el dueño supera la velocidad de carrera.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteCamara : public UCameraComponent
{
	GENERATED_BODY()

public:
	const FSAEVUSAjusteCamara& ObtenerAjuste() const;

	/** Cambia el perfil de cámara y lo aplica en el acto. Nulo vuelve a los valores del código. */
	void FijarAjustesCamara(const USAEVUSAjustesCamara* NuevosAjustes);

	//~ Inicio de la interfaz de UCameraComponent
	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;
	//~ Fin de la interfaz de UCameraComponent

protected:
	/** Perfil opcional; sin él se usan los valores por defecto de FSAEVUSAjusteCamara. */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Camara")
	TObjectPtr<const USAEVUSAjustesCamara> AjustesCamara;

private:
	/** Pasa el ajuste activo a esta cámara y a su brazo. */
	void AplicarAjustes();

	/** Contorno, viñeta y saturación del estilo visual en el post-proceso de esta cámara. */
	void AplicarEstilo(const FSAEVUSAjusteCamara& Ajuste);

	/** Campo de visión base hasta la velocidad de carrera, mezclando hacia el de esprint a velocidad de esprint. */
	float ObtenerCampoVisionObjetivo() const;
};
