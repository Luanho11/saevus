// Copyright SAEVUS. Todos los derechos reservados.

#include "Enemigos/SAEVUSDirectorCombate.h"

void USAEVUSDirectorCombate::Depurar()
{
	for (auto Iterador = Atacantes.CreateIterator(); Iterador; ++Iterador)
	{
		if (!Iterador->IsValid())
		{
			Iterador.RemoveCurrent();
		}
	}
}

bool USAEVUSDirectorCombate::PedirFicha(const UObject* Enemigo)
{
	if (!Enemigo)
	{
		return false;
	}
	Depurar();
	if (Atacantes.Contains(Enemigo))
	{
		return true;
	}
	if (Atacantes.Num() >= MaximoAtacantes)
	{
		return false;
	}
	Atacantes.Add(Enemigo);
	return true;
}

void USAEVUSDirectorCombate::SoltarFicha(const UObject* Enemigo)
{
	Atacantes.Remove(Enemigo);
}

int32 USAEVUSDirectorCombate::ObtenerFichasUsadas()
{
	Depurar();
	return Atacantes.Num();
}
