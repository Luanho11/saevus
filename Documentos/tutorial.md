# Mapa tutorial (`/Game/SAEVUS/Mapas/L_Tutorial`)

Mapa por defecto del juego y del editor: la **Mazmorra del Umbral**. Lo genera entero `saevus_crear_assets.py` (`construir_mapa_tutorial`): se rehace en cada ejecución, así que los cambios se hacen en el script, no a mano en el editor.

**Estructura (tarea 70, referencia: el mapa Dungeon de GunZ):** salas cerradas de muros altos sin techo (la luna entra y la cámara no choca con un techo), unidas por puertas de 5 × 6 m hacia +X. Dentro hay escaleras, balcones y columnas para correr por las paredes. No hay pasillos sobre el vacío: los fosos están dentro de las salas y sus muros bajan hasta el fondo. Junto a cada puerta de salida hay un brasero (luz puntual cálida sin sombras). Cada sala enseña una cosa con un aviso que dice qué pulsar y qué hacer con ello, en frases cortas.

Piezas del script: `crear_sala` (suelo, muros laterales y muros de entrada y salida con puerta; cada sala pone los dos con su ancho, así entre salas de anchos distintos no quedan huecos), `crear_muro_con_puerta`, `crear_escalera` (peldaños macizos de 40 cm como mucho, que se suben andando), `crear_columna` y `crear_antorcha`.

## Recorrido

| Sala | X (cm) | Ancho | Enseña | Obstáculo |
|---|---|---|---|---|
| S0 Celda | −600 – 1400 | 14 m | Moverse en ocho direcciones, mirar, esprintar | Sala vacía; cartel "SAEVUS" sobre la puerta |
| S1 Saltar | 1400 – 3600 | 12 m | Salto y doble salto | Escalón de 1 m y bloque de 2,2 m de pared a pared; escalera de bajada |
| S2 Dash | 3600 – 5800 | 12 m | Dash por doble toque, encadenado, también en el aire | Foso de 8 m |
| S3 Chimenea | 5800 – 7600 | 8 m | Salto de pared, rebote entre paredes, escalada | Chimenea de paredes a 3 m y cornisa de 4,5 m |
| S4 Galería | 7600 – 9800 | 8 m | Carrera por pared | Foso de 9 m junto al muro derecho (y = 450) |
| S5 Sala de armas | 9800 – 12600 | 18 m | Combo, tajo hacia arriba, guardia; ataque cargado, tajo en carrera, combo aéreo | Muñecos (uno alto, una pareja para el giro) entre cuatro columnas |
| S6 Capilla | 12600 – 14400 | 12 m | Alma, curación, banco | Dos muñecos y un banco |
| S7 Gran salón | 14400 – 19000 | 30 m | Práctica libre | Balcones a 4 m a ambos lados con escaleras, dos filas de columnas de 11 m, estrado central |
| S8 Fin | 19000 – 20400 | 12 m | — | Banco, cartel "Fin del tutorial", paso a la Zona 1 y llegada desde ella (`Llegada_DesdeZona1`) |
| S9 Reto (opcional) | 20400 – 24400 | 8 m | Todo encadenado | Foso de 8 m con pinchos, foso de 9 m con carrera por pared, cornisa de 4,5 m con un fragmento de máscara |

Puntos de control al entrar en la sala de armas, la capilla, el gran salón y el reto: al morir se reaparece en el último.

## Piezas (`Source/SAEVUS/Tutorial/`)

| Pieza | Qué hace |
|---|---|
| `USAEVUSSubsistemaTutorial` | Guarda el aviso activo (título, texto, duración). Un aviso nuevo sustituye al anterior. |
| `ASAEVUSVolumenJugador` | Base: caja que reacciona al entrar el jugador (una vez o siempre). |
| `ASAEVUSVolumenTutorial` | Publica un aviso al entrar. |
| `ASAEVUSPuntoControl` | Fija el punto de reaparición del modo de juego (`ASAEVUSModoJuego::FijarPuntoControl`). |
| `ASAEVUSVolumenCaida` | Fondo de foso: quita 10 de vida y devuelve al borde (`PuntoRetorno`), como los peligros de Hollow Knight. |
| `ASAEVUSPeligro` | Pinchos: tocarlos es como caer (daño y vuelta al borde), no reciben daño ni dan alma. |
| `ASAEVUSPuertaArena` | Bloque que se hunde cuando mueren todos sus enemigos (escucha `ASAEVUSEnemigo::AlMorirEnemigo`); aviso opcional al abrirse. |

