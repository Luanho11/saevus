// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace SAEVUSPruebasPolvo
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	// Desde 5 m de alto la caída pasa de largo la velocidad mínima del polvo.
	const FVector PosicionAlta(0.f, 0.f, 500.f);
	constexpr float TiempoMaxCaida = 3.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaPolvoAterrizaje, "SAEVUS.Efectos.PolvoAterrizaje", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaPolvoAterrizaje::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasPolvo;
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionAlta);
	USAEVUSComponentePolvoMovimiento* Polvo = Personaje->ObtenerPolvoMovimiento();
	if (!TestNotNull(TEXT("El personaje tiene polvo de movimiento"), Polvo))
	{
		return false;
	}
	TestEqual(TEXT("Cayendo, sin polvo"), Polvo->ObtenerParticulasActivas(), 0);
	if (!TestTrue(TEXT("Aterriza"), MundoPrueba.AsentarEnSuelo(TiempoMaxCaida)))
	{
		return false;
	}
	MundoPrueba.Paso();
	TestTrue(TEXT("Al aterrizar de una caída larga se levanta polvo"), Polvo->ObtenerParticulasActivas() > 0);

	// Bajar un escalón (caída lenta) no levanta polvo.
	SAEVUSPruebas::FMundoPrueba OtroMundo;
	OtroMundo.CrearBloque(CentroSuelo, TamanoSuelo);
	USAEVUSComponentePolvoMovimiento* PolvoSuave = OtroMundo.CrearPersonaje(FVector(0.f, 0.f, 150.f))->ObtenerPolvoMovimiento();
	PolvoSuave->EmitirAterrizaje(100.f);
	TestEqual(TEXT("Una caída lenta no levanta polvo"), PolvoSuave->ObtenerParticulasActivas(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
