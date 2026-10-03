// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movimiento/SAEVUSAjustesMovimiento.h"

namespace SAEVUSPruebasEscalada
{
	// Suelo de 40 m x 40 m, superficie en Z = 0.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);

	// Las dos paredes tienen su cara en el plano X = 200, mirando a -X.
	const FVector NormalPared(-1.f, 0.f, 0.f);
	// 10 m de alto: una escalada nunca llega arriba.
	const FVector CentroParedAlta(250.f, 0.f, 500.f);
	const FVector TamanoParedAlta(100.f, 2000.f, 1000.f);
	// 3,5 m de alto y 4 m de fondo: una escalada llega arriba.
	const FVector CentroParedCornisa(400.f, 0.f, 175.f);
	const FVector TamanoParedCornisa(400.f, 2000.f, 350.f);
	constexpr float ZCimaCornisa = 350.f;
	constexpr float XCaraPared = 200.f;

	// A 58 cm de la cara de la pared, en el aire, corriendo hacia ella.
	const FVector Inicio(100.f, 0.f, 150.f);
	// A 20 cm de la cara de la pared: dentro de la distancia de detección desde el primer paso.
	const FVector InicioCerca(138.f, 0.f, 150.f);
	constexpr float VelocidadAproximacion = 500.f;

	constexpr float TiempoMaxAproximacion = 0.5f;
	constexpr float TiempoMaxEscalada = 2.f;
	constexpr float TiempoMaxCornisa = 1.5f;
	constexpr float TiempoComprobarSinEscalada = 0.3f;
	constexpr int32 PasosAntesDeSaltar = 5;
	constexpr float ToleranciaVelocidad = 1.f;
	constexpr float ToleranciaGuinadaGrados = 1.f;
	constexpr float ToleranciaAltura = 0.1f;

	struct FMundoEscalada
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = nullptr;
		USAEVUSComponenteMovimiento* Movimiento = nullptr;

