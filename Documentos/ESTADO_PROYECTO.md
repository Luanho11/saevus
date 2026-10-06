# ESTADO DEL PROYECTO SAEVUS

_Actualizado: 2026-10-06 (sesión 4, fase "Producción artística")_

## Objetivo actual

**Producción artística** (dueño, 2026-10-06): las imágenes de `assets/` son la referencia visual canónica (`REFERENCIAS_ARTE.md`); la dirección artística derivada está en `ARTE_SAEVUS.md`. Pasar de "prototipo con arte de boceto" a una vertical slice coherente: protagonista → animación → espada → combate → goblin → jefe → mazmorra → VFX → UI → audio (tareas 85–100 de `PROGRESS.md`). La jugabilidad GunZ de abajo se mantiene.

Antes: **Rehacer como GunZ** (decisión del dueño, 2026-10-04): la jugabilidad es la de GunZ (movimiento, dash sin espera ni límite, cámara siempre detrás, cuerpo que mira a la cámara); Hollow Knight queda para temática, arte y sombreado. Objetivo: un prototipo escalable que se pueda probar en un ejecutable, con animaciones mejores, tutorial de salas tipo mazmorra (referencia: mapa Dungeon de GunZ), menús limpios y, más adelante, goblins y jefes al estilo GunZ. Orden en `PROGRESS.md` (tareas 63–77).

**Al volver, empieza por `RESUMEN_PARA_EL_DUENO.md`.** Para jugar: abrir el proyecto y pulsar Play (arranca en `L_Tutorial`); teclas y técnicas en `controles.md`.

## Hecho

- **Entorno:** UE 5.8.3 (binario), VS 2022 Build Tools 17.14 / MSVC 14.44, SDK 10.0.26100, .NET 4.8 SDK, Blender 4.2.16, Git + LFS (`entorno.md`). Smart App Control desactivado.
- **Render para hardware modesto:** sin Lumen/Nanite/RT/VSM/MegaLights/distance fields, TAA, SSR, iluminación dinámica.
- **Movimiento** (`movimiento.md`): caminar/correr/esprint, salto con coyote y buffer, control aéreo, dash (suelo y aire), salto de pared, carrera por pared, escalada; listo para red con movimientos guardados.
- **Combate** (`combate.md`): golpe en abanico con su arco visible (lo que se ve es lo que golpea); ataques por línea de tiempo (tajos, remate, combo aéreo de tres con flotación, tajo en carrera, ataque cargado), combos con buffer, cancelaciones por datos (dash, salto, guardia), estocada; vida, muerte con ragdoll, reaparición; muñecos de entrenamiento. **Armas de fuego retiradas del juego por ahora** (código y Data Assets inactivos): solo la espada.
- **Dash por doble toque** de WASD (`movimiento.md`): cámara bloqueada durante el dash, invulnerabilidad breve, estela de partículas (`USAEVUSComponenteEstelaDash`, partículas por CPU en mallas instanciadas).
- **Caballero boceto** (`animacion.md`): malla + esqueleto propio de 24 huesos generados por script en Blender, importados por script; animación procedural en C++ (reposo, correr, aire, dash, pared, escalada, tajos por línea de tiempo, disparo, muerte, capa).
- **Sombreado tipo Hollow Knight** (`pipeline-assets.md`, Sombreado): material de personaje dibujado con borde de luz fría, instancias por pieza del caballero, contorno por post-proceso de profundidad, viñeta y saturación baja desde la cámara.
- **Dash como GunZ** (`movimiento.md`): sin enfriamiento ni límite por salto (por datos: `Enfriamiento` 0, `MaxDashesAereos` −1), con gravedad durante el dash.
- **Mecánicas** (`movimiento.md`, `combate.md`): doble salto (con buffer cerca del suelo), tajo hacia arriba por la inclinación de la cámara (el tajo hacia abajo y el rebote se retiraron), retroceso y parada de golpe, alma al golpear y curación mantenida, invulnerabilidad con parpadeo tras recibir un golpe.
- **Goblins** (`enemigos.md`, GunZ): goblin de cuchillo, de maza y hondero y el jefe Rey Goblin, sin daño por contacto y con el arma que brilla al avisar; en el tutorial, la Mazmorra y `L_Pruebas`. Los enemigos de antes siguen en los datos, fuera de los mapas.
- **IA de enemigos:** IA de estados (patrulla, persecución, aviso, ataque, recuperación, aturdido, muerte), contacto que hace daño, siete tipos por datos (rastrero, saltador, escupidor, revoloteador volador, acorazado con escudo, mini jefe Guardián y jefe volador Madre Alada, con dos fases), el jugador sale despedido al recibir golpes.
- **Mapa tutorial** `L_Tutorial` (`tutorial.md`), mapa por defecto: Mazmorra del Umbral: diez salas cerradas tipo Dungeon de GunZ (moverse, saltar, dash, paredes, carrera por pared, sala de armas, capilla con banco, gran salón con balcones y columnas, fin y reto), avisos en pantalla, carteles, puntos de control, fosos con retorno, puertas de arena; iluminación oscura con exposición manual.
- **Secretos** (`mazmorra.md`): pared falsa que cae a tajos, fragmentos de máscara (dos = una máscara de vida más, guardados).
- **Bancos** (`mazmorra.md`): sentarse con E cura, fija el punto de control y guarda (Mazmorra y tutorial).
- **Mazmorra** (`mazmorra.md`): `L_Mazmorra`, práctica libre tipo Dungeon de GunZ (salón central con balcones y columnas; galería de fosos con pared falsa y sala secreta; pozo con chimenea y ascensor; terraza con palanca y compuerta); se llega desde el final del tutorial. Sustituye a la Zona 1 (retirada).
- **Menús** (`interfaz.md`): menú principal al arrancar el tutorial (Jugar, Continuar desde el punto de control guardado, opciones) y menú de pausa con mapa de la zona (solo lo explorado); sensibilidad y punto de control guardados entre sesiones.
- **Interfaz** (`interfaz.md`): UMG construida en C++ sin assets (máscaras de vida, orbe de alma, arma y munición, avisos del tutorial, barra del jefe), con estado calculado aparte y probado.
- **Sonido boceto** (`audio.md`): efectos sintetizados por código (tajo, golpe, disparo, daño, curación, dash, doble salto) disparados por eventos.
- **Guardia** (GunZ, `combate.md`): bloqueo frontal por filtro de daño, cancela el tajo por datos (tajo → guardia), empuje y chispas al bloquear.
- **Investigación:** `investigacion/gunz.md`, `investigacion/hollow-knight.md` (borrador), `investigacion/mecanicas-objetivo.md`.
- **Pipeline de assets** decidido y sonda Blender → UE verificada (`pipeline-assets.md`).
- **Todo en español:** código, herramientas, assets, documentos.
- **Equilibrio** (`equilibrio.md`): tablas de daño/s por técnica y de enemigos calculadas desde los datos en `SAEVUS.Equilibrio.*`, con reglas de diseño comprobadas.
- **Rendimiento** (`rendimiento.md`): Media a 720p en la Vega 8: tutorial 67–68 fps, mapa de pruebas 57–63 fps; el hilo de juego (11–13 ms) es el próximo cuello.

