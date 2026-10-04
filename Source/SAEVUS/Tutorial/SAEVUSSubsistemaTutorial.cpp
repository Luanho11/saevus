// Copyright SAEVUS. Todos los derechos reservados.

#include "Tutorial/SAEVUSSubsistemaTutorial.h"
#include "Engine/World.h"

void USAEVUSSubsistemaTutorial::MostrarMensaje(const FText& Titulo, const FText& Texto, float Duracion)
{
	Mensaje.Titulo = Titulo;
	Mensaje.Texto = Texto;
	Mensaje.Inicio = GetWorld()->GetTimeSeconds();
	Mensaje.Duracion = Duracion;
	bHayMensaje = true;
	++MensajesMostrados;
}

const FSAEVUSMensajeTutorial* USAEVUSSubsistemaTutorial::ObtenerMensajeActivo() const
{
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
