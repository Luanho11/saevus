// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSTiposCombate.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"

namespace SAEVUSPruebasTajos
{
	// Suelo de 40 m x 40 m, superficie en Z = 0.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);
	constexpr float MediaAlturaMuneco = 96.f;

	// Muñeco flotando sobre la cabeza del personaje: solo el tajo hacia arriba lo alcanza.
	const FVector PosicionMunecoAlto(30.f, 0.f, 96.f + 190.f);
	// Muñeco delante, al alcance del tajo frontal.
	const FVector PosicionMunecoDelante(150.f, 0.f, MediaAlturaMuneco);

	constexpr float TiempoMax = 2.f;

	// Inclinaciones de cámara de prueba (grados).
	constexpr float InclinacionHorizontal = 0.f;
	constexpr float InclinacionArribaClara = 50.f;
	constexpr float InclinacionAbajoClara = -60.f;

	struct FMundoTajos
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = nullptr;
		USAEVUSComponenteMovimiento* Movimiento = nullptr;
		USAEVUSComponenteCombate* Combate = nullptr;

		explicit FMundoTajos(const FVector& Posicion)
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Personaje = MundoPrueba.CrearPersonaje(Posicion);
			Movimiento = Personaje->ObtenerMovimiento();
			Combate = Personaje->ObtenerCombate();
		}

		/** Golpes que recibe el muñeco durante un ataque completo en Direccion. */
		int32 GolpesConAtaque(ESAEVUSDireccionAtaque Direccion, const ASAEVUSMunecoEntrenamiento* Muneco)
		{
			const int32 Antes = Muneco->ObtenerGolpes();
			Combate->PedirAtaqueEnDireccion(Direccion);
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Combate->EstaAtacando(); ++Indice)
			{
				MundoPrueba.Paso();
			}
			return Muneco->ObtenerGolpes() - Antes;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTajosInclinacion, "SAEVUS.Combate.Tajos.Inclinacion", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTajosInclinacion::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasTajos;

	FMundoTajos Mundo(PosicionDePie);
	TestEqual(TEXT("Cámara horizontal: tajo frontal"), Mundo.Combate->DireccionSegunInclinacion(InclinacionHorizontal), ESAEVUSDireccionAtaque::Frontal);
	TestEqual(TEXT("Cámara hacia arriba: tajo hacia arriba"), Mundo.Combate->DireccionSegunInclinacion(InclinacionArribaClara), ESAEVUSDireccionAtaque::Arriba);
	// Sin tajo hacia abajo (como en GunZ): mirar abajo da el tajo normal.
	TestEqual(TEXT("Cámara hacia abajo: tajo frontal"), Mundo.Combate->DireccionSegunInclinacion(InclinacionAbajoClara), ESAEVUSDireccionAtaque::Frontal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTajoArriba, "SAEVUS.Combate.Tajos.Arriba", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTajoArriba::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasTajos;

	FMundoTajos Mundo(PosicionDePie);
	if (!TestTrue(TEXT("Asentado"), Mundo.MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	const ASAEVUSMunecoEntrenamiento* Muneco = Mundo.MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(PosicionMunecoAlto);
	TestEqual(TEXT("El tajo frontal no alcanza lo que está encima"), Mundo.GolpesConAtaque(ESAEVUSDireccionAtaque::Frontal, Muneco), 0);
	TestEqual(TEXT("El tajo hacia arriba sí"), Mundo.GolpesConAtaque(ESAEVUSDireccionAtaque::Arriba, Muneco), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTajoImpacto, "SAEVUS.Combate.Tajos.RetrocesoYParada", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTajoImpacto::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasTajos;

	FMundoTajos Mundo(PosicionDePie);
	if (!TestTrue(TEXT("Asentado"), Mundo.MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	const ASAEVUSMunecoEntrenamiento* Muneco = Mundo.MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(PosicionMunecoDelante);
	Mundo.Combate->PedirAtaque();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Muneco->ObtenerGolpes() == 0; ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	if (!TestEqual(TEXT("Golpea al muñeco"), Muneco->ObtenerGolpes(), 1))
	{
		return false;
	}
	const FSAEVUSRepertorioMelee& Repertorio = Mundo.Combate->ObtenerRepertorio();
	TestTrue(TEXT("Parada de golpe activa"), Mundo.Combate->EstaEnParadaGolpe());
	TestEqual(TEXT("El atacante va a la escala de tiempo de la parada"), Mundo.Personaje->CustomTimeDilation, Repertorio.EscalaTiempoParada, KINDA_SMALL_NUMBER);
	AddInfo(FString::Printf(TEXT("Velocidad del atacante tras golpear %s"), *Mundo.Movimiento->Velocity.ToCompactString()));
	TestTrue(TEXT("Retroceso: el atacante sale hacia atrás"), Mundo.Movimiento->Velocity.X < 0.f);

	Mundo.MundoPrueba.Paso();
	Mundo.MundoPrueba.Paso();
	TestFalse(TEXT("La parada termina"), Mundo.Combate->EstaEnParadaGolpe());
	TestEqual(TEXT("El atacante vuelve a la escala de tiempo normal"), Mundo.Personaje->CustomTimeDilation, 1.f, KINDA_SMALL_NUMBER);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
