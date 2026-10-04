# SAEVUS — PROGRESO (cola de tareas para trabajo autónomo)

## ⚠ CERROJO DE SESIÓN — LEER PRIMERO

**Estado: trabajando (tarea 12)**
**Último latido: 2026-10-03 21:05** (la sesión activa lo actualiza en cada commit)

Si eres una iteración de `loop.ps1` (`claude -p ...`) y el último latido tiene **menos de 45 minutos**:
- otra sesión de Claude está trabajando en este repo ahora mismo;
- **no edites, no compiles, no hagas commit**; termina tu turno de inmediato sin cambios;
- si ya creaste archivos o cambios sin commitear en este turno, déjalos como están y termina (la sesión interactiva los revisará).

Si el latido tiene **45 minutos o más**, la sesión interactiva terminó: cambia el estado a "SESIÓN DEL LOOP ACTIVA", pon tu propio latido y continúa con la primera tarea pendiente de abajo.

## Reglas (del dueño, 2026-10-03)
- **Todo en español**: documentos, código (identificadores, comentarios, logs), variables, respuestas. Prefijo de clases `SAEVUS`.
- Prototipos y bocetos, no acabados finales; todo modificable y escalable (datos en structs/Data Assets).
- Sin assets externos. Hardware modesto (Vega 8, 7 GB RAM). Sin copiar código ni fórmulas de GunZ.
- Validar cada bloque: compilar, tests, assets, smoke test; commit pequeño.

## Decisiones del dueño (2026-10-03)
- Mecánicas: todas las de GunZ combinadas con Hollow Knight (investigar ambos).
- Dash con **doble toque** de WASD (no Ctrl), con partículas como GunZ; cámara bloqueada durante el dash como en GunZ; ritmo de GunZ.
- Las cancelaciones son técnicas, con animaciones fluidas.
- Personaje: animado en 3D (tipo Genshin), boceto de caballero humanoide estilo Hollow Knight, generado (no importado), **esqueleto personalizado** básico desde el principio, pulido en animaciones y efectos, no en diseño.
- Sombreado tipo Hollow Knight: oscuro, aspecto dibujado en 3D.
- Interfaz moldeable y escalable, bocetos mejorables, sin dirección visual compleja todavía.
- Tutorial: mapa pequeño que enseñe a jugar, con mini enemigos y un mini jefe, simple pero escalable.
- Presupuestos, formatos y fuentes externas: decide Claude, siempre cambiable.

## Cola de tareas (prioridad de arriba abajo)
1. [x] ~~Migración al español: CLAUDE.md, Docs, código (clases, funciones, variables, comentarios), scripts, assets regenerados, tests verdes.~~
2. [x] ~~Investigación Hollow Knight + ampliación GunZ → `Docs/investigacion/` y plan de mecánicas.~~
3. [x] ~~Dash por doble toque WASD, cámara bloqueada, invulnerabilidad breve, partículas de dash.~~
4. [x] ~~Caballero boceto: malla + esqueleto personalizado generados en Blender por script, importados; animación procedural en C++ (reposo, correr, saltar, caer, dash, wall run, escalar, ataques sincronizados con la línea de tiempo, disparo, muerte).~~
5. [x] ~~Sombreado tipo Hollow Knight (materiales oscuros, máscara clara, contorno por post-proceso barato).~~
6. [x] ~~Mecánicas Hollow Knight: doble salto, ataque hacia arriba/abajo, rebote (pogo), retroceso al golpear, almas + curación.~~
7. [x] ~~Enemigos simples (IA de estados) + mini jefe, datos en Data Assets.~~
8. [x] ~~Mapa tutorial `L_Tutorial`: secciones con carteles y avisos en pantalla, mini enemigos, mini jefe al final.~~
9. [x] ~~Interfaz UMG en C++ escalable: vida, almas, arma/munición, avisos de tutorial, barra del jefe.~~
10. [ ] Tareas propias (Claude), en este orden:
    - 10a. [x] ~~Obstáculos del tutorial comprobados con tests de entradas simuladas (`SAEVUS.Tutorial.Obstaculos.*`).~~
    - 10b. [x] ~~Sonido boceto generado por código (golpes, dash, salto, daño) con `USoundWaveProcedural` o síntesis simple; volumen por categorías.~~
    - 10c. [x] ~~Pulido de animación: anticipación y estela del arma en los tajos, aterrizaje del doble salto, giro de la cabeza hacia el objetivo, reacción al recibir golpe.~~
    - 10d. [x] ~~Rendimiento: medir el tutorial con enemigos en la Vega 8, escalabilidad del contorno y de partículas, presupuesto por sección.~~
    - 10e. [x] ~~Investigación Hollow Knight/GunZ ampliada (tarea 2) en cuanto haya búsqueda web.~~
    - 10f. [ ] Hilo de juego: la animación procedural solo se actualiza para mallas visibles (y a menos frecuencia lejos de la cámara); la IA de enemigos lejanos en reposo, a menos frecuencia.
11. [x] ~~Guardia (GunZ): mantener clic derecho con el arma cuerpo a cuerpo → postura de guardia que bloquea daño frontal (golpes de enemigos y disparos dentro de un cono), con retroceso y chispa al bloquear; la guardia cancela la recuperación del tajo (nueva acción de cancelación `Guardia` en la tabla de cada ataque); no se puede atacar ni curar en guardia; movimiento reducido. Datos en un struct; tests; aviso en el tutorial (sección de combate).~~
12. [ ] Peligros y rebote en el tutorial (Hollow Knight): pinchos (`ASAEVUSPeligro`: daño + retorno como la caída, se puede rebotar en ellos con el tajo hacia abajo) en el foso de rebote, y un reto final de técnica (cadena pared → doble salto → dash → rebote).

## Estado del turno
- Completado: tarea 11 (guardia; 96/96 tests; humo SUPERADA con etapa de guardia).
- A medias: —
- Siguiente paso exacto: tarea 10f (hilo de juego) o 12 (peligros con rebote). Orden elegido: 12 primero (jugabilidad), luego 10f.
