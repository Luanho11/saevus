// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSTiposCombate.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"

namespace SAEVUSPruebasCombate
{
	// Suelo de 40 m x 40 m, superficie en Z = 0.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);

	// Los muñecos están de pie en el suelo delante del personaje (+X).
	constexpr float MediaAlturaMuneco = 96.f;
	constexpr float DistanciaEnAlcance = 150.f;
	constexpr float DistanciaFueraAlcance = 400.f;

	constexpr float TiempoMaxAtaque = 3.f;
	constexpr int32 ToleranciaPasosTiempo = 1;
	constexpr float ToleranciaGuinadaGrados = 1.f;
	constexpr float ToleranciaVelocidad = 5.f;
	constexpr float ToleranciaDistanciaEstocada = 0.15f;
	// Un buffer más corto que el punto de encadenado del primer ataque, así la pulsación caduca sin usarse.
	constexpr float BufferEntradaCorto = 0.05f;
	// Pasos tras un salto antes de atacar, para estar en el aire.
	constexpr int32 PasosEnAire = 6;
	// Momento de la recuperación en que se comprueba la velocidad reducida, ya acabada la estocada.
	constexpr float TiempoComprobarVelocidadReducida = 0.3f;

	/** Suelo, un personaje asentado y sus componentes. */
	struct FMundoCombate
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = nullptr;
		USAEVUSComponenteMovimiento* Movimiento = nullptr;
		USAEVUSComponenteCombate* Combate = nullptr;
		bool bListo = false;

