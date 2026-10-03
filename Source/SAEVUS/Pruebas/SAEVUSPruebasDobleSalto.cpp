// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/CapsuleComponent.h"
#include "Movimiento/SAEVUSAjustesMovimiento.h"

namespace SAEVUSPruebasDobleSalto
{
	// Suelo de 40 m x 40 m, superficie en Z = 0.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);
	// Desde aquí cae lo bastante para pulsar salto justo antes de tocar suelo.
	const FVector PosicionCaida(0.f, 0.f, 400.f);

	// Pared de 6 m cuya cara es el plano X = 200 (como en los tests de salto de pared).
	const FVector CentroPared(250.f, 0.f, 300.f);
	const FVector TamanoPared(100.f, 2000.f, 600.f);
	const FVector PosicionJuntoPared(148.f, 0.f, 300.f);

	constexpr float TiempoAntesDobleSalto = 0.2f;
	constexpr float TiempoMaxAterrizar = 3.f;
	// Tras el impulso se aplica un paso de gravedad.
	constexpr float FraccionMinVelocidad = 0.9f;
	constexpr float AlturaPulsacionBuffer = 15.f;

	/** Avanza hasta tocar suelo; cierto si aterrizó. */
	bool HastaAterrizar(SAEVUSPruebas::FMundoPrueba& MundoPrueba, const USAEVUSComponenteMovimiento* Movimiento)
	{
		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAterrizar) && !Movimiento->IsMovingOnGround(); ++Indice)
		{
			MundoPrueba.Paso();
		}
		return Movimiento->IsMovingOnGround();
	}

	/** Suelta el botón un paso y vuelve a pulsar salto, como un jugador. */
	void PulsarSaltoDeNuevo(SAEVUSPruebas::FMundoPrueba& MundoPrueba, ASAEVUSPersonaje* Personaje)
	{
		Personaje->StopJumping();
		MundoPrueba.Paso();
		Personaje->PedirSalto();
		MundoPrueba.Paso();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDobleSaltoBasico, "SAEVUS.Movimiento.DobleSalto.Basico", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDobleSaltoBasico::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDobleSalto;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	if (!MundoPrueba.AsentarEnSuelo())
	{
		AddError(TEXT("El personaje nunca aterrizó"));
		return false;
	}
	const FSAEVUSAjusteMovimientoAire& Aire = Movimiento->ObtenerAjusteAire();

	Personaje->PedirSalto();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoAntesDobleSalto); ++Indice)
	{
		MundoPrueba.Paso();
	}
	if (!TestTrue(TEXT("En el aire tras el salto de suelo"), Movimiento->IsFalling()))
	{
		return false;
	}
	TestEqual(TEXT("El salto de suelo no gasta el doble salto"), Movimiento->SaltosAereosRestantes(), Aire.MaxSaltosAereos);

	PulsarSaltoDeNuevo(MundoPrueba, Personaje);
	AddInfo(FString::Printf(TEXT("Velocidad vertical tras el doble salto %.0f cm/s"), Movimiento->Velocity.Z));
	TestTrue(TEXT("El doble salto fija la velocidad de subida"), Movimiento->Velocity.Z > Aire.VelocidadDobleSalto * FraccionMinVelocidad
		&& Movimiento->Velocity.Z <= Aire.VelocidadDobleSalto);
	TestEqual(TEXT("Doble salto gastado"), Movimiento->SaltosAereosRestantes(), Aire.MaxSaltosAereos - 1);

	// Sin dobles saltos restantes, otra pulsación no hace nada.
	const float VelocidadAntes = Movimiento->Velocity.Z;
	PulsarSaltoDeNuevo(MundoPrueba, Personaje);
	TestTrue(TEXT("Sin dobles saltos, otra pulsación no impulsa"), Movimiento->Velocity.Z < VelocidadAntes);

	if (!TestTrue(TEXT("Aterriza"), HastaAterrizar(MundoPrueba, Movimiento)))
	{
		return false;
	}
	TestEqual(TEXT("Aterrizar devuelve el doble salto"), Movimiento->SaltosAereosRestantes(), Aire.MaxSaltosAereos);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDobleSaltoCercaSuelo, "SAEVUS.Movimiento.DobleSalto.CercaDelSueloGanaBuffer", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDobleSaltoCercaSuelo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDobleSalto;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionCaida);
	const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	const float MediaAltura = Personaje->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAterrizar); ++Indice)
	{
		if (Personaje->GetActorLocation().Z - MediaAltura < AlturaPulsacionBuffer)
		{
			break;
		}
		MundoPrueba.Paso();
	}
	if (!TestTrue(TEXT("Cayendo al pulsar"), Movimiento->IsFalling()))
	{
		return false;
	}
	Personaje->PedirSalto();
	MundoPrueba.Paso();
	TestEqual(TEXT("Cerca del suelo no se gasta el doble salto"), Movimiento->SaltosAereosRestantes(), Movimiento->ObtenerAjusteAire().MaxSaltosAereos);

	// El buffer ejecuta un salto de suelo completo al aterrizar.
	bool bSaltoSuelo = false;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoAntesDobleSalto) && !bSaltoSuelo; ++Indice)
	{
		MundoPrueba.Paso();
		bSaltoSuelo = Movimiento->Velocity.Z > Movimiento->JumpZVelocity * FraccionMinVelocidad;
	}
	TestTrue(TEXT("El buffer da el salto de suelo"), bSaltoSuelo);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDobleSaltoRecargaPared, "SAEVUS.Movimiento.DobleSalto.RecargaSaltoPared", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDobleSaltoRecargaPared::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDobleSalto;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	MundoPrueba.CrearBloque(CentroPared, TamanoPared);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionJuntoPared);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	const int32 Maximo = Movimiento->ObtenerAjusteAire().MaxSaltosAereos;

	// Junto a la pared, la pulsación es un salto de pared: no gasta el doble salto.
	Personaje->PedirSalto();
	MundoPrueba.Paso();
	TestEqual(TEXT("El salto de pared tiene prioridad"), Movimiento->SaltosParedRestantes(), Movimiento->ObtenerAjusteSaltoPared().MaxSaltosPared - 1);
	TestEqual(TEXT("El doble salto sigue disponible"), Movimiento->SaltosAereosRestantes(), Maximo);

	// Gasta el doble salto lejos de la pared.
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoAntesDobleSalto); ++Indice)
	{
		MundoPrueba.Paso();
	}
	PulsarSaltoDeNuevo(MundoPrueba, Personaje);
	TestEqual(TEXT("Doble salto gastado"), Movimiento->SaltosAereosRestantes(), Maximo - 1);

	Movimiento->RecargarRecursosAereos();
	TestEqual(TEXT("La recarga devuelve el doble salto"), Movimiento->SaltosAereosRestantes(), Maximo);
	TestEqual(TEXT("La recarga devuelve el dash aéreo"), Movimiento->DashesAereosRestantes(), Movimiento->ObtenerAjusteDash().MaxDashesAereos);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
