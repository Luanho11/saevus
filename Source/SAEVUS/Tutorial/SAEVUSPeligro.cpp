// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSPeligro.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSPeligro
{
	const TCHAR* const RutaCono = TEXT("/Engine/BasicShapes/Cone.Cone");
	constexpr float TamanoForma = 100.f;
	// Cada pincho ocupa casi toda su celda y toda la altura de la caja.
	constexpr float FraccionAnchoPincho = 0.8f;
}

ASAEVUSPeligro::ASAEVUSPeligro()
{
	// Objeto dinámico que responde a consultas: así lo encuentra la esfera del tajo hacia abajo.
	Caja->SetCollisionObjectType(ECC_WorldDynamic);

	Pinchos = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Pinchos"));
	Pinchos->SetupAttachment(Caja);
	Pinchos->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cono(SAEVUSPeligro::RutaCono);
	Pinchos->SetStaticMesh(Cono.Object);
}

float ASAEVUSPeligro::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Los pinchos no se rompen ni cuentan como golpe (no dan alma); solo sirven para rebotar.
	return 0.f;
}

void ASAEVUSPeligro::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ColocarPinchos();
}

void ASAEVUSPeligro::BeginPlay()
{
	Super::BeginPlay();
	if (Pinchos->GetInstanceCount() == 0)
	{
		ColocarPinchos();
	}
}

void ASAEVUSPeligro::ColocarPinchos()
{
	using namespace SAEVUSPeligro;
	Pinchos->ClearInstances();
	const FVector Extension = Caja->GetUnscaledBoxExtent();
	const int32 NumeroX = FMath::Max(1, FMath::FloorToInt32(2.f * Extension.X / SeparacionPinchos));
	const int32 NumeroY = FMath::Max(1, FMath::FloorToInt32(2.f * Extension.Y / SeparacionPinchos));
	const float Ancho = SeparacionPinchos * FraccionAnchoPincho / TamanoForma;
	const FVector Escala(Ancho, Ancho, 2.f * Extension.Z / TamanoForma);
	for (int32 IndiceX = 0; IndiceX < NumeroX; ++IndiceX)
	{
		for (int32 IndiceY = 0; IndiceY < NumeroY; ++IndiceY)
		{
			// El cono del motor tiene el pivote en el centro: base en el fondo de la caja.
			const FVector Posicion(-Extension.X + (IndiceX + 0.5f) * 2.f * Extension.X / NumeroX, -Extension.Y + (IndiceY + 0.5f) * 2.f * Extension.Y / NumeroY, 0.f);
			Pinchos->AddInstance(FTransform(FRotator::ZeroRotator, Posicion, Escala));
		}
	}
}