## Verificación

- **Todo de una vez:** `Herramientas\Verificar-Arranque.ps1 -ConJuego` (compilación, assets, tests, verificación, prueba de humo y un recorrido Mazmorra → paso → tutorial). Los logs van a `Saved\Logs\Verificacion\`; cada script conserva las 5 copias anteriores de cada uno (`Limpiar-Logs.ps1`).

- `Herramientas\Verificar-Arranque.ps1`: compilación, creación de assets, 128/128 tests `SAEVUS.*`, verificación de assets — todo superado.
- `Herramientas\Ejecutar-PruebaHumo.ps1 -Escalabilidad 1`: SUPERADA (cuerpo animado visible, correr 550, esprint 850, cima 142,8 cm, dash por doble toque real a 1600 con estela, ataque 0,5 s, 1 disparo, golpe → 90/100, muerte, reaparición a 100/100). Capturas en `Saved/Screenshots/WindowsEditor/SAEVUS_PruebaHumo_*.png`.

## Arquitectura (resumen)

`ASAEVUSPersonaje` compone `USAEVUSComponenteMovimiento` (CMC), `USAEVUSComponenteCombate`, `USAEVUSComponenteArmas`, `USAEVUSComponenteVida` y `USAEVUSComponenteCamara` sobre un brazo de cámara. Valores en structs de ajuste y Data Assets; los Blueprints y el mapa los crea `Herramientas/ScriptsEditor/saevus_crear_assets.py`. Detalle en `arquitectura.md`.

## Decisiones técnicas

- UE 5.8 binario; VS 2022 (no VS 2026).
- CMC, no Mover; peticiones por movimientos guardados.
- Dash como estado dentro de caminar/caer; carrera por pared y escalada como `MOVE_Custom`.
- Cancelaciones como datos por ataque; la animación se deriva de la línea de tiempo.
- Canal de traza propio "Arma".
- Git LFS sin `lockable` (no hay servidor de bloqueos).
- Prototipo de animación procedural en C++ sobre esqueleto propio generado en Blender.

## Problemas conocidos

- Máquina por debajo de lo recomendado (7 GB RAM, Vega 8): recompilación completa > 10 min, la primera ejecución con render es "en frío".
- `loop.ps1` del dueño puede lanzar otra sesión de Claude; se coordina con el cerrojo de `PROGRESS.md`.
- `L_Pruebas` sigue con la iluminación clara de pruebas; la oscura está en `L_Tutorial`.
- Empaquetado (tarea 14) bloqueado: el proceso muere por falta de memoria; lanzar `Empaquetar.ps1` con la máquina libre.

## Siguiente paso

En pausa: el dueño abrió el editor. La tarea 61 (esquirlas) está en `git stash`; para seguir, cerrar el editor, `git stash pop` y verificar. Después, 62 (revisión de código); después, tareas propias (ver la cola en `PROGRESS.md`).
