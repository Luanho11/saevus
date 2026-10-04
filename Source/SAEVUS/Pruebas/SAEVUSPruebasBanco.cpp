// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Armas/SAEVUSTiposArma.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Interfaz/SAEVUSEstadoInterfaz.h"
#include "Mundo/SAEVUSBanco.h"

namespace SAEVUSPruebasBanco
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionJugador(0.f, 0.f, 150.f);
	const FVector PosicionBanco(60.f, 0.f, 100.f);
	const FVector PosicionBancoLejos(1500.f, 0.f, 100.f);
	constexpr float Danio = 40.f;
	constexpr float VidaPorMascara = 20.f;
	constexpr float FundidoAviso = 0.3f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaBanco, "SAEVUS.Mundo.Banco", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaBanco::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasBanco;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
	if (!TestTrue(TEXT("Jugador de pie"), MundoPrueba.AsentarEnSuelo()))
	{
		return false;
	}
	// Una pistola con reserva limitada, medio gastada.
	USAEVUSDefinicionArma* Pistola = NewObject<USAEVUSDefinicionArma>(GetTransientPackage());
	Pistola->Tipo = ESAEVUSTipoArma::ArmaFuego;
	Pistola->ArmaFuego.MunicionReserva = 10;
	Jugador->ObtenerArmas()->FijarArsenal({ Pistola });
	Jugador->ObtenerArmas()->PedirRecarga();
	USAEVUSComponenteVida* Vida = Jugador->ObtenerVida();
	Vida->AplicarDanio(Danio, nullptr);

	ASAEVUSBanco* Banco = MundoPrueba.CrearActor<ASAEVUSBanco>(PosicionBancoLejos);
	TestTrue(TEXT("Sin nada cerca no hay aviso"), FSAEVUSEstadoInterfaz::Calcular(Jugador, MundoPrueba.ObtenerMundo(), VidaPorMascara, FundidoAviso).TextoInteraccion.IsEmpty());
	// Al llegar el banco junto al jugador, su zona lo anuncia (solape).
	Banco->SetActorLocation(PosicionBanco);
	TestTrue(TEXT("El solape anuncia el banco"), Jugador->ObtenerInteractuable() == Banco);
	TestFalse(TEXT("Con el banco cerca, la interfaz lo anuncia"), FSAEVUSEstadoInterfaz::Calcular(Jugador, MundoPrueba.ObtenerMundo(), VidaPorMascara, FundidoAviso).TextoInteraccion.IsEmpty());

	Jugador->PedirInteraccion();
	TestEqual(TEXT("Se ha usado el banco"), Banco->ObtenerUsos(), 1);
	TestTrue(TEXT("Sentado"), Jugador->EstaSentado());
	TestEqual(TEXT("Vida completa"), Vida->ObtenerVida(), Vida->ObtenerVidaMaxima());
	TestEqual(TEXT("Reserva llena"), Jugador->ObtenerArmas()->ObtenerReservaActual(), Pistola->ArmaFuego.MunicionReserva);
	TestTrue(TEXT("Sentado ya no se anuncia"), FSAEVUSEstadoInterfaz::Calcular(Jugador, MundoPrueba.ObtenerMundo(), VidaPorMascara, FundidoAviso).TextoInteraccion.IsEmpty());

	// Saltar desde el banco solo levanta; después salta con normalidad.
	Jugador->PedirSalto();
	TestFalse(TEXT("Saltar levanta"), Jugador->EstaSentado());
	TestFalse(TEXT("Y no salta todavía"), Jugador->GetCharacterMovement()->IsFalling());

	Jugador->PedirInteraccion();
	TestTrue(TEXT("Otra vez sentado"), Jugador->EstaSentado());
	Jugador->PedirInteraccion();
	TestFalse(TEXT("Interactuar sentado levanta"), Jugador->EstaSentado());
	TestEqual(TEXT("Levantarse no vuelve a usar el banco"), Banco->ObtenerUsos(), 2);

	Banco->SetActorLocation(PosicionBancoLejos);
	TestNull(TEXT("Al alejarse se olvida"), Jugador->ObtenerInteractuable());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
