// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSMapa.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interfaz/SAEVUSEstiloInterfaz.h"
#include "Tutorial/SAEVUSPuertaArena.h"
#include "Tutorial/SAEVUSPuntoControl.h"

namespace SAEVUSMapa
{
	const FName EtiquetaSinMapa(TEXT("SinMapa"));

	constexpr float Margen = 12.f;
	constexpr float TamanoJugador = 12.f;
	constexpr float TamanoPuntoControl = 8.f;
	// Grosor mínimo en píxeles: los muros finos siguen viéndose.
	constexpr float GrosorMinimo = 2.f;
	const FLinearColor ColorFondo(0.f, 0.f, 0.02f, 0.75f);
	const FLinearColor ColorBajo(0.1f, 0.11f, 0.16f, 0.95f);
	const FLinearColor ColorAlto(0.45f, 0.5f, 0.64f, 0.95f);
	const FLinearColor ColorPuerta(0.9f, 0.5f, 0.15f, 1.f);
	const FLinearColor ColorPuntoControl(0.45f, 0.85f, 1.f, 1.f);
	const FLinearColor ColorJugador(1.f, 1.f, 1.f, 1.f);

	TArray<FSAEVUSBloqueMapa> RecogerBloques(const UWorld* Mundo)
	{
		TArray<FSAEVUSBloqueMapa> Bloques;
		if (!Mundo)
		{
			return Bloques;
		}
		for (TActorIterator<AStaticMeshActor> Actor(Mundo); Actor; ++Actor)
		{
			const UStaticMeshComponent* Malla = Actor->GetStaticMeshComponent();
			if (!Malla || !Malla->IsCollisionEnabled() || Actor->ActorHasTag(EtiquetaSinMapa))
			{
				continue;
			}
			const FBox Caja = Actor->GetComponentsBoundingBox();
			FSAEVUSBloqueMapa Bloque;
			Bloque.Caja = FBox2D(FVector2D(Caja.Min), FVector2D(Caja.Max));
			Bloque.Altura = static_cast<float>(Caja.Max.Z);
			Bloques.Add(Bloque);
		}
		return Bloques;
	}

	FSAEVUSEncuadreMapa Encuadrar(const FBox2D& Limites, const FVector2D& Tamano, float MargenPixeles)
	{
		FSAEVUSEncuadreMapa Encuadre;
		const FVector2D Util = Tamano - FVector2D(2.f * MargenPixeles);
		const FVector2D Medida = Limites.GetSize();
		if (!Limites.bIsValid || Medida.X <= 0.0 || Medida.Y <= 0.0)
		{
			return Encuadre;
		}
		Encuadre.Escala = static_cast<float>(FMath::Min(Util.X / Medida.X, Util.Y / Medida.Y));
		Encuadre.Origen = Limites.Min;
		// Centrado: lo que sobra en el eje holgado se reparte a los dos lados.
		Encuadre.Desplazamiento = FVector2D(MargenPixeles) + (Util - Medida * Encuadre.Escala) * 0.5;
		return Encuadre;
	}
}

const FVector2D USAEVUSWidgetMapa::Tamano(560.f, 320.f);

TSharedRef<SWidget> USAEVUSWidgetMapa::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		Construir();
	}
	return Super::RebuildWidget();
}

void USAEVUSWidgetMapa::Construir()
{
	USizeBox* Caja = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Caja"));
	Caja->SetWidthOverride(Tamano.X);
	Caja->SetHeightOverride(Tamano.Y);
	WidgetTree->RootWidget = Caja;
	UBorder* Fondo = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Fondo"));
	Fondo->SetBrushColor(SAEVUSMapa::ColorFondo);
	Fondo->SetPadding(FMargin(0.f));
	Caja->SetContent(Fondo);
	Lienzo = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Lienzo"));
	Fondo->SetContent(Lienzo);
}

void USAEVUSWidgetMapa::AnadirRectangulo(const FVector2D& Posicion, const FVector2D& TamanoRectangulo, const FLinearColor& Color, bool bRedondo)
{
	UBorder* Rectangulo = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	if (bRedondo)
	{
		Rectangulo->SetBrush(SAEVUSEstilo::PincelRedondo(Color));
	}
	else
	{
		Rectangulo->SetBrushColor(Color);
	}
	UCanvasPanelSlot* Ranura = Lienzo->AddChildToCanvas(Rectangulo);
	Ranura->SetPosition(Posicion);
	Ranura->SetSize(TamanoRectangulo.ComponentMax(FVector2D(SAEVUSMapa::GrosorMinimo)));
}

void USAEVUSWidgetMapa::Refrescar()
{
	using namespace SAEVUSMapa;
	if (!Lienzo)
	{
		TakeWidget();
	}
	if (!Lienzo)
	{
		return;
	}
	Lienzo->ClearChildren();
	const UWorld* Mundo = GetWorld();
	const TArray<FSAEVUSBloqueMapa> Bloques = RecogerBloques(Mundo);
	if (Bloques.IsEmpty())
	{
		return;
	}
	FBox2D Limites(ForceInit);
	float AlturaMinima = TNumericLimits<float>::Max();
	float AlturaMaxima = TNumericLimits<float>::Lowest();
	for (const FSAEVUSBloqueMapa& Bloque : Bloques)
	{
		Limites += Bloque.Caja;
		AlturaMinima = FMath::Min(AlturaMinima, Bloque.Altura);
		AlturaMaxima = FMath::Max(AlturaMaxima, Bloque.Altura);
	}
	const FSAEVUSEncuadreMapa Encuadre = Encuadrar(Limites, Tamano, Margen);

	// Los bajos primero: los altos (muros, plataformas) quedan encima.
	TArray<FSAEVUSBloqueMapa> Ordenados = Bloques;
	Ordenados.Sort([](const FSAEVUSBloqueMapa& A, const FSAEVUSBloqueMapa& B) { return A.Altura < B.Altura; });
	const float RangoAltura = FMath::Max(AlturaMaxima - AlturaMinima, 1.f);
	for (const FSAEVUSBloqueMapa& Bloque : Ordenados)
	{
		const FVector2D Esquina = Encuadre.APixel(Bloque.Caja.Min);
		const FLinearColor Color = FMath::Lerp(ColorBajo, ColorAlto, (Bloque.Altura - AlturaMinima) / RangoAltura);
		AnadirRectangulo(Esquina, Bloque.Caja.GetSize() * Encuadre.Escala, Color);
	}
	for (TActorIterator<ASAEVUSPuertaArena> Puerta(Mundo); Puerta; ++Puerta)
	{
		if (!Puerta->EstaAbierta())
		{
			const FBox Caja = Puerta->GetComponentsBoundingBox();
			AnadirRectangulo(Encuadre.APixel(FVector2D(Caja.Min)), FVector2D(Caja.GetSize()) * Encuadre.Escala, ColorPuerta);
		}
	}
	for (TActorIterator<ASAEVUSPuntoControl> Punto(Mundo); Punto; ++Punto)
	{
		const FVector2D Centro = Encuadre.APixel(FVector2D(Punto->GetActorLocation()));
		AnadirRectangulo(Centro - FVector2D(TamanoPuntoControl * 0.5f), FVector2D(TamanoPuntoControl), ColorPuntoControl);
	}
	if (const APawn* Jugador = GetOwningPlayerPawn())
	{
		const FVector2D Centro = Encuadre.APixel(FVector2D(Jugador->GetActorLocation()));
		AnadirRectangulo(Centro - FVector2D(TamanoJugador * 0.5f), FVector2D(TamanoJugador), ColorJugador, true);
	}
}
