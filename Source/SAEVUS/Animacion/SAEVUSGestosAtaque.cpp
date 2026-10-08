// Copyright SAEVUS. Todos los derechos reservados.

#include "Animacion/SAEVUSGestosAtaque.h"
#include "Combate/SAEVUSTiposCombate.h"

namespace SAEVUSGestos
{
	// Valor sin dar en una clave: se queda el de la clave anterior.
	constexpr float Igual = -99999.f;

	/** Cómo se llega a una clave desde la anterior. */
	enum class ECurva : uint8
	{
		/** Arranca y frena suave (preparaciones, asentamientos). */
		Suave,
		/** Sale disparada y frena al final: el corte de la hoja. */
		Rapida
	};

	/** Una clave del gesto: lo que no se da (Igual o vector nulo) sigue como en la clave anterior. */
	struct FClave
	{
		float Tiempo = 0.f;
		ECurva Curva = ECurva::Suave;
		FVector Brazo = FVector::ZeroVector;
		FVector Hoja = FVector::ZeroVector;
		float Codo = Igual;
		FVector BrazoLibre = FVector::ZeroVector;
		float CodoLibre = Igual;
		float GiroPecho = Igual;
		float Inclinacion = Igual;
		float Ladeo = Igual;
		float GiroCadera = Igual;
		float GiroRaiz = Igual;
		float Volteo = Igual;
		float Postura = Igual;
		float Bajada = Igual;
	};

	// Postura lista (REF-04 n.º 1): brazo bajo y adelantado, hoja baja en diagonal, brazo libre suelto.
	const FVector BrazoListo(0.25f, 0.5f, -0.7f);
	const FVector HojaLista(0.3f, 0.75f, -0.55f);
	const FVector LibreListo(-0.3f, 0.3f, -0.85f);

	/** Clave inicial y final de todos los gestos de suelo: la postura lista, todo a cero. */
	FClave Lista(float Tiempo)
	{
		FClave Clave;
		Clave.Tiempo = Tiempo;
		Clave.Brazo = BrazoListo;
		Clave.Hoja = HojaLista;
		Clave.Codo = 20.f;
		Clave.BrazoLibre = LibreListo;
		Clave.CodoLibre = 25.f;
		Clave.GiroPecho = Clave.Inclinacion = Clave.Ladeo = Clave.GiroCadera = Clave.GiroRaiz = Clave.Volteo = 0.f;
		Clave.Postura = Clave.Bajada = 0.f;
		return Clave;
	}

