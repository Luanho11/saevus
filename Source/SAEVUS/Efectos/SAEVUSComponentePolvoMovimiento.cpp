// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponentePolvoMovimiento.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Movimiento/SAEVUSComponenteMovimiento.h"

namespace SAEVUSPolvo
{
	// Polvo algo más grueso y lento que las chispas; cae poco.
	constexpr float TamanoParticula = 0.12f;
	constexpr float VidaParticula = 0.55f;
	constexpr float ArrastreParticula = 4.f;
	constexpr float AceleracionVertical = 60.f;
	constexpr int32 MaxParticulas = 160;
	// El anillo sale casi horizontal, con algo de subida.
	constexpr float SubidaAnillo = 0.25f;
	// Intensidad del aterrizaje: mínimo y máximo respecto a la caída de referencia.
	constexpr float FraccionMinima = 0.3f;
	constexpr float FraccionMaxima = 1.5f;
}

USAEVUSComponentePolvoMovimiento::USAEVUSComponentePolvoMovimiento()
{
	using namespace SAEVUSPolvo;
	Ajuste.TamanoInicial = TamanoParticula;
	Ajuste.Vida = VidaParticula;
	Ajuste.Arrastre = ArrastreParticula;
	Ajuste.AceleracionVertical = AceleracionVertical;
	Ajuste.MaxParticulas = MaxParticulas;
}

USAEVUSComponenteMovimiento* USAEVUSComponentePolvoMovimiento::ObtenerMovimiento() const
{
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	return Personaje ? Cast<USAEVUSComponenteMovimiento>(Personaje->GetCharacterMovement()) : nullptr;
}

FVector USAEVUSComponentePolvoMovimiento::ObtenerPies() const
{
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	const float MediaAltura = Personaje && Personaje->GetCapsuleComponent() ? Personaje->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.f;
	return GetOwner()->GetActorLocation() - FVector(0.f, 0.f, MediaAltura);
}

void USAEVUSComponentePolvoMovimiento::BeginPlay()
{
	Super::BeginPlay();
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		ManejadorSaltoPared = Movimiento->AlSaltoPared.AddUObject(this, &ThisClass::AlSaltoPared);
	}
}

void USAEVUSComponentePolvoMovimiento::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		Movimiento->AlSaltoPared.Remove(ManejadorSaltoPared);
	}
	Super::EndPlay(EndPlayReason);
}

void USAEVUSComponentePolvoMovimiento::EmitirAterrizaje(float VelocidadCaida)
{
	using namespace SAEVUSPolvo;
	if (VelocidadCaida < AjustePolvo.VelocidadCaidaMinima)
	{
		return;
	}
	const float Fraccion = FMath::Clamp(VelocidadCaida / AjustePolvo.VelocidadCaidaReferencia, FraccionMinima, FraccionMaxima);
	const int32 Cantidad = FMath::RoundToInt32(AjustePolvo.ParticulasAterrizaje * Fraccion);
	const FVector Pies = ObtenerPies();
	for (int32 Indice = 0; Indice < Cantidad; ++Indice)
	{
		// Anillo: repartidas alrededor, hacia fuera y un poco hacia arriba.
		const float Angulo = UE_TWO_PI * (Indice + Aleatorio.FRand() * 0.5f) / FMath::Max(Cantidad, 1);
		const FVector Fuera(FMath::Cos(Angulo), FMath::Sin(Angulo), SubidaAnillo);
		Emitir(Pies, Fuera * AjustePolvo.VelocidadAnillo * Fraccion + Aleatorio.VRand() * AjustePolvo.Dispersion * 0.3f);
	}
}

void USAEVUSComponentePolvoMovimiento::AlSaltoPared(const FVector& NormalPared)
{
	const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
	const float Radio = Personaje && Personaje->GetCapsuleComponent() ? Personaje->GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.f;
	// Contra la pared, a media altura de las piernas: el pie se impulsa en ella.
	const FVector Contacto = (GetOwner()->GetActorLocation() + ObtenerPies()) * 0.5f - NormalPared * Radio;
	EmitirRafaga(Contacto, NormalPared * AjustePolvo.VelocidadSaltoPared, AjustePolvo.ParticulasSaltoPared, AjustePolvo.Dispersion);
}

void USAEVUSComponentePolvoMovimiento::EmitirContinuo(float PorSegundo, float DeltaTime, const FVector& Posicion, const FVector& Velocidad)
{
	AcumuladoEmision += PorSegundo * DeltaTime;
	const int32 Cantidad = FMath::FloorToInt32(AcumuladoEmision);
	AcumuladoEmision -= Cantidad;
	if (Cantidad > 0)
	{
		EmitirRafaga(Posicion, Velocidad, Cantidad, AjustePolvo.Dispersion);
	}
}

void USAEVUSComponentePolvoMovimiento::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (const USAEVUSComponenteMovimiento* Movimiento = ObtenerMovimiento())
	{
		const bool bEnAire = Movimiento->IsFalling();
		if (bEstabaEnAire && Movimiento->IsMovingOnGround())
		{
			EmitirAterrizaje(-VelocidadVerticalAire);
		}
		if (bEnAire)
		{
			VelocidadVerticalAire = static_cast<float>(Movimiento->Velocity.Z);
		}
		bEstabaEnAire = bEnAire;

		const FVector Normal = Movimiento->ObtenerNormalPared();
		const ACharacter* Personaje = Cast<ACharacter>(GetOwner());
		const float Radio = Personaje && Personaje->GetCapsuleComponent() ? Personaje->GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.f;
		if (!Normal.IsNearlyZero())
		{
			// Corriendo por la pared o escalando: polvo donde los pies la pisan, que se queda atrás.
			const FVector Contacto = ObtenerPies() + FVector(0.f, 0.f, Radio) - Normal * Radio;
			EmitirContinuo(AjustePolvo.ParticulasPorSegundoPared, DeltaTime, Contacto, Normal * AjustePolvo.Dispersion - Movimiento->Velocity * 0.3f);
		}
		else if (Movimiento->EstaEnDash() && Movimiento->IsMovingOnGround())
		{
			// Dash en el suelo: polvo a ras de suelo, hacia atrás.
			EmitirContinuo(AjustePolvo.ParticulasPorSegundoDashSuelo, DeltaTime, ObtenerPies(), -Movimiento->ObtenerDireccionDash() * AjustePolvo.VelocidadAnillo);
		}
		else
		{
			AcumuladoEmision = 0.f;
		}
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
