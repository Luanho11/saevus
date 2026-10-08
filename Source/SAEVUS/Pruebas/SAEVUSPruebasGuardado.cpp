// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Nucleo/SAEVUSGuardado.h"

namespace SAEVUSPruebasGuardado
{
	const TCHAR* const RanuraPrueba = TEXT("SAEVUS_Prueba");
	const FName MapaPrueba(TEXT("L_Tutorial"));
	const FTransform PuntoPrueba(FRotator(0.f, 90.f, 0.f), FVector(15100.f, 0.f, 100.f));
	constexpr float SensibilidadPrueba = 1.5f;
	constexpr float VolumenPrueba = 0.5f;
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
	TestEqual(TEXT("Volumen por defecto"), Nueva->Volumen, 1.f);
	TestFalse(TEXT("Sin punto de control"), Nueva->bTienePuntoControl);

	USAEVUSPartidaGuardada* Partida = SAEVUSGuardado::Cargar(RanuraPrueba);
	Partida->Sensibilidad = SensibilidadPrueba;
	Partida->Volumen = VolumenPrueba;
	Partida->bTienePuntoControl = true;
	Partida->MapaPuntoControl = MapaPrueba;
	Partida->PuntoControl = PuntoPrueba;
	TestTrue(TEXT("Se guarda"), SAEVUSGuardado::Guardar(Partida, RanuraPrueba));

	const USAEVUSPartidaGuardada* Leida = SAEVUSGuardado::Cargar(RanuraPrueba);
	TestTrue(TEXT("Es otra instancia, leída del disco"), Leida != Partida);
	TestEqual(TEXT("Sensibilidad guardada"), Leida->Sensibilidad, SensibilidadPrueba);
	TestEqual(TEXT("Volumen guardado"), Leida->Volumen, VolumenPrueba);
	TestTrue(TEXT("Con punto de control"), Leida->bTienePuntoControl);
	TestEqual(TEXT("Del mapa"), Leida->MapaPuntoControl, MapaPrueba);
	TestTrue(TEXT("En su sitio"), Leida->PuntoControl.Equals(PuntoPrueba));
	UGameplayStatics::DeleteGameInSlot(RanuraPrueba, 0);
	return true;
}

// GAMEPLAY-2 y GAMEPLAY-7: el estado del mundo se guarda por mapa, se puede olvidar por mapa, y una partida ilegible
// se recupera de la copia; las partidas antiguas se migran.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGuardadoEstadoMundo, "SAEVUS.Guardado.EstadoMundo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGuardadoEstadoMundo::RunTest(const FString& Parameters)
{
	USAEVUSPartidaGuardada* Partida = NewObject<USAEVUSPartidaGuardada>(GetTransientPackage());
	const FName Tutorial(TEXT("L_Tutorial"));
	const FName Mazmorra(TEXT("L_Mazmorra"));
	Partida->EstadoMundo.Add(SAEVUSGuardado::ClaveMundo(Tutorial, TEXT("Puerta_1")));
	Partida->EstadoMundo.Add(SAEVUSGuardado::ClaveMundo(Tutorial, TEXT("Palanca_2")));
	Partida->EstadoMundo.Add(SAEVUSGuardado::ClaveMundo(Mazmorra, TEXT("Puerta_1")));
	TestFalse(TEXT("Cada mapa tiene sus claves"), SAEVUSGuardado::ClaveMundo(Tutorial, TEXT("Puerta_1")) == SAEVUSGuardado::ClaveMundo(Mazmorra, TEXT("Puerta_1")));

	TestTrue(TEXT("Olvidar un mapa quita lo suyo"), SAEVUSGuardado::QuitarEstadoMapa(Partida, Tutorial));
	TestEqual(TEXT("y deja lo del otro"), Partida->EstadoMundo.Num(), 1);
	TestFalse(TEXT("Olvidar un mapa sin estado no hace nada"), SAEVUSGuardado::QuitarEstadoMapa(Partida, Tutorial));

	// Migración: una partida de la versión 0 pierde los fragmentos sin identificador y sube de versión.
	USAEVUSPartidaGuardada* Antigua = NewObject<USAEVUSPartidaGuardada>(GetTransientPackage());
	Antigua->Version = 0;
	Antigua->FragmentosMascara = { FName(TEXT("L_Tutorial.Fragmento_1")), NAME_None };
	Antigua->Sensibilidad = 99.f;
	SAEVUSGuardado::Migrar(Antigua);
	TestEqual(TEXT("Sube a la versión actual"), Antigua->Version, SAEVUSGuardado::VersionActual);
	TestEqual(TEXT("Descarta el fragmento sin nombre"), Antigua->FragmentosMascara.Num(), 1);
	TestTrue(TEXT("Acota la sensibilidad"), Antigua->Sensibilidad <= 5.f);

	// Si la partida principal no se lee, se usa la copia de seguridad (GAMEPLAY-7).
	const FString Ranura = TEXT("SAEVUS_PruebaCopia");
	const FString RanuraCopia = Ranura + SAEVUSGuardado::SufijoCopia;
	UGameplayStatics::DeleteGameInSlot(Ranura, 0);
	UGameplayStatics::DeleteGameInSlot(RanuraCopia, 0);
	USAEVUSPartidaGuardada* Buena = NewObject<USAEVUSPartidaGuardada>(GetTransientPackage());
	Buena->Version = SAEVUSGuardado::VersionActual;
	Buena->Volumen = 0.25f;
	TestTrue(TEXT("Guarda la copia"), SAEVUSGuardado::Guardar(Buena, RanuraCopia));
	// La principal, con bytes basura.
	const FString RutaPrincipal = FPaths::ProjectSavedDir() / TEXT("SaveGames") / (Ranura + TEXT(".sav"));
	const TArray<uint8> Basura = { 1, 2, 3, 4, 5, 6, 7, 8 };
	FFileHelper::SaveArrayToFile(Basura, *RutaPrincipal);
	const USAEVUSPartidaGuardada* Cargada = SAEVUSGuardado::Cargar(Ranura);
	TestTrue(TEXT("Se carga la copia"), Cargada && FMath::IsNearlyEqual(Cargada->Volumen, 0.25f));
	UGameplayStatics::DeleteGameInSlot(Ranura, 0);
	UGameplayStatics::DeleteGameInSlot(RanuraCopia, 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
