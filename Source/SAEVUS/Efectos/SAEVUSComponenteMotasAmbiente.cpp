// Copyright SAEVUS. Todos los derechos reservados.

#include "Efectos/SAEVUSComponenteMotasAmbiente.h"

namespace SAEVUSMotas
{
	// Cada mota: muy pequeña, vive mucho, apenas frena y sube despacio (aire caliente).
	constexpr float Tamano = 0.032f;
	constexpr float Vida = 7.f;
	constexpr float Arrastre = 0.05f;
	constexpr float Subida = 2.f;
	constexpr float Giro = 20.f;
}

USAEVUSComponenteMotasAmbiente::USAEVUSComponenteMotasAmbiente()
{
	using namespace SAEVUSMotas;
	Ajuste.MaxParticulas = AjusteMotas.Cantidad;
	Ajuste.Vida = Vida;
	Ajuste.TamanoInicial = Tamano;
	Ajuste.TamanoFinal = 0.f;
	Ajuste.Arrastre = Arrastre;
	Ajuste.AceleracionVertical = Subida;
	Ajuste.GiroMaximo = Giro;
}

void USAEVUSComponenteMotasAmbiente::BeginPlay()
{
	// El pool se crea con la cantidad de motas (editable en el Blueprint).
	Ajuste.MaxParticulas = FMath::Clamp(AjusteMotas.Cantidad, 1, 512);
	Super::BeginPlay();
}

void USAEVUSComponenteMotasAmbiente::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// Repone las que se apagan, repartidas por la caja alrededor del dueño: el aire nunca se vacía.
	const FVector Centro = GetOwner()->GetActorLocation();
	// Las que quedaron fuera de la caja (el jugador avanzó) se reciclan: renacen cerca en vez de esperar a morir de vieja.
	const float Margen = 1.1f;
	ApagarSi([&](const FVector& Posicion)
	{
		const FVector Relativa = Posicion - Centro;
		return FMath::Abs(Relativa.X) > AjusteMotas.Radio * Margen || FMath::Abs(Relativa.Y) > AjusteMotas.Radio * Margen
			|| Relativa.Z > AjusteMotas.Altura * Margen || Relativa.Z < -AjusteMotas.Altura * 0.3f * Margen;
	});
	const int32 Faltan = FMath::Min(AjusteMotas.Cantidad - ObtenerParticulasActivas(), FMath::Max(1, AjusteMotas.Cantidad / 20));
	for (int32 Indice = 0; Indice < Faltan; ++Indice)
	{
		const FVector Posicion = Centro + FVector(Aleatorio.FRandRange(-AjusteMotas.Radio, AjusteMotas.Radio),
			Aleatorio.FRandRange(-AjusteMotas.Radio, AjusteMotas.Radio), Aleatorio.FRandRange(-AjusteMotas.Altura * 0.3f, AjusteMotas.Altura));
		Emitir(Posicion, Aleatorio.GetUnitVector() * Aleatorio.FRandRange(0.f, AjusteMotas.Deriva));
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
