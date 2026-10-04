// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Movimiento/SAEVUSAjustesMovimiento.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/BoxComponent.h"
#include "Tutorial/SAEVUSPeligro.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

/**
 * Los obstáculos de L_Tutorial, con sus medidas (saevus_crear_assets.py, construir_mapa_tutorial),
 * superados con entradas de jugador simuladas. Si se cambia una medida en el mapa o un ajuste de
 * movimiento, estos tests dicen si el tutorial sigue siendo superable.
 */
namespace SAEVUSPruebasObstaculos
{
	constexpr float AnchoPasillo = 800.f;
	constexpr float GrosorSuelo = 100.f;
	constexpr float MediaAlturaJugador = 96.f;
	constexpr float AlturaInicio = 150.f;
	constexpr float TiempoMax = 6.f;
	// Por debajo de esto el jugador ha caído al foso.
	constexpr float ZCaida = -200.f;

	// Medidas del tutorial.
	constexpr float AlturaBloqueDobleSalto = 220.f;
	constexpr float AnchoFosoDash = 800.f;
	constexpr float AnchoFosoRebote = 1200.f;
	const float XMunecosRebote[] = { 400.f, 850.f };
	constexpr float ZMunecosRebote = -60.f;
	constexpr float AnchoFosoCarreraPared = 900.f;
	// Pinchos del foso de rebote: caja de 60 cm de alto con la cara de arriba en Z = -90.
	constexpr float ZCentroPinchos = -120.f;
	constexpr float MediaAlturaPinchos = 30.f;
	constexpr float AlturaAtaquePinchos = 140.f;
	constexpr float AlturaCornisa = 450.f;

	// Pilotaje.
	constexpr float DistanciaSaltoAntesDeCara = 280.f;
	constexpr float MargenBorde = 40.f;
	constexpr float RetardoDashTrasDobleSalto = 0.1f;
	constexpr float AlcanceRebote = 120.f;
	constexpr float YCarreraPared = 300.f;
	constexpr float XSaltoCarreraPared = -100.f;
	constexpr float XSoltarCarreraPared = 780.f;

	/** Suelo de X0 a X1 con la cara superior en Z (ancho del pasillo del tutorial). */
	void CrearSuelo(SAEVUSPruebas::FMundoPrueba& Mundo, float X0, float X1, float Z = 0.f)
	{
		Mundo.CrearBloque(FVector((X0 + X1) * 0.5f, 0.f, Z - GrosorSuelo * 0.5f), FVector(X1 - X0, AnchoPasillo, GrosorSuelo));
	}

	/** Jugador asentado en X (sobre el suelo); nulo si no aterriza. */
	ASAEVUSPersonaje* CrearJugador(SAEVUSPruebas::FMundoPrueba& Mundo, float X, const USAEVUSAjustesMovimiento* Ajustes = nullptr, float Y = 0.f)
	{
		ASAEVUSPersonaje* Jugador = Mundo.CrearPersonaje(FVector(X, Y, AlturaInicio), Ajustes);
		return Mundo.AsentarEnSuelo() ? Jugador : nullptr;
	}

	/** De pie sobre una superficie de altura ZSuperficie entre X0 y X1. */
	bool DePieSobre(const ASAEVUSPersonaje* Jugador, float X0, float X1, float ZSuperficie)
	{
		const FVector Posicion = Jugador->GetActorLocation();
		return Jugador->GetCharacterMovement()->IsMovingOnGround() && Posicion.X >= X0 && Posicion.X <= X1
			&& Posicion.Z > ZSuperficie + MediaAlturaJugador * 0.5f;
	}

