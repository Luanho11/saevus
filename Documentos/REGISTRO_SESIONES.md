# REGISTRO DE SESIONES DE SAEVUS

Historial breve de las sesiones. El estado actual está en `Documentos/ESTADO_PROYECTO.md`; la cola de tareas, en `PROGRESS.md`.

## 2026-10-02 — Sesión 1

- Inspección del entorno; instalados CMake 4.4.3, VS 2022 Build Tools 17.14 (MSVC 14.44 + SDK 10.0.26100), Blender 4.2.16 LTS y el Epic Games Launcher.
- Sistema de contexto (`CLAUDE.md`, estado del proyecto, registro de sesiones). Estructura de carpetas y primer código de movimiento (sin compilar).
- Decisiones: VS 2022 en vez de VS 2026; CMC en vez de Mover; sin Lumen / Nanite / RT / VSM / MegaLights.
- Bloqueo: la instalación de UE 5.8 necesitaba al dueño.

## 2026-10-03 — Sesión 2 (arranque, trabajo nocturno)

- UE 5.8.3 verificado; proyecto creado con perfil de render para hardware modesto; contenido de plantilla copiado; .NET Framework 4.8 SDK añadido (UBT lo exigía).
- Modo de juego, controlador, personaje, cámara, configuración de entrada, HUD de depuración, tests de movimiento; script de assets y mapa de pruebas.
- Smart App Control bloqueaba la DLL del juego (error 4551) hasta que el dueño lo desactivó.
- Revisión estática contra el código del motor: personajes de test en `MOVE_None`, malla del maniquí con su setter.
- Arranque completado: compila limpio, tests, assets, prueba de humo con render en la Vega 8.

## 2026-10-03 — Sesión 3 (fases 1–7)

- **Fase 1:** tiempo coyote, buffer de salto, paso bloqueado en el aire; prueba de humo con tiempos de juego/render/GPU; línea base de rendimiento (Media ≈ 60 fps a 720p, limitado por GPU).
- **Fase 2:** investigación de GunZ con fuentes públicas → objetivos propios (cancelaciones diseñadas como datos).
- **Fase 3:** dash, salto de pared, carrera por pared, escalada (modos personalizados), con movimiento guardado listo para red. Problemas resueltos: `lockable` de Git LFS dejaba los assets en solo lectura; el borde del que se baja contaba como pared (lo detectó el test de coyote).
- **Fase 4:** combate cuerpo a cuerpo por líneas de tiempo, combos, buffer, ventanas de cancelación, estocada, muñeco de entrenamiento.
- **Fase 5:** armas de fuego hitscan (pistola, rifle, escopeta), cambio de arma en la misma tabla de cancelaciones, canal de traza propio (los pawns ignoran Visibility).
- **Fase 6:** propuesta de pipeline de assets y sonda Blender → UE verificada (−Y en Blender → +Y en UE, como el maniquí).
- **Fase 7 (base):** vida compartida, muerte con ragdoll, reaparición.

## 2026-10-03 — Sesión 3b (feedback del dueño, todo a español)

- Feedback del dueño tras jugar: no se ve el ataque ni el dash, no hay tutorial; quiere mecánicas de GunZ + Hollow Knight, dash por doble toque con partículas, caballero boceto animado con esqueleto propio, sombreado oscuro tipo Hollow Knight, interfaz escalable, mapa tutorial con mini enemigos y mini jefe, **todo en español**, y trabajo autónomo continuo.
- `loop.ps1` del dueño lanza `claude -p` cada 30 min: se añadió un cerrojo de sesión en `PROGRESS.md` para que no trabajen dos sesiones a la vez (detener el proceso desde la sesión no estaba permitido).
- Migración completa al español de código, herramientas (`Herramientas/`), assets (`L_Pruebas`, `BP_SAEVUSPersonaje`...) y documentos (`Documentos/`). 49/49 tests, prueba de humo SUPERADA.
- Investigación de Hollow Knight (borrador; la búsqueda web llegó a su límite) y lista de mecánicas objetivo.

## 2026-10-03 — Sesión 3c (dash por doble toque y caballero boceto)

