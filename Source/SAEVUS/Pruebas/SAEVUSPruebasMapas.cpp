// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameFramework/PlayerStart.h"
#include "Nucleo/SAEVUSModoJuego.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaInicioJugador, "SAEVUS.Mundo.InicioJugador", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaInicioJugador::RunTest(const FString& Parameters)
{
	const FName EtiquetaLlegada(TEXT("DesdeZona1"));
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	// La llegada primero: el inicio normal no puede depender del orden.
	APlayerStart* Llegada = MundoPrueba.CrearActor<APlayerStart>(FVector(2000.f, 0.f, 100.f));
	Llegada->PlayerStartTag = EtiquetaLlegada;
	APlayerStart* Inicio = MundoPrueba.CrearActor<APlayerStart>(FVector(0.f, 0.f, 100.f));
	ASAEVUSModoJuego* ModoJuego = MundoPrueba.CrearActor<ASAEVUSModoJuego>(FVector::ZeroVector);
	if (!TestNotNull(TEXT("Modo de juego"), ModoJuego))
	{
		return false;
	}
	TestTrue(TEXT("Sin paso, el inicio sin etiqueta"), ModoJuego->ChoosePlayerStart(nullptr) == Inicio);
	TestTrue(TEXT("Por un paso, el inicio con su etiqueta"), ModoJuego->FindPlayerStart(nullptr, EtiquetaLlegada.ToString()) == Llegada);
	TestFalse(TEXT("Sin opción de paso en la URL"), ModoJuego->LlegoPorPaso());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
