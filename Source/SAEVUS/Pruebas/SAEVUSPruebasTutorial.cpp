// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSComponenteVida.h"
#include "Tutorial/SAEVUSPuertaArena.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"
#include "Tutorial/SAEVUSVolumenCaida.h"
#include "Tutorial/SAEVUSVolumenTutorial.h"

namespace SAEVUSPruebasTutorial
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(6000.f, 6000.f, 100.f);
	const FVector PosicionJugador(0.f, 0.f, 150.f);
	const FVector PosicionPuerta(600.f, 0.f, 200.f);
	constexpr float DanioLetal = 100000.f;
	constexpr float TiempoAbrir = 2.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTutorialAviso, "SAEVUS.Tutorial.Aviso", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTutorialAviso::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasTutorial;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
	USAEVUSSubsistemaTutorial* Tutorial = MundoPrueba.ObtenerMundo()->GetSubsystem<USAEVUSSubsistemaTutorial>();
	if (!TestNotNull(TEXT("Subsistema de tutorial"), Tutorial))
	{
		return false;
	}
	TestNull(TEXT("Sin avisos al empezar"), Tutorial->ObtenerMensajeActivo());

	ASAEVUSVolumenTutorial* Volumen = MundoPrueba.CrearActor<ASAEVUSVolumenTutorial>(PosicionJugador);
	Volumen->NotificarEntrada(Jugador);
	TestNotNull(TEXT("Entrar muestra el aviso"), Tutorial->ObtenerMensajeActivo());
	Volumen->NotificarEntrada(Jugador);
	TestEqual(TEXT("Solo la primera vez"), Tutorial->ObtenerMensajesMostrados(), 1);

	// Caduca tras su duración.
	Tutorial->MostrarMensaje(FText::FromString(TEXT("Título")), FText::FromString(TEXT("Texto")), 0.5f);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(1.f); ++Indice)
	{
		MundoPrueba.Paso();
	}
	TestNull(TEXT("El aviso caduca"), Tutorial->ObtenerMensajeActivo());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTutorialPuerta, "SAEVUS.Tutorial.PuertaArena", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTutorialPuerta::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasTutorial;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
	ASAEVUSEnemigo* Primero = MundoPrueba.CrearEnemigo(FVector(2000.f, 0.f, 100.f), nullptr, Jugador);
	ASAEVUSEnemigo* Segundo = MundoPrueba.CrearEnemigo(FVector(2000.f, 500.f, 100.f), nullptr, Jugador);
	ASAEVUSEnemigo* Ajeno = MundoPrueba.CrearEnemigo(FVector(2000.f, -500.f, 100.f), nullptr, Jugador);
	ASAEVUSPuertaArena* Puerta = MundoPrueba.CrearActor<ASAEVUSPuertaArena>(PosicionPuerta);
	Puerta->FijarEnemigos({ Primero, Segundo });

	Ajeno->ObtenerVida()->AplicarDanio(DanioLetal, Jugador);
	TestFalse(TEXT("Un enemigo de otra arena no la abre"), Puerta->EstaAbierta());
	Primero->ObtenerVida()->AplicarDanio(DanioLetal, Jugador);
	TestFalse(TEXT("Con un enemigo vivo sigue cerrada"), Puerta->EstaAbierta());
	Segundo->ObtenerVida()->AplicarDanio(DanioLetal, Jugador);
	TestTrue(TEXT("Sin enemigos se abre"), Puerta->EstaAbierta());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTutorialCaida, "SAEVUS.Tutorial.VolumenCaida", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTutorialCaida::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasTutorial;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
	if (!TestTrue(TEXT("Asentado"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	const FVector Retorno = Jugador->GetActorLocation();
	const float VidaInicial = Jugador->ObtenerVida()->ObtenerVida();

	// El volumen en otra parte; su punto de retorno es donde estaba el jugador.
	const FVector PosicionVolumen(2000.f, 0.f, -500.f);
	ASAEVUSVolumenCaida* Caida = MundoPrueba.CrearActor<ASAEVUSVolumenCaida>(PosicionVolumen);
	Caida->FijarPuntoRetorno(Retorno);

	Jugador->SetActorLocation(PosicionVolumen);
	Caida->NotificarEntrada(Jugador);
	TestTrue(TEXT("Vuelve al punto de retorno"), Jugador->GetActorLocation().Equals(Retorno, 1.f));
	TestTrue(TEXT("Caer quita vida"), Jugador->ObtenerVida()->ObtenerVida() < VidaInicial);
	TestFalse(TEXT("Sin morir"), Jugador->ObtenerVida()->EstaMuerto());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