- **Tarea 3:** dash por doble toque de WASD (detector puro `FSAEVUSDetectorDobleToque`, cuatro direcciones), cámara bloqueada durante el dash, invulnerabilidad breve, estela de partículas. La prueba de humo hace un doble toque real.
- **Tarea 4:** caballero boceto generado en Blender por script (malla de piezas rígidas + esqueleto propio de 24 huesos), importado por script y animado por código (`USAEVUSComponenteAnimacionProcedural` sobre una malla poseable).
- Problema: el caballero no se veía en juego aunque los tests de huesos pasaban. Diagnóstico en la prueba de humo (actor esquelético normal y poseable sin animar como control; sin TAA se veían puntitos donde debía estar) → la raíz del FBX trae escala 100 y la animación escribía escala 1. Arreglo: conservar la escala de reposo. Test de regresión `SAEVUS.Animacion.EscalaReposo` y comprobación de tamaño en la prueba de humo.
- 59/59 tests, prueba de humo SUPERADA con capturas de correr, dash y ataque.
- **Tarea 5:** sombreado tipo Hollow Knight: `M_SAEVUS_PersonajeDibujado` (color plano + borde de luz fría por Fresnel + autoiluminación), instancias `MI_Caballero_*` como *override* del cuerpo, `M_PP_Contorno` (laplaciano de la inversa de la profundidad), viñeta y saturación desde `FSAEVUSAjusteCamara`. Problema: un `using namespace` a nivel de archivo en la animación chocaba en el unity build con otro `Derecha` al añadir un `.cpp`; ahora va dentro de cada función. 60/60 tests, humo SUPERADA, 63 fps de media.
- **Tarea 6:** mecánicas de Hollow Knight. Doble salto (prioridad suelo/coyote > pared > doble; cerca del suelo gana el buffer). Tajos hacia arriba y abajo elegidos por la inclinación de la cámara (adaptación a 3D de arriba/abajo + ataque), rebote que recarga dash y doble salto, retroceso del atacante, empuje y parada de golpe (`CustomTimeDilation` contada en tiempo real). Alma (11 por golpe) y curación mantenida con Q (33 de alma → 25 de vida en 1 s; se corta con golpes, saltos, ataques o dash). Invulnerabilidad de 1 s con parpadeo tras recibir un golpe. Tests que suponían "no hay salto en el aire" ahora fijan `MaxSaltosAereos = 0`; la prueba de humo comprueba que el golpe letal durante la invulnerabilidad se ignora. 73/73 tests, humo SUPERADA (≈ 59 fps).
- **Tarea 7:** enemigos. `ASAEVUSEnemigo` (cuerpo de formas del motor coloreado por datos, aviso visual, muerte que se encoge), `USAEVUSComponenteIAEnemigo` (estados, ataques en rotación, fases), `USAEVUSDatosEnemigo` con rastrero, saltador y Guardián. El jugador sale despedido al recibir un golpe. Problemas: `APawn::ShouldTakeDamage` exige modo de juego (los mundos de prueba no tienen) → ahora solo pide autoridad; la embestida fijando la velocidad a mano la frenaba el CMC por encima del tope → se sube el tope durante la carrera. La prueba de humo lleva al jugador junto al primer enemigo y comprueba que reacciona. 80/80 tests, humo SUPERADA.
- **Tarea 8:** `L_Tutorial` generado por script y puesto como mapa por defecto (la prueba de humo sigue en `L_Pruebas` por defecto). Piezas: subsistema de avisos, volúmenes (aviso, punto de control, caída con retorno), puertas de arena; el HUD dibuja los avisos. Iluminación oscura (luna + relleno sin sombras, niebla, exposición manual). Problemas: Git Bash convertía `-Mapa /Game/...` en `C:/Program Files/Git/Game/...` y el juego se quedó sin mapa (se cerró esa instancia; usar `MSYS_NO_PATHCONV=1`); dos luces direccionales competían por el sombreado forward (prioridad a la luna). 83/83 tests; prueba de humo SUPERADA en `L_Pruebas` y en `L_Tutorial`.
- **Tarea 9:** interfaz. `FSAEVUSEstadoInterfaz` (lógica probada sin pantalla) y `USAEVUSWidgetInterfaz` (UMG con el árbol construido en C++, pinceles redondeados de Slate, sin texturas): máscaras de vida, orbe de alma, arma y munición, aviso del tutorial, barra del jefe activo. El HUD la crea; el aviso ya no se dibuja con el canvas. Módulos `UMG`, `Slate`, `SlateCore`. 85/85 tests, humo SUPERADA.
- **Tarea 10a:** tests de obstáculos del tutorial (`SAEVUS.Tutorial.Obstaculos.*`) con entradas simuladas: bloque de 2,2 m, fosos de 8 m (dash), 12 m (rebote) y 9 m (carrera por pared), cornisa de 4,5 m. Todos superables; el bloque no se sube solo saltando. 90/90 tests.
- **Tarea 10b:** sonido boceto sintetizado por código (`SAEVUSSintesis`, `USAEVUSComponenteSonido`): siete efectos por datos, reproducidos con `USoundWaveProcedural` desde los eventos de combate, armas, vida, alma y movimiento (nuevo `AlIniciarAtaque`). 92/92 tests, humo SUPERADA.
- **Tarea 10c:** pulido de animación: estela del aguijón en la fase activa de los tajos (`ObtenerHoja` + partículas finas en `USAEVUSComponenteEfectosCombate`) y reacción del torso al recibir un golpe. 93/93 tests, humo SUPERADA.
- **Tarea 10d:** rendimiento con el juego completo: tutorial 67–68 fps, pruebas 57–63 fps (Media, 720p); el hilo de juego sube a 11–13 ms. `ProfileGPU` no da datos con el driver de la Vega 8 (2021).
- **Tarea 10e (y 2):** investigación con búsqueda web (StrategyWiki, critpoints.net, hollowknight.wiki): la guardia es la mitad de la "mariposa" de GunZ y falta en SAEVUS → tarea 11; ideas de dash por tipo de arma y peligros de rebote. `mecanicas-objetivo.md` al día.
