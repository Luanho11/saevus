# La Mazmorra (`/Game/SAEVUS/Mapas/L_Mazmorra`)

Mapa de práctica libre que sigue al tutorial (tarea 71) y sustituye a la Zona 1, retirada el 2026-10-04: el script borra `L_Zona1` si existe. Estructura de referencia: el mapa Dungeon de GunZ, es decir, una sala central grande con balcones y columnas y salas alrededor. Lo genera entero `saevus_crear_assets.py` (`construir_mapa_mazmorra`); los cambios se hacen en el script. Sin enemigos todavía: aquí entrarán los goblins (tareas 75 y 76).

Piezas del script, además de las del tutorial: `crear_muro` (muro recto con huecos de puerta y dintel), `crear_recinto` (sala rectangular con los muros que se pidan; el muro compartido lo pone una sola sala), `crear_balcon` (losa a 8 m con apoyos) y `crear_escalera` también a lo largo de Y.

## Salas

| Sala | Interior (cm) | Qué hay |
|---|---|---|
| Salón central | X −2000 – 2000, Y −2000 – 2000, muros de 14 m | Puertas de 6 × 7 m en los cuatro lados; anillo de balcones a 8 m (abierto donde suben las dos escaleras, junto a los muros norte y sur); ocho columnas de 2 m en corro para correr y rebotar; estrado de 1,2 m en el centro; cuatro braseros |
| Entrada (oeste) | X −4800 – −2000, Y −900 – 900 | Inicio, banco, punto de control, aviso "La Mazmorra", paso al tutorial (`DesdeMazmorra`) y llegada desde él (`DesdeTutorial`) |
| Galería (este) | X 2000 – 6000, Y −800 – 800 | Foso de 9 m junto al muro derecho (carrera por pared), foso de 8 m (dash); al fondo, una **pared falsa** de 4 × 4 m da a una sala secreta con un fragmento |
| Pozo (norte) | X −800 – 800, Y 2000 – 5000, muros de 16 m | Chimenea de paredes a 3 m (11 m de alto) hasta una cornisa de 9 m, o el **ascensor** junto a ella; arriba, un fragmento |
| Terraza (sur) | X −1000 – 1000, Y −4600 – −2000 | Escalera hasta una terraza a 6 m; en ella una **palanca** abre la **compuerta** del fondo, detrás de la cual hay un fragmento |

## Piezas del mundo

| Pieza | Dónde | Qué hace |
|---|---|---|
| `ASAEVUSPalanca` | `Source/SAEVUS/Mundo/` | Base y brazo boceto, caja golpeable (`BlockAllDynamic`, la encuentran las esferas de golpe). Un tajo o un disparo la acciona una vez: el brazo gira, abre sus `Puertas` y muestra un aviso. Devuelve 0 de daño, así que no da alma ni munición. |
| Compuerta | `ASAEVUSPuertaArena` sin enemigos | Una puerta de arena sin enemigos asignados se queda cerrada hasta que la abre una palanca (`TieneEnemigos()` las distingue). |
| `ASAEVUSVolumenCambioMapa` | `Source/SAEVUS/Mundo/` | Volumen de jugador que abre otro mapa (`Mapa`, referencia blanda). Con `Llegada` abre `mapa#Llegada`: UE coloca al jugador en el `APlayerStart` con esa etiqueta (el "portal" de la URL va tras `#`; una opción `?portal=` no basta). Además pasa `?portal=<Llegada>` para que el modo de juego sepa que se llegó por un paso. `ASAEVUSModoJuego` elige el inicio sin etiqueta en el arranque normal; si se llega por un paso, no abre el menú principal y la llegada es el primer punto de control. Test `SAEVUS.Mundo.InicioJugador`. |
| `ASAEVUSParedRompible` | `Source/SAEVUS/Mundo/` | Pared falsa (Hollow Knight): un bloque dinámico que tiembla con cada golpe y cae al tercero (`GolpesParaRomper`). No da alma ni munición. |
| `ASAEVUSFragmentoMascara` | `Source/SAEVUS/Mundo/` | Se recoge al tocarlo. Cada `FragmentosPorMascara` (2) suma una máscara de vida (`FSAEVUSAjusteProgreso` en el personaje: +20 de vida máxima y vida llena). Se guarda por `Identificador`: los recogidos no vuelven a aparecer y las máscaras extra se aplican al empezar. Hay tres en la Mazmorra (sala secreta, cornisa del pozo, tras la compuerta de la terraza) y uno en lo alto del reto del tutorial. |
| `ASAEVUSPlataformaMovil` | `Source/SAEVUS/Mundo/` | Va y viene entre su sitio y `Recorrido`, con pausa en los extremos y suavizado (`PosicionEn`, función pura). Lo que está encima viaja con ella. Test `SAEVUS.Mundo.PlataformaMovil` (posiciones del ciclo y jugador transportado 400 cm). En la Mazmorra, el ascensor del pozo. |
| `ASAEVUSBanco` | `Source/SAEVUS/Mundo/` | Banco (Hollow Knight) con `ISAEVUSInteractuable`. Al entrar en su zona se anuncia al personaje y la interfaz muestra "E   Sentarse". Sentarse (`IA_Interactuar`, E o cruceta arriba) cura del todo, rellena la munición, fija el punto de control delante del banco y lo guarda, con una campanada; el caballero adopta la pose sentada. Moverse, saltar, atacar, hacer dash o volver a interactuar lo levanta. Hay bancos en la entrada de la Mazmorra y en el tutorial (capilla y fin). |

## Comprobación

`Herramientas\Ejecutar-Recorrido.ps1 -Mapa /Game/SAEVUS/Mapas/L_Mazmorra -Paso`, incluido en `Verificar-Arranque.ps1 -ConJuego` (paso 6). Con `-Paso` (`-SAEVUSRecorridoPaso`), al acabar la Mazmorra el jugador cruza el paso al tutorial, el tutorial comprueba que apareció en `DesdeMazmorra` y se recorre entero: un proceso, dos mapas y el viaje entre ellos (el log tiene dos SUPERADO). En cada mapa el recorrido:

- lleva al jugador a cada aviso, a la altura de su suelo;
- se pone delante de cada palanca, tira un tajo de verdad y comprueba que se acciona y abre sus compuertas (`SAEVUS_Recorrido_Palanca0.png`);
- se sienta en cada banco: comprueba que se anuncia, que sienta y cura, y que se levanta (`SAEVUS_Recorrido_BancoN.png`);
- por último rompe cada pared falsa a tajos de verdad y recoge cada fragmento de máscara tocándolo.

## Pendiente

- Goblins y su jefe (tareas 75 y 76).
- Piezas de mazmorra con más forma (arcos, rejas, escaleras de caracol) generadas en Blender.
