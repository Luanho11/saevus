// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Armas/SAEVUSTiposArma.h"
#include "Combate/SAEVUSTiposCombate.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"

namespace SAEVUSPruebasArmas
{
	// Suelo de 40 m x 40 m, superficie en Z = 0.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);

	// Muñeco justo delante (+X), de pie en el suelo.
	constexpr float MediaAlturaMuneco = 96.f;
	constexpr float DistanciaMuneco = 500.f;
	// Cobertura entre el personaje y el muñeco, más alta que ambos.
	const FVector CentroCobertura(250.f, 0.f, 200.f);
	const FVector TamanoCobertura(50.f, 400.f, 400.f);

	constexpr float AlcanceCorto = 300.f;
	constexpr float TiempoMantener = 1.f;
	constexpr int32 ToleranciaDisparosCadencia = 1;
	constexpr int32 Perdigones = 5;
	constexpr float MargenAsentar = 0.1f;

	/** Arma de fuego de prueba: precisa (sin dispersión), así cada disparo va adonde mira el personaje. */
	USAEVUSDefinicionArma* CrearArmaFuego(FName Nombre, bool bAutomatica, int32 TamanoCargador, float IntervaloDisparo)
	{
		USAEVUSDefinicionArma* Arma = NewObject<USAEVUSDefinicionArma>(GetTransientPackage());
		Arma->NombreVisible = Nombre;
		Arma->Tipo = ESAEVUSTipoArma::ArmaFuego;
		Arma->ArmaFuego.bAutomatica = bAutomatica;
		Arma->ArmaFuego.TamanoCargador = TamanoCargador;
		Arma->ArmaFuego.IntervaloDisparo = IntervaloDisparo;
		Arma->ArmaFuego.DispersionGrados = 0.f;
		return Arma;
	}

	USAEVUSDefinicionArma* CrearPistola() { return CrearArmaFuego(TEXT("Pistola"), false, 12, 0.25f); }
	USAEVUSDefinicionArma* CrearRifle(int32 TamanoCargador = 30) { return CrearArmaFuego(TEXT("Rifle"), true, TamanoCargador, 0.1f); }

	USAEVUSDefinicionArma* CrearEspada(const USAEVUSDatosRepertorioMelee* Repertorio = nullptr)
	{
		USAEVUSDefinicionArma* Arma = NewObject<USAEVUSDefinicionArma>(GetTransientPackage());
		Arma->NombreVisible = TEXT("Espada");
		Arma->Tipo = ESAEVUSTipoArma::CuerpoACuerpo;
		Arma->Repertorio = Repertorio;
		return Arma;
	}

	/** Suelo, un personaje asentado, un muñeco opcional y un arsenal. */
	struct FMundoArmas
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = nullptr;
		USAEVUSComponenteArmas* Armas = nullptr;
		USAEVUSComponenteCombate* Combate = nullptr;
		ASAEVUSMunecoEntrenamiento* Muneco = nullptr;
		bool bListo = false;

