// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAEVUSPersonaje.generated.h"

class USpringArmComponent;
class USAEVUSComponenteArmas;
class USAEVUSComponenteCamara;
class USAEVUSComponenteCombate;
class USAEVUSComponenteMovimiento;
class USAEVUSComponenteVida;
class USAEVUSConfigEntrada;
struct FInputActionValue;

/**
 * Personaje del jugador de SAEVUS.
 *
 * Raíz de composición: el movimiento vive en USAEVUSComponenteMovimiento, el cuerpo a cuerpo en
 * USAEVUSComponenteCombate, el arsenal y las armas de fuego en USAEVUSComponenteArmas, la vida en
 * USAEVUSComponenteVida y la cámara en USAEVUSComponenteCamara. El personaje reparte la entrada
 * entre ellos y deja que el combate vete acciones de movimiento y cambios de arma a mitad de un ataque.
 * La malla y la animación se asignan en la subclase Blueprint, no en C++.
 */
UCLASS()
class SAEVUS_API ASAEVUSPersonaje : public ACharacter
{
	GENERATED_BODY()

public:
	explicit ASAEVUSPersonaje(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Personaje")
	USAEVUSComponenteMovimiento* ObtenerMovimiento() const;

	USAEVUSComponenteCombate* ObtenerCombate() const { return Combate; }
	USAEVUSComponenteArmas* ObtenerArmas() const { return Armas; }
	USAEVUSComponenteVida* ObtenerVida() const { return Vida; }
	USpringArmComponent* ObtenerBrazoCamara() const { return BrazoCamara; }
	USAEVUSComponenteCamara* ObtenerCamara() const { return Camara; }

	/**
	 * Pulsación de salto desde cualquier entrada: salta ya, o la guarda en buffer en el aire.
	 * Se descarta durante un ataque que todavía no se puede cancelar con un salto.
	 */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void PedirSalto();

	/** Pulsación de dash desde cualquier entrada. Se descarta durante un ataque que todavía no se puede cancelar con un dash. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void PedirDash();

	/** Pulsación de la acción principal: ataque cuerpo a cuerpo, o gatillo del arma de fuego equipada. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void PedirAtaque();

	/** Suelta la acción principal (suelta el gatillo). */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void SoltarAtaque();

	/** Cambio a una ranura del arsenal. Se descarta durante un ataque que todavía no se puede cancelar con un cambio. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void PedirRanuraArma(int32 Ranura);

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void PedirSiguienteArma();

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void PedirRecarga();

	//~ Inicio de la interfaz de ACharacter
	virtual bool CanJumpWhileFalling() const override;
	//~ Fin de la interfaz de ACharacter

protected:
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Entrada")
	TObjectPtr<const USAEVUSConfigEntrada> ConfigEntrada;

private:
	void ManejarMover(const FInputActionValue& Valor);
	void ManejarMirar(const FInputActionValue& Valor);
	void ManejarEsprint(const FInputActionValue& Valor);
	void ManejarCaminar(const FInputActionValue& Valor);

	/** Detiene el cuerpo (ragdoll si la malla tiene asset de física), quita la entrada y pide la reaparición al modo de juego. */
	UFUNCTION()
	void ManejarMuerte(USAEVUSComponenteVida* ComponenteVida, AActor* Asesino);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteCombate> Combate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteArmas> Armas;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteVida> Vida;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> BrazoCamara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteCamara> Camara;
};
