// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Armas/SAEVUSTiposArma.h"
#include "Combate/SAEVUSComponenteVida.h"
#include "Interfaz/SAEVUSEstadoInterfaz.h"
#include "Interfaz/SAEVUSWidgetInterfaz.h"
#include "Interfaz/SAEVUSWidgetPausa.h"
#include "Mundo/SAEVUSInfoZona.h"
#include "Tutorial/SAEVUSSubsistemaTutorial.h"

namespace SAEVUSPruebasInterfaz
{
	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(6000.f, 6000.f, 100.f);
	const FVector PosicionJugador(0.f, 0.f, 150.f);
	constexpr float VidaPorMascara = 20.f;
	constexpr float Fundido = 0.3f;
	constexpr float Tolerancia = 0.001f;
	constexpr float DistanciaJefe = 1200.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaInterfazVitales, "SAEVUS.Interfaz.VidaAlmaArma", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaInterfazVitales::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasInterfaz;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
	UWorld* Mundo = MundoPrueba.ObtenerMundo();

	FSAEVUSEstadoInterfaz Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestEqual(TEXT("100 de vida en máscaras de 20: cinco"), Estado.Mascaras.Num(), 5);
	TestEqual(TEXT("Todas llenas"), Estado.Mascaras.Last(), 1.f, Tolerancia);

	// 70 de vida: tres llenas, una a media, una vacía.
	Jugador->ObtenerVida()->AplicarDanio(30.f, nullptr);
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestFalse(TEXT("Vivo: sin pantalla de muerte"), Estado.bJugadorMuerto);
	TestEqual(TEXT("Tercera llena"), Estado.Mascaras[2], 1.f, Tolerancia);
	TestEqual(TEXT("Cuarta a media"), Estado.Mascaras[3], 0.5f, Tolerancia);
	TestEqual(TEXT("Quinta vacía"), Estado.Mascaras[4], 0.f, Tolerancia);

	USAEVUSComponenteAlma* Alma = Jugador->ObtenerAlma();
	Alma->GanarAlma(Alma->ObtenerAjuste().CosteCuracion);
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestEqual(TEXT("Fracción de alma"), Estado.FraccionAlma, Alma->ObtenerAjuste().CosteCuracion / Alma->ObtenerAlmaMaxima(), Tolerancia);
	TestTrue(TEXT("Alma suficiente para curar"), Estado.bAlmaParaCurar);

	TestTrue(TEXT("Con una sola arma (o ninguna) no se muestra el nombre"), Estado.TextoArma.IsEmpty());
	TestTrue(TEXT("Sin arma de fuego no hay munición"), Estado.TextoMunicion.IsEmpty());
	USAEVUSDefinicionArma* Pistola = NewObject<USAEVUSDefinicionArma>(GetTransientPackage());
	Pistola->NombreVisible = TEXT("Pistola");
	Pistola->Tipo = ESAEVUSTipoArma::ArmaFuego;
	// Con dos armas se muestra el nombre de la equipada (con una sola no hay nada que elegir).
	USAEVUSDefinicionArma* Espada = NewObject<USAEVUSDefinicionArma>(GetTransientPackage());
	Espada->Tipo = ESAEVUSTipoArma::CuerpoACuerpo;
	Jugador->ObtenerArmas()->FijarArsenal({ Pistola, Espada });
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestEqual(TEXT("Nombre del arma"), Estado.TextoArma.ToString(), FString(TEXT("Pistola")));
	TestTrue(TEXT("Sin pawn: pantalla de muerte"), FSAEVUSEstadoInterfaz::Calcular(nullptr, Mundo, VidaPorMascara, Fundido).bJugadorMuerto);
	TestEqual(TEXT("Munición del cargador, reserva infinita"), Estado.TextoMunicion.ToString(), FString::Printf(TEXT("%d / ∞"), Pistola->ArmaFuego.TamanoCargador));
	constexpr int32 Reserva = 24;
	Pistola->ArmaFuego.MunicionReserva = Reserva;
	Jugador->ObtenerArmas()->FijarArsenal({ Pistola, Espada });
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestEqual(TEXT("Munición del cargador y de la reserva"), Estado.TextoMunicion.ToString(), FString::Printf(TEXT("%d / %d"), Pistola->ArmaFuego.TamanoCargador, Reserva));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaInterfazAvisoJefe, "SAEVUS.Interfaz.AvisoYJefe", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaInterfazAvisoJefe::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasInterfaz;

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
	ASAEVUSPersonaje* Jugador = MundoPrueba.CrearPersonaje(PosicionJugador);
	UWorld* Mundo = MundoPrueba.ObtenerMundo();