		FMundoCombate()
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
			Movimiento = Personaje->ObtenerMovimiento();
			Combate = Personaje->ObtenerCombate();
			bListo = MundoPrueba.AsentarEnSuelo();
		}

		ASAEVUSMunecoEntrenamiento* CrearMuneco(float Distancia)
		{
			return MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector(Distancia, 0.f, MediaAlturaMuneco));
		}

		void Paso(const FVector& Entrada = FVector::ZeroVector)
		{
			if (!Entrada.IsZero())
			{
				Personaje->AddMovementInput(Entrada, 1.f);
			}
			MundoPrueba.Paso();
		}

		/** Avanza hasta que no hay ataque en curso; falso si sigue uno tras TiempoMaxAtaque. */
		bool AvanzarHastaReposo()
		{
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && Combate->EstaAtacando(); ++Indice)
			{
				Paso();
			}
			return !Combate->EstaAtacando();
		}

		/** Avanza hasta que el ataque en curso llega a Tiempo; falso si terminó antes. */
		bool AvanzarHastaTiempoAtaque(float Tiempo)
		{
			while (Combate->EstaAtacando() && Combate->ObtenerTiempoAtaque() < Tiempo)
			{
				Paso();
			}
			return Combate->EstaAtacando();
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaLineaTiempo, "SAEVUS.Combate.LineaTiempo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaLineaTiempo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	const FSAEVUSDefinicionAtaque& Primero = Mundo.Combate->ObtenerRepertorio().ComboSuelo[0];

	Mundo.Personaje->PedirAtaque();
	TestTrue(TEXT("El ataque empieza con la pulsación"), Mundo.Combate->EstaAtacando());
	TestEqual(TEXT("Primer ataque de la cadena de suelo"), Mundo.Combate->ObtenerIndiceCombo(), 0);
	TestEqual(TEXT("Empieza en preparación"), Mundo.Combate->ObtenerFaseAtaque(), ESAEVUSFaseAtaque::Preparacion);

	int32 PasosPreparacion = 0;
	int32 PasosActivos = 0;
	int32 PasosTotales = 0;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && Mundo.Combate->EstaAtacando(); ++Indice)
	{
		const ESAEVUSFaseAtaque Fase = Mundo.Combate->ObtenerFaseAtaque();
		PasosPreparacion += Fase == ESAEVUSFaseAtaque::Preparacion ? 1 : 0;
		PasosActivos += Fase == ESAEVUSFaseAtaque::Activa ? 1 : 0;
		++PasosTotales;
		Mundo.Paso();
	}
	TestFalse(TEXT("El ataque termina"), Mundo.Combate->EstaAtacando());
	TestTrue(TEXT("La preparación dura TiempoPreparacion"), FMath::Abs(PasosPreparacion - SAEVUSPruebas::PasosPara(Primero.TiempoPreparacion)) <= ToleranciaPasosTiempo);
	TestTrue(TEXT("La fase activa dura TiempoActivo"), FMath::Abs(PasosActivos - SAEVUSPruebas::PasosPara(Primero.TiempoActivo)) <= ToleranciaPasosTiempo);
	TestTrue(TEXT("El ataque completo dura su tiempo total"), FMath::Abs(PasosTotales - SAEVUSPruebas::PasosPara(Primero.ObtenerTiempoTotal())) <= ToleranciaPasosTiempo);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGolpeaMuneco, "SAEVUS.Combate.GolpeaMuneco", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGolpeaMuneco::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	for (const float Distancia : { DistanciaEnAlcance, DistanciaFueraAlcance })
	{
		FMundoCombate Mundo;
		if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
		{
			return false;
		}
		const ASAEVUSMunecoEntrenamiento* Muneco = Mundo.CrearMuneco(Distancia);
		Mundo.Personaje->PedirAtaque();
		Mundo.AvanzarHastaReposo();

		const bool bEnAlcance = Distancia == DistanciaEnAlcance;
		const FSAEVUSDefinicionAtaque& Primero = Mundo.Combate->ObtenerRepertorio().ComboSuelo[0];
		TestEqual(bEnAlcance ? TEXT("Un muñeco al alcance recibe exactamente un golpe") : TEXT("Un muñeco fuera de alcance no recibe golpes"),
			Muneco->ObtenerGolpes(), bEnAlcance ? 1 : 0);
		TestEqual(TEXT("Daño de los datos del ataque"), Muneco->ObtenerDanioTotal(), bEnAlcance ? Primero.Danio : 0.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCombo, "SAEVUS.Combate.Combo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCombo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	ASAEVUSMunecoEntrenamiento* Muneco = Mundo.CrearMuneco(DistanciaEnAlcance);
	const TArray<FSAEVUSDefinicionAtaque>& Cadena = Mundo.Combate->ObtenerRepertorio().ComboSuelo;

	// Una pulsación por ataque, justo al empezar: queda en buffer hasta el punto de encadenado.
	Mundo.Personaje->PedirAtaque();
	for (int32 Esperado = 1; Esperado < Cadena.Num(); ++Esperado)
	{
		Mundo.Personaje->PedirAtaque();
		const float EncadenarDesde = Cadena[Esperado - 1].EncadenarDesde;
		Mundo.AvanzarHastaTiempoAtaque(EncadenarDesde);
		Mundo.Paso();
		TestEqual(FString::Printf(TEXT("La pulsación en buffer encadena el ataque %d"), Esperado + 1), Mundo.Combate->ObtenerIndiceCombo(), Esperado);
	}
	TestTrue(TEXT("La cadena termina tras su último ataque"), Mundo.AvanzarHastaReposo());
	TestTrue(TEXT("Los ataques de la cadena golpearon al muñeco"), Muneco->ObtenerGolpes() >= 2);

	// Buffer demasiado corto: una pulsación al inicio del primer ataque ya caducó en su punto de encadenado.
	USAEVUSDatosRepertorioMelee* BufferCorto = NewObject<USAEVUSDatosRepertorioMelee>(GetTransientPackage());
	BufferCorto->Repertorio.TiempoBufferEntrada = BufferEntradaCorto;
	Mundo.Combate->FijarRepertorio(BufferCorto);
	Mundo.Personaje->PedirAtaque();
	Mundo.Personaje->PedirAtaque();
	Mundo.AvanzarHastaReposo();
	TestFalse(TEXT("Una pulsación caducada no encadena"), Mundo.Combate->EstaAtacando());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCancelaciones, "SAEVUS.Combate.Cancelaciones", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCancelaciones::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	const TArray<FSAEVUSDefinicionAtaque>& Cadena = Mundo.Combate->ObtenerRepertorio().ComboSuelo;

	// Dash en la preparación: rechazado, el ataque sigue.
	Mundo.Personaje->PedirAtaque();
	Mundo.Personaje->PedirDash();
	Mundo.Paso();
	TestFalse(TEXT("No hay dash antes de la ventana de cancelación con dash"), Mundo.Movimiento->EstaEnDash());
	TestTrue(TEXT("El ataque sigue"), Mundo.Combate->EstaAtacando());

	// Dash dentro de la ventana: el ataque termina y empieza el dash.
	Mundo.AvanzarHastaTiempoAtaque(Cadena[0].CancelarConDashDesde);
	Mundo.Personaje->PedirDash();
	Mundo.Paso();
	TestFalse(TEXT("El dash en la ventana termina el ataque"), Mundo.Combate->EstaAtacando());
	TestTrue(TEXT("El dash en la ventana empieza"), Mundo.Movimiento->EstaEnDash());

	// Salto durante el remate, que nunca permite cancelar con salto.
	Mundo.AvanzarHastaReposo();
	Mundo.MundoPrueba.AsentarEnSuelo();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Mundo.Movimiento->ObtenerAjusteDash().Enfriamiento + 0.5f); ++Indice)
	{
		Mundo.Paso();
	}
	Mundo.Personaje->PedirAtaque();
	for (int32 Esperado = 1; Esperado < Cadena.Num(); ++Esperado)
	{
		Mundo.Personaje->PedirAtaque();
		Mundo.AvanzarHastaTiempoAtaque(Cadena[Esperado - 1].EncadenarDesde);
		Mundo.Paso();
	}
	if (!TestEqual(TEXT("Llegó al remate"), Mundo.Combate->ObtenerIndiceCombo(), Cadena.Num() - 1))
	{
		return false;
	}
	Mundo.AvanzarHastaTiempoAtaque(Cadena.Last().TiempoPreparacion + Cadena.Last().TiempoActivo);
	Mundo.Personaje->PedirSalto();
	Mundo.Paso();
	TestTrue(TEXT("El remate no se cancela con salto"), Mundo.Combate->EstaAtacando() && Mundo.Movimiento->IsMovingOnGround());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaMovimientoAtaque, "SAEVUS.Combate.Movimiento", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaMovimientoAtaque::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	const FSAEVUSDefinicionAtaque& Primero = Mundo.Combate->ObtenerRepertorio().ComboSuelo[0];

	// Ataque hacia la entrada: dirigir a +Y un paso y atacar sin dejar de dirigir.
	Mundo.Paso(FVector::RightVector);
	Mundo.Personaje->AddMovementInput(FVector::RightVector, 1.f);
	Mundo.Personaje->PedirAtaque();
	TestEqual(TEXT("El ataque mira hacia la entrada"), static_cast<float>(Mundo.Personaje->GetActorRotation().Yaw), 90.f, ToleranciaGuinadaGrados);

	// Movimiento guiado por la espada: la estocada lleva al personaje hacia delante.
	const FVector Inicio = Mundo.Personaje->GetActorLocation();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Primero.DuracionEstocada); ++Indice)
	{
		Mundo.Paso(FVector::RightVector);
	}
	const float DistanciaEstocada = static_cast<float>(Mundo.Personaje->GetActorLocation().Y - Inicio.Y);
	const float EstocadaEsperada = Primero.VelocidadEstocada * Primero.DuracionEstocada;
	AddInfo(FString::Printf(TEXT("Distancia de la estocada %.1f cm, esperada %.1f cm"), DistanciaEstocada, EstocadaEsperada));
	TestEqual(TEXT("Distancia de la estocada ~ VelocidadEstocada x DuracionEstocada"), DistanciaEstocada, EstocadaEsperada, EstocadaEsperada * ToleranciaDistanciaEstocada);

	// Tras la estocada, dirigir queda reducido a la escala de velocidad del ataque.
	Mundo.AvanzarHastaTiempoAtaque(TiempoComprobarVelocidadReducida);
	const float TopeReducido = Mundo.Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Correr) * Primero.EscalaVelocidadSuelo;
	TestTrue(TEXT("Velocidad en el suelo reducida durante el ataque"), Mundo.Movimiento->Velocity.Size2D() <= TopeReducido + ToleranciaVelocidad);

	Mundo.AvanzarHastaReposo();
	TestEqual(TEXT("Tope de velocidad completo tras el ataque"), Mundo.Movimiento->GetMaxSpeed(), Mundo.Movimiento->ObtenerVelocidadPaso(ESAEVUSPaso::Correr));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAtaqueAereo, "SAEVUS.Combate.AtaqueAereo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAtaqueAereo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	Mundo.Personaje->PedirSalto();
	for (int32 Indice = 0; Indice < PasosEnAire; ++Indice)
	{
		Mundo.Paso();
	}
	if (!TestTrue(TEXT("En el aire"), Mundo.Movimiento->IsFalling()))
	{
		return false;
	}
	Mundo.Personaje->PedirAtaque();
	TestTrue(TEXT("Un ataque en el aire usa la cadena aérea"), Mundo.Combate->EsCadenaAerea());
	TestEqual(TEXT("Primer ataque aéreo"), Mundo.Combate->ObtenerAtaqueActual()->Nombre, Mundo.Combate->ObtenerRepertorio().ComboAire[0].Nombre);
	Mundo.Paso();
	TestTrue(TEXT("Sigue en el aire: un ataque aéreo no detiene el salto"), Mundo.Movimiento->IsFalling());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAtaqueTerminaDash, "SAEVUS.Combate.AtaqueTerminaDash", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAtaqueTerminaDash::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	Mundo.Personaje->PedirDash();
	Mundo.Paso();
	if (!TestTrue(TEXT("En dash"), Mundo.Movimiento->EstaEnDash()))
	{
		return false;
	}
	Mundo.Personaje->PedirAtaque();
	TestTrue(TEXT("El ataque empieza durante el dash"), Mundo.Combate->EstaAtacando());
	TestFalse(TEXT("El ataque termina el dash"), Mundo.Movimiento->EstaEnDash());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
