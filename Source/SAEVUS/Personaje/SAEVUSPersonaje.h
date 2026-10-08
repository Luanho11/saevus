// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Personaje/SAEVUSDetectorDobleToque.h"
#include "SAEVUSPersonaje.generated.h"

class UPoseableMeshComponent;
class USpringArmComponent;
class USAEVUSComponenteAlma;
class USAEVUSComponenteGuardia;
class USAEVUSComponenteSonido;
class USAEVUSComponenteAnimacionProcedural;
class USAEVUSComponenteEfectosCombate;
class USAEVUSComponenteEstelaEspada;
class USAEVUSComponenteEcoDash;
class USAEVUSComponenteMotasAmbiente;
class USAEVUSComponenteArmas;
class USAEVUSComponenteCamara;
class USAEVUSComponenteCombate;
class USAEVUSComponenteEstelaDash;
class USAEVUSComponentePolvoMovimiento;
class USAEVUSComponenteMovimiento;
class USAEVUSComponenteVida;
class USAEVUSConfigEntrada;
struct FInputActionValue;

/** Reacción del jugador al recibir un golpe: sale despedido lejos del causante. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteGolpeRecibido
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadHorizontal = 380.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Golpe recibido", meta = (ClampMin = "0", ForceUnits = "cm/s"))
	float VelocidadVertical = 350.f;
};

/** Progreso del personaje (Hollow Knight): fragmentos de máscara que suman vida máxima. */
USTRUCT(BlueprintType)
struct FSAEVUSAjusteProgreso
{
	GENERATED_BODY()

	/** Fragmentos que hacen una máscara entera. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progreso", meta = (ClampMin = "1"))
	int32 FragmentosPorMascara = 2;

	/** Vida que suma cada máscara entera (la interfaz pinta una máscara cada 20). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progreso", meta = (ClampMin = "0"))
	float VidaPorMascara = 20.f;
};

/**
 * Personaje del jugador de SAEVUS.
 *
 * Raíz de composición: el movimiento vive en USAEVUSComponenteMovimiento, el cuerpo a cuerpo en
 * USAEVUSComponenteCombate, el arsenal y las armas de fuego en USAEVUSComponenteArmas, la vida en
 * USAEVUSComponenteVida y la cámara en USAEVUSComponenteCamara. El personaje reparte la entrada
 * entre ellos y deja que el combate vete acciones de movimiento y cambios de arma a mitad de un ataque.
 * El cuerpo visible es una malla poseable (`Cuerpo`, esqueleto propio) animada por
 * USAEVUSComponenteAnimacionProcedural; la malla se asigna en la subclase Blueprint, no en C++.
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
	USAEVUSComponenteAlma* ObtenerAlma() const { return Alma; }
	USAEVUSComponenteGuardia* ObtenerGuardia() const { return Guardia; }
	USAEVUSComponenteEfectosCombate* ObtenerEfectosCombate() const { return EfectosCombate; }
	USAEVUSComponenteEstelaEspada* ObtenerEstelaEspada() const { return EstelaEspada; }
	USAEVUSComponenteEcoDash* ObtenerEcoDash() const { return EcoDash; }
	USAEVUSComponenteEstelaDash* ObtenerEstelaDash() const { return EstelaDash; }
	USAEVUSComponentePolvoMovimiento* ObtenerPolvoMovimiento() const { return PolvoMovimiento; }
	UPoseableMeshComponent* ObtenerCuerpo() const { return Cuerpo; }
	USAEVUSComponenteAnimacionProcedural* ObtenerAnimacion() const { return Animacion; }
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

	/** Dash en una dirección del mundo (horizontal), p. ej. la del doble toque. Mismas reglas que PedirDash. */
	void PedirDashHacia(const FVector& Direccion);

	/**
	 * Entrada de movimiento en ejes locales a la cámara (X = derecha, Y = adelante): mueve al
	 * personaje y alimenta el detector de doble toque. La usan la acción de mover y los tests.
	 */
	void AplicarEntradaMovimiento(const FVector2D& Eje);

	/** Cierto mientras la cámara no debe girar (dash con bBloquearCamaraEnDash). */
	bool CamaraBloqueada() const;

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

	/** Acción de guardia pulsada o soltada. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void FijarGuardia(bool bPulsada);

	/** Acción de curar pulsada o soltada. */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void FijarCuracion(bool bPulsada);

	/** Acción de interactuar: usa lo que hay cerca (o se levanta si está sentado). */
	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Personaje")
	void PedirInteraccion();

