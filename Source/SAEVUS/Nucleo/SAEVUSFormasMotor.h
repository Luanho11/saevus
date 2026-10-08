// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

/**
 * Formas básicas del motor que usan los bocetos (bancos, palancas, puertas, enemigos sin cuerpo, partículas):
 * una sola ruta por forma para todo el proyecto. Todas miden 100 cm y tienen el pivote en el centro.
 */
namespace SAEVUSFormas
{
	inline const TCHAR* const Cubo = TEXT("/Engine/BasicShapes/Cube.Cube");
	inline const TCHAR* const Esfera = TEXT("/Engine/BasicShapes/Sphere.Sphere");
	inline const TCHAR* const Cono = TEXT("/Engine/BasicShapes/Cone.Cone");
	inline const TCHAR* const Cilindro = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
	constexpr float Tamano = 100.f;
}