La interfaz (`interfaz.md`) muestra el aviso activo centrado arriba, con fundido de entrada y salida.

## Aspecto

- Escenario con instancias de `M_SAEVUS_PersonajeDibujado`: `MI_Escenario_Piedra` (azul casi negro), `MI_Escenario_Acento` (algo más claro) y `MI_Escenario_Puerta` (cálida, brilla).
- Luz: "Luna" direccional azulada de 2,5 lux con sombras y prioridad de sombreado forward, más "Relleno" de 0,8 lux sin sombras desde el lado contrario. Sin atmósfera ni luz de cielo: el cielo queda oscuro. Niebla de altura densa y azul oscuro.
- Exposición manual (sin adaptación) en un volumen de post-proceso: la oscuridad no se aclara sola.
- Carteles: `TextRenderActor` mirando hacia el jugador que avanza.
- **Decorado** (sin colisión): piezas modulares generadas en Blender (`Herramientas/Arte/blender_escenario.py`, `Generar-Escenario.ps1` → `ArteFuente/Escenario/`, importadas a `/Game/SAEVUS/Arte/Escenario/`): `SM_ColumnaRota` (esquinas del gran salón) y `SM_Escombros` (la celda); `SM_ArcoSeccion` queda para la Zona 1. Menos de 100 triángulos cada una, con `MI_Escenario_Acento`.

## Pruebas

- `SAEVUS.Tutorial.Aviso`: el volumen publica el aviso una vez; los avisos caducan.
- `SAEVUS.Tutorial.PuertaArena`: solo abre con sus enemigos muertos; los de otra arena no cuentan.
- `SAEVUS.Tutorial.VolumenCaida`: devuelve al punto de retorno y quita vida sin matar.
- `SAEVUS.Tutorial.Obstaculos.*`: cada obstáculo con sus medidas del mapa, superado con entradas simuladas de jugador. Si se cambia una medida del mapa o un ajuste de movimiento, hay que actualizar las constantes del test (`SAEVUSPruebasObstaculos.cpp`) y comprobar que sigue pasando:
  - `BloqueDobleSalto`: con doble salto se sube al bloque de 2,2 m; con solo el salto normal (sin doble salto, escalada ni salto de pared), no.
  - `FosoDash`: el foso de 8 m se cruza con salto, doble salto y dash (aterriza a unos 90 cm del borde: justo, a propósito).
  - `FosoCarreraPared`: el foso de 9 m se cruza corriendo por la pared.
  - `Cornisa`: la cornisa de 4,5 m se sube escalando y rematando con doble salto.
- **Recorrido en el juego real:** `Herramientas\Ejecutar-Recorrido.ps1` (`USAEVUSSubsistemaRecorrido`, `-SAEVUSRecorrido`) lleva al jugador, invulnerable, a cada aviso y comprueba que aparece (captura `SAEVUS_Recorrido_NN.png` por sección), dispara con la pistola a la diana más alta (muñeco elevado) y comprueba el impacto (`SAEVUS_Recorrido_Diana.png`), y en cada arena hace una captura mirando atrás con los enemigos acercándose (`SAEVUS_Recorrido_ArenaN_Enemigos.png`) y comprueba que el jefe se activa al entrar y que la puerta se abre al vaciarla. Escribe `[SAEVUS recorrido] SUPERADO/FALLIDO`.
- La prueba de humo funciona también aquí: `Ejecutar-PruebaHumo.ps1 -Mapa /Game/SAEVUS/Mapas/L_Tutorial` (desde Git Bash, con `MSYS_NO_PATHCONV=1`: si no, Git convierte `/Game/...` en una ruta de Windows y el juego se queda sin mapa). Por defecto la prueba usa `L_Pruebas`.

## Pendiente

- Más piezas (suelos rotos, escaleras, estalactitas) y luces de acento.
