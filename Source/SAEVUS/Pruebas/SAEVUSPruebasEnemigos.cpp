// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animacion/SAEVUSComponenteAnimacionProcedural.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Enemigos/SAEVUSComponenteEscudo.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Audio/SAEVUSComponenteSonidoEnemigo.h"

namespace SAEVUSPruebasEnemigos
{
	// Suelo de 60 m x 60 m, superficie en Z = 0; el jugador de pie en el origen mirando a +X.
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(6000.f, 6000.f, 100.f);
	const FVector PosicionJugador(0.f, 0.f, 150.f);
	constexpr float AlturaAparicion = 100.f;

	constexpr float DistanciaLejos = 2000.f;
	// Más allá de RadioDeteccion x FactorDistanciaReposoLejos (1000 x 2,5 por defecto).
	constexpr float DistanciaReposoLejano = 2800.f;
	constexpr float DistanciaVista = 800.f;
	constexpr float DistanciaAtaque = 400.f;
	constexpr float DistanciaContacto = 80.f;
	constexpr float DistanciaAlcanceEspada = 150.f;

	constexpr float TiempoAsentar = 2.f;
	constexpr float TiempoObservar = 0.5f;
	constexpr float TiempoMax = 3.f;
	constexpr float FraccionVelocidadEmbestida = 0.8f;
	constexpr float VelocidadQuieto = 50.f;
	constexpr float DanioLetal = 100000.f;

	struct FMundoEnemigos
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Jugador = nullptr;
		bool bListo = false;

		FMundoEnemigos()
		{
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
			bListo = MundoPrueba.AsentarEnSuelo();
		}

