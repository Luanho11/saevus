# Interfaz de SAEVUS

Boceto mejorable, sin dirección visual compleja todavía (decisión del dueño): formas simples, todo ajustable en un struct, y preparado para sustituirlo por una versión con arte.

## Piezas (`Source/SAEVUS/Interfaz/`)

| Pieza | Qué hace |
|---|---|
| `FSAEVUSEstadoInterfaz` | Calcula lo que se muestra en un fotograma: relleno de cada máscara de vida, fracción de alma (y si alcanza para curar, si está curando), arma y munición, aviso del tutorial con su opacidad de fundido, y el jefe activo más cercano (nombre y vida). Sin widgets: se prueba sin pantalla y lo puede usar cualquier interfaz. |
| `USAEVUSWidgetInterfaz` | `UUserWidget` cuyo árbol se construye en C++ (`RebuildWidget`), sin assets de widget ni texturas: pinceles redondeados de Slate para máscaras, orbe y barra. Cada tick calcula el estado y lo aplica. |
| `FSAEVUSAjusteInterfaz` | Colores, tamaños, márgenes, tamaños de texto, vida por máscara, fundido de avisos, barra del jefe. |
| `ASAEVUSHUDDepuracion` | Crea la interfaz al empezar (`ClaseInterfaz`, sustituible por una subclase Blueprint) y dibuja encima el texto de depuración con F6. |

## Disposición

- **Arriba a la izquierda:** orbe de alma (se llena de abajo arriba; más brillante si alcanza para curar; late mientras cura) y a su derecha las máscaras de vida (20 de vida cada una; una máscara a medias se ve gris).
- **Arriba al centro:** aviso del tutorial (título y texto, con fundido de entrada y salida).
- **Abajo a la derecha:** arma equipada y munición como `cargador / reserva` (`∞` si la reserva es infinita), o "Recargando...".
- **Abajo al centro:** nombre y barra de vida del jefe, solo mientras hay un jefe vivo y activo (fuera del reposo).

Escala con las reglas de DPI del proyecto (por defecto, según el lado corto de la pantalla; a 720p todo es dos tercios del tamaño a 1080p). Las medidas del struct están en unidades de 1080p.

- **Pantalla de muerte:** sin pawn o con el pawn muerto (`FSAEVUSEstadoInterfaz::bJugadorMuerto`), un velo negro con "Has caído" se funde en 1 s; al reaparecer se aclara en 0,5 s (`OpacidadMuerte`, `FundidoMuerte`, `FundidoReaparicion`). La prueba de humo guarda `SAEVUS_PruebaHumo_Muerte.png`.

## Menú de pausa

`USAEVUSWidgetPausa` (también construido en C++): velo oscuro y una columna con **Continuar**, **Volver al último punto de control** (`ASAEVUSModoJuego::ReiniciarJugador`), **Calidad** (rota Baja → Media → Alta con `UGameUserSettings`), **Sensibilidad del ratón** (0,5 → 0,75 → 1 → 1,5 → 2, multiplica la entrada de mirar), **Volumen** (0 → 25 → 50 → 75 → 100 %, volumen general del dispositivo de audio) y **Salir**. Lo abre `ASAEVUSControladorJugador::AlternarPausa` con `IA_Pausa` (P o Start; Esc detiene el Play en el editor), que dispara también en pausa.

## Título de zona

Al entrar en un mapa con `ASAEVUSInfoZona` (un actor por mapa con `Nombre` y `Subtitulo`, lo pone el script de assets), la interfaz muestra el nombre grande en el tercio de arriba, con el subtítulo encima: "Tutorial / Las Afueras" y "Zona 1 / Ruinas del Paso". Como los letreros de Hollow Knight. Espera `RetardoTituloZona`, aparece en `FundidoTituloZona`, se queda `DuracionTituloZona` y se va. La curva es una función pura (`SAEVUSInterfaz::OpacidadTituloZona`, test `SAEVUS.Interfaz.TituloZona`). Cuenta con tiempo de mundo, así que en el tutorial espera a que se pulse "Jugar" en el menú principal.

## Mapa en la pausa

