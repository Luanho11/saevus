# Investigación de GunZ → objetivos de diseño de SAEVUS

Estado: hecho a partir de documentación pública de la comunidad, con las respuestas del dueño (2026-10-03).

## Reglas de este documento

- Describe lo que *hacen* los jugadores de GunZ y por qué se siente bien, con palabras sencillas. Sin código, fórmulas, datos de fotogramas, tiempos, assets ni animaciones de GunZ; no hacen falta.
- Los valores de SAEVUS se diseñan y ajustan desde cero en sus propios structs.
- Los nombres de técnicas de la comunidad (K-style, butterfly...) solo identifican de qué se habla.

## Fuentes

- Wikipedia, *GunZ: The Duel* — resumen de juego (carreras por pared, volteretas, esquivas aéreas; K-style como cancelaciones de animación y cambio de arma; modo Quest con jefes).
- StrategyWiki, *GunZ The Duel / Controls* y */Korean-Style and Advanced Techniques* (fragmentos de búsqueda).
- Wiki de GunZ en Fandom, *K-Style* (fragmentos de búsqueda).
- critpoints.net, *MTM: Gunz – Legendkiller 2* (2016) — análisis de diseño del K-style.
- MMOBomb, *GunZ 2: Some Answers and Trailer Revealed* — MAIET anunció que el K-style no volvería en GunZ 2.

## Qué hacen los jugadores de GunZ

### Acciones base (diseñadas)

| Acción | Qué hace el jugador | Qué da |
|---|---|---|
| Voltereta / dash | Doble toque de una dirección | Ráfaga de velocidad; un estado "bloqueado" en el que cámara, cambio de arma y bloqueo están limitados |
| Carrera por pared | Llegar en diagonal a una pared y saltar | Correr por la pared un tiempo y saltar |
| Escalada de pared | Correr de frente a una pared y saltar | Subir por la pared y acabar con una voltereta hacia atrás |
| Salto de pared | Saltar desde una pared | Redirige al personaje en el aire |
| Guardia (espada) | Mantener bloqueo | Desvía balas y golpes; no cubre todo el cuerpo |
| Tajo | Atacar con arma cuerpo a cuerpo | Daño; también la llave de las cancelaciones |
| Estocada / ataque pesado | Ataque especial | Ataque que desplaza al personaje |
| Cambio de arma | Teclas numéricas | Cambiar entre armas de fuego y cuerpo a cuerpo al instante |

### Técnicas emergentes (no diseñadas: las encontraron los jugadores)

La identidad real del juego vino de las cancelaciones: el tajo interrumpía muchas otras animaciones y reiniciaba sus contadores.

- **El tajo cancela estados bloqueados** (dash, salto de pared): devuelve cámara, cambio de arma y bloqueo.
- **Bloquear o cambiar de arma cancela el tajo**, así tajo → bloqueo es casi gratis.
- **Saltar cancela la recuperación del dash**, así los dashes se encadenan más rápido.
- **Cadenas** (salto → dash → tajo → bloqueo, "butterfly"): mantienen al jugador moviéndose, esquivo y peligroso; los expertos se quedaban en el aire por todo el mapa.
- **Cancelación en pared:** tajo durante la voltereta de la escalada para subir paredes más rápido.

### Por qué se siente bien

- **Siempre en movimiento.** El principio citado en todas partes: no quedarse quieto. El movimiento *es* la defensa.
- **Duelos en tres dimensiones.** Se pelea en paredes y en el aire, no solo en el suelo.
- **Expresión por secuencias.** La maestría es sobre todo secuenciar entradas con ritmo, un "baile".
- **Armas de fuego y espada encajan.** Cambiar de arma es parte del movimiento.

### Qué salió mal

- **La profundidad era un fallo.** Sin cancelaciones el juego era lento y tedioso; con ellas, era su atractivo principal.
- **Curva de aprendizaje brutal.** Dobles toques, teclas numéricas, botones del ratón y salto a la vez.
- **Frágil.** Cualquier cambio de animación o equilibrio podía romper el estilo de juego.
- **Difícil de leer** para espectadores y novatos.
- **GunZ 2 quitó el K-style** por sus "problemas derivados"; los veteranos protestaron.

## Objetivos de diseño de SAEVUS

1. **Movimiento primero, quedarse quieto es un riesgo.**
2. **Las cancelaciones son un sistema diseñado, no un fallo.** Cada acción declara cuándo y en qué se puede cancelar, como datos; las cancelaciones son técnicas, con animaciones fluidas (decisión del dueño).
3. **Suelo bajo, techo alto.** Cada acción base es una entrada; la profundidad viene del ritmo y el encadenado.
4. **Las paredes son parte del escenario:** carrera, escalada y salto de pared son modos de primera clase.
5. **El aire es un espacio de juego, con límites que elegimos** (presupuestos aéreos que se recargan).
6. **Cuerpo a cuerpo y armas de fuego encajan**, con el cambio de arma en la misma tabla de cancelaciones.
7. **Legible** para el jugador y el espectador (telegrafiado, siluetas, efectos).
8. **Preparado para red.**
9. **Enseñarlo:** un mapa tutorial muestra las técnicas a propósito.

## Respuestas del dueño (2026-10-03)

| Pregunta | Respuesta | Cómo se aplica |
|---|---|---|
| 1. Técnicas usadas y queridas | Personaje animado en 3D (tipo Genshin); caballero humanoide boceto estilo Hollow Knight, generado | Ver `animacion.md` y `pipeline-assets.md` |
| 2. Dash: tecla o doble toque | **Doble toque** | Doble toque de WASD en teclado; botón en mando |
| 3. ¿Aire ilimitado? | (sin respuesta directa) | Presupuestos aéreos recargables, ajustables por datos |
| 4. ¿Cámara bloqueada en el dash? | **Como en GunZ** | La cámara no gira durante el dash (`bBloquearCamaraEnDash`, desactivable) |
| 5. Ritmo | **Al ritmo de GunZ** | Velocidades más altas y cancelaciones rápidas; ajustable en `DA_AjustesMovimiento_Base` |
| 6. Bloqueo | Decide Claude, siempre cambiable | Guardia direccional simple más adelante (frontal), como datos |

Además: "el objetivo es poder tener todas las mecánicas de GunZ con Hollow Knight" → ver `hollow-knight.md` y `mecanicas-objetivo.md`.
