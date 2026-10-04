# Mapa tutorial (`/Game/SAEVUS/Mapas/L_Tutorial`)

Mapa por defecto del juego y del editor. Lo genera entero `saevus_crear_assets.py` (`construir_mapa_tutorial`): se rehace en cada ejecución, así que los cambios se hacen en el script, no a mano en el editor. Simple pero escalable: secciones en línea hacia +X, cada una con un aviso en pantalla, un cartel 3D y su prueba.

## Recorrido

| Sección | X (cm) | Enseña | Obstáculo |
|---|---|---|---|
| S0 Inicio | −500 – 1500 | Moverse, mirar, esprintar | Suelo llano, cartel "SAEVUS" |
| S1 Saltar | 1500 – 3500 | Salto y doble salto | Escalón de 1 m y bloque de 2,2 m (necesita doble salto) |
| S2 Dash | 3500 – 5600 | Dash por doble toque, también en el aire | Foso de 8 m |
| S3 Paredes | 5600 – 7400 | Salto de pared, escalada | Chimenea de 7 m y cornisa de 4,5 m |
| S3b Carrera por pared | 7300 – 9500 | Carrera por pared | Foso de 9 m junto a una pared de 15 m |
| S4 Combate | 9500 – 11500 | Combo, tajo hacia arriba | Tres muñecos y uno flotando |
| S5 Rebote | 11500 – 13500 | Tajo hacia abajo y pogo | Foso de 12 m con dos muñecos flotando: no se cruza sin rebotar |
| S6 Alma | 13500 – 15000 | Alma y curación (Q) | Dos muñecos |
| S7 Arena | 15000 – 17500 | Leer avisos, esquivar, castigar | Dos rastreros y un saltador; la puerta se abre al vaciarla |
| S8 Guardián | 17500 – 20500 | Todo junto | Mini jefe con dos fases; su puerta lleva al final |
| S9 Fin | 20500 – 21500 | — | Cartel "Fin del tutorial" |

Puntos de control antes del combate, del alma, de la arena y del jefe: al morir se reaparece en el último.

## Piezas (`Source/SAEVUS/Tutorial/`)

| Pieza | Qué hace |
|---|---|
| `USAEVUSSubsistemaTutorial` | Guarda el aviso activo (título, texto, duración). Un aviso nuevo sustituye al anterior. |
| `ASAEVUSVolumenJugador` | Base: caja que reacciona al entrar el jugador (una vez o siempre). |
| `ASAEVUSVolumenTutorial` | Publica un aviso al entrar. |
| `ASAEVUSPuntoControl` | Fija el punto de reaparición del modo de juego (`ASAEVUSModoJuego::FijarPuntoControl`). |
| `ASAEVUSVolumenCaida` | Fondo de foso: quita 10 de vida y devuelve al borde (`PuntoRetorno`), como los peligros de Hollow Knight. |
| `ASAEVUSPuertaArena` | Bloque que se hunde cuando mueren todos sus enemigos (escucha `ASAEVUSEnemigo::AlMorirEnemigo`); aviso opcional al abrirse. |

La interfaz (`interfaz.md`) muestra el aviso activo centrado arriba, con fundido de entrada y salida.

## Aspecto

- Escenario con instancias de `M_SAEVUS_PersonajeDibujado`: `MI_Escenario_Piedra` (azul casi negro), `MI_Escenario_Acento` (algo más claro) y `MI_Escenario_Puerta` (cálida, brilla).
- Luz: "Luna" direccional azulada de 2,5 lux con sombras y prioridad de sombreado forward, más "Relleno" de 0,8 lux sin sombras desde el lado contrario. Sin atmósfera ni luz de cielo: el cielo queda oscuro. Niebla de altura densa y azul oscuro.
- Exposición manual (sin adaptación) en un volumen de post-proceso: la oscuridad no se aclara sola.
- Carteles: `TextRenderActor` mirando hacia el jugador que avanza.

## Pruebas

- `SAEVUS.Tutorial.Aviso`: el volumen publica el aviso una vez; los avisos caducan.
- `SAEVUS.Tutorial.PuertaArena`: solo abre con sus enemigos muertos; los de otra arena no cuentan.
- `SAEVUS.Tutorial.VolumenCaida`: devuelve al punto de retorno y quita vida sin matar.
- La prueba de humo funciona también aquí: `Ejecutar-PruebaHumo.ps1 -Mapa /Game/SAEVUS/Mapas/L_Tutorial` (desde Git Bash, con `MSYS_NO_PATHCONV=1`: si no, Git convierte `/Game/...` en una ruta de Windows y el juego se queda sin mapa). Por defecto la prueba usa `L_Pruebas`.

## Pendiente

- Recorrido automático completo del tutorial (un "fantasma" que lo supere) como prueba.
- Arte del escenario: piezas modulares, decorado, luces de acento.
