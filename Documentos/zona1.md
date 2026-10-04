# Zona 1: Ruinas del Paso (boceto)

Primer nivel después del tutorial. Lo genera entero `saevus_crear_assets.py` (`construir_mapa_zona1`) en `/Game/SAEVUS/Mapas/L_Zona1`: se rehace en cada ejecución, así que los cambios se hacen en el script. Se llega desde el final del tutorial: en la S9, a la derecha del camino, el cartel "Zona 1 ->" marca un `ASAEVUSVolumenCambioMapa` que abre el mapa.

Prototipo de nivel con lo que ya existe (movimiento, combate, enemigos, puntos de control) y dos piezas nuevas: la **palanca** y el **paso entre mapas**. Geometría boceto (bloques del motor y piezas de escenario de Blender), iluminación nocturna como el tutorial.

## Recorrido del nivel (hacia +X)

| Tramo | X (cm) | Altura del suelo | Qué hay |
|---|---|---|---|
| Z0 Entrada | −500 – 1200 | 0 | Inicio, cartel "Ruinas del Paso", aviso de bienvenida |
| Z1 Descenso | 1200 – 4600 | −100, −200, −300 | Dos escalones de 1 m; encuentro 1: dos rastreros y un revoloteador |
| Z2 Grieta | 4600 – 5500 | −300 | Foso de 9 m con dos plataformas; caer devuelve al borde |
| Z3 Banco | 5500 – 6500 | −300 | Punto de control (banco) con aviso; a la derecha, la compuerta cerrada del atajo |
| Z4 Torre | 6500 – 9600 | muro de 4,5 m, después +150 | Se escala el muro; planta alta con el encuentro 2: saltador, revoloteador y escupidor |
| Z5 Arena | 9600 – 12000 | +150 | Punto de control; arena de 20 m de ancho con dos rastreros, un saltador, un escupidor y dos revoloteadores; la puerta se abre al vaciarla |
| Z6 Salida | 12000 – 13000 | +150 | Cartel de fin; por la derecha se baja a la galería |
| Galería del atajo | 6500 – 13000, Y 400 – 900 | −300 | Carril por debajo de la planta alta y de la arena; al fondo, junto al banco, una **palanca** abre la **compuerta** |

Redes de caída bajo el vacío (`ASAEVUSVolumenCaida`): si se cae antes del banco se vuelve a la entrada; después, al banco.

## Piezas nuevas

| Pieza | Dónde | Qué hace |
|---|---|---|
| `ASAEVUSPalanca` | `Source/SAEVUS/Mundo/` | Base y brazo boceto, caja golpeable (`BlockAllDynamic`, la encuentran las esferas de golpe). Un tajo o un disparo la acciona una vez: el brazo gira, abre sus `Puertas` y muestra un aviso. Devuelve 0 de daño, así que no da alma ni munición. |
| Compuerta | `ASAEVUSPuertaArena` sin enemigos | Una puerta de arena sin enemigos asignados se queda cerrada hasta que la abre una palanca (`TieneEnemigos()` las distingue). |
| `ASAEVUSVolumenCambioMapa` | `Source/SAEVUS/Mundo/` | Volumen de jugador que abre otro mapa (`Mapa`, referencia blanda). |

## Comprobación

`Herramientas\Ejecutar-Recorrido.ps1 -Mapa /Game/SAEVUS/Mapas/L_Zona1 -NombreLog recorrido-zona1`, incluido en `Verificar-Arranque.ps1 -ConJuego` (paso 7). El recorrido es el mismo que el del tutorial, generalizado:

- lleva al jugador a cada aviso, a la altura de su suelo;
- vacía la arena y comprueba su puerta (las compuertas sin enemigos no cuentan como arenas);
- por último se pone delante de cada palanca, tira un tajo de verdad y comprueba que se acciona y abre sus compuertas (`SAEVUS_Recorrido_Palanca0.png`).

La diana de armas solo se prueba si el mapa tiene una.

## Pendiente

- Arte propio de la zona (piezas de ruina más variadas, fondo lejano) y una sala de banco con identidad.
- Más tramos: secretos, un segundo atajo, un jefe de zona.
- Volver de la Zona 1 al tutorial (ahora es solo ida).
- El cartel "Banco" sale dos veces (el del aviso y el del banco): unificar cuando el banco tenga arte.
