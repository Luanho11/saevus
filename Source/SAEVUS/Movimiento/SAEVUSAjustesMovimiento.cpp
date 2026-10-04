// Copyright SAEVUS. Todos los derechos reservados.

#include "Movimiento/SAEVUSAjustesMovimiento.h"

namespace SAEVUSDash
{
	FSAEVUSAjusteDash CrearVoltereta()
	{
		FSAEVUSAjusteDash Voltereta;
		Voltereta.Velocidad = 1150.f;
		Voltereta.Duracion = 0.24f;
		Voltereta.VelocidadSalida = 450.f;
		Voltereta.Enfriamiento = 0.45f;
		Voltereta.bBloquearCamaraEnDash = false;
		Voltereta.DuracionInvulnerable = 0.1f;
		Voltereta.bEstela = false;
		return Voltereta;
	}
}