	/**
	 * Claves de cada gesto. Cada tajo de la cadena empieza donde acaba el anterior (el tajo deja la hoja a la
	 * izquierda y el revés sale de ahí; el revés la deja arriba a la derecha y el remate la sube desde ahí).
	 * Todos tienen amago (la hoja va atrás antes de salir), corte rápido en la fase activa y seguimiento que
	 * se pasa del final antes de asentarse.
	 */
	TArray<FClave> CrearClaves(ESAEVUSGestoAtaque Gesto)
	{
		using E = ESAEVUSGestoAtaque;
		const ECurva R = ECurva::Rapida;
		switch (Gesto)
		{
		case E::TajoDerecha:
			return {
				Lista(0.f),
				{ .Tiempo = 0.8f, .Brazo = { 0.8f, -0.3f, 0.35f }, .Hoja = { 0.55f, -0.75f, 0.25f }, .Codo = 55.f, .BrazoLibre = { -0.4f, 0.7f, -0.2f },
					.CodoLibre = 40.f, .GiroPecho = 40.f, .Inclinacion = 4.f, .Ladeo = 4.f, .GiroCadera = 15.f, .Postura = 0.6f, .Bajada = 6.f },
				{ .Tiempo = 1.f, .Curva = R, .Brazo = { 0.95f, 0.25f, 0.15f }, .Hoja = { 0.85f, 0.5f, 0.05f }, .Codo = 30.f, .BrazoLibre = { -0.4f, 0.6f, -0.4f },
					.GiroPecho = 30.f, .Inclinacion = 8.f, .Postura = 0.9f, .Bajada = 12.f },
				{ .Tiempo = 1.5f, .Curva = R, .Brazo = { 0.1f, 1.f, 0.05f }, .Hoja = { 0.f, 1.f, -0.05f }, .Codo = 5.f, .BrazoLibre = { -0.5f, 0.2f, -0.6f },
					.GiroPecho = 0.f, .Inclinacion = 12.f, .Ladeo = 0.f, .GiroCadera = 0.f, .Postura = 1.f, .Bajada = 15.f },
				{ .Tiempo = 2.f, .Curva = R, .Brazo = { -0.85f, 0.5f, -0.05f }, .Hoja = { -0.9f, 0.3f, -0.15f }, .Codo = 15.f, .BrazoLibre = { -0.6f, -0.3f, -0.5f },
					.CodoLibre = 30.f, .GiroPecho = -40.f, .Ladeo = -5.f, .GiroCadera = -10.f },
				{ .Tiempo = 2.3f, .Brazo = { -0.75f, 0.05f, -0.45f }, .Hoja = { -0.55f, -0.45f, -0.5f }, .Codo = 25.f, .GiroPecho = -50.f, .Inclinacion = 14.f,
					.Ladeo = -6.f, .Bajada = 16.f },
				Lista(3.f),
			};
		case E::Reves:
			return {
				{ .Tiempo = 0.f, .Brazo = { -0.5f, 0.3f, -0.65f }, .Hoja = { -0.6f, -0.1f, -0.6f }, .Codo = 30.f, .BrazoLibre = { -0.5f, 0.f, -0.6f }, .CodoLibre = 30.f,
					.GiroPecho = -35.f, .Inclinacion = 8.f, .Ladeo = -4.f, .GiroCadera = -8.f, .GiroRaiz = 0.f, .Volteo = 0.f, .Postura = 0.4f, .Bajada = 8.f },
				{ .Tiempo = 0.8f, .Brazo = { -0.75f, -0.15f, -0.35f }, .Hoja = { -0.45f, -0.8f, -0.35f }, .Codo = 70.f, .BrazoLibre = { -0.2f, 0.75f, -0.1f },
					.CodoLibre = 45.f, .GiroPecho = -50.f, .Inclinacion = 6.f, .Ladeo = -6.f, .GiroCadera = -15.f, .Postura = 0.7f, .Bajada = 10.f },
				{ .Tiempo = 1.f, .Curva = R, .Brazo = { -0.85f, 0.4f, -0.2f }, .Hoja = { -0.8f, 0.55f, -0.2f }, .Codo = 40.f, .GiroPecho = -35.f,
					.Postura = 0.9f, .Bajada = 13.f },
				{ .Tiempo = 1.5f, .Curva = R, .Brazo = { 0.f, 1.f, 0.15f }, .Hoja = { 0.1f, 0.95f, 0.25f }, .Codo = 10.f, .BrazoLibre = { -0.5f, 0.2f, -0.55f },
					.GiroPecho = 0.f, .Inclinacion = 10.f, .Ladeo = 0.f, .GiroCadera = 0.f, .Postura = 1.f, .Bajada = 15.f },
				{ .Tiempo = 2.f, .Curva = R, .Brazo = { 0.8f, 0.45f, 0.4f }, .Hoja = { 0.75f, 0.3f, 0.6f }, .Codo = 15.f, .BrazoLibre = { -0.7f, -0.1f, -0.45f },
					.GiroPecho = 40.f, .Ladeo = 5.f, .GiroCadera = 10.f },
				{ .Tiempo = 2.3f, .Brazo = { 0.6f, 0.f, 0.75f }, .Hoja = { 0.35f, -0.35f, 0.85f }, .Codo = 30.f, .GiroPecho = 45.f, .Inclinacion = 4.f, .Bajada = 12.f },
				Lista(3.f),
			};
		case E::Remate:
			return {
				{ .Tiempo = 0.f, .Brazo = { 0.6f, 0.f, 0.75f }, .Hoja = { 0.35f, -0.35f, 0.85f }, .Codo = 30.f, .BrazoLibre = { -0.5f, 0.1f, -0.5f }, .CodoLibre = 30.f,
					.GiroPecho = 20.f, .Inclinacion = 0.f, .Ladeo = 0.f, .GiroCadera = 0.f, .GiroRaiz = 0.f, .Volteo = 0.f, .Postura = 0.2f, .Bajada = 4.f },
				// A dos manos por encima de la cabeza, arqueado atrás y de puntillas.
				{ .Tiempo = 0.85f, .Brazo = { 0.15f, -0.2f, 0.97f }, .Hoja = { 0.05f, -0.85f, 0.5f }, .Codo = 70.f, .BrazoLibre = { 0.25f, -0.1f, 0.95f },
					.CodoLibre = 70.f, .GiroPecho = 10.f, .Inclinacion = -14.f, .Postura = 0.5f, .Bajada = -4.f },
				{ .Tiempo = 1.f, .Curva = R, .Brazo = { 0.1f, 0.35f, 0.93f }, .Hoja = { 0.05f, 0.4f, 0.92f }, .Codo = 40.f, .BrazoLibre = { 0.15f, 0.35f, 0.9f },
					.CodoLibre = 45.f, .Inclinacion = -6.f, .Bajada = 0.f },
				{ .Tiempo = 1.5f, .Curva = R, .Brazo = { 0.05f, 1.f, 0.1f }, .Hoja = { 0.f, 1.f, -0.1f }, .Codo = 10.f, .BrazoLibre = { 0.1f, 1.f, 0.05f },
					.CodoLibre = 15.f, .GiroPecho = 0.f, .Inclinacion = 18.f, .Postura = 1.f, .Bajada = 18.f },
				{ .Tiempo = 2.f, .Curva = R, .Brazo = { 0.05f, 0.6f, -0.8f }, .Hoja = { 0.f, 0.55f, -0.85f }, .BrazoLibre = { 0.15f, 0.6f, -0.78f },
					.Inclinacion = 30.f, .Bajada = 26.f },
				{ .Tiempo = 2.5f, .Inclinacion = 28.f, .Bajada = 25.f },
				Lista(3.f),
			};
		case E::Estocada:
			return {
				Lista(0.f),
				// La hoja recogida junto a la cadera, la punta al frente; el brazo libre apunta.
				{ .Tiempo = 0.7f, .Brazo = { 0.35f, -0.6f, -0.1f }, .Hoja = { 0.05f, 1.f, 0.f }, .Codo = 100.f, .BrazoLibre = { -0.3f, 0.8f, 0.1f },
					.CodoLibre = 20.f, .GiroPecho = 30.f, .Inclinacion = 15.f, .GiroCadera = 10.f, .Postura = 0.6f, .Bajada = 10.f },
				{ .Tiempo = 1.f, .Codo = 90.f },
				{ .Tiempo = 1.6f, .Curva = R, .Brazo = { 0.05f, 1.f, 0.f }, .Hoja = { 0.f, 1.f, 0.f }, .Codo = 0.f, .BrazoLibre = { -0.6f, -0.6f, -0.3f },
					.CodoLibre = 15.f, .GiroPecho = -15.f, .Inclinacion = 28.f, .GiroCadera = -5.f, .Postura = 1.f, .Bajada = 22.f },
				{ .Tiempo = 2.f },
				{ .Tiempo = 2.6f, .Brazo = { 0.2f, 0.8f, -0.5f }, .Hoja = { 0.15f, 0.8f, -0.55f }, .Codo = 15.f, .GiroPecho = 0.f, .Inclinacion = 15.f,
					.Postura = 0.6f, .Bajada = 12.f },
				Lista(3.f),
			};
		case E::TajoArriba:
			return {
				Lista(0.f),
				// Agachado con la hoja baja delante; sale hacia arriba estirando todo el cuerpo.
				{ .Tiempo = 0.8f, .Brazo = { 0.35f, 0.45f, -0.8f }, .Hoja = { 0.25f, 0.75f, -0.6f }, .Codo = 15.f, .BrazoLibre = { -0.4f, 0.5f, -0.5f },
					.GiroPecho = 20.f, .Inclinacion = 18.f, .Postura = 0.6f, .Bajada = 16.f },
				{ .Tiempo = 1.f, .Curva = R, .Brazo = { 0.3f, 0.8f, -0.5f }, .Hoja = { 0.2f, 0.95f, -0.25f }, .Inclinacion = 12.f, .Bajada = 12.f },
				{ .Tiempo = 1.6f, .Curva = R, .Brazo = { 0.15f, 0.6f, 0.78f }, .Hoja = { 0.1f, 0.45f, 0.88f }, .GiroPecho = -5.f, .Inclinacion = -12.f, .Bajada = -6.f },
				{ .Tiempo = 2.f, .Curva = R, .Brazo = { 0.1f, -0.1f, 0.99f }, .Hoja = { 0.05f, -0.35f, 0.94f }, .Codo = 10.f, .BrazoLibre = { -0.5f, 0.f, -0.6f },
					.Inclinacion = -18.f, .Bajada = -8.f },
				{ .Tiempo = 2.4f, .Inclinacion = -15.f },
				Lista(3.f),
			};
		case E::AereoDiagonal:
			return {
				Lista(0.f),
				{ .Tiempo = 0.8f, .Brazo = { 0.55f, -0.2f, 0.8f }, .Hoja = { 0.35f, -0.6f, 0.7f }, .Codo = 60.f, .BrazoLibre = { -0.5f, 0.4f, -0.3f },
					.GiroPecho = 30.f, .Inclinacion = -10.f, .Ladeo = 6.f, .Postura = 0.8f },
				{ .Tiempo = 1.f, .Curva = R, .Brazo = { 0.6f, 0.3f, 0.75f }, .Hoja = { 0.55f, 0.45f, 0.7f }, .Codo = 30.f, .GiroPecho = 25.f, .Inclinacion = -4.f },
				{ .Tiempo = 1.5f, .Curva = R, .Brazo = { 0.f, 1.f, 0.f }, .Hoja = { -0.1f, 1.f, -0.1f }, .Codo = 10.f, .GiroPecho = 0.f, .Inclinacion = 10.f, .Ladeo = 0.f,
					.Postura = 1.f },
				{ .Tiempo = 2.f, .Curva = R, .Brazo = { -0.45f, 0.55f, -0.7f }, .Hoja = { -0.4f, 0.45f, -0.8f }, .BrazoLibre = { -0.6f, -0.3f, -0.2f },
					.GiroPecho = -30.f, .Inclinacion = 18.f, .Ladeo = -6.f },
				{ .Tiempo = 2.3f, .Brazo = { -0.4f, 0.15f, -0.9f }, .Hoja = { -0.3f, -0.2f, -0.93f }, .GiroPecho = -35.f, .Inclinacion = 16.f },
				Lista(3.f),
			};
		case E::AereoReves:
			return {
				{ .Tiempo = 0.f, .Brazo = { -0.4f, 0.15f, -0.9f }, .Hoja = { -0.3f, -0.2f, -0.93f }, .Codo = 20.f, .BrazoLibre = { -0.6f, -0.3f, -0.2f }, .CodoLibre = 30.f,
					.GiroPecho = -30.f, .Inclinacion = 12.f, .Ladeo = 0.f, .GiroCadera = 0.f, .GiroRaiz = 0.f, .Volteo = 0.f, .Postura = 0.8f, .Bajada = 0.f },
				// El tronco se tuerce a la izquierda y las piernas se van al lado contrario.
				{ .Tiempo = 0.8f, .Brazo = { -0.8f, -0.2f, 0.f }, .Hoja = { -0.5f, -0.8f, 0.1f }, .Codo = 75.f, .BrazoLibre = { -0.2f, 0.7f, -0.2f },
					.GiroPecho = -55.f, .Inclinacion = 4.f, .Ladeo = -5.f, .GiroCadera = 20.f },
				{ .Tiempo = 1.f, .Curva = R, .Brazo = { -0.85f, 0.45f, 0.1f }, .Hoja = { -0.8f, 0.6f, 0.05f }, .Codo = 35.f, .GiroPecho = -40.f, .GiroCadera = 15.f },
				{ .Tiempo = 1.5f, .Curva = R, .Brazo = { 0.f, 1.f, 0.1f }, .Hoja = { 0.1f, 1.f, 0.05f }, .Codo = 10.f, .GiroPecho = 0.f, .Ladeo = 0.f, .GiroCadera = 0.f,
					.Postura = 1.f },
				{ .Tiempo = 2.f, .Curva = R, .Brazo = { 0.85f, 0.45f, 0.f }, .Hoja = { 0.9f, 0.3f, -0.05f }, .BrazoLibre = { -0.7f, -0.1f, -0.45f },
					.GiroPecho = 45.f, .Ladeo = 6.f, .GiroCadera = -20.f },
				{ .Tiempo = 2.3f, .Brazo = { 0.75f, -0.1f, -0.3f }, .Hoja = { 0.5f, -0.55f, -0.4f }, .Codo = 25.f, .GiroPecho = 55.f, .GiroCadera = -25.f },
				Lista(3.f),
			};
		case E::AereoRemate:
			return {
				{ .Tiempo = 0.f, .Brazo = { 0.75f, -0.1f, -0.3f }, .Hoja = { 0.5f, -0.55f, -0.4f }, .Codo = 25.f, .BrazoLibre = { -0.7f, -0.1f, -0.45f }, .CodoLibre = 30.f,
					.GiroPecho = 40.f, .Inclinacion = 0.f, .Ladeo = 0.f, .GiroCadera = 0.f, .GiroRaiz = 0.f, .Volteo = 0.f, .Postura = 0.8f, .Bajada = 0.f },
				// A dos manos arriba, el cuerpo arqueado; luego todo el cuerpo se vuelca con el tajo y las piernas quedan atrás.
				{ .Tiempo = 0.8f, .Brazo = { 0.2f, -0.25f, 0.95f }, .Hoja = { 0.1f, -0.8f, 0.6f }, .Codo = 70.f, .BrazoLibre = { 0.2f, -0.1f, 0.95f },
					.CodoLibre = 70.f, .GiroPecho = 5.f, .Inclinacion = -20.f, .Postura = 1.f },
				{ .Tiempo = 1.f, .Curva = R, .Brazo = { 0.1f, 0.35f, 0.93f }, .Hoja = { 0.05f, 0.4f, 0.9f }, .Codo = 40.f, .BrazoLibre = { 0.15f, 0.35f, 0.9f },
					.CodoLibre = 45.f, .GiroPecho = 0.f, .Inclinacion = -8.f },
				{ .Tiempo = 1.5f, .Curva = R, .Brazo = { 0.05f, 1.f, 0.f }, .Hoja = { 0.f, 1.f, -0.15f }, .Codo = 10.f, .BrazoLibre = { 0.1f, 1.f, -0.05f },
					.CodoLibre = 15.f, .Inclinacion = 25.f, .Volteo = 25.f, .Postura = 0.5f },
				{ .Tiempo = 2.f, .Curva = R, .Brazo = { 0.05f, 0.4f, -0.9f }, .Hoja = { 0.f, 0.3f, -0.95f }, .BrazoLibre = { 0.15f, 0.4f, -0.88f },
					.Inclinacion = 35.f, .Volteo = 40.f, .Postura = 0.f },
				{ .Tiempo = 2.4f, .Volteo = 30.f },
				Lista(3.f),
			};
		case E::Giratorio:
		default:
			return {
				Lista(0.f),
				// Enroscado hacia la derecha, la hoja extendida atrás; una vuelta entera con la hoja de lado.
				{ .Tiempo = 0.9f, .Brazo = { 0.9f, -0.3f, 0.1f }, .Hoja = { 0.85f, -0.45f, 0.f }, .Codo = 20.f, .BrazoLibre = { -0.7f, 0.5f, 0.f },
					.CodoLibre = 20.f, .GiroPecho = 60.f, .Inclinacion = 10.f, .GiroCadera = 20.f, .Postura = 0.8f, .Bajada = 18.f },
				{ .Tiempo = 1.f, .Curva = R, .Brazo = { 0.95f, 0.3f, 0.f }, .Hoja = { 0.95f, 0.3f, 0.f }, .Codo = 0.f, .GiroPecho = 20.f, .GiroRaiz = 0.f },
				{ .Tiempo = 2.f, .Curva = R, .GiroRaiz = -360.f },
				{ .Tiempo = 2.3f, .Brazo = { 0.7f, 0.6f, -0.3f }, .Hoja = { 0.6f, 0.7f, -0.35f }, .GiroPecho = -15.f, .Inclinacion = 12.f, .GiroRaiz = -360.f },
				Lista(3.f),
			};
		}
	}

