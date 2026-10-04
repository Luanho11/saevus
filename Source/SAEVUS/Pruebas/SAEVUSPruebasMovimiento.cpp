// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameFramework/PlayerController.h"

#include "Movimiento/SAEVUSAjustesMovimiento.h"

namespace SAEVUSPruebasMovimiento
{
	// Muy por encima de un mundo vacío, así el personaje está en el aire sin nada donde aterrizar.
	const FVector PosicionAerea(0.f, 0.f, 10000.f);

	// Error de integración permitido frente a la expectativa analítica.
	constexpr float ToleranciaCimaSalto = 0.02f;
	constexpr float ToleranciaControlAire = 0.1f;
	constexpr float DuracionControlAire = 0.5f;
	constexpr float TiempoMaxSimulacionSalto = 3.f;
}

using namespace SAEVUSPruebas;
using namespace SAEVUSPruebasMovimiento;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaPrioridadPaso, "SAEVUS.Movimiento.Paso.Prioridad", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaPrioridadPaso::RunTest(const FString& Parameters)
{
	USAEVUSComponenteMovimiento* Movimiento = NewObject<USAEVUSComponenteMovimiento>(GetTransientPackage());

	TestEqual(TEXT("Sin petición, corre"), Movimiento->ObtenerPasoActual(), ESAEVUSPaso::Correr);

	Movimiento->PedirCaminar(true);
	TestEqual(TEXT("Pedir caminar camina"), Movimiento->ObtenerPasoActual(), ESAEVUSPaso::Caminar);

	Movimiento->PedirEsprint(true);
	TestEqual(TEXT("El esprint manda sobre caminar"), Movimiento->ObtenerPasoActual(), ESAEVUSPaso::Esprint);

	Movimiento->PedirCaminar(false);
	Movimiento->PedirEsprint(false);
	TestEqual(TEXT("Soltando ambos, corre"), Movimiento->ObtenerPasoActual(), ESAEVUSPaso::Correr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaVelocidadesPaso, "SAEVUS.Movimiento.Paso.Velocidades", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaVelocidadesPaso::RunTest(const FString& Parameters)
{
	USAEVUSComponenteMovimiento* Movimiento = NewObject<USAEVUSComponenteMovimiento>(GetTransientPackage());
	const FSAEVUSAjusteMovimientoSuelo& Ajuste = Movimiento->ObtenerAjusteSuelo();

	TestEqual(TEXT("Velocidad al caminar"), Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Caminar), Ajuste.VelocidadCaminar);
	TestEqual(TEXT("Velocidad al correr"), Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Correr), Ajuste.VelocidadCorrer);
	TestEqual(TEXT("Velocidad de esprint"), Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Esprint), Ajuste.VelocidadEsprint);
	TestTrue(TEXT("Los pasos por defecto van ordenados caminar < correr < esprint"),
		Ajuste.VelocidadCaminar < Ajuste.VelocidadCorrer && Ajuste.VelocidadCorrer < Ajuste.VelocidadEsprint);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAplicarAjustes, "SAEVUS.Movimiento.Ajustes.Aplicar", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAplicarAjustes::RunTest(const FString& Parameters)
{
	USAEVUSAjustesMovimiento* Ajustes = NewObject<USAEVUSAjustesMovimiento>(GetTransientPackage());
	Ajustes->Suelo.VelocidadCorrer = Ajustes->Suelo.VelocidadCorrer + 1.f;
	Ajustes->Suelo.AceleracionMaxima = Ajustes->Suelo.AceleracionMaxima + 1.f;
	Ajustes->Aire.VelocidadSalto = Ajustes->Aire.VelocidadSalto + 1.f;
	Ajustes->Aire.EscalaGravedad = Ajustes->Aire.EscalaGravedad + 1.f;
	Ajustes->Aire.ControlAire = Ajustes->Aire.ControlAire * 0.5f;

	USAEVUSComponenteMovimiento* Movimiento = NewObject<USAEVUSComponenteMovimiento>(GetTransientPackage());
	Movimiento->FijarAjustesMovimiento(Ajustes);

	TestEqual(TEXT("MaxWalkSpeed refleja la velocidad al correr"), Movimiento->MaxWalkSpeed, Ajustes->Suelo.VelocidadCorrer);
	TestEqual(TEXT("MaxAcceleration"), Movimiento->MaxAcceleration, Ajustes->Suelo.AceleracionMaxima);
	TestEqual(TEXT("JumpZVelocity"), Movimiento->JumpZVelocity, Ajustes->Aire.VelocidadSalto);
	TestEqual(TEXT("GravityScale"), Movimiento->GravityScale, Ajustes->Aire.EscalaGravedad);
	TestEqual(TEXT("AirControl"), Movimiento->AirControl, Ajustes->Aire.ControlAire);

	Movimiento->FijarAjustesMovimiento(nullptr);
	TestEqual(TEXT("Ajustes nulos vuelven a los valores del código"), Movimiento->JumpZVelocity, FSAEVUSAjusteMovimientoAire().VelocidadSalto);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCimaSalto, "SAEVUS.Movimiento.Aire.CimaSalto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCimaSalto::RunTest(const FString& Parameters)
{
	FMundoPrueba MundoPrueba;
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionAerea);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

	// El mismo impulso que aplica el personaje al saltar (salto de altura fija: sin tiempo de mantener).
	Movimiento->Velocity.Z = Movimiento->JumpZVelocity;
	const float ZInicio = Personaje->GetActorLocation().Z;
	float ZCima = ZInicio;

	for (int32 Indice = 0; Indice < PasosPara(TiempoMaxSimulacionSalto) && Movimiento->Velocity.Z >= 0.f; ++Indice)
	{
		MundoPrueba.Paso();
		ZCima = FMath::Max(ZCima, Personaje->GetActorLocation().Z);
	}

	TestTrue(TEXT("El personaje está cayendo"), Movimiento->IsFalling());
	const float Gravedad = FMath::Abs(Movimiento->GetGravityZ());
	const float AlturaEsperada = FMath::Square(Movimiento->JumpZVelocity) / (2.f * Gravedad);
	const float Altura = ZCima - ZInicio;
	AddInfo(FString::Printf(TEXT("Cima del salto %.1f cm, esperada %.1f cm"), Altura, AlturaEsperada));
	TestNearlyEqual(TEXT("La altura de la cima coincide con v^2 / 2g"), Altura, AlturaEsperada, AlturaEsperada * ToleranciaCimaSalto);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaControlAire, "SAEVUS.Movimiento.Aire.ControlAire", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaControlAire::RunTest(const FString& Parameters)
{
	auto MedirVelocidadAerea = [this](const USAEVUSAjustesMovimiento* Ajustes, float& OutEsperada) -> float
	{
		FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionAerea, Ajustes);
		USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();

		const int32 Pasos = PasosPara(DuracionControlAire);
		for (int32 Indice = 0; Indice < Pasos; ++Indice)
		{
			Personaje->AddMovementInput(FVector::ForwardVector, 1.f);
			MundoPrueba.Paso();
		}

		const FSAEVUSAjusteMovimientoAire& Aire = Movimiento->ObtenerAjusteAire();
		const float Duracion = Pasos * PasoSimulacion;
		OutEsperada = FMath::Min(Movimiento->ObtenerAjusteSuelo().AceleracionMaxima * Aire.ControlAire * Duracion, Movimiento->GetMaxSpeed());
		return Movimiento->Velocity.Size2D();
	};

	float Esperada = 0.f;
	const float Velocidad = MedirVelocidadAerea(nullptr, Esperada);
	AddInfo(FString::Printf(TEXT("Velocidad aérea tras %.2f s de entrada: %.1f cm/s, esperada %.1f cm/s"), DuracionControlAire, Velocidad, Esperada));
	TestNearlyEqual(TEXT("El control aéreo acelera a AceleracionMaxima * ControlAire"), Velocidad, Esperada, Esperada * ToleranciaControlAire);

	USAEVUSAjustesMovimiento* SinControlAire = NewObject<USAEVUSAjustesMovimiento>(GetTransientPackage());
	SinControlAire->Aire.ControlAire = 0.f;
	const float VelocidadBloqueada = MedirVelocidadAerea(SinControlAire, Esperada);
	TestNearlyEqual(TEXT("Sin control aéreo se conserva la velocidad del despegue"), VelocidadBloqueada, 0.f, KINDA_SMALL_NUMBER);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaMirarHaciaCamara, "SAEVUS.Movimiento.MirarHaciaCamara", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaMirarHaciaCamara::RunTest(const FString& Parameters)
{
	constexpr float GuinadaCamara = 90.f;
	constexpr float ToleranciaGrados = 2.f;
	constexpr float TiempoGiro = 0.5f;
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(FVector(0.f, 0.f, -50.f), FVector(4000.f, 4000.f, 100.f));
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(FVector(0.f, 0.f, 150.f));
	if (!TestTrue(TEXT("De pie"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	// Sin jugador local el controlador no cuenta como local y el movimiento no consumiría la entrada.
	APlayerController* Controlador = MundoPrueba.ObtenerMundo()->SpawnActor<APlayerController>();
	Controlador->SetAsLocalPlayerController();
	// Al poseerlo, el personaje avisa de que no tiene configuración de entrada (aquí no hace falta).
	AddExpectedMessagePlain(TEXT("sin ConfigEntrada asignada"), ELogVerbosity::Error, EAutomationExpectedMessageFlags::Contains, 0);
	Controlador->Possess(Personaje);
	Controlador->SetControlRotation(FRotator(0.f, GuinadaCamara, 0.f));
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	TestTrue(TEXT("Con controlador el cuerpo sigue a la cámara"), Movimiento->MiraHaciaCamara());
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoGiro); ++Indice)
	{
		MundoPrueba.Paso();
	}
	TestEqual(TEXT("Mira hacia donde mira la cámara"), static_cast<float>(Personaje->GetActorRotation().Yaw), GuinadaCamara, ToleranciaGrados);

	// Moverse hacia atrás (respecto a la cámara) y hacer dash atrás no gira el cuerpo.
	const FVector Atras = -FRotator(0.f, GuinadaCamara, 0.f).Vector();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoGiro); ++Indice)
	{
		Personaje->AddMovementInput(Atras, 1.f);
		MundoPrueba.Paso();
	}
	TestTrue(TEXT("Se mueve hacia atrás"), FVector::DotProduct(Movimiento->Velocity.GetSafeNormal2D(), Atras) > 0.9f);
	TestEqual(TEXT("Andando hacia atrás sigue mirando a la cámara"), static_cast<float>(Personaje->GetActorRotation().Yaw), GuinadaCamara, ToleranciaGrados);
	Personaje->PedirDashHacia(Atras);
	MundoPrueba.Paso();
	TestTrue(TEXT("Dash atrás"), Movimiento->EstaEnDash());
	TestEqual(TEXT("El dash atrás no gira el cuerpo"), static_cast<float>(Personaje->GetActorRotation().Yaw), GuinadaCamara, ToleranciaGrados);
	Controlador->UnPossess();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
