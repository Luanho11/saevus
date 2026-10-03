// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSComponenteVida.h"
#include "Personaje/SAEVUSDetectorDobleToque.h"

namespace SAEVUSPruebasDobleToque
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);

	const FVector2D Adelante(0.f, 1.f);
	const FVector2D Derecha(1.f, 0.f);
	const FVector2D Diagonal(1.f, 1.f);

	// Tiempos para el detector puro (la ventana por defecto es 0,25 s).
	constexpr double DentroVentana = 0.15;
	constexpr double FueraVentana = 0.4;
	constexpr double Pequeno = 0.05;

	// Pasos de simulación pulsando y soltando: bien dentro de la ventana.
	constexpr int32 PasosPulsado = 3;
	constexpr int32 PasosSuelto = 3;

	constexpr float ProductoMinDireccion = 0.99f;
	constexpr float DanioPrueba = 10.f;
	constexpr float Margen = 0.1f;
	constexpr float TiempoMaxEstela = 1.5f;

	struct FMundoDobleToque
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = nullptr;
		USAEVUSComponenteMovimiento* Movimiento = nullptr;
		bool bListo = false;

		FMundoDobleToque()
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
			Movimiento = Personaje->ObtenerMovimiento();
			bListo = MundoPrueba.AsentarEnSuelo();
		}

		/** Pasos con el eje dado aplicado como entrada de movimiento (cero = suelto). */
		void Pasos(const FVector2D& Eje, int32 Cantidad)
		{
			for (int32 Indice = 0; Indice < Cantidad; ++Indice)
			{
				Personaje->AplicarEntradaMovimiento(Eje);
				MundoPrueba.Paso();
			}
		}

		/** Pulsar, soltar y volver a pulsar Eje; devuelve cierto si empezó un dash. */
		bool DobleToque(const FVector2D& Eje)
		{
			Pasos(Eje, PasosPulsado);
			Pasos(FVector2D::ZeroVector, PasosSuelto);
			Pasos(Eje, 1);
			return Movimiento->EstaEnDash();
		}

		void AvanzarDurante(float Segundos)
		{
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Segundos); ++Indice)
			{
				MundoPrueba.Paso();
			}
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDetectorDobleToque, "SAEVUS.Movimiento.DobleToque.Detector", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDetectorDobleToque::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDobleToque;

	{
		FSAEVUSDetectorDobleToque Detector;
		Detector.Actualizar(Derecha, 0.0);
		Detector.Actualizar(FVector2D::ZeroVector, Pequeno);
		TestEqual(TEXT("Pulsar, soltar y pulsar dentro de la ventana es doble toque"), Detector.Actualizar(Derecha, DentroVentana), Derecha);
	}
	{
		FSAEVUSDetectorDobleToque Detector;
		Detector.Actualizar(Derecha, 0.0);
		Detector.Actualizar(FVector2D::ZeroVector, Pequeno);
		TestTrue(TEXT("Fuera de la ventana no hay doble toque"), Detector.Actualizar(Derecha, FueraVentana).IsZero());
	}
	{
		FSAEVUSDetectorDobleToque Detector;
		bool bAlguno = false;
		for (double Tiempo = 0.0; Tiempo < FueraVentana; Tiempo += Pequeno)
		{
			bAlguno |= !Detector.Actualizar(Derecha, Tiempo).IsZero();
		}
		TestFalse(TEXT("Mantener una dirección no es doble toque"), bAlguno);
	}
	{
		FSAEVUSDetectorDobleToque Detector;
		Detector.Actualizar(Derecha, 0.0);
		Detector.Actualizar(FVector2D::ZeroVector, Pequeno);
		TestTrue(TEXT("Dos direcciones distintas no se combinan"), Detector.Actualizar(Adelante, DentroVentana).IsZero());
	}
	{
		// Mantener adelante y tocar dos veces derecha: doble toque a la derecha.
		FSAEVUSDetectorDobleToque Detector;
		Detector.Actualizar(Adelante, 0.0);
		Detector.Actualizar(Diagonal, Pequeno);
		Detector.Actualizar(Adelante, 2.0 * Pequeno);
		TestEqual(TEXT("Con adelante mantenido, el doble toque lateral cuenta"), Detector.Actualizar(Diagonal, DentroVentana), Derecha);
	}
	{
		FSAEVUSDetectorDobleToque Detector;
		Detector.Actualizar(Derecha, 0.0);
		Detector.Actualizar(FVector2D::ZeroVector, Pequeno);
		Detector.Actualizar(Derecha, 2.0 * Pequeno);
		Detector.Actualizar(FVector2D::ZeroVector, 3.0 * Pequeno);
		TestTrue(TEXT("Una tercera pulsación empieza otra serie"), Detector.Actualizar(Derecha, 4.0 * Pequeno).IsZero());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDashDobleToque, "SAEVUS.Movimiento.DobleToque.Dash", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDashDobleToque::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDobleToque;

	FMundoDobleToque Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	// El personaje mira a +X; derecha local = +Y en el mundo.
	if (!TestTrue(TEXT("Doble toque a la derecha empieza un dash"), Mundo.DobleToque(Derecha)))
	{
		return false;
	}
	TestTrue(TEXT("El dash va hacia la dirección tocada"), Mundo.Movimiento->ObtenerDireccionDash().Y > ProductoMinDireccion);
	TestTrue(TEXT("La cámara queda bloqueada durante el dash"), Mundo.Personaje->CamaraBloqueada());

	Mundo.AvanzarDurante(Mundo.Movimiento->ObtenerAjusteDash().Duracion + Margen);
	TestFalse(TEXT("Termina el dash"), Mundo.Movimiento->EstaEnDash());
	TestFalse(TEXT("La cámara se libera al terminar"), Mundo.Personaje->CamaraBloqueada());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaInvulnerabilidadDash, "SAEVUS.Movimiento.DobleToque.Invulnerabilidad", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaInvulnerabilidadDash::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDobleToque;

	FMundoDobleToque Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	USAEVUSComponenteVida* Vida = Mundo.Personaje->ObtenerVida();
	Mundo.Movimiento->PedirDash();
	Mundo.MundoPrueba.Paso();
	TestTrue(TEXT("Invulnerable al empezar el dash"), Vida->EstaInvulnerable());
	TestEqual(TEXT("El daño se ignora durante la invulnerabilidad"), Vida->AplicarDanio(DanioPrueba, nullptr), 0.f);

	Mundo.AvanzarDurante(Mundo.Movimiento->ObtenerAjusteDash().DuracionInvulnerable + Margen);
	TestFalse(TEXT("La invulnerabilidad termina"), Vida->EstaInvulnerable());
	TestEqual(TEXT("Después, el daño vuelve a contar"), Vida->AplicarDanio(DanioPrueba, nullptr), DanioPrueba);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEstelaDash, "SAEVUS.Efectos.EstelaDash", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEstelaDash::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDobleToque;

	FMundoDobleToque Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	const USAEVUSComponenteEstelaDash* Estela = Mundo.Personaje->ObtenerEstelaDash();
	TestEqual(TEXT("Sin partículas en reposo"), Estela->ObtenerParticulasActivas(), 0);

	Mundo.Movimiento->PedirDash();
	Mundo.MundoPrueba.Paso();
	const int32 TrasInicio = Estela->ObtenerParticulasActivas();
	AddInfo(FString::Printf(TEXT("Partículas tras empezar el dash: %d"), TrasInicio));
	TestTrue(TEXT("El dash lanza partículas"), TrasInicio > 0);

	Mundo.AvanzarDurante(TiempoMaxEstela);
	TestEqual(TEXT("Las partículas desaparecen tras su vida"), Estela->ObtenerParticulasActivas(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
