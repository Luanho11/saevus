// Copyright SAEVUS. Todos los derechos reservados.

// Tests de la revisión de código (E1–E12) y de la animación con vida (muelles, capa secundaria, gestos).

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animacion/SAEVUSCapaSecundaria.h"
#include "Animacion/SAEVUSGestosAtaque.h"
#include "Animacion/SAEVUSMuelle.h"
#include "Combate/SAEVUSComponenteAlma.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Combate/SAEVUSParadaGolpe.h"
#include "Combate/SAEVUSTiposCombate.h"
#include "Components/PoseableMeshComponent.h"
#include "Efectos/SAEVUSComponenteEcoDash.h"
#include "Engine/SkeletalMesh.h"
#include "Mundo/SAEVUSFuegoBrasero.h"
#include "Enemigos/SAEVUSTiposEnemigo.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Mundo/SAEVUSBanco.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"
#include "Tutorial/SAEVUSPuertaArena.h"

namespace SAEVUSPruebasRevision
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);
	constexpr float MediaAlturaMuneco = 96.f;
	constexpr float DistanciaEnAlcance = 150.f;
	constexpr float TiempoMaxAtaque = 3.f;

	/** Suelo y un personaje asentado. */
	struct FMundoRevision
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = nullptr;
		bool bListo = false;

		FMundoRevision()
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
			bListo = MundoPrueba.AsentarEnSuelo();
		}

		void AvanzarHastaReposo()
		{
			USAEVUSComponenteCombate* Combate = Personaje->ObtenerCombate();
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && Combate->EstaAtacando(); ++Indice)
			{
				MundoPrueba.Paso();
			}
		}
	};

	/** Enemigo de pruebas sin ataques ni patrulla, con escudo frontal si se pide. */
	USAEVUSDatosEnemigo* CrearDatos(float AnguloEscudo)
	{
		USAEVUSDatosEnemigo* Datos = NewObject<USAEVUSDatosEnemigo>(GetTransientPackage());
		Datos->Enemigo.VidaMaxima = 200.f;
		Datos->Enemigo.Ataques.Reset();
		Datos->Enemigo.RadioPatrulla = 0.f;
		Datos->Enemigo.AnguloEscudo = AnguloEscudo;
		Datos->Enemigo.DanioRompeEscudo = 1000.f;
		return Datos;
	}

	/** Ángulo en grados entre dos direcciones. */
	float Angulo(const FVector& A, const FVector& B)
	{
		return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(static_cast<float>(FVector::DotProduct(A.GetSafeNormal(), B.GetSafeNormal())), -1.f, 1.f)));
	}

	/** Avanza la capa secundaria Segundos con la misma entrada cada fotograma. */
	void Avanzar(FSAEVUSCapaSecundaria& Capa, const FSAEVUSAjusteSecundario& Ajuste, FSAEVUSEntradaSecundaria Entrada, float Segundos)
	{
		Entrada.DeltaTime = SAEVUSPruebas::PasoSimulacion;
		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Segundos); ++Indice)
		{
			Capa.Actualizar(Ajuste, Entrada);
		}
	}
}

// E1: un escudo de frente no deja hacer daño, y ApplyDamage lo dice (antes devolvía el daño pedido: daba alma).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscudoDevuelveCero, "SAEVUS.Combate.EscudoSinDanio", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscudoDevuelveCero::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	FMundoRevision Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	ASAEVUSEnemigo* Enemigo = Mundo.MundoPrueba.CrearEnemigo(FVector(300.f, 0.f, 100.f), CrearDatos(120.f), Mundo.Personaje);
	// Mira al jugador (en -X).
	Enemigo->SetActorRotation(FRotator(0.f, 180.f, 0.f));
	const float DeFrente = UGameplayStatics::ApplyDamage(Enemigo, 10.f, nullptr, Mundo.Personaje, UDamageType::StaticClass());
	TestEqual(TEXT("De frente, el escudo para todo: ApplyDamage devuelve 0"), DeFrente, 0.f);
	Mundo.Personaje->SetActorLocation(Enemigo->GetActorLocation() + FVector(300.f, 0.f, 0.f));
	const float PorDetras = UGameplayStatics::ApplyDamage(Enemigo, 10.f, nullptr, Mundo.Personaje, UDamageType::StaticClass());
	TestEqual(TEXT("Por detrás, devuelve la vida quitada"), PorDetras, 10.f);
	return true;
}