	/** Pulsación de salto como la de un jugador: suelta y vuelve a pulsar. */
	void Saltar(ASAEVUSPersonaje* Jugador)
	{
		Jugador->StopJumping();
		Jugador->PedirSalto();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaObstaculoDobleSalto, "SAEVUS.Tutorial.Obstaculos.BloqueDobleSalto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaObstaculoDobleSalto::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasObstaculos;

	// Devuelve cierto si el jugador acaba encima del bloque de 2,2 m.
	auto Intentar = [this](const USAEVUSAjustesMovimiento* Ajustes) -> bool
	{
		SAEVUSPruebas::FMundoPrueba Mundo;
		CrearSuelo(Mundo, -1000.f, 2000.f);
		Mundo.CrearBloque(FVector(600.f, 0.f, AlturaBloqueDobleSalto * 0.5f), FVector(400.f, AnchoPasillo, AlturaBloqueDobleSalto));
		ASAEVUSPersonaje* Jugador = CrearJugador(Mundo, -600.f, Ajustes);
		if (!Jugador)
		{
			AddError(TEXT("El jugador no aterrizó"));
			return false;
		}
		bool bSalto = false;
		bool bDobleSalto = false;
		float ZMaxima = 0.f;
		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax); ++Indice)
		{
			ZMaxima = FMath::Max(ZMaxima, static_cast<float>(Jugador->GetActorLocation().Z));
			Jugador->AddMovementInput(FVector::ForwardVector, 1.f);
			if (!bSalto && Jugador->GetActorLocation().X >= 400.f - DistanciaSaltoAntesDeCara)
			{
				Saltar(Jugador);
				bSalto = true;
			}
			else if (bSalto && !bDobleSalto && Jugador->GetActorLocation().X < 400.f && Jugador->GetVelocity().Z <= 0.f
				&& Jugador->GetCharacterMovement()->IsFalling())
			{
				// En el pico del salto, todavía delante de la cara del bloque.
				Saltar(Jugador);
				bDobleSalto = true;
			}
			Mundo.Paso();
			if (DePieSobre(Jugador, 400.f, 800.f, AlturaBloqueDobleSalto))
			{
				return true;
			}
		}
		AddInfo(FString::Printf(TEXT("Bloque: Z máxima del centro %.0f, final %s, doble salto %d"), ZMaxima, *Jugador->GetActorLocation().ToCompactString(), bDobleSalto ? 1 : 0));
		return false;
	};

	TestTrue(TEXT("Con doble salto se sube al bloque de 2,2 m"), Intentar(nullptr));
	USAEVUSAjustesMovimiento* SinAyudas = NewObject<USAEVUSAjustesMovimiento>(GetTransientPackage());
	SinAyudas->Aire.MaxSaltosAereos = 0;
	SinAyudas->Escalada.MaxEscaladas = 0;
	SinAyudas->SaltoPared.MaxSaltosPared = 0;
	// La cara del bloque también se puede escalar o usar para un salto de pared; sin nada de eso no se sube.
	TestFalse(TEXT("Con solo el salto normal no se sube: el bloque pide una técnica"), Intentar(SinAyudas));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaObstaculoFosoDash, "SAEVUS.Tutorial.Obstaculos.FosoDash", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaObstaculoFosoDash::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasObstaculos;

	SAEVUSPruebas::FMundoPrueba Mundo;
	CrearSuelo(Mundo, -2000.f, 0.f);
	CrearSuelo(Mundo, AnchoFosoDash, AnchoFosoDash + 1000.f);
	ASAEVUSPersonaje* Jugador = CrearJugador(Mundo, -1500.f);
	if (!TestNotNull(TEXT("Jugador de pie"), Jugador))
	{
		return false;
	}
	bool bSalto = false;
	bool bDobleSalto = false;
	bool bDash = false;
	float TiempoDobleSalto = 0.f;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Jugador->GetActorLocation().Z > ZCaida; ++Indice)
	{
		Jugador->AddMovementInput(FVector::ForwardVector, 1.f);
		if (!bSalto && Jugador->GetActorLocation().X >= -MargenBorde)
		{
			Saltar(Jugador);
			bSalto = true;
		}
		else if (bSalto && !bDobleSalto && Jugador->GetVelocity().Z <= 0.f)
		{
			Saltar(Jugador);
			bDobleSalto = true;
		}
		else if (bDobleSalto && !bDash && (TiempoDobleSalto += SAEVUSPruebas::PasoSimulacion) >= RetardoDashTrasDobleSalto)
		{
			Jugador->PedirDashHacia(FVector::ForwardVector);
			bDash = true;
		}
		Mundo.Paso();
		if (DePieSobre(Jugador, AnchoFosoDash, AnchoFosoDash + 1000.f, 0.f))
		{
			AddInfo(FString::Printf(TEXT("Cruzó el foso de %.0f cm; aterrizó en X = %.0f"), AnchoFosoDash, Jugador->GetActorLocation().X));
			return true;
		}
	}
	AddError(FString::Printf(TEXT("No cruzó el foso de %.0f cm con salto, doble salto y dash (X final %.0f)"), AnchoFosoDash, Jugador->GetActorLocation().X));
	return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaObstaculoFosoRebote, "SAEVUS.Tutorial.Obstaculos.FosoRebote", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaObstaculoFosoRebote::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasObstaculos;

	SAEVUSPruebas::FMundoPrueba Mundo;
	CrearSuelo(Mundo, -2000.f, 0.f);
	CrearSuelo(Mundo, AnchoFosoRebote, AnchoFosoRebote + 1000.f);
	for (const float X : XMunecosRebote)
	{
		Mundo.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector(X, 0.f, ZMunecosRebote));
	}
	ASAEVUSPersonaje* Jugador = CrearJugador(Mundo, -1500.f);
	if (!TestNotNull(TEXT("Jugador de pie"), Jugador))
	{
		return false;
	}
	USAEVUSComponenteMovimiento* Movimiento = Jugador->ObtenerMovimiento();
	USAEVUSComponenteCombate* Combate = Jugador->ObtenerCombate();
	bool bSalto = false;
	int32 Rebotes = 0;
	bool bDobleSalto = false;
	bool bDash = false;
	float VelocidadAnterior = 0.f;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Jugador->GetActorLocation().Z > ZCaida; ++Indice)
	{
		Jugador->AddMovementInput(FVector::ForwardVector, 1.f);
		const FVector Posicion = Jugador->GetActorLocation();
		if (!bSalto && Posicion.X >= -MargenBorde)
		{
			Saltar(Jugador);
			bSalto = true;
		}
		else if (bSalto && Rebotes < UE_ARRAY_COUNT(XMunecosRebote))
		{
			// Cayendo cerca del siguiente muñeco: tajo hacia abajo.
			const float XMuneco = XMunecosRebote[Rebotes];
			if (Movimiento->IsFalling() && Movimiento->Velocity.Z < 0.f && FMath::Abs(Posicion.X - XMuneco) < AlcanceRebote && !Combate->EstaAtacando())
			{
				Combate->PedirAtaqueEnDireccion(ESAEVUSDireccionAtaque::Abajo);
			}
		}
		else if (Rebotes >= UE_ARRAY_COUNT(XMunecosRebote) && !bDobleSalto && Movimiento->Velocity.Z <= 0.f)
		{
			Saltar(Jugador);
			bDobleSalto = true;
		}
		else if (bDobleSalto && !bDash)
		{
			Jugador->PedirDashHacia(FVector::ForwardVector);
			bDash = true;
		}
		Mundo.Paso();
		// Un rebote: la velocidad vertical pasa de bajar a subir sin estar en el suelo.
		if (VelocidadAnterior < 0.f && Movimiento->Velocity.Z > 0.f && Movimiento->IsFalling() && Rebotes < UE_ARRAY_COUNT(XMunecosRebote))
		{
			++Rebotes;
		}
		VelocidadAnterior = static_cast<float>(Movimiento->Velocity.Z);
		if (DePieSobre(Jugador, AnchoFosoRebote, AnchoFosoRebote + 1000.f, 0.f))
		{
			AddInfo(FString::Printf(TEXT("Cruzó el foso de %.0f cm con %d rebotes"), AnchoFosoRebote, Rebotes));
			TestTrue(TEXT("Usó los rebotes"), Rebotes > 0);
			return true;
		}
	}
	AddError(FString::Printf(TEXT("No cruzó el foso de rebote (%d rebotes, X final %.0f, Z %.0f)"), Rebotes, Jugador->GetActorLocation().X, Jugador->GetActorLocation().Z));
	return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaObstaculoCarreraPared, "SAEVUS.Tutorial.Obstaculos.FosoCarreraPared", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaObstaculoCarreraPared::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasObstaculos;

	SAEVUSPruebas::FMundoPrueba Mundo;
	CrearSuelo(Mundo, -2000.f, 0.f);
	CrearSuelo(Mundo, AnchoFosoCarreraPared, AnchoFosoCarreraPared + 1000.f);
	// Pared a lo largo del foso, en el lado +Y del pasillo (cara en Y = 400).
	Mundo.CrearBloque(FVector(450.f, AnchoPasillo * 0.5f + 50.f, 300.f), FVector(1500.f, 100.f, 600.f));
	ASAEVUSPersonaje* Jugador = CrearJugador(Mundo, -1200.f, nullptr, YCarreraPared);
	if (!TestNotNull(TEXT("Jugador de pie"), Jugador))
	{
		return false;
	}
	USAEVUSComponenteMovimiento* Movimiento = Jugador->ObtenerMovimiento();
	const FVector HaciaPared = FVector(1.f, 0.5f, 0.f).GetSafeNormal();
	const FVector PegadoAPared = FVector(1.f, 0.3f, 0.f).GetSafeNormal();
	bool bSalto = false;
	bool bCorrioPorPared = false;
	bool bSolto = false;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Jugador->GetActorLocation().Z > ZCaida; ++Indice)
	{
		const FVector Posicion = Jugador->GetActorLocation();
		bCorrioPorPared |= Movimiento->EstaEnCarreraPared();
		Jugador->AddMovementInput(!bSalto ? FVector::ForwardVector : (bCorrioPorPared && !bSolto ? PegadoAPared : (bSolto ? FVector::ForwardVector : HaciaPared)), 1.f);
		if (!bSalto && Posicion.X >= XSaltoCarreraPared)
		{
			Saltar(Jugador);
			bSalto = true;
		}
		else if (bCorrioPorPared && !bSolto && (Posicion.X >= XSoltarCarreraPared || !Movimiento->EstaEnCarreraPared()))
		{
			if (Movimiento->EstaEnCarreraPared())
			{
				Saltar(Jugador);
			}
			bSolto = true;
		}
		Mundo.Paso();
		if (DePieSobre(Jugador, AnchoFosoCarreraPared, AnchoFosoCarreraPared + 1000.f, 0.f))
		{
			TestTrue(TEXT("Cruzó corriendo por la pared"), bCorrioPorPared);
			return true;
		}
	}
	AddError(FString::Printf(TEXT("No cruzó el foso de %.0f cm (carrera por pared %d, X final %.0f, Z %.0f)"), AnchoFosoCarreraPared, bCorrioPorPared ? 1 : 0,
		Jugador->GetActorLocation().X, Jugador->GetActorLocation().Z));
	return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaObstaculoCornisa, "SAEVUS.Tutorial.Obstaculos.Cornisa", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaObstaculoCornisa::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasObstaculos;

	SAEVUSPruebas::FMundoPrueba Mundo;
	CrearSuelo(Mundo, -1000.f, 2000.f);
	Mundo.CrearBloque(FVector(700.f, 0.f, AlturaCornisa * 0.5f), FVector(600.f, AnchoPasillo, AlturaCornisa));
	ASAEVUSPersonaje* Jugador = CrearJugador(Mundo, -600.f);
	if (!TestNotNull(TEXT("Jugador de pie"), Jugador))
	{
		return false;
	}
	bool bSalto = false;
	bool bDobleSalto = false;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax); ++Indice)
	{
		Jugador->AddMovementInput(FVector::ForwardVector, 1.f);
		if (!bSalto && Jugador->GetActorLocation().X >= 400.f - DistanciaSaltoAntesDeCara * 0.5f)
		{
			Saltar(Jugador);
			bSalto = true;
		}
		else if (bSalto && !bDobleSalto && !Jugador->ObtenerMovimiento()->EstaEscalando() && Jugador->GetVelocity().Z <= 0.f
			&& Jugador->GetCharacterMovement()->IsFalling())
		{
			// Si la escalada no llegó arriba, el doble salto remata.
			Saltar(Jugador);
			bDobleSalto = true;
		}
		Mundo.Paso();
		if (DePieSobre(Jugador, 400.f, 1000.f, AlturaCornisa))
		{
			AddInfo(FString::Printf(TEXT("Subió a la cornisa de %.0f cm%s"), AlturaCornisa, bDobleSalto ? TEXT(" (con doble salto)") : TEXT("")));
			return true;
		}
	}
	AddError(FString::Printf(TEXT("No subió a la cornisa de %.0f cm (Z final %.0f)"), AlturaCornisa, Jugador->GetActorLocation().Z));
	return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaObstaculoPinchos, "SAEVUS.Tutorial.Obstaculos.FosoPinchos", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaObstaculoPinchos::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasObstaculos;

	SAEVUSPruebas::FMundoPrueba Mundo;
	CrearSuelo(Mundo, -2000.f, 0.f);
	CrearSuelo(Mundo, AnchoFosoRebote, AnchoFosoRebote + 1000.f);
	ASAEVUSPeligro* Pinchos = Mundo.CrearActor<ASAEVUSPeligro>(FVector(AnchoFosoRebote * 0.5f, 0.f, ZCentroPinchos));
	Pinchos->FindComponentByClass<UBoxComponent>()->SetBoxExtent(FVector(AnchoFosoRebote * 0.5f, AnchoPasillo * 0.5f, MediaAlturaPinchos));
	Pinchos->FijarPuntoRetorno(FVector(-100.f, 0.f, AlturaInicio));
	ASAEVUSPersonaje* Jugador = CrearJugador(Mundo, -1500.f);
	if (!TestNotNull(TEXT("Jugador de pie"), Jugador))
	{
		return false;
	}
	USAEVUSComponenteMovimiento* Movimiento = Jugador->ObtenerMovimiento();
	USAEVUSComponenteCombate* Combate = Jugador->ObtenerCombate();
	const float VidaInicial = Jugador->ObtenerVida()->ObtenerVida();
	bool bSalto = false;
	int32 Rebotes = 0;
	float VelocidadAnterior = 0.f;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax); ++Indice)
	{
		Jugador->AddMovementInput(FVector::ForwardVector, 1.f);
		const FVector Posicion = Jugador->GetActorLocation();
		if (!bSalto && Posicion.X >= -MargenBorde)
		{
			Saltar(Jugador);
			bSalto = true;
		}
		else if (bSalto && Movimiento->IsFalling() && Movimiento->Velocity.Z < 0.f && Posicion.Z < AlturaAtaquePinchos
			&& Posicion.X > 0.f && Posicion.X < AnchoFosoRebote && !Combate->EstaAtacando())
		{
			Combate->PedirAtaqueEnDireccion(ESAEVUSDireccionAtaque::Abajo);
		}
		Mundo.Paso();
		if (VelocidadAnterior < 0.f && Movimiento->Velocity.Z > 0.f && Movimiento->IsFalling())
		{
			++Rebotes;
		}
		VelocidadAnterior = static_cast<float>(Movimiento->Velocity.Z);
		if (DePieSobre(Jugador, AnchoFosoRebote, AnchoFosoRebote + 1000.f, 0.f))
		{
			AddInfo(FString::Printf(TEXT("Cruzó los pinchos con %d rebotes"), Rebotes));
			TestTrue(TEXT("Rebotando en los pinchos"), Rebotes > 0);
			TestEqual(TEXT("Sin tocarlos: la vida intacta"), Jugador->ObtenerVida()->ObtenerVida(), VidaInicial);
			TestEqual(TEXT("Los pinchos no dan alma"), Jugador->ObtenerAlma()->ObtenerAlma(), 0.f);
			return true;
		}
	}
	AddError(FString::Printf(TEXT("No cruzó los pinchos (%d rebotes, X final %.0f, vida %.0f)"), Rebotes, Jugador->GetActorLocation().X, Jugador->ObtenerVida()->ObtenerVida()));
	return false;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaPeligroTocar, "SAEVUS.Tutorial.PeligroTocar", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaPeligroTocar::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasObstaculos;

	SAEVUSPruebas::FMundoPrueba Mundo;
	CrearSuelo(Mundo, -2000.f, 2000.f);
	ASAEVUSPersonaje* Jugador = CrearJugador(Mundo, 0.f);
	if (!TestNotNull(TEXT("Jugador de pie"), Jugador))
	{
		return false;
	}
	const FVector Retorno = Jugador->GetActorLocation();
	ASAEVUSPeligro* Pinchos = Mundo.CrearActor<ASAEVUSPeligro>(FVector(1000.f, 0.f, 30.f));
	Pinchos->FijarPuntoRetorno(Retorno);
	const float VidaInicial = Jugador->ObtenerVida()->ObtenerVida();
	Jugador->SetActorLocation(FVector(1000.f, 0.f, Retorno.Z));
	Pinchos->NotificarEntrada(Jugador);
	TestTrue(TEXT("Tocar los pinchos quita vida"), Jugador->ObtenerVida()->ObtenerVida() < VidaInicial);
	TestTrue(TEXT("Y devuelve al borde"), Jugador->GetActorLocation().Equals(Retorno, 1.f));
	TestEqual(TEXT("Los pinchos no reciben daño"), UGameplayStatics::ApplyDamage(Pinchos, 10.f, nullptr, Jugador, UDamageType::StaticClass()), 0.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
