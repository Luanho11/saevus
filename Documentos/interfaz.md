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

## Tests

`SAEVUS.Interfaz.VidaAlmaArma` (máscaras con 100 y 70 de vida, alma, arma por defecto y pistola con munición) y `SAEVUS.Interfaz.AvisoYJefe` (aviso con fundido; la barra del jefe solo con el jefe activo y vivo, con su fracción de vida). La prueba de humo guarda capturas con la interfaz.

## Pendiente

- Dirección de arte: iconos de máscara y orbe pintados, tipografía propia.
- Menú de pausa, opciones (escalabilidad, sensibilidad, controles), pantalla de muerte.
- Indicadores en el mundo (enemigos fuera de pantalla, interacción).
