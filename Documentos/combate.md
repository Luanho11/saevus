# Combate de SAEVUS

Estado: base cuerpo a cuerpo, armas de fuego, cambio de arma, vida, muerte y reaparición hechos; faltan animación de ataques, ajuste por sensación y modelos de arma.

Objetivos de diseño (`investigacion/gunz.md`): las cancelaciones son un **sistema diseñado y guiado por datos**, cada acción base es una sola entrada, y movimiento y cuerpo a cuerpo se encajan.

## Piezas

| Pieza | Papel |
|---|---|
| `USAEVUSComponenteCombate` (`Combate/`) | Ejecuta líneas de tiempo de ataque, guarda pulsaciones en buffer, encadena combos, detecta golpes, responde "¿puede esta acción de movimiento cancelar el ataque ahora?" |
| `FSAEVUSDefinicionAtaque` (`Combate/SAEVUSTiposCombate.h`) | Un ataque: fases, daño, esfera de golpe, estocada, escala de velocidad en el suelo, punto de encadenado, ventanas de cancelación |
| `FSAEVUSRepertorioMelee` / `USAEVUSDatosRepertorioMelee` | Cadena de suelo, cadena aérea, buffer de entrada, relación con el dash. El constructor del struct tiene el repertorio por defecto; `DA_RepertorioMelee_Base` lo sobrescribe |
| `ASAEVUSMunecoEntrenamiento` (`Mundo/`) | Blanco del tamaño de un pawn que cuenta golpes y daño y los muestra encima. No es un enemigo |
| Ganchos de `USAEVUSComponenteMovimiento` | `IniciarEstocada`, `FijarEscalaVelocidadAccion`, `GirarHacia`, `CancelarDash` |
| `USAEVUSComponenteArmas` (`Armas/`) | Arsenal por ranuras, cambio con tiempo de equipado, cargador por ranura, gatillo, recarga, disparos hitscan |
| `USAEVUSDefinicionArma` (`Armas/SAEVUSTiposArma.h`) | Un arma: cuerpo a cuerpo (con su repertorio) o de fuego (`FSAEVUSAjusteArmaFuego`), más tiempo de equipado |
| `USAEVUSComponenteVida` (`Combate/`) | Vida compartida por jugador y enemigos; invulnerabilidad tras golpe opcional |
| `USAEVUSComponenteAlma` (`Combate/`) | Alma ganada al golpear y curación mantenida (Hollow Knight) |
| `USAEVUSComponenteEfectosCombate` (`Efectos/`) | Chispas al golpear, partículas que convergen al curar, destello al completar |
| `ASAEVUSPersonaje` | Reparte la entrada; pregunta al combate antes de un dash, salto o cambio de arma |

Entrada: `IA_Atacar` (clic izquierdo / botón izquierdo del mando) es la acción principal: ataque cuerpo a cuerpo, o gatillo del arma de fuego (mantener para fuego automático). `IA_Recargar` (R / botón superior), `IA_SiguienteArma` (rueda / hombro derecho), `IA_RanuraArma1..4` (1–4).

## Línea de tiempo de un ataque

La línea de tiempo es el ataque; la animación se deriva de ella (ver `animacion.md`). Tiempos en segundos desde el inicio:

```
0 ── Preparación ──┬── Activa ──┬──────── Recuperación ────────┐ total
                   │ esfera de  │                              │
                   │ golpe viva │  EncadenarDesde ──► el siguiente ataque puede empezar
 estocada ──────►  (DuracionEstocada)
 CancelarConDashDesde / CancelarConSaltoDesde / CancelarConCambioArmaDesde ──► esa acción puede cortar el ataque
```

- **Inicio.** Una pulsación se guarda `TiempoBufferEntrada` y empieza en cuanto se puede: en el suelo o en el aire (no en paredes). Durante un dash, termina el dash si `bAtaqueTerminaDash`. Cayendo, empieza la cadena aérea; si no, la de suelo.
- **Orientación y estocada.** El ataque mira hacia la entrada de movimiento (o de frente sin entrada), gira el cuerpo y empieza la estocada: velocidad horizontal `VelocidadEstocada` durante `DuracionEstocada`. Es el gancho de "movimiento guiado por la espada".
- **Movimiento comprometido.** Mientras dura, el tope de velocidad en el suelo se multiplica por `EscalaVelocidadSuelo` (la estocada no se limita). El impulso aéreo no cambia.
- **Golpes.** En la fase activa una esfera (`RadioGolpe`, a `DesplazamientoFrontalGolpe` delante y `AlturaGolpe` arriba) solapa pawns, cuerpos físicos y objetos dinámicos. Cada actor recibe un golpe por ataque, por la vía de daño del motor (`UGameplayStatics::ApplyDamage` → `TakeDamage`), y se dispara `AlGolpear` (para parada de golpe, efectos, sonido).
- **Cadena.** Una pulsación en buffer empieza el siguiente ataque de la misma cadena cuando el actual llega a `EncadenarDesde`. Tras el último, la cadena termina.
- **Fin.** Al acabar la recuperación, al cancelarse, o cuando el movimiento en pared se queda con el cuerpo.

