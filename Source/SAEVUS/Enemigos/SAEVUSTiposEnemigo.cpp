// Copyright SAEVUS. Todos los derechos reservados.

#include "Enemigos/SAEVUSTiposEnemigo.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "SAEVUSEnemigos"

FSAEVUSAjusteEnemigo::FSAEVUSAjusteEnemigo()
{
	// Mini enemigo por defecto: un rastrero bajo que avisa y embiste. Valores de primera pasada.
	NombreVisible = LOCTEXT("Rastrero", "Rastrero");
	FSAEVUSAtaqueEnemigo Embestida;
	Ataques = { Embestida };
}

TArray<FString> SAEVUSEnemigo::Validar(const FSAEVUSAjusteEnemigo& Ajuste)
{
	TArray<FString> Errores;
	if (Ajuste.Ataques.IsEmpty())
	{
		Errores.Add(TEXT("No tiene ataques."));
	}
	for (int32 Indice = 0; Indice < Ajuste.Ataques.Num(); ++Indice)
	{
		const FSAEVUSAtaqueEnemigo& Ataque = Ajuste.Ataques[Indice];
		if (Ataque.DistanciaMinima > Ataque.DistanciaMaxima)
		{
			Errores.Add(FString::Printf(TEXT("Ataque %d (%s): DistanciaMinima mayor que DistanciaMaxima, no sale nunca."), Indice, *Ataque.Nombre.ToString()));
		}
		if (Ataque.TiempoTelegrafiar < AvisoMinimoLegible)
		{
			Errores.Add(FString::Printf(TEXT("Ataque %d (%s): el aviso dura menos de %.2f s, no se puede leer."), Indice, *Ataque.Nombre.ToString(), AvisoMinimoLegible));
		}
		if (Ataque.Tipo == ESAEVUSTipoAtaqueEnemigo::Picado && !Ajuste.bVolador)
		{
			Errores.Add(FString::Printf(TEXT("Ataque %d (%s): un picado exige que el enemigo sea volador."), Indice, *Ataque.Nombre.ToString()));
		}
		if (Ataque.Tipo == ESAEVUSTipoAtaqueEnemigo::Proyectil && Ataque.NumeroProyectiles < 1)
		{
			Errores.Add(FString::Printf(TEXT("Ataque %d (%s): un proyectil lanza al menos uno."), Indice, *Ataque.Nombre.ToString()));
		}
	}
	for (int32 Indice = 0; Indice < Ajuste.Fases.Num(); ++Indice)
	{
		const FSAEVUSFaseEnemigo& Fase = Ajuste.Fases[Indice];
		if (Indice > 0 && Fase.FraccionVida > Ajuste.Fases[Indice - 1].FraccionVida)
		{
			Errores.Add(FString::Printf(TEXT("Fase %d: empieza a más vida que la anterior; las fases deben ir de más a menos vida."), Indice));
		}
		if (Fase.DuracionTransicion < 0.f)
		{
			Errores.Add(FString::Printf(TEXT("Fase %d: la transición no puede durar un tiempo negativo."), Indice));
		}
		for (const int32 IndiceAtaque : Fase.Ataques)
		{
			if (!Ajuste.Ataques.IsValidIndex(IndiceAtaque))
			{
				Errores.Add(FString::Printf(TEXT("Fase %d: el ataque %d no existe."), Indice, IndiceAtaque));
			}
		}
	}
	return Errores;
}

#if WITH_EDITOR
EDataValidationResult USAEVUSDatosEnemigo::IsDataValid(FDataValidationContext& Contexto) const
{
	EDataValidationResult Resultado = Super::IsDataValid(Contexto);
	for (const FString& Error : SAEVUSEnemigo::Validar(Enemigo))
	{
		Contexto.AddError(FText::FromString(Error));
		Resultado = EDataValidationResult::Invalid;
	}
	return Resultado == EDataValidationResult::NotValidated ? EDataValidationResult::Valid : Resultado;
}
#endif

#undef LOCTEXT_NAMESPACE
