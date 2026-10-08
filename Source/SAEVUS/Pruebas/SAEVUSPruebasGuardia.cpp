// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Armas/SAEVUSTiposArma.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"

namespace SAEVUSPruebasGuardia
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionJugador(0.f, 0.f, 150.f);
	constexpr float Danio = 20.f;
	constexpr float DistanciaCausante = 200.f;
	constexpr float TiempoMovimiento = 0.5f;
	constexpr float TiempoMaxAtaque = 2.f;
	constexpr float Tolerancia = 0.01f;

	struct FMundoGuardia
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Jugador = nullptr;
		USAEVUSComponenteGuardia* Guardia = nullptr;
		bool bListo = false;

		FMundoGuardia()
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
			Guardia = Jugador->ObtenerGuardia();
			bListo = MundoPrueba.AsentarEnSuelo();
			// Las reglas de la guardia se prueban sin la invulnerabilidad tras golpe.
			Jugador->ObtenerVida()->FijarInvulnerabilidadTrasGolpe(0.f);
		}

		/** Un muñeco como causante del daño, a DistanciaCausante en la dirección dada. */
		AActor* CrearCausante(const FVector& Direccion)
		{
			return MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(Jugador->GetActorLocation() + Direccion * DistanciaCausante);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGuardiaBloqueo, "SAEVUS.Guardia.BloqueaDeFrente", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGuardiaBloqueo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasGuardia;

	FMundoGuardia Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	USAEVUSComponenteVida* Vida = Mundo.Jugador->ObtenerVida();
	AActor* Delante = Mundo.CrearCausante(Mundo.Jugador->GetActorForwardVector());
	AActor* Detras = Mundo.CrearCausante(-Mundo.Jugador->GetActorForwardVector());

	TestEqual(TEXT("Sin guardia, el golpe de frente entra"), Vida->AplicarDanio(Danio, Delante), Danio);
	Mundo.Jugador->FijarGuardia(true);
	// Pasada la ventana de contraataque (tiene su propio test): bloqueo normal.
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMovimiento); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	if (!TestTrue(TEXT("En guardia"), Mundo.Guardia->EstaEnGuardia()))
	{
		return false;
	}
	const float VidaAntes = Vida->ObtenerVida();
	TestEqual(TEXT("En guardia, el golpe de frente se para"), Vida->AplicarDanio(Danio, Delante), 0.f);
	TestEqual(TEXT("La vida no cambia"), Vida->ObtenerVida(), VidaAntes);
	TestEqual(TEXT("Cuenta el bloqueo"), Mundo.Guardia->ObtenerBloqueos(), 1);
	TestEqual(TEXT("Por la espalda la guardia no sirve"), Vida->AplicarDanio(Danio, Detras), Danio);
	TestEqual(TEXT("Sin causante (caídas) tampoco"), Vida->AplicarDanio(Danio, nullptr), Danio);

	Mundo.Jugador->FijarGuardia(false);
	Mundo.MundoPrueba.Paso();
	TestFalse(TEXT("Soltar sale de la guardia"), Mundo.Guardia->EstaEnGuardia());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGuardiaMovimiento, "SAEVUS.Guardia.MovimientoYArma", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGuardiaMovimiento::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasGuardia;

	FMundoGuardia Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	USAEVUSComponenteMovimiento* Movimiento = Mundo.Jugador->ObtenerMovimiento();
	const float VelocidadNormal = Movimiento->GetMaxSpeed();
	Mundo.Jugador->FijarGuardia(true);
	Mundo.MundoPrueba.Paso();
	TestTrue(TEXT("En guardia se mueve más despacio"), Movimiento->GetMaxSpeed() < VelocidadNormal * 0.5f);
	Mundo.Jugador->FijarCuracion(true);
	TestFalse(TEXT("En guardia no se cura"), Mundo.Jugador->ObtenerAlma()->EstaCurando());

	// Con un arma de fuego no hay guardia.
	USAEVUSDefinicionArma* Pistola = NewObject<USAEVUSDefinicionArma>(GetTransientPackage());
	Pistola->Tipo = ESAEVUSTipoArma::ArmaFuego;
	Mundo.Jugador->ObtenerArmas()->FijarArsenal({ Pistola });
	Mundo.MundoPrueba.Paso();
	TestFalse(TEXT("Con arma de fuego la guardia se suelta"), Mundo.Guardia->EstaEnGuardia());
	TestEqual(TEXT("Y la velocidad vuelve"), Movimiento->GetMaxSpeed(), VelocidadNormal, Tolerancia);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGuardiaCancelaTajo, "SAEVUS.Guardia.CancelaTajo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGuardiaCancelaTajo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasGuardia;

	FMundoGuardia Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	USAEVUSComponenteCombate* Combate = Mundo.Jugador->ObtenerCombate();
	Mundo.Jugador->PedirAtaque();
	const FSAEVUSDefinicionAtaque* Ataque = Combate->ObtenerAtaqueActual();
	if (!TestNotNull(TEXT("Ataque en curso"), Ataque))
	{
		return false;
	}
	// Pedida al empezar el tajo: espera a su ventana de cancelación.
	Mundo.Jugador->FijarGuardia(true);
	Mundo.MundoPrueba.Paso();
	TestTrue(TEXT("Antes de la ventana el tajo sigue"), Combate->EstaAtacando());
	TestFalse(TEXT("Y la guardia espera"), Mundo.Guardia->EstaEnGuardia());

	float TiempoEntrada = -1.f;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && !Mundo.Guardia->EstaEnGuardia(); ++Indice)
	{
		const float TiempoAntes = Combate->ObtenerTiempoAtaque();
		Mundo.MundoPrueba.Paso();
		if (Mundo.Guardia->EstaEnGuardia())
		{
			TiempoEntrada = TiempoAntes;
		}
	}
	AddInfo(FString::Printf(TEXT("La guardia cortó el tajo en t = %.3f s (ventana %.3f s, total %.3f s)"), TiempoEntrada, Ataque->CancelarConGuardiaDesde, Ataque->ObtenerTiempoTotal()));
	TestTrue(TEXT("En la ventana la guardia corta el tajo"), Mundo.Guardia->EstaEnGuardia() && !Combate->EstaAtacando());
	TestTrue(TEXT("Antes de que el tajo acabe solo"), TiempoEntrada < Ataque->ObtenerTiempoTotal());

	// Atacar desde la guardia la suelta; mantenida, vuelve tras el tajo.
	Mundo.Jugador->PedirAtaque();
	TestTrue(TEXT("Atacar desde la guardia"), Combate->EstaAtacando() && !Mundo.Guardia->EstaEnGuardia());
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && !Mundo.Guardia->EstaEnGuardia(); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	TestTrue(TEXT("Mantenida, la guardia vuelve"), Mundo.Guardia->EstaEnGuardia());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGuardiaContraataque, "SAEVUS.Guardia.Contraataque", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGuardiaContraataque::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasGuardia;

	FMundoGuardia Mundo;
	if (!TestTrue(TEXT("Asentado"), Mundo.bListo))
	{
		return false;
	}
	// Un enemigo delante, lejos para que no ataque por su cuenta: el golpe se aplica a mano.
	ASAEVUSEnemigo* Enemigo = Mundo.MundoPrueba.CrearEnemigo(Mundo.Jugador->GetActorLocation() + FVector(DistanciaCausante, 0.f, 0.f), nullptr, nullptr);
	USAEVUSComponenteVida* Vida = Mundo.Jugador->ObtenerVida();

	// El roce de un enemigo que no ataca se bloquea, pero no es contraataque (EA-24).
	Mundo.Jugador->FijarGuardia(true);
	Mundo.MundoPrueba.Paso();
	TestEqual(TEXT("Contacto bloqueado"), Vida->AplicarDanio(Danio, Enemigo), 0.f);
	TestEqual(TEXT("El contacto no es contraataque"), Mundo.Guardia->ObtenerContraataques(), 0);

	// Golpe justo al levantar la guardia, con el enemigo atacando: contraataque.
	Enemigo->ObtenerIA()->ForzarEstado(ESAEVUSEstadoEnemigo::Atacar);
	TestEqual(TEXT("Bloqueado"), Vida->AplicarDanio(Danio, Enemigo), 0.f);
	TestEqual(TEXT("Contraataque"), Mundo.Guardia->ObtenerContraataques(), 1);
	TestEqual(TEXT("El atacante queda aturdido"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Aturdido);
	TestTrue(TEXT("El contraataque da alma"), Mundo.Jugador->ObtenerAlma()->ObtenerAlma() > 0.f);
	TestTrue(TEXT("Y un instante de invulnerabilidad"), Vida->EstaInvulnerable());

	// Con la guardia ya levantada hace rato: bloqueo normal, sin contraataque.
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMovimiento); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	TestEqual(TEXT("Bloqueado tarde"), Vida->AplicarDanio(Danio, Enemigo), 0.f);
	TestEqual(TEXT("Sin contraataque fuera de la ventana"), Mundo.Guardia->ObtenerContraataques(), 1);

	// Pulsar y soltar a ráfagas no reabre la ventana (GAMEPLAY-4): hay que esperar tras soltar.
	Mundo.Jugador->FijarGuardia(false);
	Mundo.MundoPrueba.Paso();
	Mundo.Jugador->FijarGuardia(true);
	Mundo.MundoPrueba.Paso();
	Enemigo->ObtenerIA()->ForzarEstado(ESAEVUSEstadoEnemigo::Atacar);
	TestEqual(TEXT("Bloqueado a ráfagas"), Vida->AplicarDanio(Danio, Enemigo), 0.f);
	TestEqual(TEXT("Sin contraataque al machacar la guardia"), Mundo.Guardia->ObtenerContraataques(), 1);
	Mundo.Jugador->FijarGuardia(false);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(0.5f); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	Mundo.Jugador->FijarGuardia(true);
	Mundo.MundoPrueba.Paso();
	Enemigo->ObtenerIA()->ForzarEstado(ESAEVUSEstadoEnemigo::Atacar);
	TestEqual(TEXT("Tras esperar, otra vez"), Vida->AplicarDanio(Danio, Enemigo), 0.f);
	TestEqual(TEXT("hay contraataque"), Mundo.Guardia->ObtenerContraataques(), 2);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
