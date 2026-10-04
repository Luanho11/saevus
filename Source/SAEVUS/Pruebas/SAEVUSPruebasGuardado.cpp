// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Kismet/GameplayStatics.h"
#include "Nucleo/SAEVUSGuardado.h"

namespace SAEVUSPruebasGuardado
{
	const TCHAR* const RanuraPrueba = TEXT("SAEVUS_Prueba");
	const FName MapaPrueba(TEXT("L_Tutorial"));
	const FTransform PuntoPrueba(FRotator(0.f, 90.f, 0.f), FVector(15100.f, 0.f, 100.f));
	constexpr float SensibilidadPrueba = 1.5f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGuardadoIdaYVuelta, "SAEVUS.Guardado.IdaYVuelta", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGuardadoIdaYVuelta::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasGuardado;

	UGameplayStatics::DeleteGameInSlot(RanuraPrueba, 0);
	const USAEVUSPartidaGuardada* Nueva = SAEVUSGuardado::Cargar(RanuraPrueba);
	if (!TestNotNull(TEXT("Sin archivo: partida nueva"), Nueva))
	{
		return false;
	}
	TestEqual(TEXT("Sensibilidad por defecto"), Nueva->Sensibilidad, 1.f);
	TestFalse(TEXT("Sin punto de control"), Nueva->bTienePuntoControl);

	USAEVUSPartidaGuardada* Partida = SAEVUSGuardado::Cargar(RanuraPrueba);
	Partida->Sensibilidad = SensibilidadPrueba;
	Partida->bTienePuntoControl = true;
	Partida->MapaPuntoControl = MapaPrueba;
	Partida->PuntoControl = PuntoPrueba;
	TestTrue(TEXT("Se guarda"), SAEVUSGuardado::Guardar(Partida, RanuraPrueba));

	const USAEVUSPartidaGuardada* Leida = SAEVUSGuardado::Cargar(RanuraPrueba);
	TestTrue(TEXT("Es otra instancia, leída del disco"), Leida != Partida);
	TestEqual(TEXT("Sensibilidad guardada"), Leida->Sensibilidad, SensibilidadPrueba);
	TestTrue(TEXT("Con punto de control"), Leida->bTienePuntoControl);
	TestEqual(TEXT("Del mapa"), Leida->MapaPuntoControl, MapaPrueba);
	TestTrue(TEXT("En su sitio"), Leida->PuntoControl.Equals(PuntoPrueba));
	UGameplayStatics::DeleteGameInSlot(RanuraPrueba, 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
