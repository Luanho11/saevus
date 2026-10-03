// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AnimationRuntime.h"
#include "Armas/SAEVUSTiposArma.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/SkeletalMesh.h"

namespace SAEVUSPruebasAnimacion
{
	const TCHAR* const RutaCaballero = TEXT("/Game/SAEVUS/Arte/Personajes/Caballero/SK_Caballero.SK_Caballero");

	const FVector CentroSuelo(0.f, 0.f, -50.f);
	const FVector TamanoSuelo(4000.f, 4000.f, 100.f);
	const FVector PosicionDePie(0.f, 0.f, 150.f);

	// Movimientos mínimos (cm) que demuestran que la pose cambia de verdad.
	constexpr float RecorridoMinimoPie = 20.f;
	constexpr float RecorridoMinimoMano = 40.f;
	constexpr float AvanceMinimoCabezaDash = 12.f;
	constexpr float TiempoCarrera = 0.6f;
	constexpr float TiempoAsentarPose = 0.5f;
	constexpr float ToleranciaEscala = 0.1f;

	/** Mundo con suelo y un personaje con el caballero en su malla poseable. */
	struct FMundoAnimacion
	{
		SAEVUSPruebas::FMundoPrueba MundoPrueba;
		ASAEVUSPersonaje* Personaje = nullptr;
		UPoseableMeshComponent* Cuerpo = nullptr;
		USAEVUSComponenteAnimacionProcedural* Animacion = nullptr;
		bool bListo = false;

		explicit FMundoAnimacion(FAutomationTestBase& Test)
		{
			USkeletalMesh* Caballero = LoadObject<USkeletalMesh>(nullptr, RutaCaballero);
			if (!Caballero)
			{
				Test.AddError(FString::Printf(TEXT("Falta %s: genera e importa el caballero (Generar-Caballero.ps1 + Verificar-Arranque.ps1)"), RutaCaballero));
				return;
			}
			MundoPrueba.CrearBloque(CentroSuelo, TamanoSuelo);
			Personaje = MundoPrueba.CrearPersonaje(PosicionDePie);
			Cuerpo = Personaje->ObtenerCuerpo();
			Animacion = Personaje->ObtenerAnimacion();
			Cuerpo->SetSkinnedAssetAndUpdate(Caballero, true);
			Animacion->FijarMalla(Cuerpo);
			bListo = MundoPrueba.AsentarEnSuelo();
		}

		FVector Hueso(ESAEVUSHueso Hueso) const
		{
			return Cuerpo->GetBoneLocationByName(USAEVUSComponenteAnimacionProcedural::NombreHueso(Hueso), EBoneSpaces::ComponentSpace);
		}

		/** Escala del hueso relativa a la de reposo (el FBX de Blender trae escala 100 en la raíz). */
		float EscalaHueso(ESAEVUSHueso Hueso) const
		{
			const FName Nombre = USAEVUSComponenteAnimacionProcedural::NombreHueso(Hueso);
			const FReferenceSkeleton& Referencia = Cuerpo->GetSkinnedAsset()->GetRefSkeleton();
			const FTransform Reposo = FAnimationRuntime::GetComponentSpaceTransformRefPose(Referencia, Referencia.FindBoneIndex(Nombre));
			return static_cast<float>(Cuerpo->GetBoneTransformByName(Nombre, EBoneSpaces::ComponentSpace).GetScale3D().X / Reposo.GetScale3D().X);
		}

