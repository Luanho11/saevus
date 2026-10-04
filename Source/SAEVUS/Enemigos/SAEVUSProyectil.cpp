// Copyright SAEVUS. Todos los derechos reservados.

#include "Enemigos/SAEVUSProyectil.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemigos/SAEVUSEnemigo.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Personaje/SAEVUSPersonaje.h"
#include "UObject/ConstructorHelpers.h"

namespace SAEVUSProyectil
{
	constexpr float Radio = 18.f;
	const TCHAR* const RutaEsfera = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	constexpr float TamanoForma = 100.f;
	constexpr float BrilloProyectil = 2.f;
	const FName ParametroAutoIluminacion(TEXT("AutoIluminacion"));
}

ASAEVUSProyectil::ASAEVUSProyectil()
{
	using namespace SAEVUSProyectil;
	PrimaryActorTick.bCanEverTick = true;

	Esfera = CreateDefaultSubobject<USphereComponent>(TEXT("Esfera"));
	Esfera->InitSphereRadius(Radio);
	// Objeto dinámico consultable (lo encuentra la esfera del tajo); choca con el escenario y solapa pawns.
	Esfera->SetCollisionObjectType(ECC_WorldDynamic);
	Esfera->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Esfera->SetCollisionResponseToAllChannels(ECR_Ignore);
	Esfera->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Esfera->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Esfera->SetGenerateOverlapEvents(true);
	RootComponent = Esfera;

	Malla = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Malla"));
	Malla->SetupAttachment(Esfera);
	Malla->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Malla->SetCastShadow(false);
	Malla->SetRelativeScale3D(FVector(2.f * Radio / TamanoForma));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MallaEsfera(RutaEsfera);
	Malla->SetStaticMesh(MallaEsfera.Object);
}

void ASAEVUSProyectil::Lanzar(const FVector& Direccion, float NuevaVelocidad, float NuevoDanio, AActor* NuevoLanzador, UMaterialInterface* Material)
{
	Velocidad = Direccion.GetSafeNormal() * NuevaVelocidad;
	Danio = NuevoDanio;
	Lanzador = NuevoLanzador;
	Edad = 0.f;
	bDevuelto = false;
	Esfera->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::AlSolapar);
	if (Material)
	{
		UMaterialInstanceDynamic* Brillo = UMaterialInstanceDynamic::Create(Material, this);
		Brillo->SetScalarParameterValue(SAEVUSProyectil::ParametroAutoIluminacion, SAEVUSProyectil::BrilloProyectil);
		Malla->SetMaterial(0, Brillo);
	}
}

float ASAEVUSProyectil::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Devuelto hacia quien lo lanzó (o en sentido contrario), más rápido.
	if (!bDevuelto)
	{
		bDevuelto = true;
		const FVector Hacia = Lanzador.IsValid() ? (Lanzador->GetActorLocation() - GetActorLocation()).GetSafeNormal() : -Velocidad.GetSafeNormal();
		Velocidad = Hacia * Velocidad.Size() * EscalaVelocidadDevuelto;
		Edad = 0.f;
	}
	return 0.f;
}

void ASAEVUSProyectil::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Edad += DeltaSeconds;
	if (Edad >= Duracion)
	{
		Destroy();
		return;
	}
	FHitResult Choque;
	SetActorLocation(GetActorLocation() + Velocidad * DeltaSeconds, true, &Choque);
	if (Choque.bBlockingHit)
	{
		Destroy();
	}
}

void ASAEVUSProyectil::AlSolapar(UPrimitiveComponent* Componente, AActor* Otro, UPrimitiveComponent* OtroComponente, int32 Indice, bool bBarrido, const FHitResult& Golpe)
{
	Impactar(Otro);
}

void ASAEVUSProyectil::Impactar(AActor* Objetivo)
{
	// Antes de devolverlo no toca a quien lo lanzó; devuelto, precisamente a él.
	if (!IsValid(this) || IsActorBeingDestroyed() || !Objetivo || (!bDevuelto && Objetivo == Lanzador.Get()))
	{
		return;
	}
	// Sin devolver hace daño al jugador; devuelto, a los enemigos.
	const bool bObjetivoValido = bDevuelto ? Objetivo->IsA<ASAEVUSEnemigo>() : Objetivo->IsA<ASAEVUSPersonaje>();
	if (!bObjetivoValido)
	{
		return;
	}
	// El causante es el proyectil: la guardia mira de dónde llega.
	UGameplayStatics::ApplyDamage(Objetivo, Danio, nullptr, this, UDamageType::StaticClass());
	Destroy();
}
