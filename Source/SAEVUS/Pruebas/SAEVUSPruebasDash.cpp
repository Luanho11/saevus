// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Armas/SAEVUSTiposArma.h"
#include "Movimiento/SAEVUSAjustesMovimiento.h"

namespace SAEVUSPruebasDash
{
	// Plataforma de 40 m x 40 m, superficie en Z = 0: sitio para hacer dash en cualquier dirección.
	const FVector CentroPlataforma(0.f, 0.f, -50.f);
	const FVector TamanoPlataforma(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);

	// Límite para cualquier dash; el de por defecto dura mucho menos.
	constexpr float TiempoMaxDash = 1.f;

	// La simulación de paso fijo redondea el dash a pasos enteros, así la distancia tiene tolerancia.
	constexpr float ToleranciaDistancia = 0.1f;
	constexpr float ToleranciaVelocidad = 1.f;
	constexpr float ProductoMinDireccion = 0.99f;
	constexpr float ToleranciaGuinadaGrados = 1.f;

	// La gravedad se suspende durante un dash aéreo, así la altura apenas cambia.
	constexpr float CambioMaxAlturaDashAereo = 2.f;
	// Pasos tras el despegue antes del dash aéreo, para estar claramente en el aire.
	constexpr int32 PasosEnAire = 10;

	USAEVUSAjustesMovimiento* CrearAjustes()
	{
		return NewObject<USAEVUSAjustesMovimiento>(GetTransientPackage());
	}

