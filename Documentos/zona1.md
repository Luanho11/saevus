# Zona 1: Ruinas del Paso (boceto)

Primer nivel después del tutorial. Lo genera entero `saevus_crear_assets.py` (`construir_mapa_zona1`) en `/Game/SAEVUS/Mapas/L_Zona1`: se rehace en cada ejecución, así que los cambios se hacen en el script. Se llega desde el final del tutorial: en la S9, a la derecha del camino, el cartel "Zona 1 ->" marca un `ASAEVUSVolumenCambioMapa` que abre el mapa.

Prototipo de nivel con lo que ya existe (movimiento, combate, enemigos, puntos de control) y dos piezas nuevas: la **palanca** y el **paso entre mapas**. Geometría boceto (bloques del motor y piezas de escenario de Blender), iluminación nocturna como el tutorial.

## Recorrido del nivel (hacia +X)

| Tramo | X (cm) | Altura del suelo | Qué hay |
|---|---|---|---|
| Z0 Entrada | −500 – 1200 | 0 | Inicio con el título de zona "Ruinas del Paso" y el aviso "Hacia el este"; detrás, a la derecha, el paso "<- Tutorial" (se llega junto al paso del final del tutorial) |
| Z1 Descenso | 1200 – 4600 | −100, −200, −300 | Dos escalones de 1 m; encuentro 1: dos rastreros y un revoloteador |
| Z2 Grieta | 4600 – 5500 | −300 | Foso de 9 m con una **plataforma móvil** que va y viene (hay que esperarla); caer devuelve al borde |
| Z3 Banco | 5500 – 6500 | −300 | Banco para sentarse (E), punto de control y aviso; a la derecha, la compuerta cerrada del atajo |
| Z4 Torre | 6500 – 9600 | muro de 4,5 m, después +150 | Se escala el muro, o se sube en el **ascensor** que hay junto a él (plataforma móvil vertical desde el banco); planta alta con el encuentro 2 (acorazado, saltador, revoloteador y escupidor) y una franja de **pinchos** de 4 m que hay que saltar |
| Z5 Arena | 9600 – 12000 | +150 | Punto de control; arena de 20 m de ancho con dos rastreros, un saltador, un escupidor, dos revoloteadores y un acorazado; la puerta se abre al vaciarla |
| Z6 Antesala | 12000 – 13000 | +150 | Aviso y banco; por la derecha se baja a la galería |
| Z7 Arena del jefe | 13000 – 16000 | +150 | Punto de control; **Madre Alada** (jefe volador: picados y abanicos de proyectiles, dos fases), muros de 7 m; la puerta se abre al derrotarla |
| Z8 Fin | 16000 – 17000 | +150 | Cartel "Fin de la Zona 1 (boceto)" |
| Galería del atajo | 6500 – 13000, Y 400 – 900 | −300 | Carril por debajo de la planta alta y de la arena; al fondo, junto al banco, una **palanca** abre la **compuerta** |
| Sala secreta | 13100 – 13700, Y 400 – 900 | −300 | Tras la **pared falsa** del final de la galería (tres tajos): un **fragmento de máscara**. No sale en el mapa de la pausa |

Redes de caída bajo el vacío (`ASAEVUSVolumenCaida`): si se cae antes del banco se vuelve a la entrada; después, al banco.

## Piezas nuevas

