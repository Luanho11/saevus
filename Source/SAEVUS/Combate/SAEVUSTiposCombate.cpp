// Copyright SAEVUS. Todos los derechos reservados.

#include "Combate/SAEVUSTiposCombate.h"

namespace SAEVUSCombate
{
	const FName EtiquetaBlancoPractica(TEXT("SAEVUS.BlancoPractica"));

	float SentidoBarrido(ESAEVUSGestoAtaque Gesto)
	{
		// El revés y el revés aéreo barren de izquierda a derecha; el resto, de derecha a izquierda.
		return (Gesto == ESAEVUSGestoAtaque::Reves || Gesto == ESAEVUSGestoAtaque::AereoReves) ? -1.f : 1.f;
	}
}

FSAEVUSRepertorioMelee::FSAEVUSRepertorioMelee()
{
	// Repertorio por defecto: dos tajos rápidos y un remate más pesado en el suelo, un tajo aéreo.
	// Valores de primera pasada; se ajustan en el Data Asset.

	// Los tajos rápidos usan los valores por defecto de FSAEVUSDefinicionAtaque.
	FSAEVUSDefinicionAtaque Tajo1;
	Tajo1.Nombre = TEXT("Tajo1");
	Tajo1.Gesto = ESAEVUSGestoAtaque::TajoDerecha;

	FSAEVUSDefinicionAtaque Tajo2;
	Tajo2.Nombre = TEXT("Tajo2");
	Tajo2.TonoSonido = 1.12f;
	Tajo2.Gesto = ESAEVUSGestoAtaque::Reves;
	Tajo2.Danio = 12.f;

	// Remate: más lento, más amplio, estocada más larga, y compromete: no se sale saltando y el dash llega tarde.
	FSAEVUSDefinicionAtaque Remate;
	Remate.Nombre = TEXT("Remate");
	Remate.TonoSonido = 0.75f;
	Remate.Gesto = ESAEVUSGestoAtaque::Remate;
	Remate.TiempoPreparacion = 0.18f;
	Remate.TiempoActivo = 0.12f;
	Remate.TiempoRecuperacion = 0.45f;
	Remate.Danio = 25.f;
	Remate.RadioGolpe = 75.f;
	Remate.DesplazamientoFrontalGolpe = 100.f;
	Remate.AlcanceAbanico = 190.f;
	Remate.AnguloAbanico = 60.f;
	Remate.VelocidadEstocada = 650.f;
	Remate.DuracionEstocada = 0.18f;
	Remate.EscalaVelocidadSuelo = 0.f;
	Remate.CancelarConDashDesde = 0.4f;
	Remate.CancelarConSaltoDesde = -1.f;
	Remate.CancelarConCambioArmaDesde = 0.4f;
	Remate.CancelarConGuardiaDesde = 0.4f;

	ComboSuelo = { Tajo1, Tajo2, Remate };

	// Tajo aéreo: sin estocada, así lo lleva el impulso del salto.
	FSAEVUSDefinicionAtaque TajoAereo;
	TajoAereo.Nombre = TEXT("TajoAereo");
	TajoAereo.TonoSonido = 1.05f;
	TajoAereo.Gesto = ESAEVUSGestoAtaque::AereoDiagonal;
	TajoAereo.TiempoPreparacion = 0.08f;
	TajoAereo.TiempoActivo = 0.1f;
	TajoAereo.TiempoRecuperacion = 0.25f;
	TajoAereo.Danio = 12.f;
	TajoAereo.AlturaGolpe = 0.f;
	TajoAereo.VelocidadEstocada = 0.f;
	TajoAereo.EncadenarDesde = TajoAereo.TiempoPreparacion + TajoAereo.TiempoActivo;
	TajoAereo.CancelarConDashDesde = TajoAereo.EncadenarDesde;
	TajoAereo.CancelarConCambioArmaDesde = TajoAereo.EncadenarDesde;
	TajoAereo.CancelarConGuardiaDesde = TajoAereo.EncadenarDesde;
	// Flota un poco mientras tajea: la cadena aérea se puede completar antes de caer.
	TajoAereo.EscalaGravedadEnAire = 0.35f;
	TajoAereo.FraccionCaidaConservada = 0.25f;

	// Cadena aérea de tres (GunZ): revés y un remate que empuja hacia abajo y ya no flota.
	FSAEVUSDefinicionAtaque TajoAereo2 = TajoAereo;
	TajoAereo2.Nombre = TEXT("TajoAereo2");
	TajoAereo2.TonoSonido = 1.15f;
	TajoAereo2.Gesto = ESAEVUSGestoAtaque::AereoReves;
	FSAEVUSDefinicionAtaque RemateAereo = TajoAereo;
	RemateAereo.Nombre = TEXT("RemateAereo");
	RemateAereo.TonoSonido = 0.8f;
	RemateAereo.Gesto = ESAEVUSGestoAtaque::AereoRemate;
	RemateAereo.Danio = 18.f;
	RemateAereo.TiempoRecuperacion = 0.35f;
	RemateAereo.RadioGolpe = 75.f;
	RemateAereo.AlcanceAbanico = 185.f;
	RemateAereo.EscalaGravedadEnAire = 1.f;
	RemateAereo.FraccionCaidaConservada = 1.f;

	ComboAire = { TajoAereo, TajoAereo2, RemateAereo };

	// Tajo hacia arriba: esfera sobre la cabeza, sin estocada ni retroceso (no se empuja contra el techo).
	TajoArriba.Nombre = TEXT("TajoArriba");
	TajoArriba.TonoSonido = 1.2f;
	TajoArriba.Gesto = ESAEVUSGestoAtaque::TajoArriba;
	TajoArriba.TiempoPreparacion = 0.08f;
	TajoArriba.TiempoActivo = 0.1f;
	TajoArriba.TiempoRecuperacion = 0.25f;
	TajoArriba.Danio = 12.f;
	// Sobre la cabeza: esfera, no abanico.
	TajoArriba.AnguloAbanico = 0.f;
	TajoArriba.RadioGolpe = 70.f;
	TajoArriba.DesplazamientoFrontalGolpe = 25.f;
	TajoArriba.AlturaGolpe = 130.f;
	TajoArriba.VelocidadEstocada = 0.f;
	TajoArriba.RetrocesoAtacante = 0.f;
	TajoArriba.EncadenarDesde = TajoArriba.TiempoPreparacion + TajoArriba.TiempoActivo;
	TajoArriba.CancelarConDashDesde = TajoArriba.EncadenarDesde;
	TajoArriba.CancelarConSaltoDesde = TajoArriba.EncadenarDesde;
	TajoArriba.CancelarConCambioArmaDesde = TajoArriba.EncadenarDesde;
	TajoArriba.CancelarConGuardiaDesde = TajoArriba.EncadenarDesde;


	// Tajo en carrera: el impulso del dash se convierte en una estocada larga y un golpe amplio.
	TajoEnCarrera = Tajo1;
	TajoEnCarrera.Nombre = TEXT("TajoEnCarrera");
	TajoEnCarrera.TonoSonido = 1.3f;
	TajoEnCarrera.Gesto = ESAEVUSGestoAtaque::Estocada;
	TajoEnCarrera.TiempoPreparacion = 0.05f;
	TajoEnCarrera.TiempoActivo = 0.14f;
	TajoEnCarrera.TiempoRecuperacion = 0.3f;
	TajoEnCarrera.Danio = 16.f;
	TajoEnCarrera.RadioGolpe = 80.f;
	TajoEnCarrera.AlcanceAbanico = 210.f;
	TajoEnCarrera.AnguloAbanico = 55.f;
	TajoEnCarrera.DesplazamientoFrontalGolpe = 100.f;
	TajoEnCarrera.VelocidadEstocada = 1100.f;
	TajoEnCarrera.DuracionEstocada = 0.16f;
	TajoEnCarrera.EscalaVelocidadSuelo = 0.f;
	TajoEnCarrera.EncadenarDesde = TajoEnCarrera.TiempoPreparacion + TajoEnCarrera.TiempoActivo;
	TajoEnCarrera.CancelarConDashDesde = TajoEnCarrera.EncadenarDesde;
	TajoEnCarrera.CancelarConSaltoDesde = TajoEnCarrera.EncadenarDesde;
	TajoEnCarrera.CancelarConCambioArmaDesde = TajoEnCarrera.EncadenarDesde;
	TajoEnCarrera.CancelarConGuardiaDesde = TajoEnCarrera.EncadenarDesde;

	// Ataque cargado: giro completo con la hoja extendida; golpea todo alrededor. Compromete.
	AtaqueCargado = Tajo1;
	AtaqueCargado.Nombre = TEXT("AtaqueCargado");
	AtaqueCargado.TonoSonido = 0.65f;
	AtaqueCargado.Gesto = ESAEVUSGestoAtaque::Giratorio;
	AtaqueCargado.TiempoPreparacion = 0.1f;
	AtaqueCargado.TiempoActivo = 0.25f;
	AtaqueCargado.TiempoRecuperacion = 0.4f;
	AtaqueCargado.Danio = 30.f;
	AtaqueCargado.RadioGolpe = 170.f;
	// Giro completo: el abanico es todo el círculo.
	AtaqueCargado.AlcanceAbanico = 190.f;
	AtaqueCargado.AnguloAbanico = 180.f;
	AtaqueCargado.DesplazamientoFrontalGolpe = 0.f;
	AtaqueCargado.AlturaGolpe = 0.f;
	AtaqueCargado.VelocidadEstocada = 0.f;
	AtaqueCargado.EscalaVelocidadSuelo = 0.f;
	AtaqueCargado.RetrocesoAtacante = 0.f;
	AtaqueCargado.EmpujeObjetivo = 600.f;
	AtaqueCargado.DuracionParadaGolpe = 0.09f;
	AtaqueCargado.EncadenarDesde = AtaqueCargado.TiempoPreparacion + AtaqueCargado.TiempoActivo;
	AtaqueCargado.CancelarConDashDesde = AtaqueCargado.EncadenarDesde;
	AtaqueCargado.CancelarConSaltoDesde = -1.f;
	AtaqueCargado.CancelarConCambioArmaDesde = AtaqueCargado.EncadenarDesde;
	AtaqueCargado.CancelarConGuardiaDesde = AtaqueCargado.EncadenarDesde;
}
