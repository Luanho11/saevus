// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movimiento/SAEVUSAjustesMovimiento.h"

namespace SAEVUSPruebasSaltoPared
{
	// Suelo de 40 m x 40 m, superficie en Z = 0.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);

	// Pared de 6 m de alto cuya cara es el plano X = 200, mirando a -X.
	const FVector CentroPared(250.f, 0.f, 300.f);
	const FVector TamanoPared(100.f, 2000.f, 600.f);
	const FVector NormalPared(-1.f, 0.f, 0.f);

	// Radio de cápsula 42: a 10 cm de la cara de la pared, dentro de la distancia de detección por defecto.
	const FVector PosicionJuntoPared(148.f, 0.f, 300.f);
	const FVector PosicionLejosPared(-1000.f, 0.f, 300.f);

	constexpr float ToleranciaVelocidad = 1.f;
	constexpr float ToleranciaGuinadaGrados = 1.f;
	// Tras el impulso se aplica un paso de gravedad.
	constexpr float FraccionMinVelocidadSubida = 0.9f;
	// Velocidad paralela a la pared antes del salto, para el test tangencial.
	constexpr float VelocidadParalelaPared = 400.f;

	USAEVUSAjustesMovimiento* CrearAjustes()
	{
		return NewObject<USAEVUSAjustesMovimiento>(GetTransientPackage());
	}

	/** Mundo con el suelo y la pared, y un personaje cayendo en Posicion. */
	ASAEVUSPersonaje* Preparar(SAEVUSPruebas::FMundoPrueba& MundoPrueba, const FVector& Posicion, const USAEVUSAjustesMovimiento* Ajustes = nullptr)
	{
		MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
		MundoPrueba.CrearBloque(CentroPared, TamanoPared);
		return MundoPrueba.CrearPersonaje(Posicion, Ajustes);
	}

	/** Pulsa salto en el aire y avanza un paso; cierto si el personaje salió despedido hacia arriba. */
	bool SaltarEnAire(SAEVUSPruebas::FMundoPrueba& MundoPrueba, ASAEVUSPersonaje* Personaje)
	{
		Personaje->PedirSalto();
		MundoPrueba.Paso();
		return Personaje->ObtenerMovimiento()->Velocity.Z > 0.f;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaSaltoParedBasico, "SAEVUS.Movimiento.SaltoPared.Basico", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaSaltoParedBasico::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasSaltoPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, PosicionJuntoPared);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	const FSAEVUSAjusteSaltoPared& Ajuste = Movimiento->ObtenerAjusteSaltoPared();

	TestTrue(TEXT("Encuentra la pared junto al personaje"), Movimiento->BuscarNormalSaltoPared().Equals(NormalPared, 0.01f));
	if (!TestTrue(TEXT("Saltar en el aire junto a una pared impulsa"), SaltarEnAire(MundoPrueba, Personaje)))
	{
		return false;
	}
	TestEqual(TEXT("Empujado lejos de la pared a VelocidadAlejarse"), static_cast<float>(FVector::DotProduct(Movimiento->Velocity, NormalPared)), Ajuste.VelocidadAlejarse, ToleranciaVelocidad);
	TestTrue(TEXT("Velocidad vertical de VelocidadSubida"),
		Movimiento->Velocity.Z > Ajuste.VelocidadSubida * FraccionMinVelocidadSubida && Movimiento->Velocity.Z <= Ajuste.VelocidadSubida);
	TestEqual(TEXT("El cuerpo mira lejos de la pared"), FMath::Abs(static_cast<float>(Personaje->GetActorRotation().Yaw)), 180.f, ToleranciaGuinadaGrados);
	TestEqual(TEXT("Gastado un salto de pared"), Movimiento->SaltosParedRestantes(), Ajuste.MaxSaltosPared - 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaSaltoParedSinPared, "SAEVUS.Movimiento.SaltoPared.SinPared", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaSaltoParedSinPared::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasSaltoPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, PosicionLejosPared);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	TestTrue(TEXT("No hay pared al alcance"), Movimiento->BuscarNormalSaltoPared().IsNearlyZero());
	TestFalse(TEXT("Saltar en el aire libre no hace nada"), SaltarEnAire(MundoPrueba, Personaje));
	TestEqual(TEXT("No se gasta salto de pared"), Movimiento->SaltosParedRestantes(), Movimiento->ObtenerAjusteSaltoPared().MaxSaltosPared);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaSaltoParedNoDesdeBorde, "SAEVUS.Movimiento.SaltoPared.NoDesdeBorde", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaSaltoParedNoDesdeBorde::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasSaltoPared;

