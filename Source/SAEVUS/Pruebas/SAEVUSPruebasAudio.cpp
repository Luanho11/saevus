// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Audio/SAEVUSComponenteSonido.h"
#include "Audio/SAEVUSSintesis.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"

namespace SAEVUSPruebasAudio
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionJugador(0.f, 0.f, 150.f);
	const FVector PosicionMuneco(150.f, 0.f, 96.f);
	constexpr float TiempoAtaque = 1.f;
	constexpr float MaximoInt16 = 32767.f;

	float ValorEficaz(const TArray<int16>& Muestras, int32 Desde, int32 Hasta)
	{
		double Suma = 0.0;
		for (int32 Indice = Desde; Indice < Hasta; ++Indice)
		{
			Suma += FMath::Square(static_cast<double>(Muestras[Indice]) / MaximoInt16);
		}
		return static_cast<float>(FMath::Sqrt(Suma / FMath::Max(1, Hasta - Desde)));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAudioSintesis, "SAEVUS.Audio.Sintesis", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAudioSintesis::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAudio;

	const FSAEVUSAjusteSonidos Sonidos;
	for (const FSAEVUSSonidoSintetizado* Sonido : { &Sonidos.Tajo, &Sonidos.Golpe, &Sonidos.Disparo, &Sonidos.Danio, &Sonidos.Curacion, &Sonidos.Bloqueo, &Sonidos.Dash, &Sonidos.DobleSalto })
	{
		const TArray<int16> Muestras = SAEVUSSintesis::Sintetizar(*Sonido);
		TestEqual(TEXT("Muestras = duración × frecuencia de muestreo"), Muestras.Num(), FMath::RoundToInt32(Sonido->Duracion * SAEVUSSintesis::FrecuenciaMuestreo));
		int32 Pico = 0;
		for (const int16 Muestra : Muestras)
		{
			Pico = FMath::Max(Pico, FMath::Abs(static_cast<int32>(Muestra)));
		}
		TestTrue(TEXT("Suena (no es silencio)"), Pico > 0);
		TestTrue(TEXT("No pasa del volumen"), Pico <= FMath::CeilToInt32(Sonido->Volumen * MaximoInt16) + 1);
		const int32 Quinto = Muestras.Num() / 5;
		TestTrue(TEXT("La envolvente se apaga: el final suena menos que el principio"),
			ValorEficaz(Muestras, Muestras.Num() - Quinto, Muestras.Num()) < ValorEficaz(Muestras, 0, Quinto));
	}
	// Determinista: la misma semilla da el mismo sonido.
	TestTrue(TEXT("Mismo sonido, mismas muestras"), SAEVUSSintesis::Sintetizar(Sonidos.Golpe) == SAEVUSSintesis::Sintetizar(Sonidos.Golpe));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAudioEventos, "SAEVUS.Audio.Eventos", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAudioEventos::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAudio;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
	if (!TestTrue(TEXT("Asentado"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	const USAEVUSComponenteSonido* Sonido = Jugador->FindComponentByClass<USAEVUSComponenteSonido>();
	if (!TestNotNull(TEXT("El personaje tiene sonido"), Sonido))
	{
		return false;
	}
	MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(PosicionMuneco);
	Jugador->PedirAtaque();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoAtaque); ++Indice)
	{
		MundoPrueba.Paso();
	}
	TestEqual(TEXT("Un tajo que acierta: barrido y golpe"), Sonido->ObtenerReproducciones(), 2);
	Jugador->PedirDash();
	MundoPrueba.Paso();
	TestEqual(TEXT("El dash suena"), Sonido->ObtenerReproducciones(), 3);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
