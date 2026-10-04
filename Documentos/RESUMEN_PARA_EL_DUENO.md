# Resumen para el dueño (trabajo autónomo del 2026-10-03/04)

Todo lo que pediste en tu mensaje está hecho. Al terminar la cola seguí con tareas propias: equilibrio, munición, enemigo volador, guardado, menús, la Zona 1 con su jefe, bancos y mapa. Este documento es el punto de entrada: qué hay, cómo probarlo, qué decidí y qué queda para ti.

## Cómo probarlo

1. Abre `SAEVUS.uproject` (arranca en `L_Tutorial`) y pulsa **Play**. Sale el menú principal: **Jugar**, o **Continuar** desde el último punto de control si ya jugaste.
2. Teclas y técnicas en `controles.md`. Lo básico:
   - **Moverse:** WASD; Espacio (con doble salto); **doble toque** de una dirección para el dash.
   - **Combate:** clic izquierdo para los tajos (mira arriba o abajo para los direccionales; mantén y suelta para el ataque cargado); **clic derecho** para la guardia.
   - **Curar:** **Q** mantenida.
   - **Armas y bancos:** 1–4 o la rueda cambian de arma; R recarga; **E** sienta en los bancos.
   - **Pausa y depuración:** **P** abre la pausa (con el mapa de la zona); F6 muestra los datos de depuración.
3. El tutorial ("Las Afueras") enseña todo por secciones y acaba en el mini jefe Guardián. Después hay:
   - un reto opcional;
   - a la derecha del cartel de fin, el paso a la **Zona 1, "Ruinas del Paso"** (`zona1.md`): desniveles, tres encuentros, bancos, una arena, un atajo con palanca y el jefe volador **Madre Alada**.

Si algo no compila o falta un asset, `powershell -ExecutionPolicy Bypass -File Herramientas\Verificar-Arranque.ps1` lo reconstruye todo: compila, crea assets, pasa los 122 tests y verifica. Con `-ConJuego` además juega la prueba de humo y recorre solos el tutorial y la Zona 1 (unos 15 minutos en esta máquina).

## Qué hay (todo boceto, todo cambiable por datos)

| Pediste | Estado |
|---|---|
| Mecánicas de GunZ + Hollow Knight, investigadas | Hecho. Investigación en `investigacion/` (con búsqueda web); lista viva en `investigacion/mecanicas-objetivo.md` |
| Dash por doble toque WASD con partículas, cámara bloqueada, ritmo GunZ | Hecho. Con arma de fuego es una voltereta más lenta, como en GunZ |
| Ataque visible, cancelaciones como técnica, animación fluida | Hecho: tajos animados con estela, combo de 3, tajos arriba y abajo, combo aéreo de 3 con flotación, tajo en carrera (atacar en el dash), ataque cargado (giro), cancelaciones por datos (dash, salto, cambio de arma y **guardia**, la "mariposa" de GunZ) |
| Caballero animado en 3D tipo Genshin, generado, estilo Hollow Knight, esqueleto propio | Hecho: modelado por script en Blender, esqueleto propio de 25 huesos y animación procedural (correr, saltar, paredes, dash, tajos, guardia, curar, sentarse, golpe recibido, muerte); la capa ondea y la cabeza mira al enemigo |
| Sombreado tipo Hollow Knight, oscuro, dibujado | Hecho: material plano con borde de luz fría, contorno por post-proceso, mapas nocturnos |
| Interfaz moldeable y escalable, bocetos | Hecho, todo en C++ con ajustes en structs: máscaras de vida, orbe de alma, arma y munición, avisos, título de zona, barra del jefe, aviso de interacción, menú principal, pausa con mapa, pantalla de muerte, números de daño (depuración) |
| Tutorial con mini enemigos y mini jefe, escalable | Hecho: 10 secciones + reto, puertas de arena, puntos de control, pinchos, bancos y lección de armas |
| Todo en español | Hecho: código, assets, documentos, mensajes |

**Enemigos** (`enemigos.md`), siete tipos por datos con cuerpos generados en Blender:

- rastrero, saltador y escupidor (proyectiles que se paran con la guardia o se devuelven con un tajo);
- **revoloteador**, volador que cae en picado;
- **acorazado**, con un escudo que bloquea de frente: se rodea, se le salta encima o se rompe con el giro cargado;
- **Guardián**, mini jefe con dos fases;
- **Madre Alada**, jefe volador con picados y abanicos de proyectiles, también con dos fases.