		void AvanzarDurante(float Segundos, const FVector& Entrada = FVector::ZeroVector)
		{
			for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(Segundos); ++Indice)
			{
				if (!Entrada.IsZero())
				{
					Personaje->AddMovementInput(Entrada, 1.f);
				}
				MundoPrueba.Paso();
			}
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaEsqueletoCaballero, "SAEVUS.Animacion.Esqueleto", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaEsqueletoCaballero::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.Animacion)
	{
		return false;
	}
	TestTrue(TEXT("El caballero tiene todos los huesos del esqueleto propio"), Mundo.Animacion->EsqueletoValido());
	TestTrue(TEXT("La cabeza está por encima de la pelvis"), Mundo.Hueso(ESAEVUSHueso::Cabeza).Z > Mundo.Hueso(ESAEVUSHueso::Pelvis).Z);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionEscala, "SAEVUS.Animacion.EscalaReposo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionEscala::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	// Regresión: la animación escribía escala 1 sobre una raíz con escala de reposo 100 y la malla
	// se encogía hasta ser invisible aunque las posiciones de los huesos fueran correctas.
	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	Mundo.AvanzarDurante(TiempoAsentarPose);
	for (const ESAEVUSHueso Hueso : { ESAEVUSHueso::Raiz, ESAEVUSHueso::Pelvis, ESAEVUSHueso::Cabeza, ESAEVUSHueso::PieD })
	{
		TestEqual(FString::Printf(TEXT("Escala de %s igual a la de reposo"), *USAEVUSComponenteAnimacionProcedural::NombreHueso(Hueso).ToString()),
			Mundo.EscalaHueso(Hueso), 1.f, ToleranciaEscala);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionCorrer, "SAEVUS.Animacion.Correr", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionCorrer::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	// Recorrido del pie derecho adelante-atrás (eje Y de la malla) mientras corre.
	float Minimo = TNumericLimits<float>::Max();
	float Maximo = TNumericLimits<float>::Lowest();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoCarrera); ++Indice)
	{
		Mundo.Personaje->AddMovementInput(FVector::ForwardVector, 1.f);
		Mundo.MundoPrueba.Paso();
		const float Y = static_cast<float>(Mundo.Hueso(ESAEVUSHueso::PieD).Y);
		Minimo = FMath::Min(Minimo, Y);
		Maximo = FMath::Max(Maximo, Y);
	}
	AddInfo(FString::Printf(TEXT("Recorrido del pie al correr %.1f cm"), Maximo - Minimo));
	TestTrue(TEXT("Las piernas se mueven al correr"), Maximo - Minimo > RecorridoMinimoPie);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionAtaque, "SAEVUS.Animacion.Ataque", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionAtaque::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	USAEVUSComponenteCombate* Combate = Mundo.Personaje->ObtenerCombate();
	Mundo.Personaje->PedirAtaque();
	const FSAEVUSDefinicionAtaque* Ataque = Combate->ObtenerAtaqueActual();
	if (!TestNotNull(TEXT("Ataque en curso"), Ataque))
	{
		return false;
	}
	// La mano derecha barre el arco del tajo: medir su recorrido lateral (X de la malla) en preparación + fase activa.
	float Minimo = TNumericLimits<float>::Max();
	float Maximo = TNumericLimits<float>::Lowest();
	while (Combate->EstaAtacando() && Combate->ObtenerTiempoAtaque() < Ataque->TiempoPreparacion + Ataque->TiempoActivo)
	{
		Mundo.MundoPrueba.Paso();
		const float X = static_cast<float>(Mundo.Hueso(ESAEVUSHueso::ManoD).X);
		Minimo = FMath::Min(Minimo, X);
		Maximo = FMath::Max(Maximo, X);
	}
	AddInfo(FString::Printf(TEXT("Recorrido lateral de la mano en el tajo %.1f cm"), Maximo - Minimo));
	TestTrue(TEXT("El tajo mueve la mano de un lado a otro"), Maximo - Minimo > RecorridoMinimoMano);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionDash, "SAEVUS.Animacion.Dash", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionDash::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	Mundo.AvanzarDurante(TiempoAsentarPose);
	const float CabezaQuieta = static_cast<float>(Mundo.Hueso(ESAEVUSHueso::Cabeza).Y);
	Mundo.Personaje->ObtenerMovimiento()->PedirDash();
	Mundo.AvanzarDurante(Mundo.Personaje->ObtenerMovimiento()->ObtenerAjusteDash().Duracion * 0.8f);
	const float Avance = static_cast<float>(Mundo.Hueso(ESAEVUSHueso::Cabeza).Y) - CabezaQuieta;
	AddInfo(FString::Printf(TEXT("Avance de la cabeza en el dash %.1f cm"), Avance));
	TestTrue(TEXT("El dash inclina el cuerpo hacia delante"), Avance > AvanceMinimoCabezaDash);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionArmas, "SAEVUS.Animacion.ArmaVisible", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionArmas::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	Mundo.AvanzarDurante(TiempoAsentarPose);
	TestEqual(TEXT("Sin arma de fuego: el aguijón se ve"), Mundo.EscalaHueso(ESAEVUSHueso::Arma), 1.f, ToleranciaEscala);
	TestEqual(TEXT("Sin arma de fuego: la pistola no se ve"), Mundo.EscalaHueso(ESAEVUSHueso::ArmaFuego), 0.f, ToleranciaEscala);

	USAEVUSDefinicionArma* Pistola = NewObject<USAEVUSDefinicionArma>(GetTransientPackage());
	Pistola->Tipo = ESAEVUSTipoArma::ArmaFuego;
	Mundo.Personaje->ObtenerArmas()->FijarArsenal({ Pistola });
	Mundo.AvanzarDurante(TiempoAsentarPose);
	TestEqual(TEXT("Con pistola: el aguijón se oculta"), Mundo.EscalaHueso(ESAEVUSHueso::Arma), 0.f, ToleranciaEscala);
	TestEqual(TEXT("Con pistola: la pistola se ve"), Mundo.EscalaHueso(ESAEVUSHueso::ArmaFuego), 1.f, ToleranciaEscala);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