// E1: las zonas que solo solapan (bancos, avisos) no se golpean: ni alma ni parada de golpe; un muñeco sí da alma.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaZonaNoEsGolpe, "SAEVUS.Combate.ZonaNoEsGolpe", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaZonaNoEsGolpe::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	{
		FMundoRevision Mundo;
		if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
		{
			return false;
		}
		Mundo.MundoPrueba.CrearActor<ASAEVUSBanco>(FVector(DistanciaEnAlcance, 0.f, 100.f));
		const float AlmaAntes = Mundo.Personaje->ObtenerAlma()->ObtenerAlma();
		Mundo.Personaje->PedirAtaque();
		bool bParada = false;
		for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMaxAtaque) && Mundo.Personaje->ObtenerCombate()->EstaAtacando(); ++Indice)
		{
			Mundo.MundoPrueba.Paso();
			bParada |= Mundo.Personaje->ObtenerCombate()->EstaEnParadaGolpe();
		}
		TestEqual(TEXT("Tajear un banco no da alma"), Mundo.Personaje->ObtenerAlma()->ObtenerAlma(), AlmaAntes);
		TestFalse(TEXT("Ni para el tiempo"), bParada);
	}
	{
		FMundoRevision Mundo;
		Mundo.MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector(DistanciaEnAlcance, 0.f, MediaAlturaMuneco));
		const float AlmaAntes = Mundo.Personaje->ObtenerAlma()->ObtenerAlma();
		Mundo.Personaje->PedirAtaque();
		Mundo.AvanzarHastaReposo();
		TestTrue(TEXT("Un muñeco de práctica sí da alma"), Mundo.Personaje->ObtenerAlma()->ObtenerAlma() > AlmaAntes);
	}
	return true;
}

// E10: un fotograma largo que salta entera la fase activa sigue golpeando.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaFotogramaLargo, "SAEVUS.Combate.FotogramaLargo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaFotogramaLargo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	FMundoRevision Mundo;
	if (!TestTrue(TEXT("Personaje asentado"), Mundo.bListo))
	{
		return false;
	}
	const ASAEVUSMunecoEntrenamiento* Muneco = Mundo.MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector(DistanciaEnAlcance, 0.f, MediaAlturaMuneco));
	USAEVUSComponenteCombate* Combate = Mundo.Personaje->ObtenerCombate();
	Mundo.Personaje->PedirAtaque();
	const FSAEVUSDefinicionAtaque* Ataque = Combate->ObtenerAtaqueActual();
	if (!TestNotNull(TEXT("Ataque en curso"), Ataque))
	{
		return false;
	}
	// Un solo paso de tiempo desde antes de la fase activa hasta después de ella.
	const float Salto = Ataque->TiempoPreparacion + Ataque->TiempoActivo + 0.05f;
	Combate->TickComponent(Salto, LEVELTICK_All, &Combate->PrimaryComponentTick);
	TestEqual(TEXT("El golpe entra aunque el fotograma salte la ventana activa"), Muneco->ObtenerGolpes(), 1);
	return true;
}

