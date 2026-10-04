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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaComboAereo, "SAEVUS.Combate.ComboAereo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaComboAereo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	const TArray<FSAEVUSDefinicionAtaque>& Cadena = Mundo.Combate->ObtenerRepertorio().ComboAire;
	if (!TestTrue(TEXT("La cadena aérea tiene tres tajos"), Cadena.Num() == 3))
	{
		return false;
	}
	// Salta y espera a estar cayendo.
	Mundo.Personaje->PedirSalto();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && !(Mundo.Movimiento->IsFalling() && Mundo.Movimiento->Velocity.Z < -200.f); ++Indice)
	{
		Mundo.Paso();
	}
	const float CaidaAntes = static_cast<float>(Mundo.Movimiento->Velocity.Z);
	Mundo.Personaje->PedirAtaque();
	TestTrue(TEXT("Empieza la cadena aérea"), Mundo.Combate->EsCadenaAerea());
	TestTrue(TEXT("El tajo aéreo frena la caída"), Mundo.Movimiento->Velocity.Z > CaidaAntes * 0.5f);
	const float GravedadNormal = Mundo.Movimiento->GetWorld()->GetGravityZ() * Mundo.Movimiento->GravityScale;
	TestEqual(TEXT("Y flota (gravedad reducida)"), Mundo.Movimiento->GetGravityZ(), GravedadNormal * Cadena[0].EscalaGravedadEnAire, 1.f);

	// Encadena los tres en el aire.
	int32 Maximo = 0;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && Mundo.Movimiento->IsFalling(); ++Indice)
	{
		Mundo.Personaje->PedirAtaque();
		Mundo.Paso();
		Maximo = FMath::Max(Maximo, Mundo.Combate->ObtenerIndiceCombo() + 1);
	}
	AddInfo(FString::Printf(TEXT("Tajos aéreos encadenados antes de aterrizar: %d"), Maximo));
	TestEqual(TEXT("Los tres tajos caben en el aire"), Maximo, 3);
	TestEqual(TEXT("Al acabar, gravedad normal"), Mundo.Movimiento->GetGravityZ(), GravedadNormal, 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTajoEnCarrera, "SAEVUS.Combate.TajoEnCarrera", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTajoEnCarrera::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	// Un muñeco más lejos que el alcance de un tajo normal (unos 2,5 m), en la dirección del dash (+X).
	constexpr float DistanciaLejana = 350.f;
	const ASAEVUSMunecoEntrenamiento* Muneco = Mundo.CrearMuneco(DistanciaLejana);
	Mundo.Personaje->PedirDashHacia(FVector::ForwardVector);
	Mundo.Paso();
	if (!TestTrue(TEXT("En dash"), Mundo.Movimiento->EstaEnDash()))
	{
		return false;
	}
	Mundo.Personaje->PedirAtaque();
	TestTrue(TEXT("Atacar en el dash da el tajo en carrera"), Mundo.Combate->EsTajoEnCarrera());
	const FSAEVUSDefinicionAtaque& Tajo = Mundo.Combate->ObtenerRepertorio().TajoEnCarrera;
	Mundo.Paso();
	AddInfo(FString::Printf(TEXT("Velocidad del tajo en carrera %.0f cm/s"), Mundo.Movimiento->Velocity.Size2D()));
	TestTrue(TEXT("Sigue en la dirección del dash a la velocidad de su estocada"), Mundo.Movimiento->Velocity.X > Tajo.VelocidadEstocada * 0.8f);
	Mundo.AvanzarHastaReposo();
	TestEqual(TEXT("Alcanza al muñeco lejano"), Muneco->ObtenerGolpes(), 1);
	TestEqual(TEXT("Con el daño del tajo en carrera"), Muneco->ObtenerDanioTotal(), Tajo.Danio);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAtaqueCargado, "SAEVUS.Combate.AtaqueCargado", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAtaqueCargado::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;

	FMundoCombate Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	// Dos muñecos a los lados (fuera del tajo frontal): solo el giro los alcanza a los dos.
	const ASAEVUSMunecoEntrenamiento* Izquierda = Mundo.MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector(0.f, -150.f, MediaAlturaMuneco));
	const ASAEVUSMunecoEntrenamiento* Derecha = Mundo.MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector(0.f, 150.f, MediaAlturaMuneco));
	const FSAEVUSRepertorioMelee& Repertorio = Mundo.Combate->ObtenerRepertorio();

	// Soltar antes de tiempo: no hay ataque cargado.
	Mundo.Personaje->PedirAtaque();
	Mundo.AvanzarHastaReposo();
	Mundo.Personaje->SoltarAtaque();
	TestFalse(TEXT("Sin carga completa no sale"), Mundo.Combate->EsAtaqueCargado());

	// Mantener: el primer tajo sale y, al acabar, la carga se completa.
	Mundo.Personaje->PedirAtaque();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && !Mundo.Combate->EstaCargado(); ++Indice)
	{
		Mundo.Paso();
	}
	if (!TestTrue(TEXT("Mantenido, se carga"), Mundo.Combate->EstaCargado()))
	{
		return false;
	}
	const int32 GolpesAntes = Izquierda->ObtenerGolpes() + Derecha->ObtenerGolpes();
	Mundo.Personaje->SoltarAtaque();
	TestTrue(TEXT("Soltar lanza el ataque cargado"), Mundo.Combate->EsAtaqueCargado());
	Mundo.AvanzarHastaReposo();
	TestEqual(TEXT("El giro alcanza a los dos lados"), Izquierda->ObtenerGolpes() + Derecha->ObtenerGolpes() - GolpesAntes, 2);
	TestEqual(TEXT("Con su daño"), Derecha->ObtenerUltimoDanio(), Repertorio.AtaqueCargado.Danio);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAbanico, "SAEVUS.Combate.Abanico", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAbanico::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasCombate;
	FSAEVUSDefinicionAtaque Ataque;
	Ataque.AlcanceAbanico = 170.f;
	Ataque.AnguloAbanico = 75.f;
	Ataque.MitadAlturaAbanico = 70.f;
	const FVector Origen = FVector::ZeroVector;
	const FVector Frente = FVector::ForwardVector;
	TestTrue(TEXT("Delante, al alcance"), USAEVUSComponenteCombate::DentroDelAbanico(Ataque, Origen, Frente, FVector(160.f, 0.f, 0.f)));
	TestTrue(TEXT("En diagonal (45°)"), USAEVUSComponenteCombate::DentroDelAbanico(Ataque, Origen, Frente, FVector(100.f, 100.f, 0.f)));
	TestFalse(TEXT("Más allá del alcance"), USAEVUSComponenteCombate::DentroDelAbanico(Ataque, Origen, Frente, FVector(180.f, 0.f, 0.f)));
	TestFalse(TEXT("De lado (90°)"), USAEVUSComponenteCombate::DentroDelAbanico(Ataque, Origen, Frente, FVector(0.f, 120.f, 0.f)));
	TestFalse(TEXT("Detrás"), USAEVUSComponenteCombate::DentroDelAbanico(Ataque, Origen, Frente, FVector(-100.f, 0.f, 0.f)));
	TestFalse(TEXT("Por encima de la franja"), USAEVUSComponenteCombate::DentroDelAbanico(Ataque, Origen, Frente, FVector(100.f, 0.f, 90.f)));
	Ataque.AnguloAbanico = 180.f;
	TestTrue(TEXT("Con 180° también detrás"), USAEVUSComponenteCombate::DentroDelAbanico(Ataque, Origen, Frente, FVector(-100.f, 0.f, 0.f)));

	// En el mundo: el primer tajo alcanza un muñeco a 1,6 m y no a 3,2 m (el abanico se mide desde donde
	// está el personaje en la fase activa, ya adelantado por la estocada: lo que dibuja el arco).
	for (const float Distancia : { 160.f, 320.f })
	{
		FMundoCombate Mundo;
		if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
		{
			return false;
		}
		const ASAEVUSMunecoEntrenamiento* Muneco = Mundo.CrearMuneco(Distancia);
		Mundo.Personaje->PedirAtaque();
		Mundo.AvanzarHastaReposo();
		TestEqual(FString::Printf(TEXT("Golpes a %.0f cm"), Distancia), Muneco->ObtenerGolpes(), Distancia < 250.f ? 1 : 0);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
