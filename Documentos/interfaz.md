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
- **Abajo a la derecha:** arma equipada y munición (o "Recargando...").
- **Abajo al centro:** nombre y barra de vida del jefe, solo mientras hay un jefe vivo y activo (fuera del reposo).

Escala con las reglas de DPI del proyecto (por defecto, según el lado corto de la pantalla; a 720p todo es dos tercios del tamaño a 1080p). Las medidas del struct están en unidades de 1080p.

## Menú de pausa

`USAEVUSWidgetPausa` (también construido en C++): velo oscuro y una columna con **Continuar**, **Volver al último punto de control** (`ASAEVUSModoJuego::ReiniciarJugador`), **Calidad** (rota Baja → Media → Alta con `UGameUserSettings`), **Sensibilidad del ratón** (0,5 → 0,75 → 1 → 1,5 → 2, multiplica la entrada de mirar) y **Salir**. Lo abre `ASAEVUSControladorJugador::AlternarPausa` con `IA_Pausa` (P o Start; Esc detiene el Play en el editor), que dispara también en pausa.

## Tests

`SAEVUS.Interfaz.VidaAlmaArma` (máscaras con 100 y 70 de vida, alma, arma por defecto y pistola con munición) y `SAEVUS.Interfaz.AvisoYJefe` (aviso con fundido; la barra del jefe solo con el jefe activo y vivo, con su fracción de vida) y `SAEVUS.Interfaz.OpcionesPausa` (rotación de calidad y sensibilidad). La prueba de humo abre el menú (juego en pausa) y guarda `SAEVUS_PruebaHumo_Pausa.png`. La prueba de humo guarda capturas con la interfaz.

## Pendiente

- Dirección de arte: iconos de máscara y orbe pintados, tipografía propia.
- Opciones completas (resolución, controles, volumen), pantalla de muerte, menú principal.
- Indicadores en el mundo (enemigos fuera de pantalla, interacción).
