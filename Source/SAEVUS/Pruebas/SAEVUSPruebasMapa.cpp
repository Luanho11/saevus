// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Interfaz/SAEVUSMapa.h"

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

#endif // WITH_DEV_AUTOMATION_TESTS