		explicit FMundoArmas(const TArray<const USAEVUSDefinicionArma*>& Arsenal, bool bConMuneco = true)
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			if (bConMuneco)
			{
				Muneco = MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector(DistanciaMuneco, 0.f, MediaAlturaMuneco));
			}
			Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
			Armas = Personaje->ObtenerArmas();
			Combate = Personaje->ObtenerCombate();
			Armas->FijarArsenal(Arsenal);
			bListo = MundoPrueba.AsentarEnSuelo();
		}

		void AvanzarDurante(float Segundos)
		{
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Segundos); ++Indice)
			{
				MundoPrueba.Paso();
			}
		}

		/** Pulsa y suelta el gatillo con un paso entre medias. */
		void Toque()
		{
			Personaje->PedirAtaque();
			MundoPrueba.Paso();
			Personaje->SoltarAtaque();
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaHitscan, "SAEVUS.Armas.Hitscan", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaHitscan::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasArmas;

	{
		const USAEVUSDefinicionArma* Pistola = CrearPistola();
		FMundoArmas Mundo({ Pistola });
		if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
		{
			return false;
		}
		TestTrue(TEXT("Arma de fuego equipada desde el arsenal"), Mundo.Armas->TieneArmaFuegoEquipada());
		Mundo.Toque();
		TestEqual(TEXT("Un disparo"), Mundo.Armas->ObtenerDisparosRealizados(), 1);
		TestEqual(TEXT("El disparo golpea al muñeco una vez"), Mundo.Muneco->ObtenerGolpes(), 1);
		TestEqual(TEXT("Daño de los datos del arma"), Mundo.Muneco->ObtenerDanioTotal(), Pistola->ArmaFuego.Danio);
		TestEqual(TEXT("Una bala gastada"), Mundo.Armas->ObtenerMunicionActual(), Pistola->ArmaFuego.TamanoCargador - 1);
	}
	{
		USAEVUSDefinicionArma* PistolaCorta = CrearPistola();
		PistolaCorta->ArmaFuego.Alcance = AlcanceCorto;
		FMundoArmas Mundo({ PistolaCorta });
		Mundo.Toque();
		TestEqual(TEXT("Fuera de alcance: dispara pero no golpea"), Mundo.Muneco->ObtenerGolpes(), 0);
	}
	{
		FMundoArmas Mundo({ CrearPistola() });
		Mundo.MundoPrueba.CrearBloque(CentroCobertura, TamanoCobertura);
		Mundo.Toque();
		TestEqual(TEXT("La cobertura delante de la boca detiene el disparo"), Mundo.Muneco->ObtenerGolpes(), 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaModosDisparo, "SAEVUS.Armas.ModosDisparo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaModosDisparo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasArmas;

	{
		FMundoArmas Mundo({ CrearPistola() }, false);
		Mundo.Personaje->PedirAtaque();
		Mundo.AvanzarDurante(TiempoMantener);
		Mundo.Personaje->SoltarAtaque();
		TestEqual(TEXT("Semiautomática: un disparo por pulsación, por mucho que se mantenga"), Mundo.Armas->ObtenerDisparosRealizados(), 1);
	}
	{
		const USAEVUSDefinicionArma* Rifle = CrearRifle();
		FMundoArmas Mundo({ Rifle }, false);
		Mundo.Personaje->PedirAtaque();
		Mundo.AvanzarDurante(TiempoMantener);
		Mundo.Personaje->SoltarAtaque();
		const int32 Esperados = FMath::RoundToInt32(TiempoMantener / Rifle->ArmaFuego.IntervaloDisparo);
		AddInfo(FString::Printf(TEXT("Automática: %d disparos en %.1f s, esperados %d"), Mundo.Armas->ObtenerDisparosRealizados(), TiempoMantener, Esperados));
		TestTrue(TEXT("Automática: dispara a su cadencia mientras se mantiene"), FMath::Abs(Mundo.Armas->ObtenerDisparosRealizados() - Esperados) <= ToleranciaDisparosCadencia);
	}
	{
		USAEVUSDefinicionArma* Escopeta = CrearArmaFuego(TEXT("Escopeta"), false, 6, 0.8f);
		Escopeta->ArmaFuego.PerdigonesPorDisparo = Perdigones;
		FMundoArmas Mundo({ Escopeta });
		Mundo.Toque();
		TestEqual(TEXT("Cada perdigón es un golpe"), Mundo.Muneco->ObtenerGolpes(), Perdigones);
		TestEqual(TEXT("Daño por perdigón"), Mundo.Muneco->ObtenerDanioTotal(), Escopeta->ArmaFuego.Danio * Perdigones);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaRecarga, "SAEVUS.Armas.Recarga", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaRecarga::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasArmas;

	constexpr int32 CargadorPequeno = 3;
	const USAEVUSDefinicionArma* Rifle = CrearRifle(CargadorPequeno);
	FMundoArmas Mundo({ Rifle }, false);

	// Mantener hasta vaciar el cargador: el gatillo en seco empieza una recarga.
	Mundo.Personaje->PedirAtaque();
	Mundo.AvanzarDurante(Rifle->ArmaFuego.IntervaloDisparo * (CargadorPequeno + 1));
	TestEqual(TEXT("Vacía el cargador"), Mundo.Armas->ObtenerDisparosRealizados(), CargadorPequeno);
	TestTrue(TEXT("El gatillo en seco recarga"), Mundo.Armas->EstaRecargando());
	Mundo.AvanzarDurante(Rifle->ArmaFuego.TiempoRecarga * 0.5f);
	TestEqual(TEXT("Sin disparos mientras recarga"), Mundo.Armas->ObtenerDisparosRealizados(), CargadorPequeno);
	Mundo.Personaje->SoltarAtaque();
	Mundo.AvanzarDurante(Rifle->ArmaFuego.TiempoRecarga * 0.5f + MargenAsentar);
	TestFalse(TEXT("La recarga termina"), Mundo.Armas->EstaRecargando());
	TestEqual(TEXT("Cargador lleno tras la recarga"), Mundo.Armas->ObtenerMunicionActual(), CargadorPequeno);

	// Recarga manual: rechazada con el cargador lleno, aceptada tras un disparo.
	Mundo.Personaje->PedirRecarga();
	TestFalse(TEXT("No recarga con el cargador lleno"), Mundo.Armas->EstaRecargando());
	Mundo.Toque();
	Mundo.AvanzarDurante(Rifle->ArmaFuego.IntervaloDisparo);
	Mundo.Personaje->PedirRecarga();
	TestTrue(TEXT("Recarga tras un disparo"), Mundo.Armas->EstaRecargando());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCambioArma, "SAEVUS.Armas.Cambio", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCambioArma::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasArmas;

	const USAEVUSDefinicionArma* Espada = CrearEspada();
	const USAEVUSDefinicionArma* Pistola = CrearPistola();
	FMundoArmas Mundo({ Espada, Pistola });
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	TestFalse(TEXT("Empieza con el arma cuerpo a cuerpo"), Mundo.Armas->TieneArmaFuegoEquipada());

	Mundo.Personaje->PedirRanuraArma(1);
	TestTrue(TEXT("Cambiando a la pistola"), Mundo.Armas->EstaCambiando());
	Mundo.Personaje->PedirAtaque();
	Mundo.MundoPrueba.Paso();
	Mundo.Personaje->SoltarAtaque();
	TestTrue(TEXT("Ni ataque ni disparo mientras cambia"), !Mundo.Combate->EstaAtacando() && Mundo.Armas->ObtenerDisparosRealizados() == 0);

	Mundo.AvanzarDurante(Pistola->TiempoEquipar + MargenAsentar);
	TestEqual(TEXT("Pistola equipada tras su tiempo de equipado"), Mundo.Armas->ObtenerRanuraActual(), 1);
	Mundo.Toque();
	TestEqual(TEXT("La acción principal dispara la pistola"), Mundo.Muneco->ObtenerGolpes(), 1);

	Mundo.Personaje->PedirRanuraArma(1);
	TestFalse(TEXT("Elegir el arma equipada no hace nada"), Mundo.Armas->EstaCambiando());

	Mundo.Personaje->PedirSiguienteArma();
	Mundo.AvanzarDurante(Espada->TiempoEquipar + MargenAsentar);
	TestEqual(TEXT("La siguiente arma vuelve a la espada"), Mundo.Armas->ObtenerRanuraActual(), 0);
	Mundo.Personaje->PedirAtaque();
	TestTrue(TEXT("La acción principal ataca con la espada"), Mundo.Combate->EstaAtacando());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCambioCancela, "SAEVUS.Armas.CambioCancela", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCambioCancela::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasArmas;

	{
		FMundoArmas Mundo({ CrearEspada(), CrearPistola() }, false);
		Mundo.Personaje->PedirAtaque();
		Mundo.Personaje->PedirRanuraArma(1);
		TestFalse(TEXT("No hay cambio antes de la ventana de cancelación por cambio"), Mundo.Armas->EstaCambiando());
		TestTrue(TEXT("El ataque sigue"), Mundo.Combate->EstaAtacando());

		const float CancelarDesde = Mundo.Combate->ObtenerAtaqueActual()->CancelarConCambioArmaDesde;
		while (Mundo.Combate->EstaAtacando() && Mundo.Combate->ObtenerTiempoAtaque() < CancelarDesde)
		{
			Mundo.MundoPrueba.Paso();
		}
		Mundo.Personaje->PedirRanuraArma(1);
		TestTrue(TEXT("El cambio en la ventana empieza"), Mundo.Armas->EstaCambiando());
		TestFalse(TEXT("El cambio en la ventana termina el ataque"), Mundo.Combate->EstaAtacando());
	}
	{
		const USAEVUSDefinicionArma* Pistola = CrearPistola();
		FMundoArmas Mundo({ Pistola, CrearPistola() }, false);
		Mundo.Toque();
		Mundo.AvanzarDurante(Pistola->ArmaFuego.IntervaloDisparo);
		Mundo.Personaje->PedirRecarga();
		Mundo.Personaje->PedirRanuraArma(1);
		TestFalse(TEXT("Cambiar descarta la recarga"), Mundo.Armas->EstaRecargando());
		Mundo.AvanzarDurante(Pistola->TiempoEquipar + MargenAsentar);
		Mundo.Personaje->PedirRanuraArma(0);
		Mundo.AvanzarDurante(Pistola->TiempoEquipar + Pistola->ArmaFuego.TiempoRecarga);
		TestEqual(TEXT("La recarga descartada nunca llenó el cargador"), Mundo.Armas->ObtenerMunicionActual(), Pistola->ArmaFuego.TamanoCargador - 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaRepertorioArma, "SAEVUS.Armas.RepertorioMelee", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaRepertorioArma::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasArmas;

	USAEVUSDatosRepertorioMelee* Repertorio = NewObject<USAEVUSDatosRepertorioMelee>(GetTransientPackage());
	Repertorio->Repertorio.ComboSuelo[0].Nombre = TEXT("AperturaPrueba");
	const USAEVUSDefinicionArma* Espada = CrearEspada(Repertorio);
	FMundoArmas Mundo({ CrearPistola(), Espada }, false);

	Mundo.Personaje->PedirRanuraArma(1);
	Mundo.AvanzarDurante(Espada->TiempoEquipar + MargenAsentar);
	Mundo.Personaje->PedirAtaque();
	if (!TestTrue(TEXT("Ataque cuerpo a cuerpo tras cambiar al arma cuerpo a cuerpo"), Mundo.Combate->EstaAtacando()))
	{
		return false;
	}
	TestEqual(TEXT("El combate usa el repertorio del arma"), Mundo.Combate->ObtenerAtaqueActual()->Nombre, FName(TEXT("AperturaPrueba")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
