// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Enemigos/SAEVUSTiposEnemigo.h"
#include "GameFramework/Character.h"
#include "SAEVUSEnemigo.generated.h"

class UMaterialInstanceDynamic;
class UPoseableMeshComponent;
class USAEVUSComponenteAnimacionProcedural;
class UStaticMeshComponent;
class USAEVUSComponenteIAEnemigo;
class USAEVUSComponenteParticulas;
class USAEVUSComponenteSonidoEnemigo;
class USAEVUSComponenteEscudo;
class USAEVUSComponenteVida;
class ASAEVUSEnemigo;

DECLARE_MULTICAST_DELEGATE_OneParam(FSAEVUSEventoEnemigo, ASAEVUSEnemigo* /*Enemigo*/);

/**
 * Enemigo de SAEVUS: vida, IA de estados (USAEVUSComponenteIAEnemigo) y un cuerpo boceto de
 * piezas del motor (cuerpo, máscara, cuernos) coloreado por su tipo. Todo lo define un
 * USAEVUSDatosEnemigo; sin él usa el mini enemigo por defecto.
 *
 * Aspecto procedural: se encoge al avisar y se estira al atacar, la máscara se enciende con el
 * aviso, y al morir se encoge y suelta partículas.
 */
UCLASS()
class SAEVUS_API ASAEVUSEnemigo : public ACharacter
{
	GENERATED_BODY()

public:
	ASAEVUSEnemigo();

	const FSAEVUSAjusteEnemigo& ObtenerAjuste() const;

	/** Cambia el tipo y lo aplica (cápsula, vida, velocidad, colores). */
	void FijarDatos(const USAEVUSDatosEnemigo* NuevosDatos);

	USAEVUSComponenteVida* ObtenerVida() const { return Vida; }
	USAEVUSComponenteIAEnemigo* ObtenerIA() const { return IA; }

	/** Destello al recibir un golpe: 1 justo al recibirlo, baja a 0. */
	float ObtenerDestello() const;

	/** Cierto si su tipo usa un cuerpo con esqueleto animado (goblins nuevos). */
	bool TieneEsqueleto() const;

	/** Material de sus proyectiles (el de la máscara: brilla igual). */
	UMaterialInterface* ObtenerMaterialProyectil() const;

	/** Se emite al morir cualquier enemigo (puertas del tutorial, recuentos). */
	static FSAEVUSEventoEnemigo AlMorirEnemigo;

	//~ Inicio de la interfaz de ACharacter
	virtual void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride) override;
	//~ Fin de la interfaz de ACharacter

	/**
	 * El daño solo lo decide quien tiene autoridad; no hace falta modo de juego (APawn lo exige),
	 * así funciona en cualquier mapa y en los mundos de prueba.
	 */
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const override;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "SAEVUS|Enemigo")
	TObjectPtr<const USAEVUSDatosEnemigo> Datos;

private:
	/** Cápsula, vida, velocidad y piezas según el ajuste. */
	void AplicarAjuste();
	void CrearMateriales();
	/** Escala del cuerpo y brillo de la máscara según el estado de la IA. */
	void ActualizarAspecto(float DeltaSeconds);

	UFUNCTION()
	void ManejarMuerte(USAEVUSComponenteVida* ComponenteVida, AActor* Asesino);

	/** Al recibir daño: destello blanco breve (lectura del golpe). */
	UFUNCTION()
	void ManejarCambioVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador);

	/** Pinta el destello sobre los colores del tipo. */
	void AplicarDestello(float DeltaSeconds);

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USAEVUSComponenteVida> Vida;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USAEVUSComponenteIAEnemigo> IA;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USceneComponent> Piezas;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Cuerpo;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> Mascara;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> CuernoD;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UStaticMeshComponent> CuernoI;

	/** Cuerpo con esqueleto (MallaEsqueletica de los datos), animado por Animacion desde la IA. */
	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<UPoseableMeshComponent> CuerpoAnimado;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USAEVUSComponenteAnimacionProcedural> Animacion;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USAEVUSComponenteParticulas> Efectos;

	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USAEVUSComponenteSonidoEnemigo> Sonido;

	/** Escudo (solo bloquea si el tipo tiene AnguloEscudo). */
	UPROPERTY(VisibleAnywhere, Category = "Componentes")
	TObjectPtr<USAEVUSComponenteEscudo> Escudo;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MaterialCuerpo;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MaterialMascara;

	/** Cuerpo con esqueleto: una instancia por ranura (el destello las pinta todas) y su color de base. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> MaterialesPiezas;
	TArray<FLinearColor> ColoresPiezasBase;

	FVector EscalaPiezas = FVector::OneVector;
	float TiempoMuerto = 0.f;
	float TiempoAleteo = 0.f;
	float TiempoDestello = 0.f;
	bool bDestelloPintado = false;
};
