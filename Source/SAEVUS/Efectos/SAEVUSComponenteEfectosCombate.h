// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Efectos/SAEVUSComponenteParticulas.h"
#include "SAEVUSComponenteEfectosCombate.generated.h"

class USAEVUSComponenteAlma;
class USAEVUSComponenteAnimacionProcedural;
class USAEVUSComponenteCombate;
class USAEVUSComponenteGuardia;

/** Cómo se ven los efectos de golpe, alma y curación. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteEfectosCombate
{
	GENERATED_BODY()

	/** Chispas en el actor golpeado por un ataque cuerpo a cuerpo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0"))
	int32 ParticulasGolpe = 10;

	/** Daño que da las chispas normales; más daño, más chispas (de 0,6 a 2 veces). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0.1"))
	float DanioReferenciaGolpe = 12.f;

	/** Si la hoja pasa más lejos que esto del golpeado, las chispas salen de su centro. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ForceUnits = "cm"))
	float DistanciaMaximaContacto = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadGolpe = 350.f;

	/** Destello que sube del cuerpo al completar una curación. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0"))
	int32 ParticulasCuracion = 24;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadCuracion = 220.f;

	/** Partículas por segundo que se juntan alrededor del cuerpo mientras cura. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0"))
	float ParticulasPorSegundoCurando = 30.f;

	/** Radio del que salen las partículas que convergen al curar. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Curación", meta = (ClampMin = "0", ForceUnits = "cm"))
	float RadioConvergencia = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "General", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float Dispersion = 140.f;

	/** Estela del aguijón: partículas repartidas a lo largo de la hoja en cada fotograma de la fase activa. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela del arma", meta = (ClampMin = "0"))
	int32 ParticulasEstelaPorFotograma = 1;

	/** Partículas por segundo que se juntan en la hoja mientras se carga el ataque cargado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Carga", meta = (ClampMin = "0"))
	float ParticulasPorSegundoCarga = 40.f;

	/** Chispas del bloqueo de la guardia. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guardia", meta = (ClampMin = "0"))
	int32 ParticulasBloqueo = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Guardia", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadBloqueo = 420.f;

	/** Velocidad aleatoria pequeña de la estela: se queda casi quieta y dibuja el arco. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Estela del arma", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float DispersionEstela = 15.f;

	/**
	 * Arco del tajo: durante la fase activa, partículas sobre el abanico de golpe (de su alcance hacia
	 * dentro), barriéndolo en el sentido del tajo. Dibuja exactamente lo que golpea.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arco del tajo", meta = (ClampMin = "0"))
	int32 ParticulasArcoPorPaso = 1;

	/** Pasos de ángulo por grado de barrido (el arco no deja huecos aunque el fotograma sea largo). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arco del tajo", meta = (ClampMin = "0.05"))
	float PasosArcoPorGrado = 0.45f;

	/** Desde qué fracción del alcance hacia fuera se dibuja el arco (1 = solo el borde). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arco del tajo", meta = (ClampMin = "0", ClampMax = "1"))
	float FraccionInteriorArco = 0.6f;
};

/**
 * Partículas de combate del personaje: chispas al golpear, partículas que convergen mientras
 * cura y un destello al completar la curación. Escucha USAEVUSComponenteCombate::AlGolpear y los
 * eventos de USAEVUSComponenteAlma del dueño.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteEfectosCombate : public USAEVUSComponenteParticulas
{
	GENERATED_BODY()

public:
	USAEVUSComponenteEfectosCombate();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Efectos")
	FSAEVUSAjusteEfectosCombate AjusteEfectos;

private:
	UFUNCTION()
	void AlGolpear(AActor* ActorGolpeado, float Danio);

	void AlCurar(float VidaRecuperada);
	void AlBloquear(AActor* Atacante, const FVector& PuntoContacto);

	/** Partículas a lo largo de la hoja mientras el tajo está en su fase activa. */
	void EmitirEstelaArma();
	void EmitirArcoTajo();
	/** Ángulo (grados, desde el frente) por el que iba el barrido del arco el fotograma anterior. */
	float AnguloArcoAnterior = 0.f;
	bool bArcoActivo = false;

	/** Partículas que se juntan en la hoja mientras se carga el ataque cargado. */
	void EmitirCarga(float DeltaTime);

	float AcumuladoCarga = 0.f;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteAlma> Alma;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteCombate> Combate;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteAnimacionProcedural> Animacion;

	UPROPERTY()
	TObjectPtr<USAEVUSComponenteGuardia> Guardia;

	FDelegateHandle ManejadorCurar;
	FDelegateHandle ManejadorBloqueo;
	float AcumuladoEmision = 0.f;
};
