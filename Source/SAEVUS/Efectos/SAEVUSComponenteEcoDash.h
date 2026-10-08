// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SAEVUSComponenteEcoDash.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPoseableMeshComponent;

/** Cómo se ven los ecos del dash. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteEcoDash
{
	GENERATED_BODY()

	/** Ecos que deja un dash (y tamaño del grupo que se reutiliza). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eco", meta = (ClampMin = "0", ClampMax = "8"))
	int32 EcosPorDash = 4;

	/** Cada cuánto deja uno durante el dash (s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eco", meta = (ClampMin = "0.01", ForceUnits = "s"))
	float Intervalo = 0.035f;

	/** Lo que tarda un eco en apagarse (s). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eco", meta = (ClampMin = "0.05", ForceUnits = "s"))
	float Vida = 0.28f;

	/** Brillo del eco recién dejado; se apaga con la curva al cuadrado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eco", meta = (ClampMin = "0"))
	float Brillo = 1.6f;

	/** Color del eco (frío como el filo, REF-08 D). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Eco")
	FLinearColor Color = FLinearColor(0.55f, 0.75f, 1.f);
};

/**
 * Ecos del dash: siluetas del cuerpo que se quedan atrás a lo largo del dash y se apagan, dándole al dash su
 * identidad (GunZ lo marca con la estela; aquí además se ve el recorrido). Copian la pose de la malla poseable
 * del cuerpo en el momento de dejarlos; un grupo fijo de mallas se reutiliza, sin crear nada en pleno juego.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteEcoDash : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteEcoDash();

	/** Cuerpo cuya pose se copia (por defecto, la primera malla poseable del dueño). */
	void FijarCuerpo(UPoseableMeshComponent* NuevoCuerpo) { Cuerpo = NuevoCuerpo; }

	/** Ecos visibles ahora (tests). */
	int32 ObtenerEcosVisibles() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Eco")
	FSAEVUSAjusteEcoDash Ajuste;

	/** Material del eco (aditivo, sin luz; parámetros Color y Brillo). Lo pone el script de assets. */
	UPROPERTY(EditAnywhere, Category = "SAEVUS|Eco")
	TObjectPtr<UMaterialInterface> MaterialEco;

private:
	void AlIniciarDash(const FVector& Direccion);
	/** Deja un eco con la pose actual del cuerpo. */
	void DejarEco();

	struct FEco
	{
		float Edad = 0.f;
		bool bActivo = false;
	};

	UPROPERTY(Transient)
	TObjectPtr<UPoseableMeshComponent> Cuerpo;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPoseableMeshComponent>> Mallas;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materiales;

	TArray<FEco> Ecos;
	int32 Siguiente = 0;
	int32 EcosPendientes = 0;
	float TiempoSiguienteEco = 0.f;
	FDelegateHandle ManejadorDash;
};
