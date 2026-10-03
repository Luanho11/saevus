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
	// El personaje cae desde aquí sobre un muñeco en el suelo, y ataca hacia abajo al pasar esta altura.
	const FVector PosicionSobreMuneco(0.f, 0.f, 450.f);
	constexpr float AlturaAtaqueAbajo = 380.f;

	constexpr float TiempoMax = 2.f;
	constexpr float FraccionMinRebote = 0.9f;

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
	TestEqual(TEXT("Cámara hacia abajo: tajo hacia abajo"), Mundo.Combate->DireccionSegunInclinacion(InclinacionAbajoClara), ESAEVUSDireccionAtaque::Abajo);

	// En el suelo, abajo se convierte en frontal.
	if (!TestTrue(TEXT("Asentado"), Mundo.MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	Mundo.Combate->PedirAtaqueEnDireccion(ESAEVUSDireccionAtaque::Abajo);
	TestEqual(TEXT("Abajo en el suelo es frontal"), Mundo.Combate->ObtenerDireccionAtaque(), ESAEVUSDireccionAtaque::Frontal);
	TestTrue(TEXT("Y usa el primer ataque del combo de suelo"), Mundo.Combate->ObtenerAtaqueActual() == &Mundo.Combate->ObtenerRepertorio().ComboSuelo[0]);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTajoAbajoRebote, "SAEVUS.Combate.Tajos.AbajoRebote", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTajoAbajoRebote::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasTajos;

	FMundoTajos Mundo(PosicionSobreMuneco);
	const ASAEVUSMunecoEntrenamiento* Muneco = Mundo.MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector(0.f, 0.f, MediaAlturaMuneco));
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Mundo.Personaje->GetActorLocation().Z > AlturaAtaqueAbajo; ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	if (!TestTrue(TEXT("Cayendo sobre el muñeco"), Mundo.Movimiento->IsFalling()))
	{
		return false;
	}
	// Gasta el dash aéreo para comprobar que el rebote lo devuelve.
	Mundo.Movimiento->PedirDash();
	Mundo.MundoPrueba.Paso();
	TestEqual(TEXT("Dash aéreo gastado"), Mundo.Movimiento->DashesAereosRestantes(), Mundo.Movimiento->ObtenerAjusteDash().MaxDashesAereos - 1);
	Mundo.Personaje->SetActorLocation(FVector(0.f, 0.f, AlturaAtaqueAbajo));
	Mundo.Movimiento->Velocity = FVector::ZeroVector;

	Mundo.Combate->PedirAtaqueEnDireccion(ESAEVUSDireccionAtaque::Abajo);
	TestEqual(TEXT("Tajo hacia abajo en el aire"), Mundo.Combate->ObtenerDireccionAtaque(), ESAEVUSDireccionAtaque::Abajo);
	float MaxVelocidadSubida = 0.f;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Muneco->ObtenerGolpes() == 0; ++Indice)
	{
		Mundo.MundoPrueba.Paso();
		MaxVelocidadSubida = FMath::Max(MaxVelocidadSubida, static_cast<float>(Mundo.Movimiento->Velocity.Z));
	}
	if (!TestEqual(TEXT("El tajo hacia abajo golpea al muñeco"), Muneco->ObtenerGolpes(), 1))
	{
		return false;
	}
	const float VelocidadRebote = Mundo.Combate->ObtenerRepertorio().TajoAbajo.VelocidadRebote;
	AddInfo(FString::Printf(TEXT("Velocidad vertical tras el rebote %.0f cm/s"), Mundo.Movimiento->Velocity.Z));
	TestTrue(TEXT("Rebota hacia arriba (pogo)"), Mundo.Movimiento->Velocity.Z > VelocidadRebote * FraccionMinRebote);
	TestEqual(TEXT("El rebote devuelve el dash aéreo"), Mundo.Movimiento->DashesAereosRestantes(), Mundo.Movimiento->ObtenerAjusteDash().MaxDashesAereos);
	TestEqual(TEXT("Y el doble salto"), Mundo.Movimiento->SaltosAereosRestantes(), Mundo.Movimiento->ObtenerAjusteAire().MaxSaltosAereos);
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
