// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SAEVUSSubsistemaTutorial.generated.h"

/** Un aviso de tutorial en pantalla. */
USTRUCT(BlueprintType)
struct FSAEVUSMensajeTutorial
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	FText Titulo;

	UPROPERTY(BlueprintReadOnly, Category = "Tutorial")
	FText Texto;

	/** Tiempo de mundo en que apareció y cuánto dura (0 = hasta el siguiente). */
	double Inicio = 0.0;
	float Duracion = 0.f;
};

/**
 * Avisos del tutorial: los volúmenes y las puertas publican un mensaje; la interfaz muestra el
 * activo. Un mensaje nuevo sustituye al anterior solo si este ya se pudo leer (DuracionMinima); si no, espera
 * en una cola corta, así recoger un fragmento o sentarse no borra a mitad de lectura la instrucción (auditoría GAMEPLAY-23).
 */
UCLASS()
class SAEVUS_API USAEVUSSubsistemaTutorial : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void MostrarMensaje(const FText& Titulo, const FText& Texto, float Duracion);

	/** El mensaje en pantalla, o nulo si no hay o ya caducó. */
	const FSAEVUSMensajeTutorial* ObtenerMensajeActivo() const;

	/** Segundos desde que apareció el mensaje activo (para fundidos). */
	float ObtenerEdadMensaje() const;

	/** Cuántos mensajes se han mostrado (tests y depuración). */
	int32 ObtenerMensajesMostrados() const { return MensajesMostrados; }

	/** Quita el mensaje en pantalla y la cola (el recorrido automático salta de un aviso a otro sin leerlos). */
	void LimpiarMensajes();

	/** Mensajes esperando su turno. */
	int32 ObtenerMensajesEnCola() const { return Cola.Num(); }

	/** Segundos que un mensaje se queda en pantalla antes de dejar su sitio a otro. */
	static constexpr float DuracionMinima = 2.5f;
	static constexpr int32 MaximoEnCola = 4;

private:
	void Activar(const FSAEVUSMensajeTutorial& Nuevo) const;
	bool HayMensajeLegible() const;

	// El avance de la cola ocurre al consultar (la interfaz pregunta cada fotograma): por eso son mutables.
	mutable FSAEVUSMensajeTutorial Mensaje;
	mutable bool bHayMensaje = false;
	mutable TArray<FSAEVUSMensajeTutorial> Cola;
	mutable int32 MensajesMostrados = 0;
};
