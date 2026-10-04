// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSComponenteVida.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Mundo/SAEVUSFragmentoMascara.h"
#include "Mundo/SAEVUSParedRompible.h"

namespace SAEVUSPruebasSecretos
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionJugador(0.f, 0.f, 150.f);
	const FVector PosicionPared(300.f, 0.f, 100.f);
	const FVector PosicionFragmentoLejos(1500.f, 0.f, 100.f);
	constexpr float DanioGolpe = 10.f;
	constexpr int32 GolpesPorDefecto = 3;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaParedRompible, "SAEVUS.Mundo.ParedRompible", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaParedRompible::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasSecretos;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSParedRompible* Pared = MundoPrueba.CrearActor<ASAEVUSParedRompible>(PosicionPared);
	for (int32 Golpe = 1; Golpe < GolpesPorDefecto; ++Golpe)
	{
		TestEqual(TEXT("Un golpe no cuenta como daño hecho"), UGameplayStatics::ApplyDamage(Pared, DanioGolpe, nullptr, nullptr, UDamageType::StaticClass()), 0.f);
	}
	TestFalse(TEXT("Aguanta hasta el último golpe"), Pared->EstaRota());
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(1.f); ++Indice)
	{
		Pared->Tick(SAEVUSPruebas::PasoSimulacion);
	}
	TestTrue(TEXT("El temblor la deja en su sitio"), Pared->GetActorLocation().Equals(PosicionPared, 0.1));
	TestTrue(TEXT("Mientras, bloquea el paso"), Pared->GetActorEnableCollision());
	UGameplayStatics::ApplyDamage(Pared, DanioGolpe, nullptr, nullptr, UDamageType::StaticClass());
	TestTrue(TEXT("Al último golpe se rompe"), Pared->EstaRota());
	TestFalse(TEXT("Rota, ya no bloquea"), Pared->GetActorEnableCollision());
	TestTrue(TEXT("Y no se ve"), Pared->IsHidden());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaFragmentoMascara, "SAEVUS.Mundo.FragmentoMascara", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaFragmentoMascara::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasSecretos;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
	USAEVUSComponenteVida* Vida = Jugador->ObtenerVida();
	const float VidaMaximaInicial = Vida->ObtenerVidaMaxima();
	Vida->AplicarDanio(DanioGolpe, nullptr);

	ASAEVUSFragmentoMascara* Primero = MundoPrueba.CrearActor<ASAEVUSFragmentoMascara>(PosicionFragmentoLejos);
	Primero->FijarIdentificador(TEXT("Prueba_1"));
	Primero->Recoger(Jugador);
	TestTrue(TEXT("Recogido, desaparece"), !IsValid(Primero) || Primero->IsActorBeingDestroyed());
	TestEqual(TEXT("Un fragmento"), Jugador->ObtenerFragmentosMascara(), 1);
	TestEqual(TEXT("Medio fragmento no suma vida"), Vida->ObtenerVidaMaxima(), VidaMaximaInicial);

	// Al llegar el segundo junto al jugador, el solape lo recoge.
	ASAEVUSFragmentoMascara* Segundo = MundoPrueba.CrearActor<ASAEVUSFragmentoMascara>(PosicionFragmentoLejos);
	Segundo->FijarIdentificador(TEXT("Prueba_2"));
	Segundo->SetActorLocation(Jugador->GetActorLocation());
	TestEqual(TEXT("Dos fragmentos"), Jugador->ObtenerFragmentosMascara(), 2);
	TestEqual(TEXT("La máscara entera suma vida máxima"), Vida->ObtenerVidaMaxima(), VidaMaximaInicial + FSAEVUSAjusteProgreso().VidaPorMascara);
	TestEqual(TEXT("Y deja la vida llena"), Vida->ObtenerVida(), Vida->ObtenerVidaMaxima());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
