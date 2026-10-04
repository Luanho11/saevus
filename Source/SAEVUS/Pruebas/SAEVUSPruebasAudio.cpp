// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Audio/SAEVUSComponenteSonido.h"
#include "Audio/SAEVUSComponenteSonidoEnemigo.h"
#include "Audio/SAEVUSSintesis.h"
#include "Audio/SAEVUSSubsistemaSonidoMundo.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"
#include "Mundo/SAEVUSPalanca.h"
#include "Tutorial/SAEVUSPuertaArena.h"

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
	for (const FSAEVUSSonidoSintetizado* Sonido : { &Sonidos.Tajo, &Sonidos.Golpe, &Sonidos.Disparo, &Sonidos.Danio, &Sonidos.Curacion, &Sonidos.Bloqueo, &Sonidos.Contraataque, &Sonidos.Dash, &Sonidos.DobleSalto })
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAudioMundo, "SAEVUS.Audio.SonidosMundo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAudioMundo::RunTest(const FString& Parameters)
{
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	const USAEVUSSubsistemaSonidoMundo* Sonidos = MundoPrueba.ObtenerMundo()->GetSubsystem<USAEVUSSubsistemaSonidoMundo>();
	if (!TestNotNull(TEXT("Subsistema de sonidos del mundo"), Sonidos))
	{
		return false;
	}
	ASAEVUSPuertaArena* Puerta = MundoPrueba.CrearActor<ASAEVUSPuertaArena>(FVector(300.f, 0.f, 100.f));
	ASAEVUSPalanca* Palanca = MundoPrueba.CrearActor<ASAEVUSPalanca>(FVector(0.f, 0.f, 80.f));
	Puerta->Abrir();
	TestEqual(TEXT("La puerta suena al abrirse"), Sonidos->ObtenerReproducciones(), 1);
	Palanca->Accionar();
	Palanca->Accionar();
	TestEqual(TEXT("La palanca suena una vez al accionarse"), Sonidos->ObtenerReproducciones(), 2);
	TestFalse(TEXT("Cada sonido del mundo se sintetiza"), SAEVUSSintesis::SintetizarBytes(FSAEVUSAjusteSonidosMundo().Palanca).IsEmpty()
		|| SAEVUSSintesis::SintetizarBytes(FSAEVUSAjusteSonidosMundo().Puerta).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAudioAleteo, "SAEVUS.Audio.AleteoVolador", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAudioAleteo::RunTest(const FString& Parameters)
{
	constexpr float AlturaVuelo = 300.f;
	constexpr float DistanciaCerca = 400.f;
	constexpr float DistanciaLejos = 5000.f;
	constexpr float TiempoEscucha = 1.f;
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	// Con suelo: el jugador se queda quieto y la distancia al volador es la de la prueba.
	MundoPrueba.CrearBloque(FVector(0.f, 0.f, -50.f), FVector(12000.f, 2000.f, 100.f));
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(FVector(0.f, 0.f, 150.f));
	USAEVUSDatosEnemigo* Datos = NewObject<USAEVUSDatosEnemigo>(GetTransientPackage());
	Datos->Enemigo.bVolador = true;
	Datos->Enemigo.RadioPatrulla = 0.f;
	Datos->Enemigo.Ataques.Reset();
	auto Escuchar = [&](float Distancia)
	{
		ASAEVUSEnemigo* Enemigo = MundoPrueba.CrearEnemigo(FVector(Distancia, 0.f, AlturaVuelo), Datos, Jugador);
		const USAEVUSComponenteSonidoEnemigo* Sonido = Enemigo->FindComponentByClass<USAEVUSComponenteSonidoEnemigo>();
		const int32 Antes = Sonido->ObtenerReproducciones();
		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoEscucha); ++Indice)
		{
			MundoPrueba.Paso();
		}
		return Sonido->ObtenerReproducciones() - Antes;
	};
	const int32 Lejos = Escuchar(DistanciaLejos);
	TestEqual(TEXT("Lejos no se oye aletear"), Lejos, 0);
	const int32 Cerca = Escuchar(DistanciaCerca);
	AddInfo(FString::Printf(TEXT("Aleteos en %.1f s cerca del jugador: %d"), TiempoEscucha, Cerca));
	TestTrue(TEXT("Cerca aletea al ritmo del cuerpo"), Cerca >= FMath::FloorToInt32(Datos->Enemigo.FrecuenciaAleteo * TiempoEscucha) - 1);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
