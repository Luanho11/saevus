// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Mundo/SAEVUSPlataformaMovil.h"

namespace SAEVUSPruebasPlataforma
{
	const FVector PosicionPlataforma(0.f, 0.f, -50.f);
	const FVector Recorrido(400.f, 0.f, 0.f);
	constexpr float Velocidad = 200.f;
	constexpr float Pausa = 0.5f;
	constexpr float Tolerancia = 1.f;
	constexpr float ToleranciaJugador = 40.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaPlataformaMovil, "SAEVUS.Mundo.PlataformaMovil", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaPlataformaMovil::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasPlataforma;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPlataformaMovil* Plataforma = MundoPrueba.CrearActor<ASAEVUSPlataformaMovil>(PosicionPlataforma);
	Plataforma->SetActorScale3D(FVector(3.f, 3.f, 1.f));
	Plataforma->Configurar(Recorrido, Velocidad, Pausa);
	const float Viaje = static_cast<float>(Recorrido.Size()) / Velocidad;
	TestTrue(TEXT("Empieza quieta en su sitio"), Plataforma->PosicionEn(Pausa * 0.5f).Equals(PosicionPlataforma, Tolerancia));
	TestTrue(TEXT("A mitad del viaje, a mitad del recorrido"), Plataforma->PosicionEn(Pausa + Viaje * 0.5f).Equals(PosicionPlataforma + Recorrido * 0.5f, Tolerancia));
	TestTrue(TEXT("Espera en el otro extremo"), Plataforma->PosicionEn(Pausa + Viaje + Pausa * 0.5f).Equals(PosicionPlataforma + Recorrido, Tolerancia));
	TestTrue(TEXT("Y vuelve"), Plataforma->PosicionEn(2.f * (Pausa + Viaje)).Equals(PosicionPlataforma, Tolerancia));

	// Un jugador de pie encima viaja con ella.
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionPlataforma + FVector(0.f, 0.f, 150.f));
	if (!TestTrue(TEXT("Jugador de pie en la plataforma"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	const float XInicial = static_cast<float>(Jugador->GetActorLocation().X);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Pausa + Viaje); ++Indice)
	{
		Plataforma->Tick(SAEVUSPruebas::PasoSimulacion);
		MundoPrueba.Paso();
	}
	const float Avance = static_cast<float>(Jugador->GetActorLocation().X) - XInicial;
	AddInfo(FString::Printf(TEXT("El jugador avanza %.0f cm con la plataforma (recorrido %.0f)"), Avance, Recorrido.X));
	TestEqual(TEXT("Viaja con la plataforma"), Avance, static_cast<float>(Recorrido.X), ToleranciaJugador);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
