// Copyright SAEVUS. Todos los derechos reservados.

#include "Pruebas/SAEVUSMundoPrueba.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AnimationRuntime.h"
#include "Armas/SAEVUSTiposArma.h"
#include "Components/PoseableMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Movimiento/SAEVUSAjustesMovimiento.h"

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
	constexpr float SubidaMinimaManoTajoArriba = 40.f;
	constexpr float BajadaMinimaCabezaVoltereta = 30.f;
	constexpr float FraccionDashMedida = 0.6f;

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionTajoArriba, "SAEVUS.Animacion.TajoArriba", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionTajoArriba::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	Mundo.AvanzarDurante(TiempoAsentarPose);
	const float ManoReposo = static_cast<float>(Mundo.Hueso(ESAEVUSHueso::ManoD).Z);
	USAEVUSComponenteCombate* Combate = Mundo.Personaje->ObtenerCombate();
	Combate->PedirAtaqueEnDireccion(ESAEVUSDireccionAtaque::Arriba);
	float ManoMaxima = ManoReposo;
	while (Combate->EstaAtacando())
	{
		Mundo.MundoPrueba.Paso();
		ManoMaxima = FMath::Max(ManoMaxima, static_cast<float>(Mundo.Hueso(ESAEVUSHueso::ManoD).Z));
	}
	AddInfo(FString::Printf(TEXT("La mano sube %.1f cm en el tajo hacia arriba"), ManoMaxima - ManoReposo));
	TestTrue(TEXT("El tajo hacia arriba levanta la mano por encima del reposo"), ManoMaxima - ManoReposo > SubidaMinimaManoTajoArriba);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionEstelaArma, "SAEVUS.Animacion.EstelaArma", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionEstelaArma::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	Mundo.AvanzarDurante(TiempoAsentarPose);
	FVector Base;
	FVector Punta;
	if (!TestTrue(TEXT("Con la espada visible hay hoja"), Mundo.Animacion->ObtenerHoja(Base, Punta)))
	{
		return false;
	}
	TestEqual(TEXT("La hoja mide lo ajustado"), static_cast<float>(FVector::Dist(Base, Punta)), FSAEVUSAjusteAnimacion().LongitudHoja, 1.f);
	TestTrue(TEXT("La base de la hoja está cerca de la mano"), FVector::Dist(Base, Mundo.Cuerpo->GetBoneLocationByName(TEXT("mano_d"), EBoneSpaces::WorldSpace)) < 30.f);

	const USAEVUSComponenteEfectosCombate* Efectos = Mundo.Personaje->ObtenerEfectosCombate();
	TestEqual(TEXT("Sin atacar no hay estela"), Efectos->ObtenerParticulasActivas(), 0);
	USAEVUSComponenteCombate* Combate = Mundo.Personaje->ObtenerCombate();
	Combate->PedirAtaque();
	while (Combate->EstaAtacando() && Combate->ObtenerFaseAtaque() != ESAEVUSFaseAtaque::Recuperacion)
	{
		Mundo.MundoPrueba.Paso();
	}
	AddInfo(FString::Printf(TEXT("Partículas de estela tras la fase activa: %d"), Efectos->ObtenerParticulasActivas()));
	TestTrue(TEXT("La fase activa deja estela"), Efectos->ObtenerParticulasActivas() > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionMirada, "SAEVUS.Animacion.MiradaEnemigo", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionMirada::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	Mundo.AvanzarDurante(TiempoAsentarPose);
	const FName Cabeza = USAEVUSComponenteAnimacionProcedural::NombreHueso(ESAEVUSHueso::Cabeza);
	const float GuinadaSola = static_cast<float>(Mundo.Cuerpo->GetBoneRotationByName(Cabeza, EBoneSpaces::WorldSpace).Yaw);

	// Un enemigo a la derecha del personaje (que mira a +X): +Y en el mundo. Sin objetivo, no se mueve.
	Mundo.MundoPrueba.CrearEnemigo(Mundo.Personaje->GetActorLocation() + FVector(200.f, 400.f, 0.f), nullptr, nullptr);
	Mundo.AvanzarDurante(TiempoAsentarPose);
	const float GuinadaMirando = static_cast<float>(Mundo.Cuerpo->GetBoneRotationByName(Cabeza, EBoneSpaces::WorldSpace).Yaw);
	// Guiñada positiva = de +X hacia +Y: hacia el enemigo.
	const float Diferencia = FRotator::NormalizeAxis(GuinadaMirando - GuinadaSola);
	AddInfo(FString::Printf(TEXT("La cabeza gira %.1f grados hacia el enemigo"), Diferencia));
	TestTrue(TEXT("La cabeza se gira hacia el enemigo cercano (a su derecha)"), Diferencia > 20.f);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSAEVUSPruebaAnimacionDireccional, "SAEVUS.Animacion.Direccional", SAEVUSPruebas::BanderasPrueba)
