// Copyright SAEVUS. Todos los derechos reservados.

#include "Mundo/SAEVUSFuegoBrasero.h"
#include "Components/PointLightComponent.h"
#include "Efectos/SAEVUSComponenteParticulas.h"

namespace SAEVUSFuego
{
	// La luz va por encima del bloque del brasero.
	const FVector PosicionLuz(0.f, 0.f, 60.f);
	constexpr float RadioLuz = 1100.f;
	const FLinearColor ColorLuz(1.f, 0.55f, 0.22f);
	// Brasas: diminutas, suben despacio y se apagan.
	constexpr float TamanoBrasa = 0.03f;
	constexpr float VidaBrasa = 1.6f;
	constexpr float SubidaBrasa = 25.f;
	constexpr int32 MaxBrasas = 16;
}

ASAEVUSFuegoBrasero::ASAEVUSFuegoBrasero()
{
	using namespace SAEVUSFuego;
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Raiz"));

	Luz = CreateDefaultSubobject<UPointLightComponent>(TEXT("Luz"));
	Luz->SetupAttachment(RootComponent);
	Luz->SetRelativeLocation(PosicionLuz);
	Luz->SetMobility(EComponentMobility::Movable);
	Luz->SetIntensityUnits(ELightUnits::Candelas);
	Luz->SetAttenuationRadius(RadioLuz);
	Luz->SetLightColor(ColorLuz);
	Luz->SetCastShadows(false);

	Brasas = CreateDefaultSubobject<USAEVUSComponenteParticulas>(TEXT("Brasas"));
	Brasas->SetupAttachment(RootComponent);
	FSAEVUSAjusteParticulas AjusteBrasas;
	AjusteBrasas.MaxParticulas = MaxBrasas;
	AjusteBrasas.Vida = VidaBrasa;
	AjusteBrasas.TamanoInicial = TamanoBrasa;
	AjusteBrasas.TamanoFinal = 0.f;
	AjusteBrasas.Arrastre = 0.6f;
	AjusteBrasas.AceleracionVertical = SubidaBrasa;
	AjusteBrasas.GiroMaximo = 90.f;
	Brasas->FijarAjuste(AjusteBrasas);
}

void ASAEVUSFuegoBrasero::BeginPlay()
{
	Super::BeginPlay();
	Desfase = static_cast<float>(GetUniqueID() % 101) * 1.37f;
	PosicionLuz = Luz->GetRelativeLocation();
	Luz->SetIntensity(Ajuste.Intensidad);
}

float ASAEVUSFuegoBrasero::ObtenerIntensidadActual() const
{
	return Luz ? Luz->Intensity : 0.f;
}

void ASAEVUSFuegoBrasero::Tick(float DeltaSeconds)
{
	using namespace SAEVUSFuego;
	Super::Tick(DeltaSeconds);
	// El tiempo se envuelve: con sesiones largas el float perdería precisión y el titileo iría a saltos (GAMEPLAY-27).
	Tiempo = FMath::Fmod(Tiempo + DeltaSeconds, 4096.f);
	// Fuera de la vista no titila ni suelta brasas (las que haya siguen su vida en el emisor).
	if (Ajuste.DescansoSinVerse > 0.f && Brasas && !Brasas->WasRecentlyRendered(Ajuste.DescansoSinVerse))
	{
		return;
	}
	TiempoLuz -= DeltaSeconds;
	if (TiempoLuz <= 0.f)
	{
		TiempoLuz = 1.f / Ajuste.ActualizacionesLuzPorSegundo;
		// Dos ruidos: uno lento (la llama respira) y uno rápido (tirones).
		const float Lento = FMath::PerlinNoise1D(Tiempo * Ajuste.RitmoTitileo * 0.35f + Desfase);
		const float Rapido = FMath::PerlinNoise1D(Tiempo * Ajuste.RitmoTitileo * 2.3f + Desfase * 3.f);
		Luz->SetIntensity(Ajuste.Intensidad * (1.f + Ajuste.Titileo * (0.65f * Lento + 0.35f * Rapido)));
		Luz->SetRelativeLocation(PosicionLuz + FVector(Rapido, Lento, Lento * 0.5f) * Ajuste.Baile);
	}
	const int32 Cantidad = USAEVUSComponenteParticulas::ConsumirEmision(AcumuladoBrasas, Ajuste.BrasasPorSegundo, DeltaSeconds);
	if (Cantidad > 0)
	{
		Brasas->EmitirRafaga(Luz->GetComponentLocation(), FVector::UpVector * Ajuste.VelocidadBrasas, Cantidad, Ajuste.DispersionBrasas);
	}
}