	FSAEVUSEstadoInterfaz Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestFalse(TEXT("Sin aviso"), Estado.bHayAviso);
	Mundo->GetSubsystem<USAEVUSSubsistemaTutorial>()->MostrarMensaje(FText::FromString(TEXT("Saltar")), FText::FromString(TEXT("Espacio")), 5.f);
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestTrue(TEXT("Con aviso"), Estado.bHayAviso);
	TestEqual(TEXT("Título del aviso"), Estado.TituloAviso.ToString(), FString(TEXT("Saltar")));
	TestEqual(TEXT("Empieza transparente (fundido de entrada)"), Estado.OpacidadAviso, 0.f, Tolerancia);
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Fundido * 2.f); ++Indice)
	{
		MundoPrueba.Paso();
	}
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestEqual(TEXT("Tras el fundido, opaco"), Estado.OpacidadAviso, 1.f, Tolerancia);

	// Un jefe en reposo no muestra barra; activo, sí.
	USAEVUSDatosEnemigo* Datos = NewObject<USAEVUSDatosEnemigo>(GetTransientPackage());
	Datos->Enemigo.bEsJefe = true;
	Datos->Enemigo.NombreVisible = FText::FromString(TEXT("Guardián"));
	Datos->Enemigo.VidaMaxima = 200.f;
	Datos->Enemigo.RadioDeteccion = DistanciaJefe * 0.5f;
	ASAEVUSEnemigo* Jefe = MundoPrueba.CrearEnemigo(FVector(DistanciaJefe, 0.f, 150.f), Datos, Jugador);
	MundoPrueba.Paso();
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestFalse(TEXT("Jefe en reposo: sin barra"), Estado.bJefeVisible);

	Jefe->SetActorLocation(FVector(DistanciaJefe * 0.25f, 0.f, Jefe->GetActorLocation().Z));
	MundoPrueba.Paso();
	Jefe->ObtenerVida()->AplicarDanio(50.f, Jugador);
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestTrue(TEXT("Jefe activo: barra visible"), Estado.bJefeVisible);
	TestEqual(TEXT("Nombre del jefe"), Estado.NombreJefe.ToString(), FString(TEXT("Guardián")));
	TestEqual(TEXT("Vida del jefe"), Estado.FraccionVidaJefe, 0.75f, Tolerancia);

	Jefe->ObtenerVida()->AplicarDanio(1000.f, Jugador);
	Estado = FSAEVUSEstadoInterfaz::Calcular(Jugador, Mundo, VidaPorMascara, Fundido);
	TestFalse(TEXT("Jefe muerto: sin barra"), Estado.bJefeVisible);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaInterfazPausaOpciones, "SAEVUS.Interfaz.OpcionesPausa", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaInterfazPausaOpciones::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Baja → Media"), SAEVUSPausa::SiguienteCalidad(0), 1);
	TestEqual(TEXT("Media → Alta"), SAEVUSPausa::SiguienteCalidad(1), 2);
	TestEqual(TEXT("Alta → Baja"), SAEVUSPausa::SiguienteCalidad(2), 0);
	TestEqual(TEXT("Personalizado (-1) empieza en Media tras Baja"), SAEVUSPausa::SiguienteCalidad(-1), 1);
	TestEqual(TEXT("1 → 1,5"), SAEVUSPausa::SiguienteSensibilidad(1.f), 1.5f);
	TestEqual(TEXT("2 → 0,5 (vuelta)"), SAEVUSPausa::SiguienteSensibilidad(2.f), 0.5f);
	TestFalse(TEXT("Cada calidad tiene nombre"), SAEVUSPausa::NombreCalidad(2).IsEmpty());
	TestEqual(TEXT("Volumen 0 → 25 %"), SAEVUSPausa::SiguienteVolumen(0.f), 0.25f);
	TestEqual(TEXT("Volumen 75 → 100 %"), SAEVUSPausa::SiguienteVolumen(0.75f), 1.f);
	TestEqual(TEXT("Volumen 100 % → 0 (vuelta)"), SAEVUSPausa::SiguienteVolumen(1.f), 0.f);
	TestEqual(TEXT("Un volumen suelto va al paso siguiente"), SAEVUSPausa::SiguienteVolumen(0.3f), 0.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaTituloZona, "SAEVUS.Interfaz.TituloZona", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaTituloZona::RunTest(const FString& Parameters)
{
	constexpr float Tolerancia = 0.001f;
	const FSAEVUSAjusteInterfaz Ajuste;
	const float Entrada = Ajuste.RetardoTituloZona;
	const float Salida = Entrada + Ajuste.FundidoTituloZona + Ajuste.DuracionTituloZona;
	TestEqual(TEXT("Al entrar no se ve"), SAEVUSInterfaz::OpacidadTituloZona(0.f, Ajuste), 0.f, Tolerancia);
	TestEqual(TEXT("A mitad del fundido de entrada"), SAEVUSInterfaz::OpacidadTituloZona(Entrada + Ajuste.FundidoTituloZona * 0.5f, Ajuste), 0.5f, Tolerancia);
	TestEqual(TEXT("A la vista"), SAEVUSInterfaz::OpacidadTituloZona(Salida - Ajuste.DuracionTituloZona * 0.5f, Ajuste), 1.f, Tolerancia);
	TestEqual(TEXT("A mitad del fundido de salida"), SAEVUSInterfaz::OpacidadTituloZona(Salida + Ajuste.FundidoTituloZona * 0.5f, Ajuste), 0.5f, Tolerancia);
	TestEqual(TEXT("Después se va"), SAEVUSInterfaz::OpacidadTituloZona(Salida + Ajuste.FundidoTituloZona * 2.f, Ajuste), 0.f, Tolerancia);

	SAEVUSPruebas::FMundoPrueba MundoPrueba;
	TestNull(TEXT("Sin información de zona"), ASAEVUSInfoZona::Obtener(MundoPrueba.ObtenerMundo()));
	ASAEVUSInfoZona* Zona = MundoPrueba.CrearActor<ASAEVUSInfoZona>(FVector::ZeroVector);
	TestTrue(TEXT("Se encuentra la información de la zona"), ASAEVUSInfoZona::Obtener(MundoPrueba.ObtenerMundo()) == Zona);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
