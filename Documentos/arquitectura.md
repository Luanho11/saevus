# Arquitectura de SAEVUS

Estado: describe lo que existe en el código. Lo planeado se marca como tal.

## Módulo y carpetas

Un módulo de juego, `SAEVUS` (`Source/SAEVUS/`), dividido en carpetas por función. `SAEVUS.Build.cs` añade la raíz del módulo a las rutas de inclusión, así los includes se escriben por carpeta: `#include "Movimiento/SAEVUSComponenteMovimiento.h"`.

| Carpeta | Contenido |
|---|---|
| `Nucleo/` | `ASAEVUSModoJuego` (reaparición), `ASAEVUSControladorJugador`, `SAEVUSCanalesColision.h` |
| `Personaje/` | `ASAEVUSPersonaje`, `USAEVUSConfigEntrada` |
| `Movimiento/` | `USAEVUSComponenteMovimiento` (+ `...Paredes.cpp`), structs de ajuste, `USAEVUSAjustesMovimiento`, `ESAEVUSPaso`, `ESAEVUSModoMovimientoPersonalizado` |
| `Camara/` | `USAEVUSComponenteCamara`, `FSAEVUSAjusteCamara`, `USAEVUSAjustesCamara` |
| `Combate/` | `USAEVUSComponenteCombate`, `USAEVUSComponenteVida`, tipos de ataque y repertorio |
| `Armas/` | `USAEVUSComponenteArmas`, `USAEVUSDefinicionArma` |
| `Mundo/` | `ASAEVUSMunecoEntrenamiento` |
| `Depuracion/` | `ASAEVUSHUDDepuracion`, variables de consola, `USAEVUSSubsistemaPruebaHumo` |
| `Pruebas/` | tests de automatización `SAEVUS.*` y el fixture `SAEVUSMundoPrueba.h` |

Se crean carpetas nuevas cuando llega su código (Animacion, Efectos, Interfaz, Enemigos, Jefes...).

## Mapa de clases

```
ASAEVUSModoJuego (AGameModeBase)          respaldo nativo: ASAEVUSPersonaje, ASAEVUSControladorJugador, ASAEVUSHUDDepuracion
 └─ BP_SAEVUSModoJuego                    pone BP_SAEVUSPersonaje, BP_SAEVUSControladorJugador; reaparición tras la muerte

ASAEVUSControladorJugador (APlayerController)
 ├─ añade contextos: IMC_Default, IMC_MouseLook (plantilla del motor), IMC_SAEVUS
 └─ enlaza acciones propias del controlador: IA_AlternarHUD (F6)

ASAEVUSPersonaje (ACharacter)             reparte la entrada; no tiene reglas de juego propias
 ├─ USAEVUSComponenteMovimiento           paso, dash, paredes, movimientos guardados  ← USAEVUSAjustesMovimiento
 ├─ USAEVUSComponenteCombate              líneas de tiempo de ataque, combos, cancelaciones ← USAEVUSDatosRepertorioMelee
 ├─ USAEVUSComponenteArmas                arsenal, cambio, armas de fuego              ← USAEVUSDefinicionArma[]
 ├─ USAEVUSComponenteVida                 vida, muerte
 ├─ USpringArmComponent "BrazoCamara"
 │   └─ USAEVUSComponenteCamara           encuadre, retraso, FOV de esprint           ← USAEVUSAjustesCamara
 └─ ConfigEntrada                         ← USAEVUSConfigEntrada (qué acciones enlazar)
 └─ BP_SAEVUSPersonaje                    malla, anim BP, referencias a Data Assets

ASAEVUSHUDDepuracion (AHUD)               dibuja con SAEVUS.Depuracion.HUD = 1
```

Orden de actualización por fotograma: armas y combate antes que el movimiento (prerrequisitos de tick), así una estocada, un cambio de velocidad o un giro se aplican en el movimiento del mismo fotograma.

## Reglas que sigue el código

- **Composición antes que herencia.** El personaje es una raíz de composición; cada comportamiento vive en un componente con una responsabilidad.
- **Ajustes por datos.** Todo valor de juego es un inicializador en un struct de ajuste (o el constructor de un struct de repertorio). Esos inicializadores son la única fuente de valores por defecto; un Data Asset con el struct los sobrescribe; los componentes usan una instancia `const` por defecto si no hay asset.
- **El C++ no referencia contenido.** Mallas, anim BPs, entrada y Data Assets se asignan en subclases Blueprint, creadas por `Herramientas/ScriptsEditor/saevus_crear_assets.py`, así se reconstruyen en vez de editarse a mano.
- **Listo para replicar.** Las intenciones del jugador que cambian el movimiento (esprint, caminar, dash) viajan por el camino de movimientos guardados / banderas comprimidas de CMC, junto al estado de dash y paredes. Combate y armas todavía son locales (fase de red).
- **Todo en español** (código, datos, documentos), con prefijo `SAEVUS` en las clases.

## Flujo de entrada

1. `DefaultInput.ini` hace de Enhanced Input el sistema por defecto.
2. `ASAEVUSControladorJugador::SetupInputComponent` añade los contextos del jugador local y enlaza el interruptor del HUD.
3. `ASAEVUSPersonaje::SetupPlayerInputComponent` enlaza las acciones de su `USAEVUSConfigEntrada`. Las acciones que falten se saltan; si falta la config, escribe un error.
4. Acciones mantenidas (esprint, caminar, gatillo) en `Triggered`/`Started` (cierto) y `Completed` (falso).

| Acción | Teclas | Origen |
|---|---|---|
| Mover | WASD, stick izquierdo | `IMC_Default` de plantilla |
| Mirar | ratón, stick derecho | plantilla |
| Saltar | Espacio, botón inferior del mando | plantilla |
| Esprint (mantener) | Mayús izq., L3 | `IMC_SAEVUS` |
| Caminar (mantener) | Alt izq. | `IMC_SAEVUS` |
| Dash | doble toque de WASD; botón derecho del mando | personaje / `IMC_SAEVUS` |
| Atacar / gatillo | clic izquierdo, botón izquierdo del mando | `IMC_SAEVUS` |
| Recargar | R, botón superior del mando | `IMC_SAEVUS` |
| Siguiente arma | rueda abajo, hombro derecho | `IMC_SAEVUS` |
| Ranuras de arma | 1–4 | `IMC_SAEVUS` |
| HUD de depuración | F6 | `IMC_SAEVUS` |

## Perfil de render

`Config/DefaultEngine.ini`: sin Lumen, Nanite, ray tracing, Virtual Shadow Maps, MegaLights, distance fields ni iluminación estática. Reflejos SSR; antialiasing TAA. Iluminación totalmente dinámica: una luz direccional, luz de cielo, atmósfera y niebla de altura. DX12 con SM6. Canal de traza propio "Arma" (`ECC_GameTraceChannel1`).