## Reglas de cancelación

Cada ataque declara, por acción, desde cuándo puede cortarse (negativo = nunca). El personaje pregunta `IntentarCancelarPor` antes de pasar un dash, salto o cambio de arma; fuera de la ventana, la pulsación se descarta.

| Ataque | Preparación / Activa / Recuperación (s) | Daño | Estocada | Encadenar desde | Dash desde | Salto desde | Cambio desde |
|---|---|---|---|---|---|---|---|
| Tajo1 | 0,10 / 0,08 / 0,30 | 10 | 450 cm/s × 0,12 s | 0,18 | 0,18 | 0,18 | 0,18 |
| Tajo2 | 0,10 / 0,08 / 0,30 | 12 | 450 cm/s × 0,12 s | 0,18 | 0,18 | 0,18 | 0,18 |
| Remate | 0,18 / 0,12 / 0,45 | 25 | 650 cm/s × 0,18 s | (último) | 0,40 | nunca | 0,40 |
| TajoAereo, TajoAereo2 | 0,08 / 0,10 / 0,25 | 12 | ninguna; flota (gravedad ×0,35, conserva 25 % de la caída) | 0,18 | 0,18 | 0,18 | 0,18 |
| TajoEnCarrera (atacar en un dash) | 0,05 / 0,14 / 0,30 | 16 | 1100 cm/s × 0,16 s en la dirección del dash | (no encadena) | 0,19 | 0,19 | 0,19 |
| AtaqueCargado (mantener 0,8 s y soltar) | 0,10 / 0,25 / 0,40 | 30 | ninguna; giro completo, esfera de 170 cm alrededor | (no encadena) | 0,35 | nunca | 0,35 |
| RemateAereo | 0,08 / 0,10 / 0,35 | 18 | ninguna; ya no flota | (último) | 0,18 | 0,18 | 0,18 |

Escala de velocidad en el suelo: 0,25 en los tajos, 0 en el remate. Esfera: radio 60 cm a 90 cm (remate 75 cm a 100 cm). Buffer 0,2 s. Valores de primera pasada.

Lectura: los tajos rápidos se pueden dejar en cuanto golpean (fin de la fase activa), así encajan en el movimiento. El remate compromete: no se sale saltando, y el dash solo al final.

Pares de cancelación diseñados: ataque → dash/salto/cambio de arma (tabla), dash → ataque (`bAtaqueTerminaDash`), salto durante un dash de suelo y salto de pared durante un dash (movimiento).

## Tajos direccionales, rebote e impacto (Hollow Knight)

Mecánicas de Hollow Knight adaptadas a 3D, con números propios:

- **Dirección por la cámara.** En 2D se apunta con arriba/abajo; en tercera persona, W y S ya mueven. `ASAEVUSPersonaje::PedirAtaque` lee la inclinación vertical de la cámara: desde `UmbralInclinacionArriba` (30°) sale `TajoArriba`; hasta `UmbralInclinacionAbajo` (−40°), `TajoAbajo`. Abajo solo existe en el aire (en el suelo es el combo frontal). Umbrales en el repertorio; cambiables.
- **Tajos direccionales** (`FSAEVUSRepertorioMelee::TajoArriba` / `TajoAbajo`): 0,08 / 0,10 / 0,25 s, 12 de daño, esfera de 70 cm a ±130 cm de altura, sin estocada. No encadenan: salen desde la ventana de encadenar de cualquier ataque, y tras ellos vuelve a empezar el combo.
- **Golpear sin daño:** lo que no recibe daño (pinchos, `ASAEVUSPeligro`) cuenta para el impacto y el rebote, pero no es un golpe: no emite `AlGolpear`, así no da alma ni chispas (como en Hollow Knight).
- **Rebote (pogo)**: `VelocidadRebote` (750 cm/s en el tajo hacia abajo). Al acertar en el aire, `USAEVUSComponenteMovimiento::Rebotar` fija la velocidad vertical, corta dash y estocada y **recarga dash aéreo y doble salto**: se puede encadenar rebote → doble salto → dash → rebote.
- **Impacto** (una vez por ataque, en el primer acierto):
  - `RetrocesoAtacante` (250 cm/s; 0 en los tajos direccionales): corta la estocada y echa al atacante hacia atrás, así el golpe se siente y no se atraviesa al enemigo.
  - `EmpujeObjetivo` (350 cm/s): lanza a los personajes golpeados en la dirección del tajo (horizontal, arriba o abajo).
  - **Parada de golpe** `DuracionParadaGolpe` (0,06 s reales) a la escala `EscalaTiempoParada` (0,05) del repertorio: `FSAEVUSParadaGolpe` baja `CustomTimeDilation` del atacante y de los golpeados y la devuelve a 1, contando tiempo real.