		FMundoEscalada(const FVector& CentroPared, const FVector& TamanoPared, const FVector& Velocidad = FVector(VelocidadAproximacion, 0.f, 0.f),
			const FVector& Posicion = Inicio)
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			MundoPrueba.CrearBloque(CentroPared, TamanoPared);
			Personaje = MundoPrueba.CrearPersonaje(Posicion);
			Movimiento = Personaje->ObtenerMovimiento();
			Movimiento->Velocity = Velocidad;
		}

		void Paso(const FVector& Entrada = FVector::ForwardVector)
		{
			if (!Entrada.IsZero())
			{
				Personaje->AddMovementInput(Entrada, 1.f);
			}
			MundoPrueba.Paso();
		}

		/** Corre hacia la pared hasta que empieza una escalada; falso si no ocurre en TiempoMaxAproximacion. */
		bool AproximarHastaEscalar(const FVector& Entrada = FVector::ForwardVector)
		{
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAproximacion) && !Movimiento->EstaEscalando(); ++Indice)
			{
				Paso(Entrada);
			}
			return Movimiento->EstaEscalando();
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscaladaEntrar, "SAEVUS.Movimiento.Escalada.Entrar", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscaladaEntrar::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEscalada;

	FMundoEscalada Mundo(CentroParedAlta, TamanoParedAlta);
	if (!TestTrue(TEXT("Saltar de frente a una pared con entrada hacia ella empieza una escalada"), Mundo.AproximarHastaEscalar()))
	{
		return false;
	}
	TestEqual(TEXT("Sube a la velocidad de escalada"), static_cast<float>(Mundo.Movimiento->Velocity.Z), Mundo.Movimiento->ObtenerAjusteEscalada().Velocidad, ToleranciaVelocidad);
	TestEqual(TEXT("Mira a la pared"), static_cast<float>(Mundo.Personaje->GetActorRotation().Yaw), 0.f, ToleranciaGuinadaGrados);
	TestEqual(TEXT("Gastada una escalada"), Mundo.Movimiento->EscaladasRestantes(), Mundo.Movimiento->ObtenerAjusteEscalada().MaxEscaladas - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscaladaCondiciones, "SAEVUS.Movimiento.Escalada.CondicionesEntrada", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscaladaCondiciones::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEscalada;

	struct FCaso
	{
		const TCHAR* Descripcion;
		FVector Velocidad;
		FVector Entrada;
	};
	// 60 grados respecto al frente: una llegada en ángulo.
	const FVector EnAngulo = FRotator(0.f, 60.f, 0.f).Vector();
	const FCaso Rechazados[] = {
		{ TEXT("Sin entrada: no escala"), FVector(VelocidadAproximacion, 0.f, 0.f), FVector::ZeroVector },
		{ TEXT("Entrada a lo largo de la pared: no escala"), FVector(VelocidadAproximacion, 0.f, 0.f), FVector::RightVector },
		{ TEXT("Llegada en ángulo: no escala"), EnAngulo * VelocidadAproximacion, EnAngulo },
		{ TEXT("Demasiado lento hacia la pared: no escala"), FVector(100.f, 0.f, 0.f), FVector::ForwardVector },
	};
	// Un paso dentro de la distancia de detección: en pasos siguientes el control aéreo cambiaría la llegada.
	for (const FCaso& Caso : Rechazados)
	{
		FMundoEscalada Mundo(CentroParedAlta, TamanoParedAlta, Caso.Velocidad, InicioCerca);
		Mundo.Paso(Caso.Entrada);
		TestFalse(Caso.Descripcion, Mundo.Movimiento->EstaEscalando());
	}
	FMundoEscalada Aceptado(CentroParedAlta, TamanoParedAlta, FVector(VelocidadAproximacion, 0.f, 0.f), InicioCerca);
	Aceptado.Paso();
	TestTrue(TEXT("Misma situación, de frente y con entrada: escala"), Aceptado.Movimiento->EstaEscalando());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscaladaVoltereta, "SAEVUS.Movimiento.Escalada.Voltereta", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscaladaVoltereta::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEscalada;

	FMundoEscalada Mundo(CentroParedAlta, TamanoParedAlta);
	if (!TestTrue(TEXT("La escalada empieza"), Mundo.AproximarHastaEscalar()))
	{
		return false;
	}
	const FSAEVUSAjusteEscaladaPared& Ajuste = Mundo.Movimiento->ObtenerAjusteEscalada();
	const float ZInicio = static_cast<float>(Mundo.Personaje->GetActorLocation().Z);
	// Sin entrada una vez escalando: la escalada no la necesita, y dirigir cambiaría la voltereta.
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxEscalada) && Mundo.Movimiento->EstaEscalando(); ++Indice)
	{
		Mundo.Paso(FVector::ZeroVector);
	}
	const float Altura = static_cast<float>(Mundo.Personaje->GetActorLocation().Z) - ZInicio;
	const float AlturaEsperada = Ajuste.Velocidad * Ajuste.DuracionMaxima;
	AddInfo(FString::Printf(TEXT("Altura escalada %.1f cm, esperada %.1f cm"), Altura, AlturaEsperada));
	TestEqual(TEXT("Sube más o menos Velocidad x DuracionMaxima"), Altura, AlturaEsperada, AlturaEsperada * ToleranciaAltura);
	TestTrue(TEXT("Cae tras la escalada"), Mundo.Movimiento->IsFalling());
	TestEqual(TEXT("Voltereta lejos de la pared"), static_cast<float>(FVector::DotProduct(Mundo.Movimiento->Velocity, NormalPared)), Ajuste.VelocidadVolteretaAlejarse, ToleranciaVelocidad);
	TestEqual(TEXT("Tras la voltereta mira lejos de la pared"), FMath::Abs(static_cast<float>(Mundo.Personaje->GetActorRotation().Yaw)), 180.f, ToleranciaGuinadaGrados);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscaladaCornisa, "SAEVUS.Movimiento.Escalada.Cornisa", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscaladaCornisa::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEscalada;

	FMundoEscalada Mundo(CentroParedCornisa, TamanoParedCornisa);
	if (!TestTrue(TEXT("La escalada empieza"), Mundo.AproximarHastaEscalar()))
	{
		return false;
	}
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxCornisa) && !Mundo.Movimiento->IsMovingOnGround(); ++Indice)
	{
		Mundo.Paso();
	}
	const FVector Posicion = Mundo.Personaje->GetActorLocation();
	AddInfo(FString::Printf(TEXT("Tras la escalada: %s, modo %s"), *Posicion.ToCompactString(), *Mundo.Movimiento->GetMovementName()));
	TestTrue(TEXT("Aterriza tras coronar la pared"), Mundo.Movimiento->IsMovingOnGround());
	TestTrue(TEXT("Queda de pie encima de la pared"), Posicion.X > XCaraPared && Posicion.Z > ZCimaCornisa);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscaladaUnaPorAire, "SAEVUS.Movimiento.Escalada.UnaPorEstanciaAerea", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscaladaUnaPorAire::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEscalada;

	FMundoEscalada Mundo(CentroParedAlta, TamanoParedAlta);
	if (!TestTrue(TEXT("Primera escalada"), Mundo.AproximarHastaEscalar()))
	{
		return false;
	}
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxEscalada) && Mundo.Movimiento->EstaEscalando(); ++Indice)
	{
		Mundo.Paso();
	}

	// Otra vez delante de la pared, todavía en el aire, corriendo hacia ella.
	Mundo.Personaje->SetActorLocation(Inicio);
	Mundo.Movimiento->Velocity = FVector(VelocidadAproximacion, 0.f, 0.f);
	bool bEscalo = false;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoComprobarSinEscalada); ++Indice)
	{
		Mundo.Paso();
		bEscalo |= Mundo.Movimiento->EstaEscalando();
	}
	TestFalse(TEXT("No hay segunda escalada en la misma estancia en el aire"), bEscalo);

	Mundo.Personaje->SetActorLocation(FVector(-500.f, 0.f, 150.f));
	if (!TestTrue(TEXT("Aterriza"), Mundo.MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	TestEqual(TEXT("Aterrizar recarga las escaladas"), Mundo.Movimiento->EscaladasRestantes(), Mundo.Movimiento->ObtenerAjusteEscalada().MaxEscaladas);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscaladaSaltar, "SAEVUS.Movimiento.Escalada.Saltar", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscaladaSaltar::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEscalada;

	FMundoEscalada Mundo(CentroParedAlta, TamanoParedAlta);
	if (!TestTrue(TEXT("La escalada empieza"), Mundo.AproximarHastaEscalar()))
	{
		return false;
	}
	for (int32 Indice = 0; Indice < PasosAntesDeSaltar; ++Indice)
	{
		Mundo.Paso();
	}
	Mundo.Personaje->PedirSalto();
	Mundo.Paso(FVector::ZeroVector);

	const FSAEVUSAjusteSaltoPared& SaltoPared = Mundo.Movimiento->ObtenerAjusteSaltoPared();
	TestTrue(TEXT("El salto deja la pared"), Mundo.Movimiento->IsFalling());
	TestEqual(TEXT("Empujado a la VelocidadAlejarse del salto de pared"), static_cast<float>(FVector::DotProduct(Mundo.Movimiento->Velocity, NormalPared)), SaltoPared.VelocidadAlejarse, ToleranciaVelocidad);
	TestTrue(TEXT("Sube"), Mundo.Movimiento->Velocity.Z > 0.f);
	TestEqual(TEXT("Cuenta como salto de pared"), Mundo.Movimiento->SaltosParedRestantes(), SaltoPared.MaxSaltosPared - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscaladaSoltar, "SAEVUS.Movimiento.Escalada.Soltar", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscaladaSoltar::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEscalada;

	FMundoEscalada Mundo(CentroParedAlta, TamanoParedAlta);
	if (!TestTrue(TEXT("La escalada empieza"), Mundo.AproximarHastaEscalar()))
	{
		return false;
	}
	Mundo.Paso(NormalPared);
	TestTrue(TEXT("Dirigir lejos de la pared la suelta"), Mundo.Movimiento->IsFalling());
	TestTrue(TEXT("Cae en vez de subir"), Mundo.Movimiento->Velocity.Z <= 0.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