	// Plataforma de 10 m, superficie en Z = 0, borde +X en X = 500, nada debajo. Sin tiempo coyote,
	// así solo un salto de pared podría lanzar al personaje tras salir caminando.
	const FVector CentroPlataforma(0.f, 0.f, -50.f);
	const FVector TamanoPlataforma(1000.f, 1000.f, 100.f);
	const FVector PosicionDePie(300.f, 0.f, 150.f);
	constexpr float TiempoMaxSalirBorde = 2.f;

	USAEVUSAjustesMovimiento* Ajustes = CrearAjustes();
	Ajustes->Aire.TiempoCoyote = 0.f;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie, Ajustes);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	if (!MundoPrueba.AsentarEnSuelo())
	{
		AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
		return false;
	}
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxSalirBorde) && !Movimiento->IsFalling(); ++Indice)
	{
		Personaje->AddMovementInput(FVector::ForwardVector, 1.f);
		MundoPrueba.Paso();
	}
	if (!TestTrue(TEXT("Salió del borde"), Movimiento->IsFalling()))
	{
		return false;
	}
	TestTrue(TEXT("El borde del que acaba de salir no es una pared"), Movimiento->BuscarNormalSaltoPared().IsNearlyZero());
	TestFalse(TEXT("No hay salto de pared desde el borde"), SaltarEnAire(MundoPrueba, Personaje));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaSaltoParedTangencial, "SAEVUS.Movimiento.SaltoPared.Tangencial", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaSaltoParedTangencial::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasSaltoPared;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, PosicionJuntoPared);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	Movimiento->Velocity = FVector(0.f, VelocidadParalelaPared, 0.f);
	if (!TestTrue(TEXT("Salto de pared"), SaltarEnAire(MundoPrueba, Personaje)))
	{
		return false;
	}
	TestEqual(TEXT("Conserva FraccionVelocidadTangencial de la velocidad paralela a la pared"), static_cast<float>(Movimiento->Velocity.Y),
		VelocidadParalelaPared * Movimiento->ObtenerAjusteSaltoPared().FraccionVelocidadTangencial, ToleranciaVelocidad);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaSaltoParedPresupuesto, "SAEVUS.Movimiento.SaltoPared.Presupuesto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaSaltoParedPresupuesto::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasSaltoPared;

	USAEVUSAjustesMovimiento* Ajustes = CrearAjustes();
	Ajustes->SaltoPared.MaxSaltosPared = 1;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, PosicionJuntoPared, Ajustes);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	TestTrue(TEXT("Primer salto de pared"), SaltarEnAire(MundoPrueba, Personaje));

	// De vuelta contra la pared, todavía en el aire.
	Personaje->SetActorLocation(PosicionJuntoPared);
	Movimiento->Velocity = FVector::ZeroVector;
	TestFalse(TEXT("Segundo salto de pared rechazado (presupuesto 1)"), SaltarEnAire(MundoPrueba, Personaje));

	if (!TestTrue(TEXT("Aterriza"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	TestEqual(TEXT("Aterrizar recarga los saltos de pared"), Movimiento->SaltosParedRestantes(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaSaltoParedRecargaDash, "SAEVUS.Movimiento.SaltoPared.RecargaDashAereo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaSaltoParedRecargaDash::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasSaltoPared;

	for (const bool bRecarga : { true, false })
	{
		USAEVUSAjustesMovimiento* Ajustes = CrearAjustes();
		Ajustes->SaltoPared.bRecargaDashesAereos = bRecarga;

		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = Preparar(MundoPrueba, PosicionJuntoPared, Ajustes);
		USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

		Movimiento->PedirDash();
		MundoPrueba.Paso();
		TestEqual(TEXT("Dash aéreo gastado"), Movimiento->DashesAereosRestantes(), 0);

		TestTrue(TEXT("Salto de pared durante el dash aéreo"), SaltarEnAire(MundoPrueba, Personaje));
		TestFalse(TEXT("El salto de pared termina el dash"), Movimiento->EstaEnDash());
		TestEqual(bRecarga ? TEXT("Con recarga: vuelven los dashes aéreos") : TEXT("Sin recarga: los dashes aéreos siguen gastados"),
			Movimiento->DashesAereosRestantes(), bRecarga ? Ajustes->Dash.MaxDashesAereos : 0);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