	/** Rellena lo que una clave no da con lo de la anterior. */
	void Resolver(TArray<FClave>& Claves)
	{
		auto Heredar = [](float& Valor, float Anterior) { Valor = Valor == Igual ? Anterior : Valor; };
		auto HeredarVector = [](FVector& Valor, const FVector& Anterior) { Valor = Valor.IsZero() ? Anterior : Valor.GetSafeNormal(); };
		for (int32 Indice = 0; Indice < Claves.Num(); ++Indice)
		{
			FClave& Clave = Claves[Indice];
			const FClave Anterior = Indice > 0 ? Claves[Indice - 1] : Lista(0.f);
			HeredarVector(Clave.Brazo, Anterior.Brazo);
			HeredarVector(Clave.Hoja, Anterior.Hoja);
			HeredarVector(Clave.BrazoLibre, Anterior.BrazoLibre);
			Heredar(Clave.Codo, Anterior.Codo);
			Heredar(Clave.CodoLibre, Anterior.CodoLibre);
			Heredar(Clave.GiroPecho, Anterior.GiroPecho);
			Heredar(Clave.Inclinacion, Anterior.Inclinacion);
			Heredar(Clave.Ladeo, Anterior.Ladeo);
			Heredar(Clave.GiroCadera, Anterior.GiroCadera);
			Heredar(Clave.GiroRaiz, Anterior.GiroRaiz);
			Heredar(Clave.Volteo, Anterior.Volteo);
			Heredar(Clave.Postura, Anterior.Postura);
			Heredar(Clave.Bajada, Anterior.Bajada);
		}
	}