// E4: curación, guardia y ataque frenan cada uno por su lado; manda el que más frena.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEscalasVelocidad, "SAEVUS.Movimiento.EscalasVelocidad", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEscalasVelocidad::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	FMundoRevision Mundo;
	USAEVUSComponenteMovimiento* Movimiento = Mundo.Personaje->ObtenerMovimiento();
	Movimiento->FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad::Curacion, 0.f);
	Movimiento->FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad::Ataque, 0.25f);
	TestEqual(TEXT("Curando y atacando: lo que más frena"), Movimiento->ObtenerEscalaVelocidadAccion(), 0.f);
	Movimiento->FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad::Ataque, 1.f);
	TestEqual(TEXT("Acabar el ataque no suelta la curación"), Movimiento->ObtenerEscalaVelocidadAccion(), 0.f);
	Movimiento->FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad::Curacion, 1.f);
	Movimiento->FijarEscalaVelocidadAccion(ESAEVUSFuenteEscalaVelocidad::Guardia, 0.5f);
	TestEqual(TEXT("Solo la guardia"), Movimiento->ObtenerEscalaVelocidadAccion(), 0.5f);
	return true;
}

// E7: una parada de golpe a escala 0 termina, y devuelve la dilatación que había (no siempre 1).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaParadaEscalaCero, "SAEVUS.Combate.ParadaEscalaCero", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaParadaEscalaCero::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	AActor* Actor = MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(FVector::ZeroVector);
	Actor->CustomTimeDilation = 0.5f;
	FSAEVUSParadaGolpe Parada;
	TArray<AActor*> Actores = { Actor };
	Parada.Iniciar(Actores, 0.f, 0.1f);
	TestEqual(TEXT("Congelado"), Actor->CustomTimeDilation, 0.f);
	Parada.Actualizar(0.06f);
	Parada.Actualizar(0.06f);
	TestFalse(TEXT("La parada a escala 0 termina con el tiempo del mundo"), Parada.EstaActiva());
	TestEqual(TEXT("Vuelve la dilatación de antes"), Actor->CustomTimeDilation, 0.5f);
	return true;
}

