// Copyright SAEVUS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SAEVUSTrainingDummy.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

/**
 * Target for testing attacks: a pawn-sized capsule that counts the hits and damage it takes
 * and shows them above itself. It never dies, moves or fights back (not an enemy).
 */
UCLASS()
class SAEVUS_API ASAEVUSTrainingDummy : public AActor
{
	GENERATED_BODY()

public:
	ASAEVUSTrainingDummy();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Training Dummy")
	int32 GetHitCount() const { return HitCount; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Training Dummy")
	float GetTotalDamage() const { return TotalDamage; }

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Training Dummy")
	float GetLastDamage() const { return LastDamage; }

	UFUNCTION(BlueprintCallable, Category = "SAEVUS|Training Dummy")
	void ResetCounters();

protected:
	virtual void BeginPlay() override;

private:
	void UpdateReadout();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UTextRenderComponent> Readout;

	int32 HitCount = 0;
	float TotalDamage = 0.f;
	float LastDamage = 0.f;
};
