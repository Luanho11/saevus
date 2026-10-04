// Copyright SAEVUS. Todos los derechos reservados.

#include "Combate/SAEVUSTiposCombate.h"

FSAEVUSRepertorioMelee::FSAEVUSRepertorioMelee()
{
	// Repertorio por defecto: dos tajos rápidos y un remate más pesado en el suelo, un tajo aéreo.
	// Valores de primera pasada; se ajustan en el Data Asset.

	// Los tajos rápidos usan los valores por defecto de FSAEVUSDefinicionAtaque.
	FSAEVUSDefinicionAtaque Tajo1;
	Tajo1.Nombre = TEXT("Tajo1");

	FSAEVUSDefinicionAtaque Tajo2;
	Tajo2.Nombre = TEXT("Tajo2");
	Tajo2.Danio = 12.f;

	// Remate: más lento, más amplio, estocada más larga, y compromete: no se sale saltando y el dash llega tarde.
	FSAEVUSDefinicionAtaque Remate;
	Remate.Nombre = TEXT("Remate");
	Remate.TiempoPreparacion = 0.18f;
	Remate.TiempoActivo = 0.12f;
	Remate.TiempoRecuperacion = 0.45f;
	Remate.Danio = 25.f;
	Remate.RadioGolpe = 75.f;
	Remate.DesplazamientoFrontalGolpe = 100.f;
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
	FSAEVUSDefinicionAtaque RemateAereo = TajoAereo;
	RemateAereo.Nombre = TEXT("RemateAereo");
	RemateAereo.Danio = 18.f;
	RemateAereo.TiempoRecuperacion = 0.35f;
	RemateAereo.RadioGolpe = 75.f;
	RemateAereo.EscalaGravedadEnAire = 1.f;
	RemateAereo.FraccionCaidaConservada = 1.f;

	ComboAire = { TajoAereo, TajoAereo2, RemateAereo };

	// Tajo hacia arriba: esfera sobre la cabeza, sin estocada ni retroceso (no se empuja contra el techo).
	TajoArriba.Nombre = TEXT("TajoArriba");
	TajoArriba.TiempoPreparacion = 0.08f;
	TajoArriba.TiempoActivo = 0.1f;
	TajoArriba.TiempoRecuperacion = 0.25f;
	TajoArriba.Danio = 12.f;
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

	// Tajo hacia abajo: esfera bajo los pies; al acertar rebota (pogo), así se encadenan saltos sobre enemigos.
	TajoAbajo = TajoArriba;
	TajoAbajo.Nombre = TEXT("TajoAbajo");
	TajoAbajo.DesplazamientoFrontalGolpe = 15.f;
	TajoAbajo.AlturaGolpe = -130.f;
	TajoAbajo.VelocidadRebote = 750.f;
}
