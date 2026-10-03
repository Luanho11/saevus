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