- Animación: trayectorias propias para arriba (cuerpo echado atrás) y abajo (volcado hacia el golpe).

## Guardia (GunZ)

La otra mitad de la "mariposa" de GunZ (salto → dash → tajo → guardia), diseñada como cancelación por datos:

- **Mantener `IA_Guardia`** (clic derecho / gatillo izquierdo) con el arma cuerpo a cuerpo (`USAEVUSComponenteGuardia`, `FSAEVUSAjusteGuardia`). Bloquea el daño de causantes dentro de un cono frontal de ±70° (`FraccionBloqueada` 1: todo). No cubre la espalda ni el daño sin causante (caídas).
- El bloqueo pasa por el **filtro de daño** de la vida (`USAEVUSComponenteVida::FiltroDanio`): un golpe parado no da invulnerabilidad ni corta nada.
- **Reacción** (como mucho cada 0,2 s): el jugador retrocede (300 cm/s), el atacante sale despedido (450 cm/s, escalado por su `EscalaEmpujeRecibido`), chispas y un choque metálico.
- **Cancelación:** nueva acción `ESAEVUSAccionCancelacion::Guardia` con `CancelarConGuardiaDesde` en cada ataque (0,18 s en los tajos, 0,4 s en el remate, el final de la fase activa en los aéreos y direccionales). Pedida durante un tajo, la guardia entra en cuanto la ventana lo permite y corta el ataque. Atacar desde la guardia la suelta; si sigue pulsada, vuelve en la siguiente ventana: tajo → guardia → tajo.
- **Contraataque (parry):** un golpe bloqueado en los primeros 0,15 s de la guardia (`VentanaContraataque`) aturde al atacante (los jefes no se aturden), da 22 de alma y 0,3 s de invulnerabilidad, con una campanada propia. Premia levantar la guardia en el momento justo en vez de mantenerla siempre.
- En guardia: velocidad ×0,35, el cuerpo mira hacia la cámara, no se cura (pulsar guardia corta la curación). Con un arma de fuego o durante un dash no hay guardia.
- Animación: brazos al frente y el aguijón cruzado en diagonal.
- `DA_RepertorioMelee_Base` se reescribe con el repertorio del código en cada ejecución del script de assets (mientras no se ajuste a mano), así recibe los campos nuevos con sus valores de diseño.

## Sacudida de cámara

`USAEVUSComponenteCamara` escucha a su dueño y suma "trauma" (0..1) en cada impacto: golpe dado 0,3, golpe recibido 0,65, bloqueo 0,4. La sacudida es el trauma al cuadrado (los golpes pequeños apenas mueven, los grandes sacuden), con ruido Perlin en desplazamiento (máx. 10 cm) y giro (máx. 2°), y se calma a 2,5 por segundo. Todo en `FSAEVUSAjusteCamara` (`bSacudida` la desactiva). Test `SAEVUS.Estilo.SacudidaCamara`.

## Alma y curación (Hollow Knight)