	constexpr int32 NumGestos = static_cast<int32>(ESAEVUSGestoAtaque::Giratorio) + 1;

	/** Claves ya resueltas de cada gesto (se crean la primera vez). */
	const TArray<FClave>& Claves(ESAEVUSGestoAtaque Gesto)
	{
		static const TArray<TArray<FClave>> Tabla = []
		{
			TArray<TArray<FClave>> Resultado;
			Resultado.SetNum(NumGestos);
			for (int32 Indice = 0; Indice < NumGestos; ++Indice)
			{
				Resultado[Indice] = CrearClaves(static_cast<ESAEVUSGestoAtaque>(Indice));
				Resolver(Resultado[Indice]);
			}
			return Resultado;
		}();
		return Tabla[FMath::Clamp(static_cast<int32>(Gesto), 0, NumGestos - 1)];
	}

	float Suavizar(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return X * X * (3.f - 2.f * X);
	}

	float SalidaRapida(float X)
	{
		X = FMath::Clamp(X, 0.f, 1.f);
		return 1.f - (1.f - X) * (1.f - X) * (1.f - X);
	}

	/** Interpolación esférica de direcciones unitarias (el brazo barre un arco, no una cuerda). */
	FVector Arco(const FVector& A, const FVector& B, float Alfa)
	{
		const FQuat Total = FQuat::FindBetweenNormals(A, B);
		return FQuat::Slerp(FQuat::Identity, Total, Alfa).RotateVector(A);
	}
}