// E8: un enemigo que desaparece sin morir (caído fuera del mundo) no deja la puerta cerrada para siempre.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaPuertaEnemigoDesaparece, "SAEVUS.Tutorial.PuertaEnemigoDesaparece", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaPuertaEnemigoDesaparece::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	FMundoRevision Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.MundoPrueba.CrearEnemigo(FVector(800.f, 0.f, 100.f), CrearDatos(0.f), Mundo.Personaje);
	ASAEVUSPuertaArena* Puerta = Mundo.MundoPrueba.CrearActor<ASAEVUSPuertaArena>(FVector(0.f, 800.f, 100.f));
	Puerta->FijarEnemigos({ Enemigo });
	TestFalse(TEXT("Cerrada con el enemigo vivo"), Puerta->EstaAbierta());
	Enemigo->Destroy();
	TestTrue(TEXT("Abierta cuando el enemigo desaparece sin morir"), Puerta->EstaAbierta());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaMuelle, "SAEVUS.Animacion.Muelle", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaMuelle::RunTest(const FString& Parameters)
{
	FSAEVUSMuelle Vivo;
	float Maximo = 0.f;
	for (int32 Indice = 0; Indice < 120; ++Indice)
	{
		Vivo.Avanzar(10.f, 2.f, 0.3f, 1.f / 60.f);
		Maximo = FMath::Max(Maximo, Vivo.Valor);
	}
	TestTrue(TEXT("Con poca amortiguación se pasa del objetivo"), Maximo > 10.5f);
	TestEqual(TEXT("y se asienta en él"), Vivo.Valor, 10.f, 0.5f);

	FSAEVUSMuelle Critico;
	float MaximoCritico = 0.f;
	for (int32 Indice = 0; Indice < 120; ++Indice)
	{
		Critico.Avanzar(10.f, 2.f, 1.f, 1.f / 60.f);
		MaximoCritico = FMath::Max(MaximoCritico, Critico.Valor);
	}
	TestTrue(TEXT("Con amortiguación crítica no se pasa"), MaximoCritico <= 10.05f);

	// Un fotograma larguísimo no lo hace explotar.
	FSAEVUSMuelleVector Largo;
	Largo.Avanzar(FVector(5.f, 0.f, 0.f), 8.f, 0.2f, 0.5f);
	TestTrue(TEXT("Estable con un fotograma de medio segundo"), FMath::Abs(Largo.Valor.X) < 50.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaMuelleEstable, "SAEVUS.Animacion.MuelleEstable", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaMuelleEstable::RunTest(const FString& Parameters)
{
	// Todas las combinaciones reales (cadera hasta ~44 Hz) con fotogramas desde 1/240 s hasta 0,4 s: ninguna explota (ANIMVFX-1).
	const float Pasos[] = { 1.f / 240.f, 1.f / 144.f, 1.f / 60.f, 1.f / 30.f, 1.f / 15.f, 0.1f, 0.25f, 0.4f };
	const float Frecuencias[] = { 1.f, 2.2f, 8.f, 20.f, 43.6f };
	const float Amortiguaciones[] = { 0.1f, 0.3f, 0.7f, 1.f };
	for (const float Paso : Pasos)
	{
		for (const float Frecuencia : Frecuencias)
		{
			for (const float Amortiguacion : Amortiguaciones)
			{
				FSAEVUSMuelle Muelle;
				Muelle.Valor = 55.f;
				Muelle.Velocidad = 1000.f;
				float Maximo = 0.f;
				for (int32 Indice = 0; Indice < FMath::CeilToInt(30.f / FMath::Min(Paso, 0.1f)); ++Indice)
				{
					Muelle.Avanzar(0.f, Frecuencia, Amortiguacion, Paso);
					Maximo = FMath::Max(Maximo, FMath::Abs(Muelle.Valor));
				}
				const FString Caso = FString::Printf(TEXT("paso %.4f, %.1f Hz, amortiguación %.1f"), Paso, Frecuencia, Amortiguacion);
				TestTrue(TEXT("Finito: ") + Caso, FMath::IsFinite(Muelle.Valor) && FMath::IsFinite(Muelle.Velocidad));
				TestTrue(TEXT("Acotado: ") + Caso, Maximo < 400.f);
				TestTrue(TEXT("Se asienta: ") + Caso, FMath::Abs(Muelle.Valor) < 1.f);
			}
		}
	}
	// Un NaN heredado se descarta en vez de contagiar al siguiente fotograma.
	FSAEVUSMuelle Roto;
	Roto.Valor = NAN;
	Roto.Avanzar(3.f, 4.f, 0.5f, 1.f / 60.f);
	TestEqual(TEXT("El muelle roto parte del objetivo"), Roto.Valor, 3.f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGiroPorCapas, "SAEVUS.Animacion.GiroPorCapas", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGiroPorCapas::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	const FSAEVUSAjusteSecundario Ajuste;
	FSAEVUSCapaSecundaria Capa;
	FSAEVUSEntradaSecundaria Entrada;
	Entrada.bReposoLibre = true;
	Avanzar(Capa, Ajuste, Entrada, 0.5f);
	// Giro rápido a la derecha (la cámara): 60 grados en un cuarto de segundo.
	float CaderaMinima = 0.f;
	float CabezaMaxima = 0.f;
	Entrada.DeltaTime = SAEVUSPruebas::PasoSimulacion;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(0.25f); ++Indice)
	{
		Entrada.Guinada += 60.f / SAEVUSPruebas::PasosPara(0.25f);
		Capa.Actualizar(Ajuste, Entrada);
		CaderaMinima = FMath::Min(CaderaMinima, Capa.ObtenerSalida().GiroCadera);
		CabezaMaxima = FMath::Max(CabezaMaxima, Capa.ObtenerSalida().GiroCuello + Capa.ObtenerSalida().GiroCabeza);
	}
	AddInfo(FString::Printf(TEXT("Cadera %.1f grados atrás, cabeza %.1f grados por delante del pecho"), -CaderaMinima, CabezaMaxima));
	TestTrue(TEXT("La cadera se queda atrás del giro"), CaderaMinima < -15.f);
	TestTrue(TEXT("La cabeza se adelanta al pecho"), CabezaMaxima > 5.f);
	Avanzar(Capa, Ajuste, Entrada, 3.f);
	TestEqual(TEXT("Y todo vuelve a su sitio"), Capa.ObtenerSalida().GiroCadera, 0.f, 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaInerciaFrenada, "SAEVUS.Animacion.InerciaFrenada", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaInerciaFrenada::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	const FSAEVUSAjusteSecundario Ajuste;
	FSAEVUSCapaSecundaria Capa;
	FSAEVUSEntradaSecundaria Entrada;
	// Corriendo hacia delante (guiñada 0 = +X).
	Entrada.Velocidad = FVector2D(550.f, 0.f);
	Avanzar(Capa, Ajuste, Entrada, 1.f);
	// Frenada en seco: el tronco sigue hacia delante y luego vuelve.
	Entrada.Velocidad = FVector2D::ZeroVector;
	Entrada.DeltaTime = SAEVUSPruebas::PasoSimulacion;
	float Maximo = 0.f;
	float Minimo = 0.f;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(1.5f); ++Indice)
	{
		Capa.Actualizar(Ajuste, Entrada);
		Maximo = FMath::Max(Maximo, static_cast<float>(Capa.ObtenerSalida().InerciaTronco.X));
		Minimo = FMath::Min(Minimo, static_cast<float>(Capa.ObtenerSalida().InerciaTronco.X));
	}
	AddInfo(FString::Printf(TEXT("El tronco se va %.1f grados adelante y vuelve %.1f atrás"), Maximo, -Minimo));
	TestTrue(TEXT("Al frenar, el tronco sigue hacia delante"), Maximo > 4.f);
	TestTrue(TEXT("y se balancea de vuelta (no se para en seco)"), Minimo < -0.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaCansancio, "SAEVUS.Animacion.Cansancio", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaCansancio::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	const FSAEVUSAjusteSecundario Ajuste;
	FSAEVUSCapaSecundaria Capa;
	FSAEVUSEntradaSecundaria Entrada;
	Entrada.bEsfuerzo = true;
	Avanzar(Capa, Ajuste, Entrada, 3.f);
	TestTrue(TEXT("El esfuerzo cansa"), Capa.ObtenerSalida().Cansancio > 0.8f);
	Entrada.bEsfuerzo = false;
	Entrada.bReposoLibre = true;
	// Respiración agitada: más honda que en calma.
	float Honda = 0.f;
	Entrada.DeltaTime = SAEVUSPruebas::PasoSimulacion;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(1.5f); ++Indice)
	{
		Capa.Actualizar(Ajuste, Entrada);
		Honda = FMath::Max(Honda, Capa.ObtenerSalida().Respiracion);
	}
	TestTrue(TEXT("Cansado, respira hondo"), Honda > Ajuste.AmplitudRespiracionCalma * 1.5f);
	Avanzar(Capa, Ajuste, Entrada, 10.f);
	TestTrue(TEXT("En calma se le pasa"), Capa.ObtenerSalida().Cansancio < 0.1f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaReposoVivo, "SAEVUS.Animacion.ReposoVivo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaReposoVivo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	const FSAEVUSAjusteSecundario Ajuste;
	FSAEVUSCapaSecundaria Capa;
	FSAEVUSEntradaSecundaria Entrada;
	Entrada.bReposoLibre = true;
	Entrada.DeltaTime = SAEVUSPruebas::PasoSimulacion;
	float VaivenMaximo = 0.f;
	bool bGesto = false;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Ajuste.TiempoGestoReposo + 6.f); ++Indice)
	{
		Capa.Actualizar(Ajuste, Entrada);
		VaivenMaximo = FMath::Max(VaivenMaximo, FMath::Abs(Capa.ObtenerSalida().VaivenPeso));
		bGesto |= Capa.ObtenerSalida().GestoReposo != ESAEVUSGestoReposo::Ninguno;
	}
	TestTrue(TEXT("Quieto, el peso pasa de un pie a otro"), VaivenMaximo > 0.5f);
	TestTrue(TEXT("Tras un rato quieto sale un gesto de reposo"), bGesto);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGestosDistintos, "SAEVUS.Animacion.GestosDistintos", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGestosDistintos::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	using E = ESAEVUSGestoAtaque;
	const E Gestos[] = { E::TajoDerecha, E::Reves, E::Remate, E::Estocada, E::TajoArriba, E::AereoDiagonal, E::AereoReves, E::AereoRemate, E::Giratorio };
	// Cada gesto tiene su propio recorrido de la hoja: comparados dos a dos, en algún momento de la fase activa van distinto.
	for (int32 A = 0; A < UE_ARRAY_COUNT(Gestos); ++A)
	{
		for (int32 B = A + 1; B < UE_ARRAY_COUNT(Gestos); ++B)
		{
			float Diferencia = 0.f;
			for (const float Tiempo : { 0.8f, 1.f, 1.5f, 2.f })
			{
				const FSAEVUSPoseGesto PoseA = SAEVUSGestos::Evaluar(Gestos[A], Tiempo);
				const FSAEVUSPoseGesto PoseB = SAEVUSGestos::Evaluar(Gestos[B], Tiempo);
				Diferencia = FMath::Max(Diferencia, Angulo(PoseA.Hoja, PoseB.Hoja));
				Diferencia = FMath::Max(Diferencia, FMath::Abs(PoseA.GiroPecho - PoseB.GiroPecho));
				Diferencia = FMath::Max(Diferencia, FMath::Abs(PoseA.Volteo - PoseB.Volteo));
				Diferencia = FMath::Max(Diferencia, FMath::Abs(PoseA.GiroRaiz - PoseB.GiroRaiz));
			}
			TestTrue(FString::Printf(TEXT("Los gestos %d y %d se ven distintos"), A, B), Diferencia > 20.f);
		}
	}
	// El barrido dibujado va hacia donde va la hoja.
	for (const E Gesto : { E::TajoDerecha, E::Reves, E::AereoReves })
	{
		const float Desde = static_cast<float>(SAEVUSGestos::Evaluar(Gesto, 1.f).Hoja.X);
		const float Hasta = static_cast<float>(SAEVUSGestos::Evaluar(Gesto, 2.f).Hoja.X);
		TestEqual(TEXT("Sentido del barrido = sentido de la hoja"), FMath::Sign(Desde - Hasta), SAEVUSCombate::SentidoBarrido(Gesto));
	}
	// El combo fluye: cada tajo empieza cerca de donde acabó el anterior.
	TestTrue(TEXT("El revés sale de donde acaba el tajo"), Angulo(SAEVUSGestos::Evaluar(E::TajoDerecha, 2.3f).Hoja, SAEVUSGestos::Evaluar(E::Reves, 0.f).Hoja) < 45.f);
	TestTrue(TEXT("El remate sale de donde acaba el revés"), Angulo(SAEVUSGestos::Evaluar(E::Reves, 2.3f).Hoja, SAEVUSGestos::Evaluar(E::Remate, 0.f).Hoja) < 10.f);
	// El repertorio por defecto usa un gesto distinto en cada golpe de cada cadena.
	const FSAEVUSRepertorioMelee Repertorio;
	TSet<E> Usados;
	for (const FSAEVUSDefinicionAtaque& Ataque : Repertorio.ComboSuelo)
	{
		Usados.Add(Ataque.Gesto);
	}
	for (const FSAEVUSDefinicionAtaque& Ataque : Repertorio.ComboAire)
	{
		Usados.Add(Ataque.Gesto);
	}
	Usados.Add(Repertorio.TajoArriba.Gesto);
	Usados.Add(Repertorio.TajoEnCarrera.Gesto);
	Usados.Add(Repertorio.AtaqueCargado.Gesto);
	TestEqual(TEXT("Nueve golpes, nueve gestos"), Usados.Num(), 9);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaGestosEnemigo, "SAEVUS.Enemigo.Gestos", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaGestosEnemigo::RunTest(const FString& Parameters)
{
	FSAEVUSAtaqueEnemigo Ataque;
	Ataque.Tipo = ESAEVUSTipoAtaqueEnemigo::Golpe;
	TestEqual(TEXT("Un golpe sin gesto es un tajo"), Ataque.ObtenerGesto(), ESAEVUSGestoEnemigo::Tajo);
	Ataque.Tipo = ESAEVUSTipoAtaqueEnemigo::Proyectil;
	TestEqual(TEXT("Un proyectil se lanza"), Ataque.ObtenerGesto(), ESAEVUSGestoEnemigo::Lanzamiento);
	Ataque.Gesto = ESAEVUSGestoEnemigo::Mazazo;
	TestEqual(TEXT("El gesto elegido manda"), Ataque.ObtenerGesto(), ESAEVUSGestoEnemigo::Mazazo);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEcoDash, "SAEVUS.Efectos.EcoDash", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEcoDash::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasRevision;
	USkeletalMesh* Caballero = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/SAEVUS/Arte/Personajes/Caballero/SK_Caballero.SK_Caballero"));
	if (!TestNotNull(TEXT("Caballero importado"), Caballero))
	{
		return false;
	}
	FMundoRevision Mundo;
	Mundo.Personaje->ObtenerCuerpo()->SetSkinnedAssetAndUpdate(Caballero, true);
	Mundo.Personaje->ObtenerAnimacion()->FijarMalla(Mundo.Personaje->ObtenerCuerpo());
	const USAEVUSComponenteEcoDash* Eco = Mundo.Personaje->ObtenerEcoDash();
	if (!TestNotNull(TEXT("El personaje deja ecos"), Eco))
	{
		return false;
	}
	Mundo.Personaje->ObtenerMovimiento()->PedirDash();
	int32 Maximo = 0;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(0.2f); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
		Maximo = FMath::Max(Maximo, Eco->ObtenerEcosVisibles());
	}
	AddInfo(FString::Printf(TEXT("Ecos a la vista durante el dash: %d"), Maximo));
	TestTrue(TEXT("El dash deja varios ecos"), Maximo >= 2);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(1.f); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	TestEqual(TEXT("Y se apagan"), Eco->ObtenerEcosVisibles(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaFuegoDescansa, "SAEVUS.Mundo.FuegoBrasero", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaFuegoDescansa::RunTest(const FString& Parameters)
{
	// Sin render (como fuera de la vista), el fuego no titila ni suelta brasas: no cuesta nada.
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSFuegoBrasero* Fuego = MundoPrueba.CrearActor<ASAEVUSFuegoBrasero>(FVector::ZeroVector);
	const float Inicial = Fuego->ObtenerIntensidadActual();
	for (int32 Indice = 0; Indice < 30; ++Indice)
	{
		Fuego->Tick(SAEVUSPruebas::PasoSimulacion);
	}
	TestTrue(TEXT("Tiene luz"), Inicial > 0.f);
	TestEqual(TEXT("Sin verse no titila"), Fuego->ObtenerIntensidadActual(), Inicial);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTonoGolpes, "SAEVUS.Audio.TonoGolpes", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTonoGolpes::RunTest(const FString& Parameters)
{
	// Cada golpe suena distinto: el remate y el giro cargado graves, la estocada aguda.
	const FSAEVUSRepertorioMelee Repertorio;
	const float Tajo = Repertorio.ComboSuelo[0].TonoSonido;
	TestTrue(TEXT("El remate suena más grave que el tajo"), Repertorio.ComboSuelo[2].TonoSonido < Tajo);
	TestTrue(TEXT("El giro cargado, el más grave"), Repertorio.AtaqueCargado.TonoSonido < Repertorio.ComboSuelo[2].TonoSonido);
	TestTrue(TEXT("La estocada en carrera suena aguda"), Repertorio.TajoEnCarrera.TonoSonido > Tajo);
	TestNotEqual(TEXT("El revés no suena como el tajo"), Repertorio.ComboSuelo[1].TonoSonido, Tajo);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
