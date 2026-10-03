// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Camara/SAEVUSComponenteCamara.h"
#include "Materials/Material.h"

namespace SAEVUSPruebasEstilo
{
	const FVector PosicionPersonaje(0.f, 0.f, 150.f);
	constexpr float Tolerancia = 0.001f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEstiloCamara, "SAEVUS.Estilo.PostProcesoCamara", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEstiloCamara::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEstilo;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionPersonaje);
	USAEVUSComponenteCamara* Camara = Personaje->ObtenerCamara();

	USAEVUSAjustesCamara* Ajustes = NewObject<USAEVUSAjustesCamara>(GetTransientPackage());
	Ajustes->Camara.MaterialContorno = UMaterial::GetDefaultMaterial(MD_PostProcess);
	Ajustes->Camara.IntensidadVineta = 0.3f;
	Ajustes->Camara.Saturacion = 0.6f;
	Camara->FijarAjustesCamara(Ajustes);

	const FPostProcessSettings& Ajuste = Camara->PostProcessSettings;
	TestEqual(TEXT("Un blendable: el contorno"), Ajuste.WeightedBlendables.Array.Num(), 1);
	if (Ajuste.WeightedBlendables.Array.Num() == 1)
	{
		TestTrue(TEXT("El blendable es el material de contorno"), Ajuste.WeightedBlendables.Array[0].Object == Ajustes->Camara.MaterialContorno);
	}
	TestTrue(TEXT("Viñeta activada"), Ajuste.bOverride_VignetteIntensity != 0);
	TestEqual(TEXT("Intensidad de viñeta"), Ajuste.VignetteIntensity, 0.3f, Tolerancia);
	TestEqual(TEXT("Saturación"), static_cast<float>(Ajuste.ColorSaturation.X), 0.6f, Tolerancia);

	// Sin material: sin contorno, y reaplicar no acumula blendables.
	Ajustes->Camara.MaterialContorno = nullptr;
	Camara->FijarAjustesCamara(Ajustes);
	Camara->FijarAjustesCamara(Ajustes);
	TestEqual(TEXT("Sin material no hay contorno"), Camara->PostProcessSettings.WeightedBlendables.Array.Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