| Pieza | Dónde | Qué hace |
|---|---|---|
| `ASAEVUSPalanca` | `Source/SAEVUS/Mundo/` | Base y brazo boceto, caja golpeable (`BlockAllDynamic`, la encuentran las esferas de golpe). Un tajo o un disparo la acciona una vez: el brazo gira, abre sus `Puertas` y muestra un aviso. Devuelve 0 de daño, así que no da alma ni munición. |
| Compuerta | `ASAEVUSPuertaArena` sin enemigos | Una puerta de arena sin enemigos asignados se queda cerrada hasta que la abre una palanca (`TieneEnemigos()` las distingue). |
| `ASAEVUSVolumenCambioMapa` | `Source/SAEVUS/Mundo/` | Volumen de jugador que abre otro mapa (`Mapa`, referencia blanda). Con `Llegada` abre `mapa#Llegada`: UE coloca al jugador en el `APlayerStart` con esa etiqueta (el "portal" de la URL va tras `#`; una opción `?portal=` no basta). Además pasa `?portal=<Llegada>` para que el modo de juego sepa que se llegó por un paso. `ASAEVUSModoJuego` elige el inicio sin etiqueta en el arranque normal; si se llega por un paso, no abre el menú principal y la llegada es el primer punto de control. Test `SAEVUS.Mundo.InicioJugador`. |
| `ASAEVUSParedRompible` | `Source/SAEVUS/Mundo/` | Pared falsa (Hollow Knight): un bloque dinámico que tiembla con cada golpe y cae al tercero (`GolpesParaRomper`). No da alma ni munición. |
| `ASAEVUSFragmentoMascara` | `Source/SAEVUS/Mundo/` | Se recoge al tocarlo. Cada `FragmentosPorMascara` (2) suma una máscara de vida (`FSAEVUSAjusteProgreso` en el personaje: +20 de vida máxima y vida llena). Se guarda por `Identificador`: los recogidos no vuelven a aparecer y las máscaras extra se aplican al empezar. Hay uno en la sala secreta de la Zona 1 y otro en lo alto del reto del tutorial. |
| `ASAEVUSPlataformaMovil` | `Source/SAEVUS/Mundo/` | Va y viene entre su sitio y `Recorrido`, con pausa en los extremos y suavizado (`PosicionEn`, función pura). Lo que está encima viaja con ella. Test `SAEVUS.Mundo.PlataformaMovil` (posiciones del ciclo y jugador transportado 400 cm). |
| `ASAEVUSBanco` | `Source/SAEVUS/Mundo/` | Banco (Hollow Knight) con `ISAEVUSInteractuable`. Al entrar en su zona se anuncia al personaje y la interfaz muestra "E   Sentarse". Sentarse (`IA_Interactuar`, E o cruceta arriba) cura del todo, rellena la munición, fija el punto de control delante del banco y lo guarda, con una campanada; el caballero adopta la pose sentada. Moverse, saltar, atacar, hacer dash o volver a interactuar lo levanta. Hay bancos en el banco de la Zona 1 (Z3), en la antesala del jefe (Z6) y en el tutorial (S6 y S9). |

## Comprobación

`Herramientas\Ejecutar-Recorrido.ps1 -Mapa /Game/SAEVUS/Mapas/L_Zona1 -Paso`, incluido en `Verificar-Arranque.ps1 -ConJuego` (paso 6). Con `-Paso` (`-SAEVUSRecorridoPaso`), al acabar la Zona 1 el jugador cruza el paso de vuelta, el tutorial comprueba que apareció en `DesdeZona1` y se recorre entero: un proceso, dos mapas y el viaje entre ellos (el log tiene dos SUPERADO). El recorrido es el mismo que el del tutorial, generalizado:

- lleva al jugador a cada aviso, a la altura de su suelo;
- vacía la arena y comprueba su puerta (las compuertas sin enemigos no cuentan como arenas);
- se pone delante de cada palanca, tira un tajo de verdad y comprueba que se acciona y abre sus compuertas (`SAEVUS_Recorrido_Palanca0.png`);
- se sienta en cada banco: comprueba que se anuncia, que sienta y cura, y que se levanta (`SAEVUS_Recorrido_BancoN.png`);
- por último rompe cada pared falsa a tajos de verdad y recoge cada fragmento de máscara tocándolo.

La diana de armas solo se prueba si el mapa tiene una.

## Pendiente

- Arte propio de la zona (piezas de ruina más variadas, fondo lejano) y una sala de banco con identidad.
- Más tramos: secretos, un segundo atajo.

- Arte del banco (ahora dos cubos).