bool FSAEVUSPruebaAnimacionDireccional::RunTest(const FString& Parameters)
{
	using namespace SAEVUSPruebasAnimacion;

	FMundoAnimacion Mundo(*this);
	if (!Mundo.bListo || !TestTrue(TEXT("Esqueleto válido"), Mundo.Animacion->EsqueletoValido()))
	{
		return false;
	}
	// De lado (el cuerpo sigue de frente, como en GunZ): el pie oscila de lado (eje X de la malla).
	const FVector Derecha = Mundo.Personaje->GetActorRightVector();
	float Minimo = TNumericLimits<float>::Max();
	float Maximo = TNumericLimits<float>::Lowest();
	for (int32 Indice = 0; Indice < SAEVUSPruebas::PasosPara(TiempoCarrera); ++Indice)
	{
		Mundo.Personaje->AddMovementInput(Derecha, 1.f);
		Mundo.MundoPrueba.Paso();
		const float X = static_cast<float>(Mundo.Hueso(ESAEVUSHueso::PieD).X);
		Minimo = FMath::Min(Minimo, X);
		Maximo = FMath::Max(Maximo, X);
	}
	AddInfo(FString::Printf(TEXT("Recorrido lateral del pie al correr de lado %.1f cm"), Maximo - Minimo));
	TestTrue(TEXT("Las piernas oscilan de lado al correr de lado"), Maximo - Minimo > RecorridoMinimoPie);

	// Dash a un lado: voltereta (a media voltereta la cabeza queda muy por debajo de su altura de pie).
	Mundo.AvanzarDurante(TiempoAsentarPose);
	const float CabezaDePie = static_cast<float>(Mundo.Hueso(ESAEVUSHueso::Cabeza).Z);
	// Sin controlador el cuerpo se giraría hacia el dash (lo haría frontal); en el juego sigue a la cámara.
	USAEVUSComponenteMovimiento* Movimiento = Mundo.Personaje->ObtenerMovimiento();
	USAEVUSAjustesMovimiento* Ajustes = NewObject<USAEVUSAjustesMovimiento>(GetTransientPackage());
	Ajustes->Dash.bGirarHaciaDash = false;
	Movimiento->FijarAjustesMovimiento(Ajustes);
	Movimiento->PedirDashEnDireccion(Derecha);
	Mundo.AvanzarDurante(Movimiento->ObtenerAjusteDash().Duracion * FraccionDashMedida);
	TestTrue(TEXT("En dash"), Movimiento->EstaEnDash());
	const float Bajada = CabezaDePie - static_cast<float>(Mundo.Hueso(ESAEVUSHueso::Cabeza).Z);
	AddInfo(FString::Printf(TEXT("Bajada de la cabeza en la voltereta %.1f cm"), Bajada));
	TestTrue(TEXT("El dash lateral es una voltereta"), Bajada > BajadaMinimaCabezaVoltereta);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
