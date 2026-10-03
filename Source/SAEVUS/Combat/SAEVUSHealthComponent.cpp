// Copyright SAEVUS. All Rights Reserved.

#include "Combat/SAEVUSHealthComponent.h"
#include "GameFramework/Actor.h"

USAEVUSHealthComponent::USAEVUSHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void USAEVUSHealthComponent::InitializeComponent()
{
	Super::InitializeComponent();
	// Valid before BeginPlay, so other components can read it while initializing.
	Health = MaxHealth;
}

void USAEVUSHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddUniqueDynamic(this, &ThisClass::HandleTakeAnyDamage);
	}
}

void USAEVUSHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIgnoreSelfDamage && DamageCauser == GetOwner())
	{
		return;
	}
	ApplyDamage(Damage, DamageCauser);
}

float USAEVUSHealthComponent::ApplyDamage(float Amount, AActor* DamageCauser)
{
	if (bDead || Amount <= 0.f)
	{
		return 0.f;
	}
	const float Removed = FMath::Min(Amount, Health);
	Health -= Removed;
	OnHealthChanged.Broadcast(this, -Removed, DamageCauser);
	if (Health <= 0.f)
	{
		Health = 0.f;
		bDead = true;
		OnDeath.Broadcast(this, DamageCauser);
	}
	return Removed;
}

float USAEVUSHealthComponent::Heal(float Amount)
{
	if (bDead || Amount <= 0.f)
	{
		return 0.f;
	}
	const float Added = FMath::Min(Amount, MaxHealth - Health);
	if (Added > 0.f)
	{
		Health += Added;
		OnHealthChanged.Broadcast(this, Added, nullptr);
	}
	return Added;
}

void USAEVUSHealthComponent::ResetHealth()
{
	const float Delta = MaxHealth - Health;
	Health = MaxHealth;
	bDead = false;
	if (Delta != 0.f)
	{
		OnHealthChanged.Broadcast(this, Delta, nullptr);
	}
}