- Cada actor golpeado por un ataque cuerpo a cuerpo da `AlmaPorGolpe` (11) hasta `AlmaMaxima` (99) (`FSAEVUSAjusteAlma`).
- **Curar**: mantener `IA_Curar` (Q / hombro izquierdo del mando) en el suelo, sin atacar ni hacer dash, con al menos `CosteCuracion` (33) de alma y vida por debajo del máximo. Tras `DuracionCuracion` (1 s) recupera `VidaPorCuracion` (25) y gasta el coste; mantenida, encadena otra mientras quede alma.
- Mientras cura el personaje no avanza (escala de velocidad 0). Un golpe recibido, saltar, atacar o hacer dash la **cortan sin gastar alma**, y hay que volver a pulsar. En el aire o sin alma solo se pausa: mantenida, empieza en cuanto se pueda.
- Animación: agachado con las manos juntas y la cabeza baja; partículas de alma que convergen hacia el cuerpo y un destello al completar.

## Golpe recibido

- `USAEVUSComponenteVida::DuracionInvulnerableTrasGolpe`: tras un golpe que no mata, ignora el daño ese tiempo (1 s el jugador, constante del personaje; 0 los enemigos y muñecos). El cuerpo parpadea (`FrecuenciaParpadeo`, 12 Hz) mientras dura; la invulnerabilidad del dash no parpadea.
- Pendiente con los enemigos: retroceso del jugador al recibir un golpe.

## Armas y cambio

El arsenal es una lista de `USAEVUSDefinicionArma` en el componente de armas. Por defecto (`BP_SAEVUSPersonaje`): 1 Espada, 2 Pistola, 3 Rifle, 4 Escopeta.

- **Cambio.** Elegir una ranura empieza su `TiempoEquipar`; no se dispara ni se ataca hasta que acaba. Cambiar descarta una recarga y un disparo pendiente. A mitad de un ataque obedece `CancelarConCambioArmaDesde`. Equipar un arma cuerpo a cuerpo pasa su repertorio al combate (nulo = el del código).
- **Disparo.** Una pulsación pide un disparo (si llega en el enfriamiento, dispara al acabar); un arma automática sigue mientras se mantiene, a `IntervaloDisparo` (el enfriamiento arrastra como mucho un fotograma, así la cadencia se mantiene a cualquier fps). Cada ranura tiene su cargador; el gatillo en seco recarga. Disparar nunca bloquea el movimiento.
- **Munición de reserva.** La recarga pasa balas de la reserva (`MunicionReserva`, negativo = infinita) al cargador; con poca reserva el cargador se llena en parte, y sin reserva el gatillo en seco solo suena (`AlGatilloEnSeco`). Cada golpe cuerpo a cuerpo que acierta devuelve `ReservaPorGolpeCuerpoACuerpo` balas a la reserva de cada arma de fuego, hasta su máximo: la espada alimenta las armas (GunZ), así las armas no superan al combo sin riesgo (`equilibrio.md`). Los puntos de control y la reaparición lo rellenan todo.
- **Hitscan, dos trazas por perdigón.** (1) Desde la vista (cámara del jugador, ojos en otro caso) para encontrar el punto de mira. (2) Desde la boca (`DesplazamientoBoca` del dueño) hacia ese punto, hasta `Alcance`: la cobertura delante del personaje bloquea aunque la cámara vea más allá. Daño por `ApplyPointDamage`. El cuerpo gira hacia el disparo.
- **Colisión.** Canal de traza propio **Arma** (`ECC_GameTraceChannel1`, `Config/DefaultEngine.ini`, `Nucleo/SAEVUSCanalesColision.h`). Bloquea por defecto, también a los pawns, que ignoran Visibility.
- **Dispersión.** Cada perdigón cae al azar en un cono de `DispersionGrados` (flujo aleatorio propio del componente, listo para sembrar por disparo en red).

| Arma (`DA_Arma_*`) | Tipo | Daño | Perdigones | Intervalo | Modo | Cargador | Reserva (por golpe) | Recarga | Alcance | Dispersión | Equipar |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Espada | cuerpo a cuerpo | — | — | — | — | — | — | — | — | — | 0,2 s |
| Pistola | fuego | 20 | 1 | 0,25 s | semi | 12 | 24 (+2) | 1,2 s | 50 m | 0,5° | 0,25 s |
| Rifle | fuego | 9 | 1 | 0,1 s | auto | 30 | 60 (+5) | 1,8 s | 60 m | 1,5° | 0,4 s |
| Escopeta | fuego | 7 | 8 | 0,8 s | semi | 6 | 12 (+1) | 2,0 s | 20 m | 6° | 0,45 s |

Valores en la tabla `ARMAS` del script de assets y en los Data Assets; arquetipos genéricos. Recogida de armas e inventario quedan fuera por ahora.

## Vida, muerte y reaparición