	/** Lo que se puede usar ahora (lo anuncian los objetos al acercarse; ver ISAEVUSInteractuable). */
	void FijarInteractuable(AActor* Actor);
	/** Lo olvida si es el actual. */
	void QuitarInteractuable(AActor* Actor);
	AActor* ObtenerInteractuable() const { return Interactuable.Get(); }
	const USAEVUSConfigEntrada* ObtenerConfigEntrada() const { return ConfigEntrada; }
	const FSAEVUSAjusteProgreso& ObtenerProgreso() const { return Progreso; }

	/** Se sienta en Asiento (banco): quieto hasta que se mueve, salta, ataca o vuelve a interactuar. */
	void Sentarse(const FTransform& Asiento);
	/** Solo de pie en el suelo, vivo, sin dash y sin atacar. */
	bool PuedeSentarse() const;
	void Levantarse();
	bool EstaSentado() const { return bSentado; }

	/** Suma un fragmento de máscara (se guarda): con FragmentosPorMascara, una máscara más y vida llena. */
	void RecogerFragmentoMascara(FName Identificador);
	int32 ObtenerFragmentosMascara() const { return FragmentosMascara; }

	//~ Inicio de la interfaz de ACharacter
	virtual bool CanJumpWhileFalling() const override;
	//~ Fin de la interfaz de ACharacter

	/**
	 * El daño solo lo decide quien tiene autoridad; no hace falta modo de juego (APawn lo exige),
	 * así funciona en cualquier mapa y en los mundos de prueba.
	 */
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const override;
	/** Devuelve la vida que se perdió de verdad (0 si la guardia o la invulnerabilidad lo paran todo). */
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Entrada")
	TObjectPtr<const USAEVUSConfigEntrada> ConfigEntrada;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Combate")
	FSAEVUSAjusteGolpeRecibido GolpeRecibido;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Progreso")
	FSAEVUSAjusteProgreso Progreso;

	virtual void BeginPlay() override;

private:
	void ManejarMover(const FInputActionValue& Valor);
	void ManejarMoverSoltar(const FInputActionValue& Valor);
	void ManejarMirar(const FInputActionValue& Valor);
	void ManejarEsprint(const FInputActionValue& Valor);
	void ManejarCaminar(const FInputActionValue& Valor);
	void ManejarCurar(const FInputActionValue& Valor);
	void ManejarGuardia(const FInputActionValue& Valor);

	TWeakObjectPtr<AActor> Interactuable;
	bool bSentado = false;
	int32 FragmentosMascara = 0;
	/** Vida máxima sin máscaras extra (la de la vida al empezar). */
	float VidaMaximaBase = 0.f;

	/** Caer por debajo del mundo es morir y reaparecer, no quedarse sin personaje (GAMEPLAY-3). */
	virtual void FellOutOfWorld(const UDamageType& TipoDanio) override;

public:
	/** Opciones de comodidad del jugador (sacudida de cámara, campo de visión dinámico, parpadeo tras un golpe). */
	void AplicarComodidad(bool bSacudida, bool bCampoVision, bool bParpadeo);

private:
	/** Al empezar un dash: invulnerabilidad breve. */
	void AlIniciarDash(const FVector& Direccion);

	/** Momento del último dash (segundos del mundo): decide si el siguiente es una racha sin invulnerabilidad. */
	float TiempoUltimoDash = -1000.f;

	/** Rotación que orienta la entrada: la del controlador, o los ejes del mundo sin controlador (tests). */
	FRotator ObtenerRotacionEntrada() const;

	/** Detiene el cuerpo (ragdoll si la malla tiene asset de física), quita la entrada y pide la reaparición al modo de juego. */
	UFUNCTION()
	void ManejarMuerte(USAEVUSComponenteVida* ComponenteVida, AActor* Asesino);

	/** Al recibir daño de otro actor: sale despedido lejos de él. */
	UFUNCTION()
	void ManejarCambioVida(USAEVUSComponenteVida* ComponenteVida, float Delta, AActor* Instigador);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteCombate> Combate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteArmas> Armas;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteVida> Vida;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteAlma> Alma;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteGuardia> Guardia;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteEstelaDash> EstelaDash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponentePolvoMovimiento> PolvoMovimiento;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteEfectosCombate> EfectosCombate;

	/** Media luna que sigue al filo en la fase activa de los tajos (REF-08 E). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteEstelaEspada> EstelaEspada;

	/** Polvo en suspensión alrededor del jugador (atmósfera). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteMotasAmbiente> MotasAmbiente;

	/** Siluetas del cuerpo que quedan atrás en el dash (su identidad visual). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteEcoDash> EcoDash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteSonido> Sonido;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPoseableMeshComponent> Cuerpo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteAnimacionProcedural> Animacion;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> BrazoCamara;

	FSAEVUSDetectorDobleToque DetectorDobleToque;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componentes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USAEVUSComponenteCamara> Camara;
};
