# Mapa tutorial (`/Game/SAEVUS/Mapas/L_Tutorial`)

Mapa por defecto del juego y del editor. Lo genera entero `saevus_crear_assets.py` (`construir_mapa_tutorial`): se rehace en cada ejecución, así que los cambios se hacen en el script, no a mano en el editor. Simple pero escalable: secciones en línea hacia +X, cada una con un aviso en pantalla, un cartel 3D y su prueba.

> **2026-10-04:** sin enemigos, jefes, armas de fuego ni rebote (decisión del dueño). Este mapa en línea se sustituirá por un tutorial de salas tipo mazmorra con el mapa Dungeon de GunZ como referencia de estructura (tarea 70 de `PROGRESS.md`).

## Recorrido

| Sección | X (cm) | Enseña | Obstáculo |
|---|---|---|---|
| S0 Inicio | −500 – 1500 | Moverse, mirar, esprintar | Suelo llano, cartel "SAEVUS" |
| S1 Saltar | 1500 – 3500 | Salto y doble salto | Escalón de 1 m y bloque de 2,2 m (necesita doble salto) |
| S2 Dash | 3500 – 5600 | Dash por doble toque, también en el aire | Foso de 8 m |
| S3 Paredes | 5600 – 7400 | Salto de pared, escalada | Chimenea de 7 m y cornisa de 4,5 m |
| S3b Carrera por pared | 7300 – 9500 | Carrera por pared | Foso de 9 m junto a una pared de 15 m |
| S4 Combate | 9500 – 11500 | Combo, tajo hacia arriba; técnicas (desde X ≈ 10650): ataque cargado, tajo en carrera, combo aéreo | Tres muñecos y uno flotando; una pareja a los lados (X = 10900) para el giro cargado |
| S5 Paso | 11500 – 13500 | — | Suelo llano (el foso del rebote se retiró) |
| S6 Alma | 13500 – 15000 | Alma y curación (Q) | Dos muñecos para llenar el alma |
| S7 Arena | 15000 – 17500 | — (sin enemigos por ahora) | Sala vacía; sin enemigos no se pone su puerta (`ENEMIGOS_EN_MAPAS` en el script) |
| S8 Sala del jefe | 17500 – 20500 | — (sin jefe por ahora) | Sala vacía |
| S9 Fin | 20500 – 21500 | — | Cartel "Fin del tutorial"; a la derecha, "Zona 1 ->" lleva a la Zona 1 (`zona1.md`); al volver de ella se aparece aquí (`Llegada_DesdeZona1`) |
| S10 Reto (opcional) | 21500 – 24800 | Todo encadenado | Foso de 8 m con pinchos, foso de 9 m con pared, cornisa de 4,5 m; arriba, el cartel "Reto superado" y un fragmento de máscara |

Puntos de control antes del combate, del alma, de la arena, del jefe y del reto: al morir se reaparece en el último.

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
- **Decorado** (sin colisión, fuera del recorrido probado): piezas modulares generadas en Blender (`Herramientas/Arte/blender_escenario.py`, `Generar-Escenario.ps1` → `ArteFuente/Escenario/`, importadas a `/Game/SAEVUS/Arte/Escenario/`): `SM_ArcoSeccion` (arco roto en cada paso de sección, con pilares que bajan al vacío), `SM_ColumnaRota` (a ambos lados del pasillo) y `SM_Escombros` (en las zonas de descanso). Menos de 100 triángulos cada una, con `MI_Escenario_Acento`.

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
