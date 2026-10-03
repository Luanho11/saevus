// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movimiento/SAEVUSAjustesMovimiento.h"

namespace SAEVUSPruebasCarreraPared
{
	// Suelo de 40 m x 40 m, superficie en Z = 0.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);

	// Pared de 40 m de largo y 10 m de alto cuya cara es el plano Y = 200, mirando a -Y. Termina en X = 2000.
	const FVector CentroPared(0.f, 250.f, 500.f);
	const FVector TamanoPared(4000.f, 100.f, 1000.f);
	const FVector NormalPared(0.f, -1.f, 0.f);
	constexpr float XFinPared = 2000.f;

	// Radio de cápsula 42: a 10 cm de la cara de la pared.
	constexpr float YJuntoPared = 148.f;
	// Lo bastante alto para que una carrera completa nunca llegue al suelo.
	const FVector InicioAlto(-1000.f, YJuntoPared, 800.f);
	// Pies a unos 45 cm del suelo.
	const FVector InicioBajo(-1000.f, YJuntoPared, 140.f);
	// 2 m antes del final de la pared.
	const FVector InicioCercaFinPared(XFinPared - 200.f, YJuntoPared, 600.f);

	// Velocidad horizontal paralela a la pared al llegar a ella.
	constexpr float VelocidadAproximacion = 600.f;

	constexpr float ToleranciaVelocidad = 1.f;
	constexpr float ToleranciaGuinadaGrados = 1.f;
	constexpr int32 ToleranciaPasosDuracion = 2;
	constexpr float TiempoComprobarSinReenganche = 0.3f;
	constexpr float TiempoMaxAterrizaje = 1.f;
	constexpr float TiempoComprobarFinPared = 0.6f;
	constexpr int32 PasosAntesDeSaltar = 5;

	/** Suelo, pared y un personaje cayendo en Posicion mientras avanza paralelo a la pared. */
	ASAEVUSPersonaje* Preparar(SAEVUSPruebas::FMundoPrueba& MundoPrueba, const FVector& Posicion, const FVector& Velocidad = FVector(VelocidadAproximacion, 0.f, 0.f))
	{
		MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
		MundoPrueba.CrearBloque(CentroPared, TamanoPared);
		ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(Posicion);
		Personaje->ObtenerMovimiento()->Velocity = Velocidad;
		return Personaje;
	}

	/** Un paso con entrada de movimiento en Direccion. */
	void PasoConEntrada(SAEVUSPruebas::FMundoPrueba& MundoPrueba, ASAEVUSPersonaje* Personaje, const FVector& Direccion = FVector::ForwardVector)
	{
		Personaje->AddMovementInput(Direccion, 1.f);
		MundoPrueba.Paso();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCarreraParedEntrar, "SAEVUS.Movimiento.CarreraPared.Entrar", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCarreraParedEntrar::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCarreraPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, InicioAlto);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	PasoConEntrada(MundoPrueba, Personaje);
	if (!TestTrue(TEXT("Caer junto a una pared con entrada empieza una carrera por pared"), Movimiento->EstaEnCarreraPared()))
	{
		return false;
	}
	TestEqual(TEXT("Corre por la pared a la velocidad de carrera"), static_cast<float>(Movimiento->Velocity.X), Movimiento->ObtenerAjusteCarreraPared().Velocidad, ToleranciaVelocidad);
	TestEqual(TEXT("El cuerpo mira a lo largo de la pared"), static_cast<float>(Personaje->GetActorRotation().Yaw), 0.f, ToleranciaGuinadaGrados);
	TestTrue(TEXT("Sigue pegado a la pared"), FMath::Abs(Personaje->GetActorLocation().Y - YJuntoPared) < Movimiento->ObtenerAjusteCarreraPared().DistanciaDeteccion);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCarreraParedCondiciones, "SAEVUS.Movimiento.CarreraPared.CondicionesEntrada", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCarreraParedCondiciones::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCarreraPared;