	/** Avanza hasta que termina el dash actual; falso si sigue tras TiempoMaxDash. */
	bool AvanzarHastaFinDash(SAEVUSPruebas::FMundoPrueba& MundoPrueba, const USAEVUSComponenteMovimiento* Movimiento)
	{
		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxDash) && Movimiento->EstaEnDash(); ++Indice)
		{
			MundoPrueba.Paso();
		}
		return !Movimiento->EstaEnDash();
	}

	/** Pulsa dash y avanza un paso; cierto si empezó un dash. */
	bool DashAhora(SAEVUSPruebas::FMundoPrueba& MundoPrueba, USAEVUSComponenteMovimiento* Movimiento)
	{
		Movimiento->PedirDash();
		MundoPrueba.Paso();
		return Movimiento->EstaEnDash();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDashSuelo, "SAEVUS.Movimiento.Dash.Suelo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDashSuelo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDash;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	if (!MundoPrueba.AsentarEnSuelo())
	{
		AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
		return false;
	}
	const FSAEVUSAjusteDash& Ajuste = Movimiento->ObtenerAjusteDash();
	const FVector Inicio = Personaje->GetActorLocation();

	// Sin entrada: el dash va hacia donde mira el personaje (+X al aparecer).
	if (!TestTrue(TEXT("El dash empieza de pie"), DashAhora(MundoPrueba, Movimiento)))
	{
		return false;
	}
	TestEqual(TEXT("La velocidad del dash viene del ajuste"), static_cast<float>(Movimiento->Velocity.Size2D()), Ajuste.Velocidad, ToleranciaVelocidad);
	TestTrue(TEXT("Sin entrada: el dash sigue la orientación"), Movimiento->Velocity.GetSafeNormal2D().X > ProductoMinDireccion);
	TestTrue(TEXT("Sigue en el suelo"), Movimiento->IsMovingOnGround());

	if (!TestTrue(TEXT("El dash termina"), AvanzarHastaFinDash(MundoPrueba, Movimiento)))
	{
		return false;
	}
	const float Distancia = static_cast<float>(FVector::Dist2D(Inicio, Personaje->GetActorLocation()));
	const float DistanciaEsperada = Ajuste.Velocidad * Ajuste.Duracion;
	AddInfo(FString::Printf(TEXT("Distancia del dash %.1f cm, esperada %.1f cm"), Distancia, DistanciaEsperada));
	TestEqual(TEXT("Distancia ~ Velocidad x Duracion"), Distancia, DistanciaEsperada, DistanciaEsperada * ToleranciaDistancia);
	TestTrue(TEXT("Al terminar, la velocidad se limita a VelocidadSalida"), Movimiento->Velocity.Size2D() <= Ajuste.VelocidadSalida + ToleranciaVelocidad);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDashDireccion, "SAEVUS.Movimiento.Dash.Direccion", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDashDireccion::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDash;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	if (!MundoPrueba.AsentarEnSuelo())
	{
		AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
		return false;
	}

	// Entrada lateral mirando a +X: el dash sigue la entrada, no la orientación.
	Personaje->AddMovementInput(FVector::RightVector, 1.f);
	if (!TestTrue(TEXT("El dash empieza"), DashAhora(MundoPrueba, Movimiento)))
	{
		return false;
	}
	TestTrue(TEXT("El dash sigue la dirección de la entrada"), Movimiento->Velocity.GetSafeNormal2D().Y > ProductoMinDireccion);
	TestEqual(TEXT("El cuerpo gira hacia el dash"), static_cast<float>(Personaje->GetActorRotation().Yaw), 90.f, ToleranciaGuinadaGrados);

	// Dirigir a mitad del dash no hace nada: la dirección queda fija todo el dash.
	Personaje->AddMovementInput(FVector::ForwardVector, 1.f);
	MundoPrueba.Paso();
	TestTrue(TEXT("La dirección queda fija durante el dash"), Movimiento->Velocity.GetSafeNormal2D().Y > ProductoMinDireccion);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDashEnfriamiento, "SAEVUS.Movimiento.Dash.Enfriamiento", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDashEnfriamiento::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDash;

	// El enfriamiento sigue disponible por datos (por defecto es 0, como en GunZ).
	constexpr float EnfriamientoPrueba = 0.3f;
	USAEVUSAjustesMovimiento* Ajustes = CrearAjustes();
	Ajustes->Dash.Enfriamiento = EnfriamientoPrueba;
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie, Ajustes);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	if (!MundoPrueba.AsentarEnSuelo())
	{
		AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
		return false;
	}

	TestTrue(TEXT("Empieza el primer dash"), DashAhora(MundoPrueba, Movimiento));
	TestFalse(TEXT("No hay dash nuevo durante un dash"), Movimiento->PuedeDash());
	AvanzarHastaFinDash(MundoPrueba, Movimiento);
	TestFalse(TEXT("Dash rechazado justo después del anterior (enfriamiento)"), DashAhora(MundoPrueba, Movimiento));

	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Movimiento->ObtenerAjusteDash().Enfriamiento); ++Indice)
	{
		MundoPrueba.Paso();
	}
	TestTrue(TEXT("Dash permitido otra vez tras el enfriamiento"), DashAhora(MundoPrueba, Movimiento));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDashPresupuestoAereo, "SAEVUS.Movimiento.Dash.PresupuestoAereo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDashPresupuestoAereo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDash;

	// Presupuesto de un dash aéreo y gravedad suspendida (opciones por datos; por defecto, sin límite).
	USAEVUSAjustesMovimiento* Ajustes = CrearAjustes();
	Ajustes->Dash.Enfriamiento = 0.f;
	Ajustes->Dash.MaxDashesAereos = 1;
	Ajustes->Dash.bSuspenderGravedadEnDash = true;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie, Ajustes);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	if (!MundoPrueba.AsentarEnSuelo())
	{
		AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
		return false;
	}

	Personaje->PedirSalto();
	for (int32 Indice = 0; Indice < PasosEnAire; ++Indice)
	{
		MundoPrueba.Paso();
	}
	if (!TestTrue(TEXT("En el aire tras el salto"), Movimiento->IsFalling()))
	{
		return false;
	}

	const float AlturaAntes = static_cast<float>(Personaje->GetActorLocation().Z);
	TestTrue(TEXT("Empieza el dash aéreo"), DashAhora(MundoPrueba, Movimiento));
	TestEqual(TEXT("Presupuesto de dash aéreo gastado"), Movimiento->DashesAereosRestantes(), Ajustes->Dash.MaxDashesAereos - 1);
	AvanzarHastaFinDash(MundoPrueba, Movimiento);
	const float CambioAltura = FMath::Abs(static_cast<float>(Personaje->GetActorLocation().Z) - AlturaAntes);
	AddInfo(FString::Printf(TEXT("Cambio de altura en el dash aéreo %.2f cm"), CambioAltura));
	TestTrue(TEXT("Gravedad suspendida durante el dash aéreo"), CambioAltura < CambioMaxAlturaDashAereo);

	TestFalse(TEXT("Segundo dash aéreo rechazado (presupuesto 1)"), DashAhora(MundoPrueba, Movimiento));

	if (!TestTrue(TEXT("Vuelve a aterrizar"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	TestEqual(TEXT("Aterrizar recarga los dashes aéreos"), Movimiento->DashesAereosRestantes(), Ajustes->Dash.MaxDashesAereos);
	TestTrue(TEXT("El dash de suelo funciona tras aterrizar"), DashAhora(MundoPrueba, Movimiento));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDashComoGunZ, "SAEVUS.Movimiento.Dash.ComoGunZ", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDashComoGunZ::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDash;

	// Con los ajustes por defecto: un dash detrás de otro sin espera, y varios en el mismo salto.
	constexpr int32 DashesEncadenados = 3;
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	if (!MundoPrueba.AsentarEnSuelo())
	{
		AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
		return false;
	}
	TestTrue(TEXT("Primer dash"), DashAhora(MundoPrueba, Movimiento));
	AvanzarHastaFinDash(MundoPrueba, Movimiento);
	TestTrue(TEXT("Otro dash en el acto, sin enfriamiento"), DashAhora(MundoPrueba, Movimiento));
	AvanzarHastaFinDash(MundoPrueba, Movimiento);

	Personaje->PedirSalto();
	for (int32 Indice = 0; Indice < PasosEnAire; ++Indice)
	{
		MundoPrueba.Paso();
	}
	int32 Hechos = 0;
	for (int32 Dash = 0; Dash < DashesEncadenados && Movimiento->IsFalling(); ++Dash)
	{
		Hechos += DashAhora(MundoPrueba, Movimiento) ? 1 : 0;
		AvanzarHastaFinDash(MundoPrueba, Movimiento);
	}
	AddInfo(FString::Printf(TEXT("Dashes aéreos en un salto: %d"), Hechos));
	TestTrue(TEXT("Más de un dash por salto"), Hechos >= 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDashCancelarSalto, "SAEVUS.Movimiento.Dash.CancelarConSalto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDashCancelarSalto::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDash;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroPlataforma, TamanoPlataforma);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	if (!MundoPrueba.AsentarEnSuelo())
	{
		AddError(TEXT("El personaje nunca aterrizó en la plataforma"));
		return false;
	}

	if (!TestTrue(TEXT("El dash empieza"), DashAhora(MundoPrueba, Movimiento)))
	{
		return false;
	}
	Personaje->PedirSalto();
	MundoPrueba.Paso();
	TestFalse(TEXT("El salto cancela el dash"), Movimiento->EstaEnDash());
	TestTrue(TEXT("El salto ocurrió"), Movimiento->IsFalling() && Movimiento->Velocity.Z > 0.f);
	TestTrue(TEXT("El salto sale con la velocidad de salida del dash, no con la del dash"),
		Movimiento->Velocity.Size2D() <= Movimiento->ObtenerAjusteDash().VelocidadSalida + ToleranciaVelocidad);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDashArmaFuego, "SAEVUS.Movimiento.Dash.SegunArma", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDashArmaFuego::RunTest(const FString& Parameters)
{
	// Con aguijón, el dash normal; con arma de fuego, la voltereta: más lenta, sin estela ni cámara fija.
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(FVector(0.f, 0.f, -50.f), FVector(4000.f, 4000.f, 100.f));
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(FVector(0.f, 0.f, 150.f));
	if (!TestTrue(TEXT("Asentado"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	USAEVUSComponenteMovimiento* Movimiento = Personaje->ObtenerMovimiento();
	const FSAEVUSAjusteDash Voltereta = SAEVUSDash::CrearVoltereta();

	Personaje->PedirDash();
	MundoPrueba.Paso();
	const float VelocidadAguijon = static_cast<float>(Movimiento->Velocity.Size2D());
	TestTrue(TEXT("Con aguijón: dash con estela"), Movimiento->ObtenerAjusteDash().bEstela);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(1.f); ++Indice)
	{
		MundoPrueba.Paso();
	}

	USAEVUSDefinicionArma* Pistola = NewObject<USAEVUSDefinicionArma>(GetTransientPackage());
	Pistola->Tipo = ESAEVUSTipoArma::ArmaFuego;
	Personaje->ObtenerArmas()->FijarArsenal({ Pistola });
	MundoPrueba.Paso();
	Personaje->PedirDash();
	MundoPrueba.Paso();
	AddInfo(FString::Printf(TEXT("Velocidad: aguijón %.0f, arma de fuego %.0f cm/s"), VelocidadAguijon, Movimiento->Velocity.Size2D()));
	TestTrue(TEXT("Con arma de fuego: la voltereta"), Movimiento->EstaEnDash());
	TestEqual(TEXT("A la velocidad de la voltereta"), static_cast<float>(Movimiento->Velocity.Size2D()), Voltereta.Velocidad, 5.f);
	TestTrue(TEXT("Más lenta que el dash con aguijón"), Movimiento->Velocity.Size2D() < VelocidadAguijon);
	TestFalse(TEXT("Sin estela"), Movimiento->ObtenerAjusteDash().bEstela);
	TestFalse(TEXT("Sin cámara fija"), Personaje->CamaraBloqueada());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
