# Hoja de ruta de SAEVUS

Actualizada 2026-10-03 con las prioridades del dueño. Orden de arriba abajo; el detalle vivo está en `PROGRESS.md`.

## Hecho

- Arranque, movimiento base y avanzado (dash, salto de pared, carrera por pared, escalada).
- Combate cuerpo a cuerpo por datos, armas de fuego, cambio de arma, vida, muerte y reaparición.
- Investigación GunZ + Hollow Knight (`investigacion/`), propuesta y sonda del pipeline de assets.
- Todo el proyecto en español.

## Ahora: que se vea y se sienta (prioridad del dueño)

1. **Dash por doble toque** de WASD, cámara bloqueada durante el dash, invulnerabilidad breve, partículas/estela.
2. **Caballero boceto** con esqueleto personalizado generado en Blender; **animación procedural** en C++ sincronizada con el movimiento y las líneas de tiempo de ataque (reposo, correr, saltar, caer, dash, paredes, tajos, disparo, muerte).
3. **Sombreado tipo Hollow Knight:** oscuro, aspecto dibujado, contorno por post-proceso barato.
4. **Respuesta al golpe:** retroceso, pausa de impacto, destellos, partículas de golpe.

## Después: mecánicas Hollow Knight

5. Doble salto, tajo hacia arriba y hacia abajo con rebote (pogo), alma + curación, daño recibido con invulnerabilidad.

## Después: contenido jugable

6. Enemigos simples (IA de estados) y un mini jefe con ataques telegrafiados, por Data Assets.
7. **Mapa tutorial** `L_Tutorial`: secciones que enseñan cada técnica con carteles y avisos en pantalla, mini enemigos, mini jefe al final.
8. **Interfaz** UMG escalable (bocetos): vida, alma, arma/munición, avisos de tutorial, barra de jefe.

## Más adelante

- Guardia/bloqueo, hechizos, súper dash, ataque cargado, contraataque.
- Red (movimiento ya preparado; combate y armas por llevar a datos predichos), PvP.
- Arte final con el pipeline aprobado.
- Fuera por ahora: inicio de sesión, backend, inventario, progresión, monetización.
