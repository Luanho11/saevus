// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

class APawn;
class ASAEVUSEnemigo;
class UWorld;

/**
 * Lo que muestra la interfaz en un fotograma, calculado del juego. Separado del widget para
 * poder probarlo sin pantalla y para que cualquier interfaz (UMG en C++, Blueprint futuro) lo use.
 */
struct SAEVUS_API FSAEVUSEstadoInterfaz
{
	/** Relleno de cada máscara de vida, de 0 a 1 (la vida se reparte en máscaras de VidaPorMascara). */
	TArray<float> Mascaras;
	float FraccionAlma = 0.f;
	/** Se puede curar ya (alma suficiente): el orbe brilla. */
	bool bAlmaParaCurar = false;
	bool bCurando = false;

	FText TextoArma;
	FText TextoMunicion;

	bool bHayAviso = false;
	FText TituloAviso;
	FText TextoAviso;
	/** Opacidad del aviso con fundidos de entrada y salida. */
	float OpacidadAviso = 0.f;

	/** Sin pawn o con el pawn muerto (esperando la reaparición): pantalla de muerte. */
	bool bJugadorMuerto = false;

	bool bJefeVisible = false;
	FText NombreJefe;
	float FraccionVidaJefe = 0.f;

	/**
	 * Calcula el estado para el pawn del jugador. El jefe visible es el más cercano que esté vivo
	 * y activo (ya no en reposo).
	 */
	static FSAEVUSEstadoInterfaz Calcular(const APawn* Pawn, const UWorld* Mundo, float VidaPorMascara, float FundidoAviso);
};