**Más mecánicas:**

- doble salto, rebote (pogo, también en pinchos), alma y curación, contraataque (parry);
- retroceso y parada de golpe, sacudida de cámara;
- munición de reserva que se rellena con la espada (ver la pregunta de abajo);
- bancos para sentarse (curan, rellenan munición y guardan) y palancas que abren atajos.

**Sistemas:**

- guardado entre sesiones (sensibilidad y punto de control; la calidad la guarda el motor);
- sonido boceto sintetizado (sin archivos de audio);
- tablas de equilibrio calculadas por los tests (`equilibrio.md`).

## Capturas de referencia (`capturas/`)

- **Tutorial:** `Correr.png` (inicio), `Dash.png`, `Ataque.png` (estela del tajo), `Guardia.png`, `Enemigos.png` (arena).
- **Menús:** `Pausa.png` (con el mapa de la zona), `Menu.png` (menú principal), `Muerte.png`.
- **Armas:** `Armas.png` (disparo a la diana alta, munición `cargador / reserva`).
- **Zona 1:** `Zona1_Titulo.png` (título de zona al entrar), `Zona1_Banco.png` (banco y compuerta del atajo).

## Decisiones que tomé (todas cambiables)

- **Arriba y abajo en 3D:** los tajos direccionales salen de la **inclinación de la cámara** (mirar arriba ≥ 30°; abajo ≤ −40°, en el aire), porque W y S ya mueven. Los umbrales están en `DA_RepertorioMelee_Base`.
- **Teclas:** guardia en clic derecho, pausa en P (Esc detiene el Play en el editor), bancos con E.
- **Curar más rápido que en Hollow Knight** (1 s en vez de ~1,5 s), por el ritmo de GunZ.
- **Animación procedural** en vez de clips: nunca se desincroniza con la jugabilidad y se ajusta por datos. La sustituirán animaciones hechas a mano cuando haya diseño final.
- **Todo generado** (Blender por script, sonido sintetizado, formas del motor): cero assets externos.
- **Armas limitadas por munición, no por daño:** la reserva vuelve con los golpes de espada, los bancos y los puntos de control.
- **Los bancos guardan; los puntos de control también fijan dónde reapareces.** "Continuar" lleva al último punto guardado del mapa.
- **Números de equilibrio de primera pasada** en Data Assets (`/Game/SAEVUS/Datos/`). Los tests comprueban unas reglas de diseño: avisos de al menos 0,25 s, el jugador aguanta tres golpes, los enemigos menores caen en uno o dos combos y los jefes aguantan al menos cuatro.

## Lo que queda para ti

- **Empaquetado bloqueado:** `Herramientas\Empaquetar.ps1` existe, pero el sistema lo detuvo por falta de memoria (7 GB de RAM no dan para compilar el juego y cocinar a la vez). Ejecútalo tú con el resto de programas cerrados si quieres un .exe. También sirve para medir el rendimiento real: dentro del editor, el tutorial va a ≈ 60 fps a 720p en Media, y el "hilo de juego" del editor engaña (`rendimiento.md`).
- **Preguntas abiertas:**
  - ¿Te gusta la inclinación de cámara para los tajos arriba y abajo, o prefieres una tecla?
  - ¿Te parece bien el ritmo del dash, la guardia y la curación? (Todo está en Data Assets.)
  - **Armas de fuego frente a espada:** con la munición limitada, ¿te convence, o prefieres bajarles el daño?
  - Los jefes caen en 6–7 s de combo puro (30–60 s reales esquivando). ¿Los quieres más largos?
  - Dirección de arte cuando quieras pasar de bocetos: personaje final, iconos de interfaz, escenario.
- **Siguiente en la cola** (`PROGRESS.md`): lo que propongo después; puedes reordenarlo.

## Dónde mirar

- `ESTADO_PROYECTO.md`: estado actual.
- `REGISTRO_SESIONES.md`: qué se hizo y qué problemas hubo.
- `arquitectura.md`: cómo está montado.
- `PROGRESS.md`: la cola de tareas.
- Por tema: `combate.md`, `enemigos.md`, `movimiento.md`, `interfaz.md`, `audio.md`, `animacion.md`, `tutorial.md`, `zona1.md`, `equilibrio.md`, `rendimiento.md`.

En el historial de git hay un commit por tarea, todos con el prefijo `auto:`.
