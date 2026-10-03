// Copyright SAEVUS. All Rights Reserved.

#include "World/SAEVUSTrainingDummy.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "SAEVUS.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSTrainingDummy
{
	// Same size as the player capsule, so hit ranges read the same as against a character.
	constexpr float CapsuleRadius = 42.f;
	constexpr float CapsuleHalfHeight = 96.f;

	// Engine cylinder (100 cm across and tall, pivot at the centre) stretched to the capsule.
	const TCHAR* const BodyMeshPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	constexpr float CylinderSize = 100.f;

	constexpr float ReadoutHeightAboveCapsule = 30.f;
	constexpr float ReadoutWorldSize = 24.f;
}

ASAEVUSTrainingDummy::ASAEVUSTrainingDummy()
{
	using namespace SAEVUSTrainingDummy;
	PrimaryActorTick.bCanEverTick = false;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	Capsule->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = Capsule;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(Capsule);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Body->SetRelativeScale3D(FVector(2.f * CapsuleRadius / CylinderSize, 2.f * CapsuleRadius / CylinderSize, 2.f * CapsuleHalfHeight / CylinderSize));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(BodyMeshPath);
	if (CylinderMesh.Succeeded())
	{
		Body->SetStaticMesh(CylinderMesh.Object);
	}

	Readout = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Readout"));
	Readout->SetupAttachment(Capsule);
	Readout->SetRelativeLocation(FVector(0.f, 0.f, CapsuleHalfHeight + ReadoutHeightAboveCapsule));
	Readout->SetHorizontalAlignment(EHTA_Center);
	Readout->SetWorldSize(ReadoutWorldSize);
	Readout->SetTextRenderColor(FColor::White);
}

void ASAEVUSTrainingDummy::BeginPlay()
{
	Super::BeginPlay();
	UpdateReadout();
}

float ASAEVUSTrainingDummy::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	++HitCount;
	LastDamage = DamageAmount;
	TotalDamage += DamageAmount;
	UE_LOG(LogSAEVUS, Verbose, TEXT("%s hit by %s for %.1f (hits %d, total %.1f)"), *GetName(), *GetNameSafe(DamageCauser), DamageAmount, HitCount, TotalDamage);
	UpdateReadout();
	return Applied;
}

void ASAEVUSTrainingDummy::ResetCounters()
{
	HitCount = 0;
	TotalDamage = 0.f;
	LastDamage = 0.f;
	UpdateReadout();
}

void ASAEVUSTrainingDummy::UpdateReadout()
{
	Readout->SetText(FText::FromString(FString::Printf(TEXT("Hits %d | Last %.0f | Total %.0f"), HitCount, LastDamage, TotalDamage)));
}
