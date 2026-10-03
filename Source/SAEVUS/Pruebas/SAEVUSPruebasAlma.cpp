// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSComponenteVida.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"

namespace SAEVUSPruebasAlma
{
	// Suelo de 40 m x 40 m, superficie en Z = 0.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);
	const FVector PosicionMuneco(150.f, 0.f, 96.f);

	constexpr float DanioPrueba = 50.f;
	constexpr float DanioPequeno = 10.f;
	constexpr float TiempoMaxAtaque = 2.f;
	// Margen tras la duración de la curación para que se complete.
	constexpr float MargenCuracion = 0.1f;
	constexpr float MargenInvulnerabilidad = 0.1f;
	constexpr float VelocidadQuieto = 1.f;

	struct FMundoAlma
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = nullptr;
		USAEVUSComponenteAlma* Alma = nullptr;
		USAEVUSComponenteVida* Vida = nullptr;
		bool bListo = false;

		FMundoAlma()
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
			Alma = Personaje->ObtenerAlma();
			Vida = Personaje->ObtenerVida();
			bListo = MundoPrueba.AsentarEnSuelo();
		}

		void AvanzarDurante(float Segundos, const FVector& Entrada = FVector::ZeroVector)
		{
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Segundos); ++Indice)
			{
				if (!Entrada.IsZero())
				{
					Personaje->AddMovementInput(Entrada, 1.f);
				}
				MundoPrueba.Paso();
			}
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAlmaGolpe, "SAEVUS.Alma.GanarAlGolpear", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAlmaGolpe::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAlma;

	FMundoAlma Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	TestEqual(TEXT("Empieza sin alma"), Mundo.Alma->ObtenerAlma(), 0.f);
	const ASAEVUSMunecoEntrenamiento* Muneco = Mundo.MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(PosicionMuneco);
	Mundo.Personaje->PedirAtaque();
	Mundo.AvanzarDurante(TiempoMaxAtaque);
	TestEqual(TEXT("Golpeó al muñeco"), Muneco->ObtenerGolpes(), 1);
	TestEqual(TEXT("Un golpe da AlmaPorGolpe"), Mundo.Alma->ObtenerAlma(), Mundo.Alma->ObtenerAjuste().AlmaPorGolpe);

	Mundo.Alma->GanarAlma(Mundo.Alma->ObtenerAlmaMaxima() * 2.f);
	TestEqual(TEXT("El alma no pasa del máximo"), Mundo.Alma->ObtenerAlma(), Mundo.Alma->ObtenerAlmaMaxima());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAlmaCuracion, "SAEVUS.Alma.Curacion", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAlmaCuracion::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAlma;

	FMundoAlma Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	const FSAEVUSAjusteAlma& Ajuste = Mundo.Alma->ObtenerAjuste();
	Mundo.Alma->GanarAlma(Ajuste.CosteCuracion);
	Mundo.Vida->AplicarDanio(DanioPrueba, nullptr);
	const float VidaHerido = Mundo.Vida->ObtenerVida();

	Mundo.Personaje->FijarCuracion(true);
	Mundo.AvanzarDurante(Ajuste.DuracionCuracion * 0.5f, FVector::ForwardVector);
	TestTrue(TEXT("Cura mientras se mantiene"), Mundo.Alma->EstaCurando());
	TestTrue(TEXT("Curando no se mueve"), Mundo.Personaje->ObtenerMovimiento()->Velocity.Size2D() < VelocidadQuieto);
	TestEqual(TEXT("A mitad todavía no cura"), Mundo.Vida->ObtenerVida(), VidaHerido);

	Mundo.AvanzarDurante(Ajuste.DuracionCuracion * 0.5f + MargenCuracion);
	TestEqual(TEXT("La curación completa recupera VidaPorCuracion"), Mundo.Vida->ObtenerVida(), VidaHerido + Ajuste.VidaPorCuracion);
	TestEqual(TEXT("Y gasta CosteCuracion"), Mundo.Alma->ObtenerAlma(), 0.f);
	TestFalse(TEXT("Sin alma deja de curar"), Mundo.Alma->EstaCurando());

	Mundo.Personaje->FijarCuracion(false);
	Mundo.AvanzarDurante(MargenCuracion, FVector::ForwardVector);
	TestTrue(TEXT("Al soltar vuelve a moverse"), Mundo.Personaje->ObtenerMovimiento()->Velocity.Size2D() > VelocidadQuieto);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAlmaInterrupcion, "SAEVUS.Alma.CuracionInterrumpida", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAlmaInterrupcion::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAlma;

	FMundoAlma Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	const FSAEVUSAjusteAlma& Ajuste = Mundo.Alma->ObtenerAjuste();
	// Sin invulnerabilidad, así el segundo golpe llega durante la curación.
	Mundo.Vida->FijarInvulnerabilidadTrasGolpe(0.f);
	Mundo.Alma->GanarAlma(Ajuste.CosteCuracion);
	Mundo.Vida->AplicarDanio(DanioPrueba, nullptr);

	// Un golpe a mitad corta la curación sin gastar alma.
	Mundo.Personaje->FijarCuracion(true);
	Mundo.AvanzarDurante(Ajuste.DuracionCuracion * 0.5f);
	Mundo.Vida->AplicarDanio(DanioPequeno, nullptr);
	const float VidaTrasGolpe = Mundo.Vida->ObtenerVida();
	Mundo.AvanzarDurante(Ajuste.DuracionCuracion + MargenCuracion);
	TestFalse(TEXT("El golpe corta la curación"), Mundo.Alma->EstaCurando());
	TestEqual(TEXT("No se cura tras el golpe aunque se siga manteniendo"), Mundo.Vida->ObtenerVida(), VidaTrasGolpe);
	TestEqual(TEXT("No se gasta alma"), Mundo.Alma->ObtenerAlma(), Ajuste.CosteCuracion);

	// Saltar también la corta.
	Mundo.Personaje->FijarCuracion(true);
	Mundo.AvanzarDurante(Ajuste.DuracionCuracion * 0.5f);
	TestTrue(TEXT("Vuelve a curar al pulsar de nuevo"), Mundo.Alma->EstaCurando());
	Mundo.Personaje->PedirSalto();
	Mundo.MundoPrueba.Paso();
	TestFalse(TEXT("Saltar corta la curación"), Mundo.Alma->EstaCurando());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAlmaSinAlma, "SAEVUS.Alma.SinAlmaNoCura", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAlmaSinAlma::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAlma;

	FMundoAlma Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	Mundo.Vida->AplicarDanio(DanioPrueba, nullptr);
	const float VidaHerido = Mundo.Vida->ObtenerVida();
	Mundo.Personaje->FijarCuracion(true);
	Mundo.AvanzarDurante(Mundo.Alma->ObtenerAjuste().DuracionCuracion + MargenCuracion);
	TestFalse(TEXT("Sin alma no cura"), Mundo.Alma->EstaCurando());
	TestEqual(TEXT("La vida no cambia"), Mundo.Vida->ObtenerVida(), VidaHerido);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaVidaInvulnerableTrasGolpe, "SAEVUS.Vida.InvulnerableTrasGolpe", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaVidaInvulnerableTrasGolpe::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAlma;

	FMundoAlma Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	const float VidaInicial = Mundo.Vida->ObtenerVida();
	TestEqual(TEXT("Primer golpe"), Mundo.Vida->AplicarDanio(DanioPequeno, nullptr), DanioPequeno);
	TestTrue(TEXT("Invulnerable tras el golpe"), Mundo.Vida->EstaEnInvulnerabilidadTrasGolpe());
	TestEqual(TEXT("Un segundo golpe inmediato no hace daño"), Mundo.Vida->AplicarDanio(DanioPequeno, nullptr), 0.f);

	// La duración por defecto del jugador: un segundo (constante del personaje).
	Mundo.AvanzarDurante(1.f + MargenInvulnerabilidad);
	TestFalse(TEXT("La invulnerabilidad termina"), Mundo.Vida->EstaEnInvulnerabilidadTrasGolpe());
	TestEqual(TEXT("Pasado el tiempo, el golpe hace daño"), Mundo.Vida->AplicarDanio(DanioPequeno, nullptr), DanioPequeno);
	TestEqual(TEXT("Vida final"), Mundo.Vida->ObtenerVida(), VidaInicial - 2.f * DanioPequeno);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
