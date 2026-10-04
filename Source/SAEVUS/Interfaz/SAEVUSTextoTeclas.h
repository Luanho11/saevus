// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"

/** Un trozo de una línea de aviso: texto normal o el nombre de una tecla (se dibuja como tecla). */
struct FSAEVUSTrozoTexto
{
	FString Texto;
	bool bTecla = false;
};

namespace SAEVUSInterfaz
{
	/**
	 * Parte una línea en trozos: lo que va entre corchetes es una tecla ("[W] [A]: moverte" da W, " ",
	 * A y ": moverte"). Un corchete sin cerrar o vacío se queda como texto.
	 */
	SAEVUS_API TArray<FSAEVUSTrozoTexto> TrocearTeclas(const FString& Linea);
}
