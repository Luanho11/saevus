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
| `USAEVUSComponenteVida` (`Combate/`) | Vida compartida por jugador y enemigos |
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
| TajoAereo | 0,08 / 0,10 / 0,25 | 12 | ninguna | 0,18 | 0,18 | 0,18 | 0,18 |

Escala de velocidad en el suelo: 0,25 en los tajos, 0 en el remate. Esfera: radio 60 cm a 90 cm (remate 75 cm a 100 cm). Buffer 0,2 s. Valores de primera pasada.

Lectura: los tajos rápidos se pueden dejar en cuanto golpean (fin de la fase activa), así encajan en el movimiento. El remate compromete: no se sale saltando, y el dash solo al final.

Pares de cancelación diseñados: ataque → dash/salto/cambio de arma (tabla), dash → ataque (`bAtaqueTerminaDash`), salto durante un dash de suelo y salto de pared durante un dash (movimiento).

## Armas y cambio

El arsenal es una lista de `USAEVUSDefinicionArma` en el componente de armas. Por defecto (`BP_SAEVUSPersonaje`): 1 Espada, 2 Pistola, 3 Rifle, 4 Escopeta.

- **Cambio.** Elegir una ranura empieza su `TiempoEquipar`; no se dispara ni se ataca hasta que acaba. Cambiar descarta una recarga y un disparo pendiente. A mitad de un ataque obedece `CancelarConCambioArmaDesde`. Equipar un arma cuerpo a cuerpo pasa su repertorio al combate (nulo = el del código).
- **Disparo.** Una pulsación pide un disparo (si llega en el enfriamiento, dispara al acabar); un arma automática sigue mientras se mantiene, a `IntervaloDisparo` (el enfriamiento arrastra como mucho un fotograma, así la cadencia se mantiene a cualquier fps). Cada ranura tiene su cargador; el gatillo en seco recarga. Disparar nunca bloquea el movimiento.
- **Hitscan, dos trazas por perdigón.** (1) Desde la vista (cámara del jugador, ojos en otro caso) para encontrar el punto de mira. (2) Desde la boca (`DesplazamientoBoca` del dueño) hacia ese punto, hasta `Alcance`: la cobertura delante del personaje bloquea aunque la cámara vea más allá. Daño por `ApplyPointDamage`. El cuerpo gira hacia el disparo.
- **Colisión.** Canal de traza propio **Arma** (`ECC_GameTraceChannel1`, `Config/DefaultEngine.ini`, `Nucleo/SAEVUSCanalesColision.h`). Bloquea por defecto, también a los pawns, que ignoran Visibility.
- **Dispersión.** Cada perdigón cae al azar en un cono de `DispersionGrados` (flujo aleatorio propio del componente, listo para sembrar por disparo en red).

| Arma (`DA_Arma_*`) | Tipo | Daño | Perdigones | Intervalo | Modo | Cargador | Recarga | Alcance | Dispersión | Equipar |
|---|---|---|---|---|---|---|---|---|---|---|
| Espada | cuerpo a cuerpo | — | — | — | — | — | — | — | — | 0,2 s |
| Pistola | fuego | 20 | 1 | 0,25 s | semi | 12 | 1,2 s | 50 m | 0,5° | 0,25 s |
| Rifle | fuego | 9 | 1 | 0,1 s | auto | 30 | 1,8 s | 60 m | 1,5° | 0,4 s |
| Escopeta | fuego | 7 | 8 | 0,8 s | semi | 6 | 2,0 s | 20 m | 6° | 0,45 s |

Valores en la tabla `ARMAS` del script de assets y en los Data Assets; arquetipos genéricos. Munición de reserva, recogida de armas e inventario quedan fuera por ahora.

## Vida, muerte y reaparición

- `USAEVUSComponenteVida` escucha el daño del motor del dueño (`OnTakeAnyDamage`), ignora el daño propio (`bIgnorarDanioPropio`). `VidaMaxima` (100 el jugador), `AplicarDanio`, `Curar` (hasta el máximo), `RestablecerVida`; eventos `AlCambiarVida(delta, instigador)` y `AlMorir(asesino)` (una vez). Nada cambia estando muerto.
- **Muerte del jugador** (`ASAEVUSPersonaje::ManejarMuerte`): suelta el gatillo, desactiva el movimiento y la colisión de la cápsula, ragdoll si la malla tiene asset de física, quita la entrada y avisa al modo de juego.
- **Reaparición** (`ASAEVUSModoJuego`): tras `RetardoReaparicion` (3 s) el controlador deja el cuerpo (que queda `DuracionCadaver`, 10 s) y `RestartPlayer` crea un personaje nuevo en un punto de inicio.
- Detalle del motor: `APawn` ignora el daño si el mundo no tiene modo de juego; los tests unitarios usan el componente o un actor simple; la prueba de humo cubre la vía real.

## Red

Todavía no. Combate y armas son locales; estocada y escala de velocidad no van en el movimiento guardado. La fase de red llevará ataques, cambios y disparos a los datos de movimiento predichos (como el dash) y hará que el servidor confirme golpes; la dispersión necesitará una semilla por disparo compartida.

## Depuración

- HUD F6: `Ataque: <nombre> #<índice> <fase> <tiempo>` y `(en buffer)`; `Arma: <nombre> [ranura/total] munición n/m`, `(cambiando a n)`, `(recargando)`; `Vida: n/max`.
- `SAEVUS.Depuracion.Combate 1`: dibuja la esfera de golpe activa y cada traza de disparo (amarillo: sin golpe, rojo: golpe).
- Mapa de pruebas: tres muñecos cerca del inicio y una galería de tiro a 10, 20 y 30 m.

## Tests

`SAEVUS.Combate.*`: `LineaTiempo`, `GolpeaMuneco`, `Combo`, `Cancelaciones`, `Movimiento` (estocada 60 vs 54 cm), `AtaqueAereo`, `AtaqueTerminaDash`.
`SAEVUS.Armas.*`: `Hitscan` (golpe, alcance, cobertura), `ModosDisparo` (semi, 10 disparos/s automático, perdigones), `Recarga`, `Cambio`, `CambioCancela`, `RepertorioMelee`.
`SAEVUS.Vida.*`: `Reglas`, `DanioMotor`, `MuertePersonaje`.

## Pendiente

Animación de ataques y disparos (ver `animacion.md`), reacciones al golpe, parada de golpe, bloqueo, modo de apuntado, munición de reserva.
