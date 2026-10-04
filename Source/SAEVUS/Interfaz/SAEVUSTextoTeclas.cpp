// Copyright SAEVUS. Todos los derechos reservados.

#include "Interfaz/SAEVUSTextoTeclas.h"

TArray<FSAEVUSTrozoTexto> SAEVUSInterfaz::TrocearTeclas(const FString& Linea)
{
	TArray<FSAEVUSTrozoTexto> Trozos;
	FString Normal;
	int32 Indice = 0;
	while (Indice < Linea.Len())
	{
		const int32 Cierre = Linea[Indice] == TEXT('[') ? Linea.Find(TEXT("]"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Indice + 1) : INDEX_NONE;
		if (Cierre > Indice + 1)
		{
			if (!Normal.IsEmpty())
			{
				Trozos.Add({ Normal, false });
				Normal.Reset();
			}
			Trozos.Add({ Linea.Mid(Indice + 1, Cierre - Indice - 1), true });
			Indice = Cierre + 1;
			continue;
		}
		Normal.AppendChar(Linea[Indice]);
		++Indice;
	}
	if (!Normal.IsEmpty())
	{
		Trozos.Add({ Normal, false });
	}
	return Trozos;
}
