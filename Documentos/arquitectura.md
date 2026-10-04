# Arquitectura de SAEVUS

Estado: describe lo que existe en el código (2026-10-04). Lo planeado se marca como tal. Cada sistema tiene su documento: `movimiento.md`, `combate.md`, `animacion.md`, `enemigos.md`, `tutorial.md`, `interfaz.md`, `audio.md`, `rendimiento.md`, `pipeline-assets.md`.

## Módulo y carpetas

Un módulo de juego, `SAEVUS` (`Source/SAEVUS/`), dividido en carpetas por función. `SAEVUS.Build.cs` añade la raíz del módulo a las rutas de inclusión (`#include "Movimiento/SAEVUSComponenteMovimiento.h"`). Dependencias: Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG (públicas); RenderCore, RHI, Slate, SlateCore (privadas).

| Carpeta | Contenido |
|---|---|
| `Nucleo/` | `ASAEVUSModoJuego` (reaparición, puntos de control, reinicio), `ASAEVUSControladorJugador` (contextos de entrada, pausa, sensibilidad), `SAEVUSGuardado` (partida guardada y su subsistema), `SAEVUSCanalesColision.h` |
| `Personaje/` | `ASAEVUSPersonaje`, `USAEVUSConfigEntrada`, `FSAEVUSDetectorDobleToque`, `FSAEVUSAjusteGolpeRecibido` |
| `Movimiento/` | `USAEVUSComponenteMovimiento` (+ `...Paredes.cpp`): paso, salto con coyote/buffer, doble salto, dash (y voltereta con arma de fuego), paredes, rebote; `USAEVUSAjustesMovimiento` |
| `Camara/` | `USAEVUSComponenteCamara` (encuadre, FOV, estilo: contorno, viñeta, saturación), `USAEVUSAjustesCamara` |
| `Combate/` | `USAEVUSComponenteCombate` (tajos por línea de tiempo, direccionales, impacto, parada de golpe), `USAEVUSComponenteVida` (filtro de daño, invulnerabilidad), `USAEVUSComponenteAlma`, `USAEVUSComponenteGuardia` (bloqueo, contraataque), `FSAEVUSParadaGolpe`, tipos y repertorio |
| `Armas/` | `USAEVUSComponenteArmas`, `USAEVUSDefinicionArma` |
| `Animacion/` | `USAEVUSComponenteAnimacionProcedural` (FK propio sobre el esqueleto del caballero) |
| `Efectos/` | `USAEVUSComponenteParticulas` (CPU + mallas instanciadas), `USAEVUSComponenteEstelaDash`, `USAEVUSComponenteEfectosCombate` |
| `Audio/` | `SAEVUSSintesis` (sonido sintetizado), `USAEVUSComponenteSonido`, `USAEVUSComponenteSonidoEnemigo` |
| `Enemigos/` | `ASAEVUSEnemigo`, `USAEVUSComponenteIAEnemigo`, `ASAEVUSProyectil`, `USAEVUSDatosEnemigo` |
| `Tutorial/` | `USAEVUSSubsistemaTutorial`, volúmenes (aviso, punto de control, caída), `ASAEVUSPeligro`, `ASAEVUSPuertaArena` |
| `Interfaz/` | `FSAEVUSEstadoInterfaz`, `USAEVUSWidgetInterfaz`, `USAEVUSWidgetPausa` (UMG construida en C++) |
| `Mundo/` | `ASAEVUSMunecoEntrenamiento` |
| `Depuracion/` | `ASAEVUSHUDDepuracion` (crea la interfaz; texto de depuración con F6), variables de consola, `USAEVUSSubsistemaPruebaHumo`, `USAEVUSSubsistemaNumerosDanio` (números de daño flotantes) |
| `Pruebas/` | 105 tests `SAEVUS.*` y el fixture `SAEVUSMundoPrueba.h` |

## Mapa de clases

```
ASAEVUSModoJuego (AGameModeBase)          reaparición (RetardoReaparicion), punto de control, ReiniciarJugador
 └─ BP_SAEVUSModoJuego                    pone BP_SAEVUSPersonaje, BP_SAEVUSControladorJugador

ASAEVUSControladorJugador                 contextos de entrada, IA_AlternarHUD, IA_Pausa → USAEVUSWidgetPausa, sensibilidad
ASAEVUSHUDDepuracion (AHUD)               crea USAEVUSWidgetInterfaz; texto de depuración con F6; números de daño

ASAEVUSPersonaje (ACharacter)             reparte la entrada y coordina; sin reglas propias salvo el empuje al recibir golpes
 ├─ USAEVUSComponenteMovimiento (CMC)     ← USAEVUSAjustesMovimiento
 ├─ USAEVUSComponenteCombate              ← USAEVUSDatosRepertorioMelee
 ├─ USAEVUSComponenteGuardia              filtro de daño de la vida; cancelación "Guardia"
 ├─ USAEVUSComponenteArmas                ← USAEVUSDefinicionArma[]
 ├─ USAEVUSComponenteVida                 invulnerabilidad tras golpe (1 s)
 ├─ USAEVUSComponenteAlma                 alma por golpe, curación mantenida
 ├─ USAEVUSComponenteEstelaDash           partículas del dash y del doble salto
 ├─ USAEVUSComponenteEfectosCombate       chispas, estela del arma, curación, bloqueo
 ├─ USAEVUSComponenteSonido               sonidos sintetizados por evento
 ├─ UPoseableMeshComponent "Cuerpo"       SK_Caballero (esqueleto propio)
 │   └─ USAEVUSComponenteAnimacionProcedural
 └─ USpringArmComponent → USAEVUSComponenteCamara ← USAEVUSAjustesCamara

ASAEVUSEnemigo (ACharacter, sin controlador) ← USAEVUSDatosEnemigo (rastrero, saltador, escupidor, Guardián)
 ├─ USAEVUSComponenteVida, USAEVUSComponenteIAEnemigo (estados, ataques, fases)
 ├─ piezas de formas del motor (cuerpo, máscara, cuernos), USAEVUSComponenteParticulas
 └─ USAEVUSComponenteSonidoEnemigo
ASAEVUSProyectil                          bloqueable con la guardia, devuelto por un tajo
```

