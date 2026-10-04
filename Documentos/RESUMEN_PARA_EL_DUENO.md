# Resumen para el dueño (trabajo autónomo del 2026-10-03/04)

Todo lo que pediste en tu mensaje está hecho, más lo que fui añadiendo al terminar la cola. Este documento es el punto de entrada: qué hay, cómo probarlo, qué decidí y qué queda para ti.

## Cómo probarlo

1. Abre `SAEVUS.uproject` (arranca en `L_Tutorial`) y pulsa **Play**.
2. Teclas y técnicas: `controles.md`. Lo básico: WASD, Espacio (doble salto), **doble toque** de dirección (dash), clic izquierdo (tajos; mira arriba/abajo para tajos direccionales), **clic derecho** (guardia), **Q** mantenida (curar), **P** (pausa), F6 (datos de depuración).
3. El tutorial enseña todo por secciones y acaba en el mini jefe; tras él hay un reto opcional.

Si algo no compila o falta un asset: `powershell -ExecutionPolicy Bypass -File Herramientas\Verificar-Arranque.ps1` lo reconstruye todo (compila, crea assets, 105 tests, verificación).

## Qué hay (todo boceto, todo cambiable por datos)

| Pediste | Estado |
|---|---|
| Mecánicas de GunZ + Hollow Knight, investigadas | Hecho. Investigación en `investigacion/` (con búsqueda web); lista viva en `investigacion/mecanicas-objetivo.md` |
| Dash por doble toque WASD con partículas, cámara bloqueada, ritmo GunZ | Hecho. Con arma de fuego es una voltereta más lenta, como en GunZ |
| Ataque visible, cancelaciones como técnica, animación fluida | Hecho: tajos animados con estela, combo de 3, tajos arriba/abajo, cancelaciones por datos (dash, salto, cambio de arma, **guardia**: la "mariposa" de GunZ) |
| Caballero animado en 3D tipo Genshin, generado, estilo Hollow Knight, esqueleto propio | Hecho: modelado por script en Blender, esqueleto propio de 25 huesos, animación procedural (correr, saltar, paredes, dash, tajos, guardia, curar, golpe recibido, muerte; la capa ondea, la cabeza mira al enemigo) |
| Sombreado tipo Hollow Knight, oscuro, dibujado | Hecho: material plano con borde de luz fría, contorno por post-proceso, tutorial nocturno |
| Interfaz moldeable y escalable, bocetos | Hecho: máscaras de vida, orbe de alma, arma, avisos, barra del jefe, pausa, pantalla de muerte; todo en C++ con ajustes en un struct |
| Tutorial con mini enemigos y mini jefe, escalable | Hecho: 10 secciones + reto, 4 tipos de enemigo (rastrero, saltador, escupidor, Guardián con 2 fases), puertas de arena, puntos de control, pinchos |
| Todo en español | Hecho: código, assets, documentos, mensajes |

También: doble salto, rebote (pogo, también en pinchos), alma y curación, contraataque (parry), proyectiles que se paran con la guardia y se devuelven con un tajo, retroceso y parada de golpe, invulnerabilidad con parpadeo, sonido boceto sintetizado (sin archivos de audio), decorado de ruinas generado en Blender.

## Capturas de referencia (`capturas/`)

`Correr.png` (inicio del tutorial), `Dash.png`, `Ataque.png` (estela del tajo), `Guardia.png`, `Enemigos.png` (arena), `Pausa.png`, `Muerte.png`.

## Decisiones que tomé (todas cambiables)

- **Arriba/abajo en 3D:** los tajos direccionales salen de la **inclinación de la cámara** (mirar arriba ≥ 30°, abajo ≤ −40° en el aire), porque W/S ya mueven. Umbrales en `DA_RepertorioMelee_Base`.
- **Guardia en clic derecho y pausa en P** (Esc detiene el Play en el editor).
- **Curar más rápido que Hollow Knight** (1 s en vez de ~1,5 s) por el ritmo de GunZ.
- **Animación procedural** en vez de clips: nunca se desincroniza con la jugabilidad y se ajusta por datos; la sustituirán animaciones hechas a mano cuando haya diseño final.
- **Todo generado** (Blender por script, sonido sintetizado, formas del motor): cero assets externos.
- Números de equilibrio de primera pasada en Data Assets (`/Game/SAEVUS/Datos/`) y documentados en `combate.md`, `enemigos.md`, `movimiento.md`.

## Lo que queda para ti

- **Empaquetado bloqueado:** `Herramientas\Empaquetar.ps1` existe, pero el sistema lo detuvo por falta de memoria (7 GB de RAM no dan para compilar el juego y cocinar a la vez). Ejecútalo tú con el resto de programas cerrados si quieres un .exe; también sirve para medir el rendimiento real (dentro del editor: tutorial ≈ 60–68 fps a 720p en Media).
- **Preguntas abiertas:**
  - ¿Te gusta la inclinación de cámara para los tajos arriba/abajo, o prefieres una tecla?
  - ¿Ritmo del dash, la guardia y la curación? (todo en Data Assets).
  - Dirección de arte cuando quieras pasar de bocetos: personaje final, iconos de interfaz, escenario.
- **Siguiente en la cola** (`PROGRESS.md`): lo que propongo después; puedes reordenarlo.

## Dónde mirar

`ESTADO_PROYECTO.md` (estado), `REGISTRO_SESIONES.md` (qué se hizo y qué problemas hubo), `arquitectura.md` (cómo está montado), `PROGRESS.md` (cola). Historial de git: un commit por tarea, todos con prefijo `auto:`.
