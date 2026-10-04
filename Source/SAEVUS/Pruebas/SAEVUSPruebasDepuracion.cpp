// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSComponenteVida.h"
#include "Depuracion/SAEVUSSubsistemaNumerosDanio.h"
#include "Depuracion/SAEVUSVariablesDepuracion.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Mundo/SAEVUSMunecoEntrenamiento.h"

namespace SAEVUSPruebasDepuracion
{
	const FVector PosicionMuneco(300.f, 0.f, 96.f);
	const FVector PosicionPersonaje(0.f, 0.f, 150.f);
	constexpr float DanioMuneco = 12.f;
	constexpr float DanioPersonaje = 7.f;
	constexpr float MargenCaducar = 0.1f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaNumerosDanio, "SAEVUS.Depuracion.NumerosDanio", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaNumerosDanio::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasDepuracion;

	const bool bAntes = SAEVUSDepuracion::NumerosDanioActivos();
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	ASAEVUSMunecoEntrenamiento* Muneco = MundoPrueba.CrearActor<ASAEVUSMunecoEntrenamiento>(PosicionMuneco);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionPersonaje);
	USAEVUSSubsistemaNumerosDanio* Subsistema = MundoPrueba.ObtenerMundo()->GetSubsystem<USAEVUSSubsistemaNumerosDanio>();
	if (!TestNotNull(TEXT("Subsistema de números de daño"), Subsistema))
	{
		return false;
	}

	SAEVUSDepuracion::ActivarNumerosDanio(false);
	UGameplayStatics::ApplyDamage(Muneco, DanioMuneco, nullptr, nullptr, UDamageType::StaticClass());
	TestEqual(TEXT("Apagado: no se apunta nada"), Subsistema->ObtenerNumeros().Num(), 0);

	SAEVUSDepuracion::ActivarNumerosDanio(true);
	UGameplayStatics::ApplyDamage(Muneco, DanioMuneco, nullptr, nullptr, UDamageType::StaticClass());
	Personaje->ObtenerVida()->AplicarDanio(DanioPersonaje, nullptr);
	const TArray<FSAEVUSNumeroDanio>& Numeros = Subsistema->ObtenerNumeros();
	if (TestEqual(TEXT("Un número por golpe (muñeco y vida)"), Numeros.Num(), 2))
	{
		TestEqual(TEXT("Daño del muñeco"), Numeros[0].Danio, DanioMuneco);
		TestTrue(TEXT("Encima del muñeco"), Numeros[0].Posicion.Z > Muneco->GetActorLocation().Z);
		TestEqual(TEXT("Daño quitado a la vida"), Numeros[1].Danio, DanioPersonaje);
	}

	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(SAEVUSNumerosDanio::Duracion + MargenCaducar); ++Indice)
	{
		MundoPrueba.Paso();
	}
	TestEqual(TEXT("Los números caducan"), Subsistema->ObtenerNumeros().Num(), 0);
	SAEVUSDepuracion::ActivarNumerosDanio(bAntes);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
