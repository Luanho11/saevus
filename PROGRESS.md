# SAEVUS — PROGRESO (cola de tareas para trabajo autónomo)

## ⚠ CERROJO DE SESIÓN — LEER PRIMERO

**Estado: trabajando (tarea 39)**
**Último latido: 2026-10-04 02:15** (la sesión activa lo actualiza en cada commit)

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
    - 10f. [x] ~~Hilo de juego: la animación procedural solo se actualiza para mallas visibles (y a menos frecuencia lejos de la cámara); la IA de enemigos lejanos en reposo, a menos frecuencia.~~ (sin cambio medible; ver rendimiento.md)
11. [x] ~~Guardia (GunZ): mantener clic derecho con el arma cuerpo a cuerpo → postura de guardia que bloquea daño frontal (golpes de enemigos y disparos dentro de un cono), con retroceso y chispa al bloquear; la guardia cancela la recuperación del tajo (nueva acción de cancelación `Guardia` en la tabla de cada ataque); no se puede atacar ni curar en guardia; movimiento reducido. Datos en un struct; tests; aviso en el tutorial (sección de combate).~~
12. [x] ~~Peligros y rebote en el tutorial (Hollow Knight): pinchos (`ASAEVUSPeligro`: daño + retorno como la caída, se puede rebotar en ellos con el tajo hacia abajo) en el foso de rebote, y un reto final de técnica (cadena pared → doble salto → dash → rebote).~~ (el reto final de técnica queda para la 13)
13. [x] ~~Reto de técnica al final del tutorial (opcional, tras el Guardián): pasillo con pared → doble salto → dash → rebote en pinchos → carrera por pared, con su test de obstáculo.~~
14. [ ] Medición real: empaquetar Development para Win64 (`RunUAT BuildCookRun`) en `Saved/Empaquetado/`, script `Herramientas/Empaquetar.ps1`, y medir el tutorial con `stat unit`/Unreal Insights fuera del editor. **BLOQUEADA (2026-10-03):** el primer intento (`Herramientas/Empaquetar.ps1`) lo detuvo el sistema por falta de memoria al compilar el juego con 7 GB de RAM. No relanzar en autónomo; pedir al dueño que lo ejecute con el resto de programas cerrados.
15. [x] ~~Sonidos de enemigos (aviso, embestida, muerte) con la síntesis existente.~~
16. [x] ~~Dash según el arma (GunZ): con aguijón, más rápido y con estela; con arma de fuego, voltereta más lenta (ajuste del dash por tipo de arma).~~
17. [x] ~~Cabeza que mira al enemigo cercano o al objetivo de la cámara (animación), con límites de giro.~~
18. [x] ~~Contraataque (parry, Hollow Knight/GunZ): guardia pulsada justo antes del golpe → el atacante queda aturdido y el jugador gana alma; ventana por datos.~~
19. [x] ~~Enemigo a distancia (escupidor): nuevo tipo de ataque `Proyectil` (actor `ASAEVUSProyectil` simple, bloqueable con la guardia, se puede devolver con un tajo), Data Asset y uno en la arena del tutorial; tests.~~
20. [x] ~~Menú de pausa básico (UMG en C++): Esc pausa; continuar, reiniciar en el último punto de control, salir; opción de escalabilidad (Baja/Media/Alta) y sensibilidad del ratón.~~
21. [x] ~~Pantalla de muerte simple (fundido a negro + "Has caído" durante la espera de reaparición) y fundido de entrada al reaparecer.~~
22. [x] ~~Vista previa del caballero mejorada: segundo boceto de Blender con más volumen (capa con 3 huesos, cuernos más marcados, aguijón con guarda), manteniendo los mismos nombres de hueso; regenerar e importar; tests de animación verdes.~~
23. [x] ~~Revisión de código general (limpieza, comentarios, números mágicos que queden) y actualización de `arquitectura.md` con todos los sistemas nuevos.~~
24. [x] ~~Cuerpos de enemigos generados en Blender (como el caballero, piezas rígidas sobre un esqueleto pequeño) en vez de formas del motor, con animación procedural sencilla (respirar, avisar, atacar).~~
25. [x] ~~Escenario boceto del tutorial: piezas modulares generadas en Blender (bordes rotos, columnas, escombros) colocadas por el script del mapa; presupuesto de triángulos.~~
26. [x] ~~Resumen para el dueño al volver: `Documentos/RESUMEN_PARA_EL_DUENO.md` con qué hay, cómo probarlo (controles, tutorial), capturas de referencia, decisiones tomadas y preguntas abiertas (empaquetado bloqueado, dirección de arte, números de equilibrio).~~
27. [x] ~~Recorrido automático del tutorial completo en el juego real (prueba de humo `-SAEVUSRecorrido`): teletransporta al jugador a cada sección y comprueba avisos, puertas y que el jefe aparece; captura por sección.~~
28. [x] ~~Añadir el recorrido a `Verificar-Arranque.ps1` como paso opcional (`-ConJuego`: prueba de humo + recorrido), para tener una sola orden que lo compruebe todo.~~
29. [x] ~~Combo aéreo (GunZ): dos o tres tajos en el aire, con flotación breve (gravedad reducida durante el tajo aéreo) por datos; tests.~~
30. [x] ~~Tajo en carrera (GunZ): atacar durante el dash da un tajo propio que conserva la velocidad del dash y hace más daño; tests.~~
31. [x] ~~Ataque cargado (Hollow Knight, arte del aguijón): mantener ataque ~0,8 s y soltar → tajo giratorio amplio; aviso visual de carga; tests.~~
32. [x] ~~Destello del enemigo al recibir un golpe (cuerpo y máscara en blanco un instante); test.~~
33. [x] ~~Sacudida de cámara procedural (golpe dado, golpe recibido, bloqueo, aterrizaje fuerte), con intensidad por datos y opción de desactivarla.~~
34. [x] ~~Muñecos del tutorial que enseñan las técnicas nuevas: un aviso y un muñeco para el ataque cargado, el tajo en carrera y el combo aéreo (sección de combate ampliada).~~ Además, `Verificar-Arranque` exige el "hecho" del script de assets (un error de sintaxis de Python pasaba sin aviso).
35. [x] ~~Equilibrio de primera pasada con los tests: tabla de daño por segundo de cada técnica y tiempos para derrotar a cada enemigo, en un test informativo (`SAEVUS.Equilibrio.*`) que escribe la tabla en el log.~~ `equilibrio.md`; reglas de diseño comprobadas.
36. [x] ~~Munición de reserva limitada para las armas de fuego (las armas superan al combo en daño/s, ver `equilibrio.md`): reserva por arma en datos, recarga desde la reserva, se rellena en los puntos de control; interfaz muestra cargador/reserva; tests.~~ La reserva también vuelve con los golpes de espada; chasquido en seco.
37. [x] ~~Números de daño flotantes de depuración (CVar `SAEVUS.Depuracion.NumerosDanio`) para ajustar el equilibrio jugando.~~
38. [x] ~~Hilo de juego: medir el tutorial con `stat game`/`stat unit` en la prueba de humo, localizar lo más caro (animación procedural, IA, partículas) y bajar del presupuesto actual (11–13 ms).~~ `-PerfilJuego dumpave|dumpmax`: el "coste" era un tirón único de 1,5 s del editor (primera captura) y espera al render; nuestro código ≈ 2,5 ms. Movimiento de enemigos lejanos a intervalos. Ver `rendimiento.md`.
39. [ ] Rotación de logs de verificación: `Saved/Logs/Verificacion` acumula decenas de `*-backup-*.log` (≈ 50 MB); conservar los 5 últimos de cada tipo desde `Verificar-Arranque` y los scripts de ejecución.
40. [ ] Enemigo volador ("Revoloteador", Hollow Knight): flota a media altura, se lanza en picado tras avisar; obliga a usar el tajo hacia arriba y el pogo. Cuerpo en Blender, datos, IA (estado de vuelo sin gravedad), tests, en el tutorial (S4b o arena).
41. [ ] Sección de armas de fuego en el tutorial: aviso ("Las armas gastan reserva; la espada la rellena"), dianas a distancia y un muñeco cerca para rellenar; comprobado por el recorrido.

## Estado del turno
- Completado: tarea 38 (perfil del hilo de juego).
- A medias: —
- Siguiente paso exacto: tarea 39 — rotación de logs de verificación.
