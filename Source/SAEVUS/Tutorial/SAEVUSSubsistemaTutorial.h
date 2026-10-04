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
 * activo. Un mensaje nuevo sustituye al anterior.
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

private:
	FSAEVUSMensajeTutorial Mensaje;
	bool bHayMensaje = false;
	int32 MensajesMostrados = 0;
};
