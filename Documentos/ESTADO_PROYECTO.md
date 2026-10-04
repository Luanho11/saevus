# ESTADO DEL PROYECTO SAEVUS

_Actualizado: 2026-10-03 (sesión 3c)_

## Objetivo actual

Que el juego **se vea y se sienta**: dash por doble toque con partículas, caballero boceto animado con esqueleto propio, sombreado tipo Hollow Knight, mecánicas de Hollow Knight, enemigos y mini jefe, mapa tutorial e interfaz. Orden y detalle en `PROGRESS.md` y `hoja-de-ruta.md`.

**Al volver, empieza por `RESUMEN_PARA_EL_DUENO.md`.** Para jugar: abrir el proyecto y pulsar Play (arranca en `L_Tutorial`); teclas y técnicas en `controles.md`.

## Hecho

- **Entorno:** UE 5.8.3 (binario), VS 2022 Build Tools 17.14 / MSVC 14.44, SDK 10.0.26100, .NET 4.8 SDK, Blender 4.2.16, Git + LFS (`entorno.md`). Smart App Control desactivado.
- **Render para hardware modesto:** sin Lumen/Nanite/RT/VSM/MegaLights/distance fields, TAA, SSR, iluminación dinámica.
- **Movimiento** (`movimiento.md`): caminar/correr/esprint, salto con coyote y buffer, control aéreo, dash (suelo y aire), salto de pared, carrera por pared, escalada; listo para red con movimientos guardados.
- **Combate** (`combate.md`): ataques por línea de tiempo (tajos, remate, tajo aéreo), combos con buffer, cancelaciones por datos (dash, salto, cambio de arma), estocada; armas de fuego hitscan (pistola, rifle, escopeta), cambio de arma, recarga; vida, muerte con ragdoll, reaparición; muñecos de entrenamiento.
- **Dash por doble toque** de WASD (`movimiento.md`): cámara bloqueada durante el dash, invulnerabilidad breve, estela de partículas (`USAEVUSComponenteEstelaDash`, partículas por CPU en mallas instanciadas).
- **Caballero boceto** (`animacion.md`): malla + esqueleto propio de 24 huesos generados por script en Blender, importados por script; animación procedural en C++ (reposo, correr, aire, dash, pared, escalada, tajos por línea de tiempo, disparo, muerte, capa).
- **Sombreado tipo Hollow Knight** (`pipeline-assets.md`, Sombreado): material de personaje dibujado con borde de luz fría, instancias por pieza del caballero, contorno por post-proceso de profundidad, viñeta y saturación baja desde la cámara.
- **Mecánicas de Hollow Knight** (`movimiento.md`, `combate.md`): doble salto (con buffer cerca del suelo), tajos hacia arriba y abajo por la inclinación de la cámara, rebote (pogo) que recarga dash y doble salto, retroceso y parada de golpe, alma al golpear y curación mantenida, invulnerabilidad con parpadeo tras recibir un golpe.
- **Enemigos** (`enemigos.md`): IA de estados (patrulla, persecución, aviso, ataque, recuperación, aturdido, muerte), contacto que hace daño, tres tipos por datos (rastrero, saltador, mini jefe Guardián con dos fases), el jugador sale despedido al recibir golpes.
- **Mapa tutorial** `L_Tutorial` (`tutorial.md`), mapa por defecto: diez secciones (moverse, saltar, dash, paredes, carrera por pared, combate, rebote, alma, arena de mini enemigos, Guardián), avisos en pantalla, carteles, puntos de control, fosos con retorno, puertas de arena; iluminación oscura con exposición manual.
- **Interfaz** (`interfaz.md`): UMG construida en C++ sin assets (máscaras de vida, orbe de alma, arma y munición, avisos del tutorial, barra del jefe), con estado calculado aparte y probado.
- **Sonido boceto** (`audio.md`): efectos sintetizados por código (tajo, golpe, disparo, daño, curación, dash, doble salto) disparados por eventos.
- **Guardia** (GunZ, `combate.md`): bloqueo frontal por filtro de daño, cancela el tajo por datos (tajo → guardia), empuje y chispas al bloquear.
- **Investigación:** `investigacion/gunz.md`, `investigacion/hollow-knight.md` (borrador), `investigacion/mecanicas-objetivo.md`.
- **Pipeline de assets** decidido y sonda Blender → UE verificada (`pipeline-assets.md`).
- **Todo en español:** código, herramientas, assets, documentos.
- **Rendimiento** (`rendimiento.md`): Media a 720p en la Vega 8: tutorial 67–68 fps, mapa de pruebas 57–63 fps; el hilo de juego (11–13 ms) es el próximo cuello.

## Verificación

- **Todo de una vez:** `Herramientas\Verificar-Arranque.ps1 -ConJuego` (compilación, assets, tests, verificación, prueba de humo y recorrido del tutorial).

- `Herramientas\Verificar-Arranque.ps1`: compilación, creación de assets, 107/107 tests `SAEVUS.*`, verificación de assets — todo superado.
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
- Los mapas siguen con la iluminación clara de pruebas (la oscura llega con `L_Tutorial`). Sin tutorial, enemigos ni interfaz de juego.

## Siguiente paso

Tarea 10 de `PROGRESS.md`: tareas propias (ver la cola en `PROGRESS.md`).
