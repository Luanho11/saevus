// Copyright SAEVUS. Todos los derechos reservados.

#pragma once

#include "Armas/SAEVUSTiposArma.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "SAEVUSComponenteArmas.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSAEVUSFirmaDisparo, FVector, Boca, FVector, Impacto, AActor*, ActorGolpeado);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSAEVUSFirmaGatilloEnSeco);

/**
 * Arsenal, cambio de arma y armas de fuego. Las armas cuerpo a cuerpo pasan su repertorio a
 * USAEVUSComponenteCombate; las de fuego disparan hitscan aquí. Cada ranura tiene su propio
 * cargador y su reserva; la reserva se rellena con golpes cuerpo a cuerpo y en los puntos de
 * control. Ver Documentos/combate.md.
 */
UCLASS(ClassGroup = (SAEVUS), meta = (BlueprintSpawnableComponent))
class SAEVUS_API USAEVUSComponenteArmas : public UActorComponent
{
	GENERATED_BODY()

public:
	USAEVUSComponenteArmas();

	/** Sustituye el arsenal y equipa RanuraInicio en el acto (sin tiempo de equipado). Llena cargadores y reservas. */
	void FijarArsenal(const TArray<const USAEVUSDefinicionArma*>& NuevoArsenal, int32 RanuraInicio = 0);

	int32 ObtenerNumeroRanuras() const { return Arsenal.Num(); }
	int32 ObtenerRanuraActual() const { return RanuraActual; }

	/** Arma equipada; nula con el arsenal vacío o mientras se equipa la primera. */
	const USAEVUSDefinicionArma* ObtenerArmaActual() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Armas")
	bool TieneArmaFuegoEquipada() const;

	/** Falso si Ranura no es válida, ya está equipada (sin cambio en curso) o ya se está cambiando a ella. */
	bool PuedeCambiarA(int32 Ranura) const;

	/** Empieza el cambio a Ranura; falso si PuedeCambiarA dice que no. */
	bool PedirCambio(int32 Ranura);

	/** Ranura siguiente a la equipada (o pendiente), dando la vuelta; INDEX_NONE con menos de dos armas. */
	int32 ObtenerSiguienteRanura() const;

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Armas")
	bool EstaCambiando() const { return TiempoCambioRestante > 0.f; }

	/** Ranura a la que se está cambiando, o INDEX_NONE. */
	int32 ObtenerRanuraPendiente() const { return RanuraPendiente; }

	/** Pulsar (cierto) y soltar (falso) el gatillo. Una pulsación siempre pide un disparo. */
	void FijarGatillo(bool bPresionado);

	/** Recarga el arma de fuego equipada si su cargador no está lleno. */
	void PedirRecarga();

	UFUNCTION(BlueprintPure, Category = "SAEVUS|Armas")
	bool EstaRecargando() const { return TiempoRecargaRestante > 0.f; }

	/** Balas que quedan en el cargador de Ranura (0 para cuerpo a cuerpo o ranuras no válidas). */
	int32 ObtenerMunicion(int32 Ranura) const;
	int32 ObtenerMunicionActual() const { return ObtenerMunicion(RanuraActual); }

	/** Balas de reserva de Ranura; INDEX_NONE si es infinita, 0 para cuerpo a cuerpo o ranuras no válidas. */
	int32 ObtenerReserva(int32 Ranura) const;
	int32 ObtenerReservaActual() const { return ObtenerReserva(RanuraActual); }

	/** Llena cargadores y reservas de todas las armas de fuego (puntos de control). */
	void RellenarMunicion();

	/** Disparos hechos desde que apareció; para depuración y tests. */
	int32 ObtenerDisparosRealizados() const { return DisparosRealizados; }

	/** Se dispara una vez por cada traza de perdigón. */
	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Armas")
	FSAEVUSFirmaDisparo AlDisparar;

	/** Gatillo con el cargador y la reserva vacíos. */
	UPROPERTY(BlueprintAssignable, Category = "SAEVUS|Armas")
	FSAEVUSFirmaGatilloEnSeco AlGatilloEnSeco;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	/** Armas por ranura (teclas 1, 2, 3...). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Armas")
	TArray<TObjectPtr<const USAEVUSDefinicionArma>> Arsenal;

	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Armas", meta = (ClampMin = "0"))
	int32 RanuraInicial = 0;

	/** Desde dónde salen los disparos, relativo al dueño (espacio del actor). */
	UPROPERTY(EditDefaultsOnly, Category = "SAEVUS|Armas")
	FVector DesplazamientoBoca = FVector(0.f, 0.f, 50.f);

private:
	/** Un golpe cuerpo a cuerpo acertado devuelve balas a la reserva de cada arma de fuego. */
	UFUNCTION()
	void AlGolpearCuerpoACuerpo(AActor* ActorGolpeado, float Danio);
	/** Pasa balas de la reserva al cargador de la ranura equipada. */
	void CompletarRecarga();
	/** Hace de Ranura el arma equipada; las cuerpo a cuerpo pasan su repertorio al combate. */
	void Equipar(int32 Ranura);
	void IntentarDisparar(float DeltaTime);
	void Disparar(const FSAEVUSAjusteArmaFuego& Ajuste);
	/** Vista de cámara para jugadores; ojos en otro caso (también en los tests, sin controlador). */
	void ObtenerPuntoVista(FVector& OutPosicion, FRotator& OutRotacion) const;

	TArray<int32> Cargadores;
	/** Reserva por ranura; INDEX_NONE = infinita. */
	TArray<int32> Reservas;
	int32 RanuraActual = INDEX_NONE;
	int32 RanuraPendiente = INDEX_NONE;
	float TiempoCambioRestante = 0.f;
	float TiempoRecargaRestante = 0.f;
	/** Tiempo hasta el siguiente disparo; puede bajar de cero hasta un fotograma para mantener la cadencia automática. */
	float EnfriamientoDisparo = 0.f;
	bool bGatilloPresionado = false;
	bool bDisparoPedido = false;
	int32 DisparosRealizados = 0;
	FRandomStream FlujoDispersion;
};