`USAEVUSWidgetMapa` (`Interfaz/SAEVUSMapa`), a la derecha de las opciones de la pausa: una vista cenital de la zona generada de la geometría al abrir la pausa. Cada malla estática con colisión es un rectángulo, más claro cuanto más alta es su cara de arriba. Las puertas cerradas salen en naranja, los puntos de control en cian y el jugador es un punto blanco. Los fondos de los fosos llevan la etiqueta `SinMapa` (`crear_foso`) y no salen, así el foso se lee como un hueco. El cálculo está aparte del widget y se prueba sin pantalla: `SAEVUSMapa::RecogerBloques` y `Encuadrar` (escala uniforme, centrado, margen), con el test `SAEVUS.Interfaz.Mapa`. La captura de la pausa de la prueba de humo lo muestra.

## Menú principal

**Mando y teclado:** al abrir un menú el foco va al primer botón visible (`ObtenerPrimerBoton`); se navega con la cruceta, el stick o las flechas y se confirma con el botón inferior, Intro o Espacio (navegación de Slate). El botón con el foco se resalta con `EstiloBotonMenuEnfocado`, porque el mando no tiene "pasar por encima". La prueba de humo comprueba que la pausa abre con el foco en un botón.

Los botones de los dos menús usan `SAEVUSEstilo::EstiloBotonMenu` (`SAEVUSEstiloInterfaz`, compartido con la interfaz): píldora oscura translúcida con borde, que se aclara al pasar por encima.

El mismo widget en modo `Principal` (`ESAEVUSModoMenu`): velo más opaco, título **SAEVUS** con el subtítulo "Boceto jugable", y debajo **Jugar**, **Continuar desde el último punto de control** (solo si hay uno guardado para el mapa: lleva al jugador allí con `ASAEVUSModoJuego::FijarPuntoControl` y `ReiniciarJugador`), **Calidad**, **Sensibilidad** y **Salir**. Se abre al arrancar los mapas de `ASAEVUSModoJuego::MapasConMenuPrincipal` (`L_Tutorial`), también con Play en el editor, con el juego en pausa. La tecla de pausa no lo cierra. No sale en las pruebas en el juego (`-SAEVUSPruebaHumo`, `-SAEVUSRecorrido`) ni con `-SAEVUSSinMenu`. La prueba de humo lo abre, comprueba la pausa, pulsa la tecla de pausa (sigue abierto), lo cierra con "Jugar" y guarda `SAEVUS_PruebaHumo_Menu.png`.

**Guardado** (`Nucleo/SAEVUSGuardado`): la calidad la guarda el motor (`GameUserSettings.ini`). La sensibilidad, el volumen y el último punto de control (por mapa) van en `USAEVUSPartidaGuardada` (`Saved/SaveGames/SAEVUS.sav`), que mantiene `USAEVUSSubsistemaGuardado`: se carga al arrancar, el controlador aplica la sensibilidad y el volumen en `BeginPlay`, y se escribe al cambiar una opción o al pasar por un punto de control. El punto de control guardado no se usa al arrancar: es para "Continuar" del menú principal. Con `-SAEVUSSinGuardar` (lo pasan la prueba de humo y el recorrido) no se escribe nada. Test `SAEVUS.Guardado.IdaYVuelta`.

## Tests

`SAEVUS.Interfaz.VidaAlmaArma` (máscaras con 100 y 70 de vida, alma, arma por defecto y pistola con munición) y `SAEVUS.Interfaz.AvisoYJefe` (aviso con fundido; la barra del jefe solo con el jefe activo y vivo, con su fracción de vida) y `SAEVUS.Interfaz.OpcionesPausa` (rotación de calidad y sensibilidad). La prueba de humo abre el menú (juego en pausa) y guarda `SAEVUS_PruebaHumo_Pausa.png`. La prueba de humo guarda capturas con la interfaz.

## Pendiente

- Dirección de arte: iconos de máscara y orbe pintados, tipografía propia.
- Opciones completas (resolución, controles, volumen).
- Indicadores en el mundo (enemigos fuera de pantalla, interacción).
