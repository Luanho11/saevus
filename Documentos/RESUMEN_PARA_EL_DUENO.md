# Resumen para el dueño (fase "Rehacer como GunZ", 2026-10-04)

Seguí tu mensaje: la jugabilidad es la de GunZ y Hollow Knight queda para la temática, el arte y el sombreado. Este documento es el punto de entrada: qué cambió, cómo probarlo y qué queda para ti.

## Cómo probarlo

1. Abre `SAEVUS.uproject` y pulsa **Play**. Arranca en el tutorial con el menú principal: **Tutorial**, **Mazmorra**, **Continuar** (si hay partida guardada), **Opciones** y **Salir**.
2. Teclas en `controles.md`. Lo básico:
   - **Moverse:** WASD en ocho direcciones; el cuerpo mira siempre a la cámara.
   - **Saltar:** Espacio, con doble salto.
   - **Dash:** **doble toque** de una dirección. Sin espera entre dashes ni límite por salto.
   - **Combate:** clic izquierdo para los tajos (mira arriba para el tajo hacia arriba; mantén y suelta para el giro cargado), **clic derecho** para la guardia, **Q** para curar, **E** para sentarte en los bancos.
   - **Pausa:** **P**. F6 muestra los datos de depuración.
3. **Tutorial ("Mazmorra del Umbral"):** diez salas cerradas tipo mazmorra; cada una enseña una cosa con un aviso que dice qué pulsar. Al final hay goblins en el gran salón, un banco, el paso a la Mazmorra y un reto opcional.
4. **Mazmorra** (`mazmorra.md`): práctica libre con el Dungeon de GunZ como referencia de estructura:
   - salón central con balcones, columnas y estrado;
   - galería de fosos con una pared falsa;
   - pozo con chimenea y ascensor;
   - terraza con palanca y compuerta;
   - goblins y el **Rey Goblin** en la sala del trono.

Si algo no compila o falta un asset, `powershell -ExecutionPolicy Bypass -File Herramientas\Verificar-Arranque.ps1 -ConJuego` lo reconstruye y lo prueba todo, en unos 25 minutos en esta máquina:
- compila, crea los assets y pasa los 131 tests;
- juega la prueba de humo;
- recorre sola la Mazmorra, cruza al tutorial y lo recorre entero.

## Qué cambió (todo boceto, todo cambiable por datos)

| Pediste | Hecho |
|---|---|
| Quitar armas de fuego, enemigos y jefes actuales | Solo la espada; sin teclas de armas. Los enemigos de antes siguen en los datos, fuera de los mapas. El código de las armas queda dormido. |
| Quitar el tajo hacia abajo | Fuera, con su rebote y su foso. |
| Dash como GunZ, sin latencia ni 1 por salto | Sin enfriamiento ni límite por salto. La gravedad sigue actuando, así que encadenar dashes no hace volar. |
| La cámara siempre detrás; el cuerpo no gira la cabeza atrás en el dash | El cuerpo sigue a la cámara. Andar hacia atrás es andar de espaldas, y ni el dash, ni el salto de pared, ni los tajos lo giran. |
| Animaciones (lo principal) | Piernas en ocho direcciones con giro de cadera. Volteretas en los dashes laterales y hacia atrás. Vuelta en el salto de pared. Aterrizaje según la caída. |
| Sombreado dibujado tipo Hollow Knight en 3D | Post-proceso dibujado: luz en bandas planas con sombra fría, tinta en siluetas y pliegues, niebla en capas y grano de papel. Sin desenfoque de movimiento. Colores de luna y braseros corregidos. |
| Mapas tipo tutorial con el Dungeon de GunZ de referencia | Tutorial rehecho en salas cerradas y Mazmorra nueva, que sustituye a la Zona 1. |
| Alcance de espada intuitivo y limpio | Los tajos golpean en un abanico delante y el arco del tajo dibuja ese abanico: lo que ves es lo que golpea. |
| Goblins y jefes de GunZ, mejorados, renderizados en Blender | Goblin de cuchillo, de maza y hondero, y el Rey Goblin. Sin daño por contacto. Antes de cada ataque se agachan y su arma brilla. Render en `capturas/Goblins.png`. |
| Más partículas | Arco del tajo, polvo al aterrizar, en las paredes, en el salto de pared y en el dash por el suelo. Se suman a la estela del dash. |
| Menús sin cosas genéricas; mejor fuente; mensajes claros | Títulos espaciados, menús de solo texto, teclas dibujadas en los avisos y textos que dicen qué hacer en cada sala. El marcador de los muñecos solo sale con F6. |
| Listo para probar en un ejecutable | Preparado y seguro de memoria, pero no lo lancé (ver abajo). |

Rendimiento: la prueba de humo va a ≈ 65 fps de media a 720p en Media dentro del editor. Antes eran ≈ 44; la mejora viene sobre todo de quitar el desenfoque de movimiento.

## Capturas (`capturas/`)

- `Menu.png`: menú principal.
- `Pausa.png`: pausa con el mapa.
- `Tutorial_Inicio.png`: primera sala con su aviso y el título de zona.
- `Tutorial_SalaArmas.png` y `Tutorial_Goblins.png`.
- `Mazmorra_Terraza.png`: palanca y compuerta.
- `Ataque.png`: arco del tajo.
- `Dash.png`, `Correr.png`, `Guardia.png`, `Muerte.png`.
- `Enemigos.png`: un goblin reaccionando.
- `Goblins.png`: render en Blender.

## Lo que queda para ti

- **Ejecutable:** con 7 GB de RAM, el primer empaquetado lo cortó el sistema. Ahora `Herramientas\Empaquetar.ps1`:
  - compila de una acción en una;
  - deja libres casi todos los hilos al compilar shaders.

  Es más lento, pero cabe en memoria. Ciérralo todo (editor incluido) y ejecútalo. Después, `Herramientas\Probar-Ejecutable.ps1` arranca el .exe con la prueba de humo, y para jugar está `Saved\Empaquetado\Windows\SAEVUS.exe`. No lo lancé yo porque quedó pedido no relanzarlo en autónomo.
- **Fuente:** solo uso la Roboto del motor, con estilos. Una fuente con más carácter, como Cinzel (licencia libre OFL, parecida a la de Hollow Knight), sería un recurso externo. ¿La quieres?
- **Preguntas:**
  - ¿El abanico de la espada (1,7 m, ±75°) se siente bien?
  - ¿Los goblins se dejan golpear como esperabas?
  - ¿Las volteretas de los dashes laterales te gustan, o prefieres un deslizamiento?
- **Tarea 61 aparcada:** las esquirlas (moneda de los enemigos) están en `git stash` desde antes del cambio de rumbo. Las retomo si las quieres para los goblins.

## Dónde mirar

- `ESTADO_PROYECTO.md`: estado actual.
- `REGISTRO_SESIONES.md`: qué se hizo y qué problemas hubo.
- `PROGRESS.md`: cola de tareas.
- Por tema: `movimiento.md`, `combate.md`, `animacion.md`, `enemigos.md`, `tutorial.md`, `mazmorra.md`, `interfaz.md`, `pipeline-assets.md`, `controles.md`.

En el historial de git hay un commit por tarea, todos con el prefijo `auto:`.