- `USAEVUSComponenteVida` escucha el daño del motor del dueño (`OnTakeAnyDamage`), ignora el daño propio (`bIgnorarDanioPropio`). `VidaMaxima` (100 el jugador), `AplicarDanio`, `Curar` (hasta el máximo), `RestablecerVida`; eventos `AlCambiarVida(delta, instigador)` y `AlMorir(asesino)` (una vez). Nada cambia estando muerto.
- **Muerte del jugador** (`ASAEVUSPersonaje::ManejarMuerte`): suelta el gatillo, desactiva el movimiento y la colisión de la cápsula, ragdoll si la malla tiene asset de física, quita la entrada y avisa al modo de juego.
- **Reaparición** (`ASAEVUSModoJuego`): tras `RetardoReaparicion` (3 s) el controlador deja el cuerpo (que queda `DuracionCadaver`, 10 s) y `RestartPlayer` crea un personaje nuevo en un punto de inicio.
- Detalle del motor: `APawn` ignora el daño si el mundo no tiene modo de juego; los tests unitarios usan el componente o un actor simple; la prueba de humo cubre la vía real.

## Red

Todavía no. Combate y armas son locales (la parada de golpe del atacante dilata su simulación de movimiento: en red tendrá que ser solo visual); estocada y escala de velocidad no van en el movimiento guardado. La fase de red llevará ataques, cambios y disparos a los datos de movimiento predichos (como el dash) y hará que el servidor confirme golpes; la dispersión necesitará una semilla por disparo compartida.

## Depuración

- HUD F6: `Ataque: <nombre> #<índice> <fase> <tiempo>` y `(en buffer)`; `Arma: <nombre> [ranura/total] munición n/m reserva r` (−1 = infinita), `(cambiando a n)`, `(recargando)`; `Vida: n/max`.
- `SAEVUS.Depuracion.Combate 1`: dibuja la esfera de golpe activa y cada traza de disparo (amarillo: sin golpe, rojo: golpe).
- `SAEVUS.Depuracion.NumerosDanio 1`: números de daño flotantes sobre quien recibe el golpe, que suben y se desvanecen en 0,9 s (claros: enemigos y muñecos; rojos: el jugador). La vida y los muñecos los apuntan en `USAEVUSSubsistemaNumerosDanio`, y el HUD los dibuja. Apagado no cuesta nada. Test `SAEVUS.Depuracion.NumerosDanio`.
- Mapa de pruebas: tres muñecos cerca del inicio y una galería de tiro a 10, 20 y 30 m.

## Tests

`SAEVUS.Combate.*`: `LineaTiempo`, `GolpeaMuneco`, `Combo`, `Cancelaciones`, `Movimiento` (estocada 60 vs 54 cm), `AtaqueAereo`, `AtaqueTerminaDash`, `Tajos.Inclinacion` (cámara → dirección; abajo en el suelo es frontal), `Tajos.Arriba` (solo el tajo hacia arriba alcanza lo de encima), `Tajos.AbajoRebote` (rebote y recarga de dash y doble salto), `Tajos.RetrocesoYParada`.
`SAEVUS.Armas.*`: `Hitscan` (golpe, alcance, cobertura), `ModosDisparo` (semi, 10 disparos/s automático, perdigones), `Recarga`, `Reserva` (recarga parcial, sin reserva no recarga y suena en seco, golpes que devuelven balas hasta el máximo, rellenar), `Cambio`, `CambioCancela`, `RepertorioMelee`.
`SAEVUS.Vida.*`: `Reglas`, `DanioMotor`, `MuertePersonaje`, `InvulnerableTrasGolpe`.
`SAEVUS.Guardia.*`: `Contraataque` (dentro de la ventana aturde, da alma e invulnerabilidad; fuera, bloqueo normal), `BloqueaDeFrente` (frente sí; espalda y sin causante no), `MovimientoYArma` (más lenta, sin curación, se suelta con arma de fuego), `CancelaTajo` (espera la ventana, corta el tajo, atacar la suelta y vuelve).
`SAEVUS.Alma.*`: `GanarAlGolpear`, `Curacion` (no se mueve, cura y gasta), `CuracionInterrumpida` (golpe y salto la cortan sin gastar), `SinAlmaNoCura`.

## Pendiente

Reacciones al golpe en enemigos (llegan con los enemigos), rebote en pinchos y peligros, bloqueo, modo de apuntado, munición de reserva.