float SAEVUSGestos::TiempoEnFases(float Tiempo, float Preparacion, float Activo, float Recuperacion)
{
	if (Tiempo < Preparacion)
	{
		return Tiempo / FMath::Max(Preparacion, UE_KINDA_SMALL_NUMBER);
	}
	Tiempo -= Preparacion;
	if (Tiempo < Activo)
	{
		return 1.f + Tiempo / FMath::Max(Activo, UE_KINDA_SMALL_NUMBER);
	}
	Tiempo -= Activo;
	return FMath::Min(3.f, 2.f + Tiempo / FMath::Max(Recuperacion, UE_KINDA_SMALL_NUMBER));
}

FSAEVUSPoseGesto SAEVUSGestos::Evaluar(ESAEVUSGestoAtaque Gesto, float TiempoFases)
{
	const TArray<FClave>& ClavesGesto = Claves(Gesto);
	FSAEVUSPoseGesto Pose;
	if (ClavesGesto.IsEmpty())
	{
		return Pose;
	}
	// Tramo entre la última clave que ya pasó y la siguiente.
	int32 Siguiente = 0;
	while (Siguiente < ClavesGesto.Num() && ClavesGesto[Siguiente].Tiempo <= TiempoFases)
	{
		++Siguiente;
	}
	const FClave& A = ClavesGesto[FMath::Clamp(Siguiente - 1, 0, ClavesGesto.Num() - 1)];
	const FClave& B = ClavesGesto[FMath::Min(Siguiente, ClavesGesto.Num() - 1)];
	const float Largo = B.Tiempo - A.Tiempo;
	const float U = Largo > UE_KINDA_SMALL_NUMBER ? FMath::Clamp((TiempoFases - A.Tiempo) / Largo, 0.f, 1.f) : 1.f;
	const float Alfa = B.Curva == ECurva::Rapida ? SalidaRapida(U) : Suavizar(U);

	Pose.Brazo = Arco(A.Brazo, B.Brazo, Alfa);
	Pose.Hoja = Arco(A.Hoja, B.Hoja, Alfa);
	Pose.BrazoLibre = Arco(A.BrazoLibre, B.BrazoLibre, Alfa);
	Pose.Codo = FMath::Lerp(A.Codo, B.Codo, Alfa);
	Pose.CodoLibre = FMath::Lerp(A.CodoLibre, B.CodoLibre, Alfa);
	Pose.GiroPecho = FMath::Lerp(A.GiroPecho, B.GiroPecho, Alfa);
	Pose.Inclinacion = FMath::Lerp(A.Inclinacion, B.Inclinacion, Alfa);
	Pose.Ladeo = FMath::Lerp(A.Ladeo, B.Ladeo, Alfa);
	Pose.GiroCadera = FMath::Lerp(A.GiroCadera, B.GiroCadera, Alfa);
	Pose.GiroRaiz = FMath::Lerp(A.GiroRaiz, B.GiroRaiz, Alfa);
	Pose.Volteo = FMath::Lerp(A.Volteo, B.Volteo, Alfa);
	Pose.Postura = FMath::Lerp(A.Postura, B.Postura, Alfa);
	Pose.Bajada = FMath::Lerp(A.Bajada, B.Bajada, Alfa);
	return Pose;
}

FSAEVUSPisadaGesto SAEVUSGestos::Pisada(ESAEVUSGestoAtaque Gesto)
{
	FSAEVUSPisadaGesto Pisada;
	switch (Gesto)
	{
	case ESAEVUSGestoAtaque::Reves:
		Pisada.bPieIzquierdoDelante = true;
		break;
	case ESAEVUSGestoAtaque::Remate:
		Pisada.EscalaZancada = 1.3f;
		break;
	case ESAEVUSGestoAtaque::Estocada:
		Pisada.EscalaZancada = 1.5f;
		break;
	case ESAEVUSGestoAtaque::TajoArriba:
		Pisada.EscalaZancada = 0.7f;
		break;
	case ESAEVUSGestoAtaque::Giratorio:
		Pisada.EscalaZancada = 0.8f;
		break;
	default:
		break;
	}
	return Pisada;
}

bool SAEVUSGestos::EsAereo(ESAEVUSGestoAtaque Gesto)
{
	return Gesto == ESAEVUSGestoAtaque::AereoDiagonal || Gesto == ESAEVUSGestoAtaque::AereoReves || Gesto == ESAEVUSGestoAtaque::AereoRemate;
}
