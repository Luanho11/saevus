// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Interfaz/SAEVUSMapa.h"
#include "Mundo/SAEVUSExploracion.h"

namespace SAEVUSPruebasMapa
{
	const FVector2D TamanoMapa(560.f, 320.f);
	constexpr float Margen = 10.f;
	constexpr double Tolerancia = 0.01;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaMapa, "SAEVUS.Interfaz.Mapa", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaMapa::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasMapa;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	// Suelo, un muro alto y el fondo de un foso (marcado: no sale en el mapa).
	MundoPrueba.CrearBloque(FVector(1000.f, 0.f, -50.f), FVector(2000.f, 800.f, 100.f));
	MundoPrueba.CrearBloque(FVector(1500.f, 0.f, 200.f), FVector(200.f, 800.f, 400.f));
	AStaticMeshActor* Fondo = MundoPrueba.CrearBloque(FVector(3000.f, 0.f, -750.f), FVector(800.f, 1600.f, 100.f));
	Fondo->Tags.Add(SAEVUSMapa::EtiquetaSinMapa);

	const TArray<FSAEVUSBloqueMapa> Bloques = SAEVUSMapa::RecogerBloques(MundoPrueba.ObtenerMundo());
	if (!TestEqual(TEXT("Dos bloques: el fondo del foso no cuenta"), Bloques.Num(), 2))
	{
		return false;
	}
	FBox2D Limites(ForceInit);
	for (const FSAEVUSBloqueMapa& Bloque : Bloques)
	{
		Limites += Bloque.Caja;
	}
	TestTrue(TEXT("El muro es más alto que el suelo"), FMath::Max(Bloques[0].Altura, Bloques[1].Altura) > FMath::Min(Bloques[0].Altura, Bloques[1].Altura));

	// 2000 x 800 cm en 560 x 320 px con margen: manda el ancho, sin deformar, centrado en alto.
	const FSAEVUSEncuadreMapa Encuadre = SAEVUSMapa::Encuadrar(Limites, TamanoMapa, Margen);
	const FVector2D Min = Encuadre.APixel(Limites.Min);
	const FVector2D Max = Encuadre.APixel(Limites.Max);
	TestEqual(TEXT("Empieza en el margen izquierdo"), Min.X, static_cast<double>(Margen), Tolerancia);
	TestEqual(TEXT("Acaba en el margen derecho"), Max.X, TamanoMapa.X - Margen, Tolerancia);
	TestEqual(TEXT("Misma escala en los dos ejes"), (Max.Y - Min.Y) / (Max.X - Min.X), Limites.GetSize().Y / Limites.GetSize().X, Tolerancia);
	TestEqual(TEXT("Centrado en alto"), (Min.Y + Max.Y) * 0.5, TamanoMapa.Y * 0.5, Tolerancia);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaMapaExplorado, "SAEVUS.Interfaz.MapaExplorado", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaMapaExplorado::RunTest(const FString& Parameters)
{
	using namespace SAEVUSExploracion;
	constexpr double Tolerancia = 0.01;

	TestEqual(TEXT("Celda del origen"), Celda(FVector(10.f, 10.f, 0.f)), FIntPoint(0, 0));
	TestEqual(TEXT("Celda con coordenadas negativas"), Celda(FVector(-10.f, TamanoCelda + 10.f, 0.f)), FIntPoint(-1, 1));

	// Un suelo largo (tres celdas) del que solo se han visitado las dos primeras.
	FSAEVUSBloqueMapa Suelo;
	Suelo.Caja = FBox2D(FVector2D(0.f, 100.f), FVector2D(3.f * TamanoCelda, 500.f));
	Suelo.Altura = 0.f;
	FSAEVUSBloqueMapa Lejano;
	Lejano.Caja = FBox2D(FVector2D(10.f * TamanoCelda, 100.f), FVector2D(11.f * TamanoCelda, 500.f));
	const TSet<FIntPoint> Visitadas = { FIntPoint(0, 0), FIntPoint(1, 0) };
	const TArray<FSAEVUSBloqueMapa> Trozos = RecortarBloques({ Suelo, Lejano }, Visitadas);
	if (!TestEqual(TEXT("Un trozo por celda visitada del suelo; el bloque lejano no sale"), Trozos.Num(), 2))
	{
		return false;
	}
	double Area = 0.0;
	for (const FSAEVUSBloqueMapa& Trozo : Trozos)
	{
		Area += Trozo.Caja.GetArea();
		TestTrue(TEXT("Cada trozo dentro del suelo"), Suelo.Caja.IsInside(Trozo.Caja.GetCenter()));
	}
	TestEqual(TEXT("Se ve el suelo de dos celdas"), Area, 2.0 * TamanoCelda * 400.0, Tolerancia);

	// El subsistema descubre un cuadrado alrededor del jugador y no repite.
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	USAEVUSSubsistemaExploracion* Exploracion = MundoPrueba.ObtenerMundo()->GetSubsystem<USAEVUSSubsistemaExploracion>();
	if (!TestNotNull(TEXT("Subsistema de exploración"), Exploracion))
	{
		return false;
	}
	TestTrue(TEXT("La primera vez descubre"), Exploracion->Descubrir(FVector(100.f, 100.f, 0.f)));
	const int32 Lado = 2 * RadioDescubrir + 1;
	TestEqual(TEXT("Un cuadrado de celdas"), Exploracion->ObtenerCeldas().Num(), Lado * Lado);
	TestFalse(TEXT("En el mismo sitio no hay nada nuevo"), Exploracion->Descubrir(FVector(150.f, 120.f, 0.f)));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
