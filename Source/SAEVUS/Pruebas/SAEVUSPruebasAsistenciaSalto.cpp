// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/CapsuleComponent.h"
#include "Movimiento/SAEVUSAjustesMovimiento.h"

namespace SAEVUSPruebasAsistenciaSalto
{
	// Plataforma de 10 m x 10 m: superficie en Z = 0, borde +X en X = 500, vacío detrás.
	const FVector CentroPlataforma(0.f, 0.f, -50.f);
	const FVector TamanoPlataforma(1000.f, 1000.f, 100.f);

	// Se suelta un poco por encima de la plataforma, así aterriza y empieza de pie.
	const FVector PosicionDePie(300.f, 0.f, 150.f);
	// Se suelta desde más alto, para pulsar salto mientras cae.
	const FVector PosicionCaida(0.f, 0.f, 250.f);

	constexpr float TiempoMaxSalirBorde = 2.f;
	constexpr float TiempoMaxCaida = 2.f;
	constexpr float TiempoComprobarAterrizaje = 0.5f;

	// La pulsación en buffer ocurre así de cerca del suelo: bien dentro de la ventana por defecto.
	constexpr float AlturaPulsacionBuffer = 15.f;

	// Un salto real despega a JumpZVelocity; aterrizar nunca produce más que una fracción.
	constexpr float FraccionDeteccionSalto = 0.5f;

	USAEVUSAjustesMovimiento* CrearAjustes()
	{
		return NewObject<USAEVUSAjustesMovimiento>(GetTransientPackage());
	}

	/** Camina hacia +X hasta que el personaje sale del borde de la plataforma. */
	bool SalirDelBorde(SAEVUSPruebas::FMundoPrueba& MundoPrueba, ASAEVUSPersonaje* Personaje)
	{
		const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxSalirBorde) && !Movimiento->IsFalling(); ++Indice)
		{
			Personaje->AddMovementInput(FVector::ForwardVector, 1.f);
			MundoPrueba.Paso();
		}
		return Movimiento->IsFalling();
	}

	bool HaSaltado(const USAEVUSComponenteMovimiento* Movimiento)
	{
		return Movimiento->Velocity.Z > Movimiento->JumpZVelocity * FraccionDeteccionSalto;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTiempoCoyote, "SAEVUS.Movimiento.AsistenciaSalto.TiempoCoyote", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTiempoCoyote::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAsistenciaSalto;

	// Devuelve cierto si un salto pulsado justo tras salir del borde lanza al personaje.
	auto SaltoTrasSalirDelBorde = [this](const USAEVUSAjustesMovimiento* Ajustes) -> bool
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
		ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie, Ajustes);
		if (!MundoPrueba.AsentarEnSuelo())
		{
			AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
			return false;
		}
		if (!SalirDelBorde(MundoPrueba, Personaje))
		{
			AddError(TEXT("El personaje nunca salió del borde"));
			return false;
		}
		TestTrue(TEXT("Salir del borde abre la ventana coyote cuando está activa"),
			Personaje->ObtenerMovimiento()->EnVentanaCoyote() == (Personaje->ObtenerMovimiento()->ObtenerAjusteAire().TiempoCoyote > 0.f));

		Personaje->PedirSalto();
		MundoPrueba.Paso();
		return HaSaltado(Personaje->ObtenerMovimiento());
	};

	TestTrue(TEXT("Saltar justo tras salir del borde está permitido"), SaltoTrasSalirDelBorde(CrearAjustes()));

	USAEVUSAjustesMovimiento* SinCoyote = CrearAjustes();
	SinCoyote->Aire.TiempoCoyote = 0.f;
	TestFalse(TEXT("Sin tiempo coyote, el mismo salto se rechaza"), SaltoTrasSalirDelBorde(SinCoyote));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaBufferSalto, "SAEVUS.Movimiento.AsistenciaSalto.BufferSalto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaBufferSalto::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAsistenciaSalto;

	// Devuelve cierto si un salto pulsado justo antes de tocar suelo se ejecuta al aterrizar.
	auto SaltoAntesDeAterrizar = [this](const USAEVUSAjustesMovimiento* Ajustes) -> bool
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
		ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionCaida, Ajustes);
		const USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
		const float MediaAltura = Personaje->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxCaida); ++Indice)
		{
			if (Personaje->GetActorLocation().Z - MediaAltura < AlturaPulsacionBuffer)
			{
				break;
			}
			MundoPrueba.Paso();
		}
		if (!Movimiento->IsFalling())
		{
			AddError(TEXT("El personaje no estaba cayendo al pulsar salto"));
			return false;
		}

		Personaje->PedirSalto();
		Personaje->StopJumping();

		bool bAterrizo = false;
		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoComprobarAterrizaje); ++Indice)
		{
			MundoPrueba.Paso();
			bAterrizo |= Movimiento->IsMovingOnGround();
			if (bAterrizo && HaSaltado(Movimiento))
			{
				return true;
			}
		}
		TestTrue(TEXT("El personaje aterrizó"), bAterrizo);
		return false;
	};

	TestTrue(TEXT("Un salto pulsado justo antes de aterrizar se ejecuta al tocar suelo"), SaltoAntesDeAterrizar(CrearAjustes()));

	USAEVUSAjustesMovimiento* SinBuffer = CrearAjustes();
	SinBuffer->Aire.TiempoBufferSalto = 0.f;
	TestFalse(TEXT("Sin buffer de salto, la pulsación temprana se pierde"), SaltoAntesDeAterrizar(SinBuffer));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaBloqueoPasoAire, "SAEVUS.Movimiento.AsistenciaSalto.BloqueoPasoAire", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaBloqueoPasoAire::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAsistenciaSalto;

	for (const bool bBloquear : { true, false })
	{
		USAEVUSAjustesMovimiento* Ajustes = CrearAjustes();
		Ajustes->Aire.bBloquearPasoEnAire = bBloquear;

		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
		ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie, Ajustes);
		USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
		if (!MundoPrueba.AsentarEnSuelo())
		{
			AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
			continue;
		}

		Personaje->PedirSalto();
		MundoPrueba.Paso();
		TestTrue(TEXT("Salto desde el suelo"), Movimiento->IsFalling() && HaSaltado(Movimiento));
		TestFalse(TEXT("Un despegue con salto no abre la ventana coyote"), Movimiento->EnVentanaCoyote());

		Movimiento->PedirEsprint(true);
		const ESAEVUSPaso PasoEsperado = bBloquear ? ESAEVUSPaso::Correr : ESAEVUSPaso::Esprint;
		TestEqual(bBloquear ? TEXT("Bloqueado: esprint en el aire mantiene el tope de carrera") : TEXT("Sin bloqueo: esprint en el aire sube el tope"),
			Movimiento->GetMaxSpeed(), Movimiento->ObtenerVelocidadPaso(PasoEsperado));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
