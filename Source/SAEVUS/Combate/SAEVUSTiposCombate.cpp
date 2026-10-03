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

	ComboAire = { TajoAereo };
}