		/**
		 * Enemigo de pie a Distancia del jugador en +X; falso en bListo si no aterriza. Se asienta
		 * sin objetivo (no ataca mientras cae) y vuelve a su sitio antes de fijarle el jugador.
		 */
		ASAEVUSEnemigo* CrearEnemigo(float Distancia, const USAEVUSDatosEnemigo* Datos = nullptr)
		{
			const FVector Posicion(Distancia, 0.f, AlturaAparicion);
			ASAEVUSEnemigo* Enemigo = MundoPrueba.CrearEnemigo(Posicion, Datos, nullptr);
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoAsentar) && !Enemigo->GetCharacterMovement()->IsMovingOnGround(); ++Indice)
			{
				MundoPrueba.Paso();
			}
			bListo &= Enemigo->GetCharacterMovement()->IsMovingOnGround();
			Enemigo->SetActorLocation(FVector(Posicion.X, Posicion.Y, Enemigo->GetActorLocation().Z));
			Enemigo->GetCharacterMovement()->Velocity = FVector::ZeroVector;
			Enemigo->ObtenerIA()->FijarObjetivo(Jugador);
			return Enemigo;
		}

		void AvanzarDurante(float Segundos)
		{
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Segundos); ++Indice)
			{
				MundoPrueba.Paso();
			}
		}

		/** Avanza hasta que la IA está en Estado; falso si no llega en TiempoMax. */
		bool AvanzarHastaEstado(const ASAEVUSEnemigo* Enemigo, ESAEVUSEstadoEnemigo Estado)
		{
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Enemigo->ObtenerIA()->ObtenerEstado() != Estado; ++Indice)
			{
				MundoPrueba.Paso();
			}
			return Enemigo->ObtenerIA()->ObtenerEstado() == Estado;
		}
	};

	USAEVUSDatosEnemigo* CrearDatos()
	{
		return NewObject<USAEVUSDatosEnemigo>(GetTransientPackage());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoDeteccion, "SAEVUS.Enemigo.Deteccion", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoDeteccion::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaLejos);
	if (!TestTrue(TEXT("Jugador y enemigo de pie"), Mundo.bListo))
	{
		return false;
	}
	const FVector Inicio = Enemigo->GetActorLocation();
	Mundo.AvanzarDurante(TiempoObservar);
	TestEqual(TEXT("Lejos: en reposo"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Reposo);
	TestTrue(TEXT("En reposo patrulla"), FVector::Dist2D(Enemigo->GetActorLocation(), Inicio) > 1.f);

	Enemigo->SetActorLocation(FVector(DistanciaVista, 0.f, Enemigo->GetActorLocation().Z));
	Mundo.MundoPrueba.Paso();
	TestEqual(TEXT("A la vista: persigue"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Perseguir);
	const float DistanciaAntes = static_cast<float>(FVector::Dist2D(Enemigo->GetActorLocation(), Mundo.Jugador->GetActorLocation()));
	Mundo.AvanzarDurante(TiempoObservar);
	const float DistanciaDespues = static_cast<float>(FVector::Dist2D(Enemigo->GetActorLocation(), Mundo.Jugador->GetActorLocation()));
	AddInfo(FString::Printf(TEXT("Distancia al jugador: %.0f -> %.0f cm"), DistanciaAntes, DistanciaDespues));
	TestTrue(TEXT("Se acerca al jugador"), DistanciaDespues < DistanciaAntes);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoReposoLejano, "SAEVUS.Enemigo.ReposoLejano", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoReposoLejano::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaReposoLejano);
	if (!TestTrue(TEXT("Jugador y enemigo de pie"), Mundo.bListo))
	{
		return false;
	}
	const USAEVUSComponenteIAEnemigo* IA = Enemigo->ObtenerIA();
	const float Intervalo = IA->ObtenerAjuste().IntervaloReposoLejos;
	Mundo.AvanzarDurante(TiempoObservar);
	TestEqual(TEXT("Lejos del todo: en reposo"), IA->ObtenerEstado(), ESAEVUSEstadoEnemigo::Reposo);
	TestEqual(TEXT("La IA descansa"), IA->GetComponentTickInterval(), Intervalo);
	TestEqual(TEXT("El movimiento también"), Enemigo->GetCharacterMovement()->GetComponentTickInterval(), Intervalo);

	// Al acercarse el jugador, despierta en un intervalo como mucho y todo vuelve a cada fotograma.
	Enemigo->SetActorLocation(FVector(DistanciaVista, 0.f, Enemigo->GetActorLocation().Z));
	Mundo.AvanzarDurante(Intervalo + SAEVUSPruebas::PasoSimulacion * 2.f);
	TestEqual(TEXT("Despierta y persigue"), IA->ObtenerEstado(), ESAEVUSEstadoEnemigo::Perseguir);
	TestEqual(TEXT("IA cada fotograma"), IA->GetComponentTickInterval(), 0.f);
	TestEqual(TEXT("Movimiento cada fotograma"), Enemigo->GetCharacterMovement()->GetComponentTickInterval(), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoVolador, "SAEVUS.Enemigo.Volador", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoVolador::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	constexpr float AlturaInicio = 400.f;
	constexpr float ToleranciaAltura = 80.f;
	FMundoEnemigos Mundo;
	if (!TestTrue(TEXT("Jugador de pie"), Mundo.bListo))
	{
		return false;
	}
	USAEVUSDatosEnemigo* Datos = CrearDatos();
	FSAEVUSAjusteEnemigo& Ajuste = Datos->Enemigo;
	Ajuste.bVolador = true;
	Ajuste.RadioPatrulla = 0.f;
	FSAEVUSAtaqueEnemigo Picado;
	Picado.Nombre = TEXT("Picado");
	Picado.Tipo = ESAEVUSTipoAtaqueEnemigo::Picado;
	Picado.DistanciaMaxima = DistanciaAtaque;
	Picado.Velocidad = 900.f;
	Picado.Duracion = 0.5f;
	Ajuste.Ataques = { Picado };

	// Sin objetivo: vuela en su sitio, no cae.
	ASAEVUSEnemigo* Enemigo = Mundo.MundoPrueba.CrearEnemigo(FVector(DistanciaLejos, 0.f, AlturaInicio), Datos, nullptr);
	UCharacterMovementComponent* Movimiento = Enemigo->GetCharacterMovement();
	Mundo.AvanzarDurante(TiempoObservar);
	TestTrue(TEXT("Vuela"), Movimiento->IsFlying());
	TestEqual(TEXT("No cae"), static_cast<float>(Enemigo->GetActorLocation().Z), AlturaInicio, ToleranciaAltura * 0.25f);

	// Con objetivo a la vista: se pone a su altura de vuelo sobre el jugador y avisa.
	Enemigo->ObtenerIA()->FijarObjetivo(Mundo.Jugador);
	Enemigo->SetActorLocation(FVector(DistanciaVista, 0.f, AlturaInicio));
	if (!TestTrue(TEXT("Persigue y avisa"), Mundo.AvanzarHastaEstado(Enemigo, ESAEVUSEstadoEnemigo::Telegrafiar)))
	{
		return false;
	}
	const float AlturaDeseada = static_cast<float>(Mundo.Jugador->GetActorLocation().Z) + Ajuste.AlturaVuelo;
	AddInfo(FString::Printf(TEXT("Altura al avisar %.0f cm, deseada %.0f cm"), Enemigo->GetActorLocation().Z, AlturaDeseada));
	TestEqual(TEXT("A su altura de vuelo sobre el jugador"), static_cast<float>(Enemigo->GetActorLocation().Z), AlturaDeseada, ToleranciaAltura);

	// Picado: hacia abajo, hacia el jugador, a la velocidad del ataque.
	if (!TestTrue(TEXT("Ataca"), Mundo.AvanzarHastaEstado(Enemigo, ESAEVUSEstadoEnemigo::Atacar)))
	{
		return false;
	}
	Mundo.MundoPrueba.Paso();
	AddInfo(FString::Printf(TEXT("Velocidad del picado %s"), *Movimiento->Velocity.ToCompactString()));
	TestTrue(TEXT("Cae en picado"), Movimiento->Velocity.Z < 0.f);
	TestTrue(TEXT("Hacia el jugador"), Movimiento->Velocity.X < 0.f);
	TestTrue(TEXT("A la velocidad del picado"), Movimiento->Velocity.Size() > Picado.Velocidad * 0.8f);

	// Un golpe lo derriba (cae aturdido) y después vuelve a volar.
	UGameplayStatics::ApplyDamage(Enemigo, 1.f, nullptr, nullptr, UDamageType::StaticClass());
	Movimiento->SetMovementMode(MOVE_Falling);
	TestEqual(TEXT("Aturdido"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Aturdido);
	Mundo.MundoPrueba.Paso();
	TestTrue(TEXT("Aturdido cae"), Movimiento->IsFalling());
	Mundo.AvanzarDurante(Ajuste.TiempoAturdido + TiempoObservar);
	TestTrue(TEXT("Vuelve a volar"), Movimiento->IsFlying());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoEscudo, "SAEVUS.Enemigo.Escudo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoEscudo::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	constexpr float DanioFlojo = 10.f;
	constexpr float DanioFuerte = 30.f;
	FMundoEnemigos Mundo;
	USAEVUSDatosEnemigo* Datos = CrearDatos();
	Datos->Enemigo.AnguloEscudo = 120.f;
	Datos->Enemigo.DanioRompeEscudo = 28.f;
	Datos->Enemigo.VidaMaxima = 200.f;
	Datos->Enemigo.Ataques.Reset();
	Datos->Enemigo.RadioPatrulla = 0.f;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaVista, Datos);
	if (!TestTrue(TEXT("Jugador y enemigo de pie"), Mundo.bListo))
	{
		return false;
	}
	// El enemigo mira al jugador (que está en -X).
	Enemigo->SetActorRotation(FRotator(0.f, 180.f, 0.f));
	const USAEVUSComponenteVida* Vida = Enemigo->ObtenerVida();
	const USAEVUSComponenteEscudo* Escudo = Enemigo->FindComponentByClass<USAEVUSComponenteEscudo>();
	const float VidaInicial = Vida->ObtenerVida();
	UGameplayStatics::ApplyDamage(Enemigo, DanioFlojo, nullptr, Mundo.Jugador, UDamageType::StaticClass());
	TestEqual(TEXT("De frente, el escudo bloquea"), Vida->ObtenerVida(), VidaInicial);
	TestEqual(TEXT("Un bloqueo"), Escudo->ObtenerBloqueos(), 1);

	// Por detrás pasa.
	Mundo.Jugador->SetActorLocation(Enemigo->GetActorLocation() + FVector(300.f, 0.f, 0.f));
	UGameplayStatics::ApplyDamage(Enemigo, DanioFlojo, nullptr, Mundo.Jugador, UDamageType::StaticClass());
	TestEqual(TEXT("Por detrás, el golpe entra"), Vida->ObtenerVida(), VidaInicial - DanioFlojo);

	// De frente, un golpe fuerte lo rompe y pasa; mientras está roto, los flojos también pasan.
	Mundo.Jugador->SetActorLocation(Enemigo->GetActorLocation() - FVector(300.f, 0.f, 0.f));
	UGameplayStatics::ApplyDamage(Enemigo, DanioFuerte, nullptr, Mundo.Jugador, UDamageType::StaticClass());
	TestTrue(TEXT("El golpe fuerte rompe el escudo"), Escudo->EstaRoto());
	TestEqual(TEXT("y hace daño"), Vida->ObtenerVida(), VidaInicial - DanioFlojo - DanioFuerte);
	UGameplayStatics::ApplyDamage(Enemigo, DanioFlojo, nullptr, Mundo.Jugador, UDamageType::StaticClass());
	TestEqual(TEXT("Roto, no bloquea"), Vida->ObtenerVida(), VidaInicial - 2.f * DanioFlojo - DanioFuerte);
	TestEqual(TEXT("Sin bloqueos nuevos"), Escudo->ObtenerBloqueos(), 1);

	// Desde encima (pogo) no hay dirección: pasa.
	Mundo.MundoPrueba.ObtenerMundo()->TimeSeconds += Datos->Enemigo.TiempoEscudoRoto + 1.f;
	TestFalse(TEXT("El escudo se recupera"), Escudo->EstaRoto());
	Mundo.Jugador->SetActorLocation(Enemigo->GetActorLocation() + FVector(0.f, 0.f, 300.f));
	const float VidaAntesPogo = Vida->ObtenerVida();
	UGameplayStatics::ApplyDamage(Enemigo, DanioFlojo, nullptr, Mundo.Jugador, UDamageType::StaticClass());
	TestEqual(TEXT("Desde encima pasa (pogo)"), Vida->ObtenerVida(), VidaAntesPogo - DanioFlojo);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoEmbestida, "SAEVUS.Enemigo.AvisoYEmbestida", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoEmbestida::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaAtaque);
	if (!TestTrue(TEXT("Jugador y enemigo de pie"), Mundo.bListo) || !TestTrue(TEXT("Avisa"), Mundo.AvanzarHastaEstado(Enemigo, ESAEVUSEstadoEnemigo::Telegrafiar)))
	{
		return false;
	}
	const FSAEVUSAtaqueEnemigo* Ataque = Enemigo->ObtenerIA()->ObtenerAtaqueActual();
	if (!TestNotNull(TEXT("Ataque elegido"), Ataque))
	{
		return false;
	}
	Mundo.AvanzarDurante(Ataque->TiempoTelegrafiar * 0.5f);
	TestEqual(TEXT("A mitad del aviso sigue avisando"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Telegrafiar);
	TestTrue(TEXT("Quieto mientras avisa"), Enemigo->GetVelocity().Size2D() < VelocidadQuieto);

	if (!TestTrue(TEXT("Ataca tras el aviso"), Mundo.AvanzarHastaEstado(Enemigo, ESAEVUSEstadoEnemigo::Atacar)))
	{
		return false;
	}
	Mundo.MundoPrueba.Paso();
	const FVector Velocidad = Enemigo->GetVelocity();
	AddInfo(FString::Printf(TEXT("Velocidad de la embestida %s"), *Velocidad.ToCompactString()));
	TestTrue(TEXT("Embiste hacia el jugador (-X)"), Velocidad.X < -Ataque->Velocidad * FraccionVelocidadEmbestida);
	TestTrue(TEXT("Se recupera al acabar"), Mundo.AvanzarHastaEstado(Enemigo, ESAEVUSEstadoEnemigo::Recuperarse));
	// Sonido: aviso y ataque.
	TestTrue(TEXT("El aviso y el ataque suenan"), Enemigo->FindComponentByClass<USAEVUSComponenteSonidoEnemigo>()->ObtenerReproducciones() >= 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoContacto, "SAEVUS.Enemigo.DanioContacto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoContacto::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	FMundoEnemigos Mundo;
	USAEVUSComponenteVida* VidaJugador = Mundo.Jugador->ObtenerVida();
	const float VidaInicial = VidaJugador->ObtenerVida();
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaContacto);
	if (!TestTrue(TEXT("Jugador y enemigo de pie"), Mundo.bListo))
	{
		return false;
	}
	Mundo.MundoPrueba.Paso();
	TestEqual(TEXT("Tocar al enemigo quita DanioContacto"), VidaJugador->ObtenerVida(), VidaInicial - Enemigo->ObtenerAjuste().DanioContacto);
	TestTrue(TEXT("El jugador queda invulnerable un momento"), VidaJugador->EstaEnInvulnerabilidadTrasGolpe());
	// El empuje se aplica en la siguiente actualización del movimiento del jugador.
	Mundo.MundoPrueba.Paso();
	TestTrue(TEXT("El golpe echa al jugador lejos del enemigo (-X)"), Mundo.Jugador->GetVelocity().X < 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoAturdido, "SAEVUS.Enemigo.Aturdido", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoAturdido::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaAlcanceEspada);
	if (!TestTrue(TEXT("Jugador y enemigo de pie"), Mundo.bListo))
	{
		return false;
	}
	const float VidaEnemigo = Enemigo->ObtenerVida()->ObtenerVida();
	Mundo.Jugador->PedirAtaque();
	if (!TestTrue(TEXT("El tajo aturde al enemigo"), Mundo.AvanzarHastaEstado(Enemigo, ESAEVUSEstadoEnemigo::Aturdido)))
	{
		return false;
	}
	TestTrue(TEXT("Y le quita vida"), Enemigo->ObtenerVida()->ObtenerVida() < VidaEnemigo);
	TestTrue(TEXT("El golpe hace destellar al enemigo"), Enemigo->ObtenerDestello() > 0.f);
	TestEqual(TEXT("El jugador gana alma"), Mundo.Jugador->ObtenerAlma()->ObtenerAlma(), Mundo.Jugador->ObtenerAlma()->ObtenerAjuste().AlmaPorGolpe);
	Mundo.AvanzarDurante(Enemigo->ObtenerAjuste().TiempoAturdido + SAEVUSPruebas::PasoSimulacion * 2.f);
	TestNotEqual(TEXT("El aturdimiento pasa"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Aturdido);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoMuerte, "SAEVUS.Enemigo.Muerte", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoMuerte::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaLejos);
	if (!TestTrue(TEXT("Jugador y enemigo de pie"), Mundo.bListo))
	{
		return false;
	}
	int32 Muertes = 0;
	const FDelegateHandle Manejador = ASAEVUSEnemigo::AlMorirEnemigo.AddLambda([&Muertes](ASAEVUSEnemigo*) { ++Muertes; });
	Enemigo->ObtenerVida()->AplicarDanio(DanioLetal, Mundo.Jugador);
	ASAEVUSEnemigo::AlMorirEnemigo.Remove(Manejador);

	TestEqual(TEXT("Muerto"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Muerto);
	TestEqual(TEXT("Avisa de su muerte una vez"), Muertes, 1);
	TestTrue(TEXT("Desaparece al acabar la animación de muerte"), Enemigo->GetLifeSpan() > 0.f);
	TestEqual(TEXT("Sin colisión al morir"), Enemigo->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);

	// Muerto no hace daño por contacto.
	Enemigo->SetActorLocation(Mundo.Jugador->GetActorLocation() + FVector(DistanciaContacto, 0.f, 0.f));
	const float VidaJugador = Mundo.Jugador->ObtenerVida()->ObtenerVida();
	Mundo.AvanzarDurante(TiempoObservar);
	TestEqual(TEXT("Un enemigo muerto no hace daño"), Mundo.Jugador->ObtenerVida()->ObtenerVida(), VidaJugador);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoFases, "SAEVUS.Enemigo.FasesJefe", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoFases::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	USAEVUSDatosEnemigo* Datos = CrearDatos();
	FSAEVUSAjusteEnemigo& Jefe = Datos->Enemigo;
	Jefe.bEsJefe = true;
	Jefe.VidaMaxima = 100.f;
	FSAEVUSFaseEnemigo Primera;
	FSAEVUSFaseEnemigo Segunda;
	Segunda.FraccionVida = 0.5f;
	Segunda.EscalaVelocidad = 1.5f;
	Segunda.EscalaTiempos = 0.6f;
	Jefe.Fases = { Primera, Segunda };

	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaLejos, Datos);
	if (!TestTrue(TEXT("Jugador y jefe de pie"), Mundo.bListo))
	{
		return false;
	}
	TestEqual(TEXT("Empieza en la primera fase"), Enemigo->ObtenerIA()->ObtenerFase(), 0);
	Enemigo->ObtenerVida()->AplicarDanio(40.f, Mundo.Jugador);
	TestEqual(TEXT("Con el 60 % sigue en la primera"), Enemigo->ObtenerIA()->ObtenerFase(), 0);
	TestNotEqual(TEXT("Un jefe no se aturde"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Aturdido);
	Enemigo->ObtenerVida()->AplicarDanio(20.f, Mundo.Jugador);
	TestEqual(TEXT("Con el 40 % pasa a la segunda"), Enemigo->ObtenerIA()->ObtenerFase(), 1);
	Mundo.MundoPrueba.Paso();
	TestEqual(TEXT("La segunda fase acelera el paso"), Enemigo->GetCharacterMovement()->MaxWalkSpeed, Jefe.VelocidadMovimiento * Segunda.EscalaVelocidad, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoSalto, "SAEVUS.Enemigo.Salto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoSalto::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	USAEVUSDatosEnemigo* Datos = CrearDatos();
	FSAEVUSAtaqueEnemigo Salto;
	Salto.Nombre = TEXT("Salto");
	Salto.Tipo = ESAEVUSTipoAtaqueEnemigo::Salto;
	Salto.DistanciaMaxima = 600.f;
	Salto.Velocidad = 400.f;
	Salto.VelocidadVertical = 700.f;
	Salto.Duracion = 2.f;
	Datos->Enemigo.Ataques = { Salto };

	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaAtaque + 100.f, Datos);
	if (!TestTrue(TEXT("Jugador y saltador de pie"), Mundo.bListo) || !TestTrue(TEXT("Ataca"), Mundo.AvanzarHastaEstado(Enemigo, ESAEVUSEstadoEnemigo::Atacar)))
	{
		return false;
	}
	Mundo.MundoPrueba.Paso();
	TestTrue(TEXT("El salto lo lanza al aire"), Enemigo->GetCharacterMovement()->IsFalling());
	TestTrue(TEXT("Hacia el jugador"), Enemigo->GetVelocity().X < 0.f);
	TestTrue(TEXT("Al aterrizar se recupera"), Mundo.AvanzarHastaEstado(Enemigo, ESAEVUSEstadoEnemigo::Recuperarse));
	TestTrue(TEXT("Ya en el suelo"), Enemigo->GetCharacterMovement()->IsMovingOnGround());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoEsqueleto, "SAEVUS.Enemigo.Esqueleto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoEsqueleto::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	// Goblin con esqueleto (REF-05): la IA lo anima; al avisar alza el arma, así se lee el golpe que viene.
	constexpr float SubidaMinimaManoAviso = 20.f;
	const TCHAR* const RutaGoblin = TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Goblin.DA_Goblin");
	const USAEVUSDatosEnemigo* Datos = LoadObject<USAEVUSDatosEnemigo>(nullptr, RutaGoblin);
	if (!TestNotNull(TEXT("DA_Goblin (créalo con Verificar-Arranque.ps1)"), Datos) || !TestNotNull(TEXT("Goblin con malla esquelética"), Datos->MallaEsqueletica.Get()))
	{
		return false;
	}
	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaAtaque, Datos);
	if (!TestTrue(TEXT("Jugador y goblin de pie"), Mundo.bListo))
	{
		return false;
	}
	UPoseableMeshComponent* Cuerpo = Enemigo->FindComponentByClass<UPoseableMeshComponent>();
	USAEVUSComponenteAnimacionProcedural* Animacion = Enemigo->FindComponentByClass<USAEVUSComponenteAnimacionProcedural>();
	TestTrue(TEXT("Usa el cuerpo con esqueleto"), Enemigo->TieneEsqueleto() && Cuerpo && Cuerpo->IsVisible());
	if (!TestTrue(TEXT("Esqueleto válido para la animación"), Animacion && Animacion->EsqueletoValido()))
	{
		return false;
	}
	auto Avanzar = [&](int32 Pasos)
	{
		for (int32 Indice = 0; Indice < Pasos; ++Indice)
		{
			Mundo.MundoPrueba.Paso();
			Animacion->Actualizar(SAEVUSPruebas::PasoSimulacion);
		}
	};
	Enemigo->ObtenerIA()->FijarObjetivo(nullptr);
	Avanzar(SAEVUSPruebas::PasosPara(TiempoObservar));
	const float ManoReposo = static_cast<float>(Cuerpo->GetBoneLocationByName(TEXT("mano_d"), EBoneSpaces::ComponentSpace).Z);

	Enemigo->ObtenerIA()->FijarObjetivo(Mundo.Jugador);
	float ManoMaxima = ManoReposo;
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoMax) && Enemigo->ObtenerIA()->ObtenerEstado() != ESAEVUSEstadoEnemigo::Atacar; ++Indice)
	{
		Avanzar(1);
		ManoMaxima = FMath::Max(ManoMaxima, static_cast<float>(Cuerpo->GetBoneLocationByName(TEXT("mano_d"), EBoneSpaces::ComponentSpace).Z));
	}
	AddInfo(FString::Printf(TEXT("Mano en reposo %.0f cm, al avisar hasta %.0f cm"), ManoReposo, ManoMaxima));
	TestEqual(TEXT("Llega a atacar"), Enemigo->ObtenerIA()->ObtenerEstado(), ESAEVUSEstadoEnemigo::Atacar);
	TestTrue(TEXT("Al avisar alza el arma"), ManoMaxima - ManoReposo > SubidaMinimaManoAviso);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEnemigoTexturaReferencia, "SAEVUS.Enemigo.TexturaReferencia", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEnemigoTexturaReferencia::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasEnemigos;

	// Rey Goblin reconstruido desde las vistas de REF-06: su ranura "Referencia" lleva la pintura horneada, en blanco.
	const TCHAR* const RutaRey = TEXT("/Game/SAEVUS/Datos/Enemigos/DA_Jefe_ReyGoblin.DA_Jefe_ReyGoblin");
	const FName RanuraReferencia(TEXT("Referencia"));
	const FName ParametroTextura(TEXT("Textura"));
	const FName ParametroColor(TEXT("Color"));
	const USAEVUSDatosEnemigo* Datos = LoadObject<USAEVUSDatosEnemigo>(nullptr, RutaRey);
	if (!TestNotNull(TEXT("DA_Jefe_ReyGoblin (créalo con Verificar-Arranque.ps1)"), Datos))
	{
		return false;
	}
	const TObjectPtr<UTexture>* Textura = Datos->TexturasPiezas.Find(RanuraReferencia);
	if (!TestTrue(TEXT("El Rey tiene la pintura de la referencia"), Textura && *Textura))
	{
		return false;
	}
	FMundoEnemigos Mundo;
	ASAEVUSEnemigo* Enemigo = Mundo.CrearEnemigo(DistanciaAtaque * 3.f, Datos);
	UPoseableMeshComponent* Cuerpo = Enemigo ? Enemigo->FindComponentByClass<UPoseableMeshComponent>() : nullptr;
	if (!TestTrue(TEXT("Rey con cuerpo de esqueleto"), Enemigo && Enemigo->TieneEsqueleto() && Cuerpo))
	{
		return false;
	}
	const int32 Ranura = Cuerpo->GetMaterialIndex(RanuraReferencia);
	UMaterialInstanceDynamic* Material = Ranura != INDEX_NONE ? Cast<UMaterialInstanceDynamic>(Cuerpo->GetMaterial(Ranura)) : nullptr;
	if (!TestNotNull(TEXT("Ranura Referencia con material propio"), Material))
	{
		return false;
	}
	UTexture* Aplicada = nullptr;
	Material->GetTextureParameterValue(FHashedMaterialParameterInfo(ParametroTextura), Aplicada);
	TestEqual(TEXT("Usa la textura de los datos"), Aplicada, Textura->Get());
	FLinearColor Color;
	Material->GetVectorParameterValue(FHashedMaterialParameterInfo(ParametroColor), Color);
	TestTrue(TEXT("Sin tinte: la pintura tal cual"), Color.Equals(FLinearColor::White));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
