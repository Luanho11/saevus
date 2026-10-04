// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSComponenteVida.h"

namespace SAEVUSPruebasProyectiles
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(6000.f, 6000.f, 100.f);
	const FVector PosicionJugador(0.f, 0.f, 150.f);
	constexpr float DistanciaEnemigo = 800.f;
	constexpr float Danio = 15.f;
	constexpr float VelocidadLenta = 300.f;
	constexpr float DistanciaLanzamiento = 400.f;
	constexpr float DistanciaParaDevolver = 130.f;
	constexpr float TiempoMax = 3.f;
	constexpr float TiempoAsentar = 2.f;

	struct FMundoProyectiles
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Jugador = nullptr;
		bool bListo = false;

		FMundoProyectiles()
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
			bListo = MundoPrueba.AsentarEnSuelo();
		}

		ASAEVUSEnemigo* CrearEnemigo(const USAEVUSDatosEnemigo* Datos, APawn* Objetivo)
		{
			ASAEVUSEnemigo* Enemigo = MundoPrueba.CrearEnemigo(FVector(DistanciaEnemigo, 0.f, 100.f), Datos, nullptr);
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoAsentar) && !Enemigo->GetCharacterMovement()->IsMovingOnGround(); ++Indice)
			{
				MundoPrueba.Paso();
			}
			Enemigo->ObtenerIA()->FijarObjetivo(Objetivo);
			return Enemigo;
		}

		/** Proyectil del lanzador dado, delante del jugador (+X) a Distancia, hacia él. */
		ASAEVUSProyectil* Lanzar(AActor* Lanzador, float Distancia, float Velocidad)
		{
			ASAEVUSProyectil* Proyectil = MundoPrueba.CrearActor<ASAEVUSProyectil>(Jugador->GetActorLocation() + FVector(Distancia, 0.f, 0.f));
			Proyectil->Lanzar(-FVector::ForwardVector, Velocidad, Danio, Lanzador, nullptr);
			return Proyectil;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaProyectilEscupidor, "SAEVUS.Enemigo.Proyectil", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaProyectilEscupidor::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasProyectiles;

	USAEVUSDatosEnemigo* Datos = NewObject<USAEVUSDatosEnemigo>(GetTransientPackage());
	FSAEVUSAtaqueEnemigo Escupitajo;
	Escupitajo.Tipo = ESAEVUSTipoAtaqueEnemigo::Proyectil;
	Escupitajo.DistanciaMinima = 300.f;
	Escupitajo.DistanciaMaxima = 1500.f;
	Escupitajo.TiempoTelegrafiar = 0.3f;
	Escupitajo.Duracion = 0.3f;
	Escupitajo.Danio = Danio;
	Escupitajo.Velocidad = 900.f;
	Datos->Enemigo.Ataques = { Escupitajo };
	Datos->Enemigo.DanioContacto = 0.f;

	FMundoProyectiles Mundo;
	if (!TestTrue(TEXT("Jugador de pie"), Mundo.bListo))
	{
		return false;
	}
	USAEVUSComponenteVida* Vida = Mundo.Jugador->ObtenerVida();
	const float VidaInicial = Vida->ObtenerVida();
	Mundo.CrearEnemigo(Datos, Mundo.Jugador);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Vida->ObtenerVida() == VidaInicial; ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	TestEqual(TEXT("El escupitajo alcanza al jugador"), Vida->ObtenerVida(), VidaInicial - Danio);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaProyectilGuardia, "SAEVUS.Guardia.BloqueaProyectil", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaProyectilGuardia::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasProyectiles;

	FMundoProyectiles Mundo;
	if (!TestTrue(TEXT("Jugador de pie"), Mundo.bListo))
	{
		return false;
	}
	Mundo.Jugador->FijarGuardia(true);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(0.5f); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	USAEVUSComponenteVida* Vida = Mundo.Jugador->ObtenerVida();
	const float VidaInicial = Vida->ObtenerVida();
	TWeakObjectPtr<ASAEVUSProyectil> Proyectil = Mundo.Lanzar(nullptr, DistanciaLanzamiento, 900.f);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Proyectil.IsValid(); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	TestFalse(TEXT("El proyectil se deshace al chocar"), Proyectil.IsValid());
	TestEqual(TEXT("La guardia lo para"), Vida->ObtenerVida(), VidaInicial);
	TestEqual(TEXT("Cuenta como bloqueo"), Mundo.Jugador->ObtenerGuardia()->ObtenerBloqueos(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaProyectilDevolver, "SAEVUS.Combate.DevolverProyectil", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaProyectilDevolver::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasProyectiles;

	FMundoProyectiles Mundo;
	if (!TestTrue(TEXT("Jugador de pie"), Mundo.bListo))
	{
		return false;
	}
	// El lanzador, quieto (sin objetivo), al que vuelve el proyectil.
	ASAEVUSEnemigo* Lanzador = Mundo.CrearEnemigo(nullptr, nullptr);
	USAEVUSComponenteVida* VidaLanzador = Lanzador->ObtenerVida();
	const float VidaInicial = VidaLanzador->ObtenerVida();
	TWeakObjectPtr<ASAEVUSProyectil> Proyectil = Mundo.Lanzar(Lanzador, DistanciaParaDevolver, VelocidadLenta);
	Mundo.Jugador->PedirAtaque();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Proyectil.IsValid() && !Proyectil->FueDevuelto(); ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	if (!TestTrue(TEXT("El tajo lo devuelve"), Proyectil.IsValid() && Proyectil->FueDevuelto()))
	{
		return false;
	}
	TestTrue(TEXT("Ahora va hacia el lanzador (+X)"), Proyectil->ObtenerVelocidad().X > 0.f);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && VidaLanzador->ObtenerVida() == VidaInicial; ++Indice)
	{
		Mundo.MundoPrueba.Paso();
	}
	TestEqual(TEXT("Devuelto, daña al lanzador"), VidaLanzador->ObtenerVida(), VidaInicial - Danio);
	TestEqual(TEXT("Y no al jugador"), Mundo.Jugador->ObtenerVida()->ObtenerVida(), Mundo.Jugador->ObtenerVida()->ObtenerVidaMaxima());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
