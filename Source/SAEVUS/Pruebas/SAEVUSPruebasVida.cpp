// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combate/SAEVUSComponenteVida.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

namespace SAEVUSPruebasVida
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);
	const FVector PosicionObjeto(500.f, 0.f, 100.f);

	constexpr float Golpe = 30.f;
	constexpr float DanioExcesivo = 1000.f;
	constexpr float CuraPequena = 10.f;
	constexpr float DanioMotor = 25.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaReglasVida, "SAEVUS.Vida.Reglas", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaReglasVida::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasVida;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
	USAEVUSComponenteVida* Vida = Personaje->ObtenerVida();
	const float Maxima = Vida->ObtenerVidaMaxima();
	// Reglas base del componente; la invulnerabilidad tras golpe del jugador tiene su propio test.
	Vida->FijarInvulnerabilidadTrasGolpe(0.f);

	TestEqual(TEXT("Empieza con la vida completa"), Vida->ObtenerVida(), Maxima);
	TestEqual(TEXT("El daño quita vida"), Vida->AplicarDanio(Golpe, nullptr), Golpe);
	TestEqual(TEXT("Vida tras el golpe"), Vida->ObtenerVida(), Maxima - Golpe);
	TestEqual(TEXT("Curar suma vida"), Vida->Curar(CuraPequena), CuraPequena);
	TestEqual(TEXT("Curar nunca pasa del máximo"), Vida->Curar(DanioExcesivo), Golpe - CuraPequena);
	TestEqual(TEXT("Daño cero o negativo no hace nada"), Vida->AplicarDanio(-Golpe, nullptr), 0.f);

	TestEqual(TEXT("El daño excesivo solo quita lo que queda"), Vida->AplicarDanio(DanioExcesivo, nullptr), Maxima);
	TestTrue(TEXT("Muerto al llegar a cero"), Vida->EstaMuerto());
	TestEqual(TEXT("Sin daño mientras está muerto"), Vida->AplicarDanio(Golpe, nullptr), 0.f);
	TestEqual(TEXT("Sin curación mientras está muerto"), Vida->Curar(CuraPequena), 0.f);

	Vida->RestablecerVida();
	TestTrue(TEXT("Restablecer: vivo con la vida completa"), !Vida->EstaMuerto() && Vida->ObtenerVida() == Maxima);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaDanioMotor, "SAEVUS.Vida.DanioMotor", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaDanioMotor::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasVida;

	// Los pawns ignoran el daño en un mundo sin modo de juego (APawn::ShouldTakeDamage), así la vía
	// de daño del motor se comprueba en un actor simple; la prueba de humo la cubre en el jugador.
	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	AActor* Objeto = MundoPrueba.CrearActor<AActor>(PosicionObjeto);
	USAEVUSComponenteVida* Vida = NewObject<USAEVUSComponenteVida>(Objeto);
	Vida->RegisterComponent();
	const float Maxima = Vida->ObtenerVidaMaxima();

	UGameplayStatics::ApplyDamage(Objeto, DanioMotor, nullptr, nullptr, UDamageType::StaticClass());
	TestEqual(TEXT("El daño del motor llega al componente de vida"), Vida->ObtenerVida(), Maxima - DanioMotor);

	UGameplayStatics::ApplyDamage(Objeto, DanioMotor, nullptr, Objeto, UDamageType::StaticClass());
	TestEqual(TEXT("El daño propio se ignora"), Vida->ObtenerVida(), Maxima - DanioMotor);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaMuertePersonaje, "SAEVUS.Vida.MuertePersonaje", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaMuertePersonaje::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasVida;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
	if (!TestTrue(TEXT("Personaje asentado"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}

	// Aquí no hay modo de juego ni controlador: la muerte tiene que ser segura igualmente.
	Personaje->ObtenerVida()->AplicarDanio(DanioExcesivo, nullptr);
	TestTrue(TEXT("Muerto"), Personaje->ObtenerVida()->EstaMuerto());
	TestEqual(TEXT("El movimiento se detiene"), Personaje->ObtenerMovimiento()->MovementMode.GetValue(), MOVE_None);
	TestEqual(TEXT("La cápsula deja de colisionar"), Personaje->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);

	const FVector Posicion = Personaje->GetActorLocation();
	Personaje->PedirSalto();
	Personaje->PedirDash();
	MundoPrueba.Paso();
	TestTrue(TEXT("Un personaje muerto no se mueve"), Personaje->GetActorLocation().Equals(Posicion));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
