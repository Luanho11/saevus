// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSSubsistemaTutorial.h"
#include "Engine/World.h"

void USAEVUSSubsistemaTutorial::Activar(const FSAEVUSMensajeTutorial& Nuevo) const
{
	Mensaje = Nuevo;
	Mensaje.Inicio = GetWorld()->GetTimeSeconds();
	bHayMensaje = true;
	++MensajesMostrados;
}

bool USAEVUSSubsistemaTutorial::HayMensajeLegible() const
{
	// Hay un mensaje en pantalla que aún no ha tenido su tiempo de lectura.
	if (!bHayMensaje)
	{
		return false;
	}
	const float Edad = ObtenerEdadMensaje();
	const bool bCaducado = Mensaje.Duracion > 0.f && Edad > Mensaje.Duracion;
	return !bCaducado && Edad < DuracionMinima;
}

void USAEVUSSubsistemaTutorial::MostrarMensaje(const FText& Titulo, const FText& Texto, float Duracion)
{
	FSAEVUSMensajeTutorial Nuevo;
	Nuevo.Titulo = Titulo;
	Nuevo.Texto = Texto;
	Nuevo.Duracion = Duracion;
	if (!HayMensajeLegible())
	{
		Cola.Reset();
		Activar(Nuevo);
		return;
	}
	// Se está leyendo otro: este espera (sin repetirse ni pasar del tope).
	const bool bRepetido = Cola.ContainsByPredicate([&](const FSAEVUSMensajeTutorial& Esperando) { return Esperando.Titulo.EqualTo(Titulo) && Esperando.Texto.EqualTo(Texto); });
	if (!bRepetido && Cola.Num() < MaximoEnCola)
	{
		Cola.Add(Nuevo);
	}
}

const FSAEVUSMensajeTutorial* USAEVUSSubsistemaTutorial::ObtenerMensajeActivo() const
{
	// Con el actual leído (o caducado) y otro esperando, le toca al siguiente.
	if (!Cola.IsEmpty() && !HayMensajeLegible())
	{
		const FSAEVUSMensajeTutorial Siguiente = Cola[0];
		Cola.RemoveAt(0);
		Activar(Siguiente);
	}
	if (!bHayMensaje)
	{
		return nullptr;
	}
	const bool bCaducado = Mensaje.Duracion > 0.f && ObtenerEdadMensaje() > Mensaje.Duracion;
	return bCaducado ? nullptr : &Mensaje;
}

float USAEVUSSubsistemaTutorial::ObtenerEdadMensaje() const
{
	return bHayMensaje ? static_cast<float>(GetWorld()->GetTimeSeconds() - Mensaje.Inicio) : 0.f;
}