## Comunicación entre sistemas

Los componentes no se conocen más de lo necesario; se hablan por eventos y por consultas pequeñas:

| Evento | Lo emite | Lo escuchan |
|---|---|---|
| `AlIniciarDash`, `AlSaltoAereo` | movimiento | estela, sonido, animación, personaje (invulnerabilidad del dash) |
| `AlIniciarAtaque`, `AlGolpear` (solo si hubo daño) | combate | sonido, efectos, alma |
| `AlCambiarVida`, `AlMorir`, `FiltroDanio` | vida | personaje (empuje, muerte), alma (interrumpe), animación (reacción), sonido, IA enemiga (aturdir, fases), guardia (filtro) |
| `AlCurar`, `AlBloquear`, `AlContraatacar` | alma, guardia | efectos, sonido |
| `AlDisparar` | armas | animación, sonido |
| `AlCambiarEstado`, `AlCambiarFase` | IA enemiga | sonido del enemigo |
| `ASAEVUSEnemigo::AlMorirEnemigo` (estático) | enemigo | puertas de arena |

Cancelaciones: el personaje pregunta `USAEVUSComponenteCombate::IntentarCancelarPor(Dash | Salto | CambioArma | Guardia)` antes de cada acción; la ventana de cada una está en los datos del ataque.

Orden de actualización por fotograma: armas, combate, guardia (tras combate) y alma antes que el movimiento (prerrequisitos de tick), así una estocada, un cambio de velocidad o un giro se aplican en el movimiento del mismo fotograma; la animación va en `TG_PostPhysics`.

## Reglas que sigue el código

- **Composición antes que herencia.** El personaje y el enemigo son raíces de composición; cada comportamiento vive en un componente con una responsabilidad.
- **Ajustes por datos.** Todo valor de juego es un inicializador en un struct de ajuste (o el constructor de un struct de repertorio o de sonidos). Esos inicializadores son la única fuente de valores por defecto; un Data Asset los sobrescribe.
- **El contenido lo crea un script.** Mallas del motor y generadas en Blender, materiales, Data Assets, Blueprints y mapas los crea `Herramientas/ScriptsEditor/saevus_crear_assets.py`; se reconstruyen en vez de editarse a mano. `DA_RepertorioMelee_Base` sigue al repertorio del código mientras no se ajuste a mano.
- **Lógica separada de la presentación.** La interfaz calcula un `FSAEVUSEstadoInterfaz` y el widget solo lo pinta; la síntesis de audio son funciones puras; los efectos escuchan eventos.
- **Probado sin pantalla.** El fixture simula el personaje, los enemigos y los proyectiles a pasos fijos; los obstáculos del tutorial se prueban con entradas simuladas; la prueba de humo cubre el juego real con capturas.
- **Listo para replicar (en parte).** Esprint, caminar, dash, dobles saltos y estado de pared viajan en los movimientos guardados de CMC. Combate, guardia, alma, IA y parada de golpe son locales (fase de red: la IA solo en el servidor, la parada de golpe solo visual, el tipo de dash en el movimiento guardado).
- **Todo en español**, con prefijo `SAEVUS` en las clases.

## Perfil de render

`Config/DefaultEngine.ini`: sin Lumen, Nanite, ray tracing, Virtual Shadow Maps, MegaLights, distance fields ni iluminación estática. Reflejos SSR; antialiasing TAA; DX12 con SM6. Iluminación dinámica. Post-proceso propio: contorno por profundidad, viñeta y saturación desde la cámara. Canal de traza propio "Arma" (`ECC_GameTraceChannel1`). Mapas: `L_Tutorial` (por defecto, nocturno con exposición manual) y `L_Pruebas` (prueba de humo).

## Entrada

Teclas y técnicas en `controles.md`. Las acciones mantenidas (esprint, caminar, gatillo, guardia, curar) usan `Started` (cierto) y `Completed` (falso); `IA_Pausa` dispara también en pausa.