	struct FCaso
	{
		const TCHAR* Descripcion;
		FVector Velocidad;
		FVector Entrada;
	};
	const FCaso Rechazados[] = {
		{ TEXT("Sin entrada: no hay carrera por pared"), FVector(VelocidadAproximacion, 0.f, 0.f), FVector::ZeroVector },
		{ TEXT("Demasiado lento a lo largo de la pared: no hay carrera"), FVector(100.f, 0.f, 0.f), FVector::ForwardVector },
		{ TEXT("Entrada alejándose de la pared: no hay carrera"), FVector(VelocidadAproximacion, 0.f, 0.f), NormalPared },
		{ TEXT("Cayendo demasiado rápido: no hay carrera"), FVector(VelocidadAproximacion, 0.f, -1000.f), FVector::ForwardVector },
	};
	for (const FCaso& Caso : Rechazados)
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, InicioAlto, Caso.Velocidad);
		if (!Caso.Entrada.IsZero())
		{
			Personaje->AddMovementInput(Caso.Entrada, 1.f);
		}
		MundoPrueba.Paso();
		TestFalse(Caso.Descripcion, Personaje->ObtenerMovimiento()->EstaEnCarreraPared());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCarreraParedDuracion, "SAEVUS.Movimiento.CarreraPared.Duracion", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCarreraParedDuracion::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCarreraPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, InicioAlto);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	const float DuracionMaxima = Movimiento->ObtenerAjusteCarreraPared().DuracionMaxima;

	int32 PasosEnPared = 0;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(DuracionMaxima * 2.f); ++Indice)
	{
		PasoConEntrada(MundoPrueba, Personaje);
		if (!Movimiento->EstaEnCarreraPared())
		{
			break;
		}
		++PasosEnPared;
	}
	AddInfo(FString::Printf(TEXT("La carrera por pared duró %d pasos, esperados %d"), PasosEnPared, SAEVUSPruebas::PasosPara(DuracionMaxima)));
	TestTrue(TEXT("La carrera dura DuracionMaxima"), FMath::Abs(PasosEnPared - SAEVUSPruebas::PasosPara(DuracionMaxima)) <= ToleranciaPasosDuracion);
	TestTrue(TEXT("Cae al acabarse el tiempo"), Movimiento->IsFalling());

	// Todavía junto a la misma pared y dirigiendo a lo largo de ella: no hay segunda carrera.
	bool bReenganchado = false;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoComprobarSinReenganche); ++Indice)
	{
		PasoConEntrada(MundoPrueba, Personaje);
		bReenganchado |= Movimiento->EstaEnCarreraPared();
	}
	TestFalse(TEXT("La misma pared no se puede recorrer otra vez antes de aterrizar"), bReenganchado);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCarreraParedAterrizaje, "SAEVUS.Movimiento.CarreraPared.Aterrizaje", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCarreraParedAterrizaje::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCarreraPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, InicioBajo);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	PasoConEntrada(MundoPrueba, Personaje);
	if (!TestTrue(TEXT("La carrera empieza abajo en la pared"), Movimiento->EstaEnCarreraPared()))
	{
		return false;
	}
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAterrizaje) && !Movimiento->IsMovingOnGround(); ++Indice)
	{
		PasoConEntrada(MundoPrueba, Personaje);
	}
	TestTrue(TEXT("Bajando hasta el suelo, aterriza"), Movimiento->IsMovingOnGround());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCarreraParedFinPared, "SAEVUS.Movimiento.CarreraPared.FinPared", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCarreraParedFinPared::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCarreraPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, InicioCercaFinPared);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	PasoConEntrada(MundoPrueba, Personaje);
	if (!TestTrue(TEXT("La carrera empieza"), Movimiento->EstaEnCarreraPared()))
	{
		return false;
	}
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoComprobarFinPared); ++Indice)
	{
		PasoConEntrada(MundoPrueba, Personaje);
	}
	TestTrue(TEXT("Pasó el final de la pared"), Personaje->GetActorLocation().X > XFinPared);
	TestTrue(TEXT("Salirse por el final de la pared pasa a caída"), Movimiento->IsFalling());
	TestTrue(TEXT("La velocidad de la carrera sigue en la caída"), Movimiento->Velocity.X > Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Correr));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCarreraParedSaltar, "SAEVUS.Movimiento.CarreraPared.Saltar", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCarreraParedSaltar::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCarreraPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, InicioAlto);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	for (int32 Indice = 0; Indice < PasosAntesDeSaltar; ++Indice)
	{
		PasoConEntrada(MundoPrueba, Personaje);
	}
	if (!TestTrue(TEXT("Corriendo por la pared"), Movimiento->EstaEnCarreraPared()))
	{
		return false;
	}
	Personaje->PedirSalto();
	MundoPrueba.Paso();

	const FSAEVUSAjusteSaltoPared& SaltoPared = Movimiento->ObtenerAjusteSaltoPared();
	TestTrue(TEXT("El salto deja la pared"), Movimiento->IsFalling());
	TestEqual(TEXT("Empujado desde la pared a la VelocidadAlejarse del salto de pared"), static_cast<float>(FVector::DotProduct(Movimiento->Velocity, NormalPared)), SaltoPared.VelocidadAlejarse, ToleranciaVelocidad);
	TestEqual(TEXT("Conserva parte de la velocidad de la carrera"), static_cast<float>(Movimiento->Velocity.X),
		Movimiento->ObtenerAjusteCarreraPared().Velocidad * SaltoPared.FraccionVelocidadTangencial, ToleranciaVelocidad);
	TestTrue(TEXT("Sube"), Movimiento->Velocity.Z > 0.f);
	TestEqual(TEXT("Cuenta como salto de pared"), Movimiento->SaltosParedRestantes(), SaltoPared.MaxSaltosPared - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCarreraParedSoltar, "SAEVUS.Movimiento.CarreraPared.Soltar", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCarreraParedSoltar::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCarreraPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, InicioAlto);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	PasoConEntrada(MundoPrueba, Personaje);
	if (!TestTrue(TEXT("Corriendo por la pared"), Movimiento->EstaEnCarreraPared()))
	{
		return false;
	}
	PasoConEntrada(MundoPrueba, Personaje, NormalPared);
	TestTrue(TEXT("Dirigir lejos de la pared la suelta"), Movimiento->IsFalling());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
