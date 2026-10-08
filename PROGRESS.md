# SAEVUS — PROGRESO (cola de tareas para trabajo autónomo)

## ⚠ CERROJO DE SESIÓN — LEER PRIMERO

**Estado: SESIÓN INTERACTIVA ACTIVA (iteración integral: revisión externa + vida del personaje)**
**Último latido: 2026-10-07 20:58** (la sesión activa lo actualiza en cada commit)

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

## Decisiones del dueño (2026-10-04) — cambio de rumbo, mandan sobre las anteriores
- **Rehacer como GunZ**: la jugabilidad es la de GunZ; Hollow Knight solo aporta temática, diseño e inspiración artística (se estaba priorizando Hollow Knight).
- Quitar por ahora **armas de fuego, enemigos y jefes** actuales (difíciles de golpear, alcance de espada poco intuitivo, ataques básicos). Más adelante: **goblins y jefes estilo GunZ** (modo quest) mejorados, modelados en Blender.
- Quitar el **tajo hacia abajo** (pogo): con doble salto no sirve.
- **Dash como GunZ**: sin latencia ni límite de uno por salto.
- **Cámara** siempre en tercera persona desde atrás; el personaje no gira el cuerpo al hacer dash atrás (mira a donde mira la cámara).
- Mejorar el **sombreado dibujado** (Hollow Knight en 3D): no se nota.
- **Mapas**: tutorial mejor, sin texturas, con la estructura del mapa **Dungeon de GunZ** como referencia (salas cerradas, muros para correr, escaleras, balcones, pilares).
- Lo principal: **personaje y animaciones**; más animaciones y partículas; limpieza de movimientos, espaciado e intuición.
- Objetivo: **prototipo escalable probable en un ejecutable**; menús mejorados sin cosas genéricas; tutorial con mejor fuente y mensajes claros.

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
39. [x] ~~Rotación de logs de verificación: `Saved/Logs/Verificacion` acumula decenas de `*-backup-*.log` (≈ 50 MB); conservar los 5 últimos de cada tipo desde `Verificar-Arranque` y los scripts de ejecución.~~ `Herramientas/Limpiar-Logs.ps1` (59 MB → 10 MB).
40. [x] ~~Enemigo volador ("Revoloteador", Hollow Knight): flota a media altura, se lanza en picado tras avisar; obliga a usar el tajo hacia arriba y el pogo. Cuerpo en Blender, datos, IA (estado de vuelo sin gravedad), tests, en el tutorial (S4b o arena).~~ `bVolador`, ataque `Picado`, `DA_Enemigo_Revoloteador` en la arena S7 y en `L_Pruebas`; captura de enemigos por arena en el recorrido.
41. [x] ~~Sección de armas de fuego en el tutorial: aviso ("Las armas gastan reserva; la espada la rellena"), dianas a distancia y un muñeco cerca para rellenar; comprobado por el recorrido.~~ Aviso en S6 y dos muñecos altos; el recorrido dispara con la pistola a la diana y comprueba el impacto.
42. [x] ~~Guardar opciones y progreso (`USaveGame`): calidad y sensibilidad del menú de pausa y el último punto de control; se aplican al arrancar. Tests.~~ La calidad ya la guardaba el motor; sensibilidad y punto de control en `SAEVUS.sav`; `-SAEVUSSinGuardar` en las pruebas.
43. [x] ~~Menú principal boceto en C++ (Jugar, Continuar desde el punto de control, Opciones, Salir) sobre el tutorial en pausa, con la estética de la interfaz; prueba de humo.~~ Modo `Principal` del widget de pausa, al arrancar `L_Tutorial`; botones con estilo propio.
44. [x] ~~Zona 1 boceto (`L_Zona1`, generado por script): ruinas tras el tutorial con plataformas, tres encuentros de enemigos mezclados, un banco (punto de control) y un atajo; recorrido propio.~~ `L_Zona1` con palanca y compuerta, paso desde el tutorial, recorrido generalizado (alturas, palancas) en `Verificar-Arranque -ConJuego`.
45. [x] ~~Sonidos de mundo: palanca, compuerta/puerta que se abre, aleteo del revoloteador y chasquido del picado (síntesis, como el resto).~~ Palanca y puerta en `USAEVUSSubsistemaSonidoMundo`; aleteo de los voladores; el picado usa el sonido de ataque común.
46. [x] ~~Jefe de la Zona 1 boceto: un segundo mini jefe con un patrón nuevo (p. ej. volador grande con picados y proyectiles en abanico), arena al final de la galería.~~ Madre Alada (volador, picado, abanicos de 5 y 7 proyectiles, dos fases) en su arena tras la antesala; `NumeroProyectiles` y `AperturaAbanico` en los ataques.
47. [x] ~~Mapa de la zona en la pausa: esquema 2D generado de la geometría (bloques vistos desde arriba) con la posición del jugador y los bancos.~~ `USAEVUSWidgetMapa` en la pausa.
48. [x] ~~Bancos de verdad (Hollow Knight): `IA_Interactuar` (E / botón inferior derecho del mando); sentarse en un banco cura del todo, rellena la munición, fija el punto de control y guarda; el caballero se sienta (pose procedural). En la Zona 1 y al final de las arenas del tutorial.~~ `ASAEVUSBanco` + `ISAEVUSInteractuable`, aviso "E   Sentarse", pose sentada, campanada; el recorrido se sienta en cada banco.
49. [x] ~~Fundido y título al entrar en un mapa ("Ruinas del Paso" grande que aparece y se va), también al reaparecer en un banco.~~ Título de zona con fundido (`ASAEVUSInfoZona`); la reaparición ya tenía su fundido (pantalla de muerte).
50. [x] ~~Poner al día `RESUMEN_PARA_EL_DUENO.md` con todo lo nuevo (Zona 1, jefes, menús, guardado, mapa) y capturas frescas.~~
51. [x] ~~Medir el rendimiento en los recorridos: fotogramas medio y peor por mapa (tutorial y Zona 1, con sus 13 enemigos y el jefe), escrito en el log y en `rendimiento.md`.~~ ≈ 52 fps en los dos mapas, en Media.
52. [x] ~~Volumen general en los menús (0–100 % en pasos), guardado con las demás opciones.~~
53. [x] ~~Menús con mando y teclado: foco en el primer botón al abrir, navegación con la cruceta y confirmar con el botón inferior; prueba de humo.~~ Botón enfocado resaltado.
54. [x] ~~Enemigo con escudo ("Acorazado"): bloquea los golpes de frente (filtro de daño como la guardia del jugador), hay que rodearlo, saltarle por encima (pogo) o romper su guardia con el giro cargado; en la Zona 1.~~ `USAEVUSComponenteEscudo`, `DA_Enemigo_Acorazado` (cuerpo con escudo en Blender).
55. [x] ~~Secreto en la Zona 1: pared falsa que se rompe a golpes y da a una sala con un fragmento de máscara (+1 máscara de vida al juntar dos; guardado).~~ `ASAEVUSParedRompible`, `ASAEVUSFragmentoMascara`; el segundo fragmento es el premio del reto del tutorial.
56. [x] ~~Regreso de la Zona 1 al tutorial (paso en la entrada) y aparición junto al paso correspondiente al cambiar de mapa.~~ Opción `portal` de UE con `PlayerStart` etiquetados; sin menú principal al llegar por un paso.
57. [x] ~~Prueba del paso entre mapas de punta a punta: el recorrido de la Zona 1 cruza el paso de vuelta y el del tutorial comprueba que aparece en la llegada (`-SAEVUSRecorridoPaso`).~~ Encontró que la llegada necesita `mapa#Llegada` en la URL; corregido. `Verificar-Arranque -ConJuego` usa la cadena.
58. [x] ~~Más variedad en la Zona 1: plataformas que se mueven (ascensor boceto) y un tramo de pinchos con rebote, con su comprobación en el recorrido.~~ Plataforma móvil en la grieta (`ASAEVUSPlataformaMovil`), con test; los pinchos con rebote quedan para otra tarea.
59. [x] ~~Pinchos con rebote en la Zona 1 (un tramo sobre la galería) y una segunda plataforma móvil vertical (ascensor) hacia la planta alta.~~ Pinchos en la planta alta y ascensor junto al muro (`crear_plataforma_movil`).
60. [x] ~~Mapa que se descubre al explorar (Hollow Knight): celdas visitadas guardadas por mapa; el mapa de la pausa solo dibuja lo visitado (y el jugador).~~ `USAEVUSSubsistemaExploracion`, bloques recortados por celdas.
61. [-] ~~Esquirlas (moneda)~~ Aparcada por el cambio de rumbo (sin enemigos); el trabajo está en `git stash` ("WIP tarea 61").
62. [-] ~~Revisión de código~~ Se hace dentro de las tareas de la fase nueva.

### Fase "Rehacer como GunZ" (2026-10-04)
63. [x] Quitar las armas de fuego del juego: arsenal solo con la espada, sin cambio de arma ni recarga en la entrada, sin munición en la interfaz, sin lección de armas ni dianas. El código queda dormido y probado.
64. [x] Quitar enemigos y jefes de los mapas (tutorial, Zona 1, pruebas) y del recorrido; la IA queda para los goblins.
65. [x] Quitar el tajo hacia abajo (pogo) y la sección de rebote del tutorial.
66. [x] Dash como GunZ: sin enfriamiento ni límite por salto (por datos).
67. [x] Cámara y orientación de GunZ: el personaje mira siempre a donde mira la cámara y se mueve en ocho direcciones sin girar el cuerpo; dash en cualquier dirección sin girarse.
68. [x] Animación direccional: correr adelante/atrás/lados, volteretas de dash por dirección, carrera y salto de pared con giro, aterrizajes; más fluida y limpia.
69. [x] Sombreado dibujado de verdad (Hollow Knight en 3D): luz en bandas planas, tinta gruesa en contornos, sombras frías, niebla en capas, grano de papel procedural; medir en la Vega 8.
70. [x] Tutorial nuevo estilo mazmorra (estructura de referencia: Dungeon de GunZ): salas cerradas con muros para correr, escaleras, balcones, pilares y arcos; enseña paso a paso con mensajes claros.
71. [x] Mapa de práctica "Mazmorra" (arena estilo Dungeon de GunZ) en lugar de la Zona 1; el paso del tutorial lleva a ella.
72. [x] Interfaz y menús: tipografía (Roboto con contorno, espaciado y pesos), menús limpios (Tutorial, Mazmorra, Opciones, Salir), avisos con las teclas resaltadas; quitar lo genérico.
73. [x] Personaje: caballero mejorado en Blender (proporciones, capa, máscara, espada clara) y alcance de la espada visible e intuitivo (estela = zona de golpe).
74. [x] Partículas: estela y polvo direccional del dash, aterrizaje, carrera por pared, salto de pared, arcos de los tajos.
75. [x] Goblins estilo GunZ (Blender, mejorados): tipos con ataques variados y avisos claros, sin daño por contacto, golpeables con claridad; sala de goblins en el tutorial.
76. [x] Jefe goblin (rey goblin) con patrones claros.
77. [x] Ejecutable: empaquetado que quepa en la memoria de la máquina (cocinar solo los mapas del juego) y prueba de arranque del .exe. Hecho: 959 MB en `Saved/Empaquetado/Windows/SAEVUS.exe`; el .exe pasa la prueba de humo a 78 fps (Media, 720p).

### Fase "Calidad" (dueño, 2026-10-04 tarde: "mejóralo más, debe sentirse calidad")
78. [ ] Fuente Cinzel (OFL) en títulos, menús y avisos, con Roboto de respaldo si falta. Bloqueado: la descarga la frenó el permiso automático; el dueño pondrá los .ttf en `ArteFuente/Fuentes/` o autorizará la descarga. El código puede ir antes, con respaldo.
79. [ ] Sensación de golpe: destello del objetivo, chispas más grandes en la dirección del tajo, parada de golpe y sacudida ajustadas por ataque, sonido por capas.
80. [ ] Cámara: suavizado de seguimiento, ligera anticipación en el dash, empuje del campo de visión al esprintar y que no se meta en los muros estrechos (sonda y distancia mínima).
81. [ ] Animación: transiciones con anticipación y asentamiento (squash/stretch leve), postura de reposo con el arma lista y giro de la capa en las volteretas.
82. [ ] Goblins: animación propia (andar, agacharse al avisar, golpe, aturdido, muerte que se deshace en polvo) y sonidos.
83. [ ] Interfaz: transición de menús (fundido y desplazamiento), sonido al navegar, barra del jefe con nombre en Cinzel.
84. [ ] Rendimiento en el ejecutable: perfil con el recorrido empaquetado y presupuesto de partículas.

### Fase "Producción artística" (dueño, 2026-10-06) — manda sobre la fase "Calidad"
Las imágenes de `assets/` son la **referencia visual canónica** (`Documentos/REFERENCIAS_ARTE.md`, `REF-01`…`REF-08`); la dirección derivada está en `Documentos/ARTE_SAEVUS.md`. No cambiar la dirección artística por iniciativa propia; si algo no se deduce de la referencia, decisión mínima y reversible, documentada. Orden: protagonista → animación → espada → combate → goblin → jefe → mazmorra → VFX → UI → audio → vertical slice. Cada bloque: implementar, compilar, tests, humo, **comparar con la referencia en la cámara del juego**, documentar, commit pequeño. Si un asset actual contradice una referencia, corregirlo antes de crear contenido nuevo. Las tareas 78–84 quedan absorbidas aquí.

85. [x] Dirección artística documentada: `ARTE_SAEVUS.md` (paleta medida, proporciones medidas, protagonista, goblins, Rey Goblin, mazmorra, iluminación, sombreado, VFX, UI, tipografía, decisiones) y `REFERENCIAS_ARTE.md` (tabla archivo → elemento → uso → prioridad → asset).
86. [x] **Protagonista según REF-01/02/03** (el caballero actual los contradice: máscara blanca redonda con cuernos, capa en tubo, aguijón corto, cabezón). `blender_caballero.py` v3: proporciones de REF-02 (H 1,92 m, cabeza ≈ 1/8), casco de placas en hoja con aletas barridas, máscara de hueso en V con ojos cian, capa con capucha y tiras dentadas, broche de anillo, correas en X, cinturón, faldón, brazales de hueso, pantalón holgado, espinillas vendadas, pies de tres garras, espada larga (hoja ≈ 0,88 m) con guarda de anillo. Mismos 25 huesos (compatibles con `ESAEVUSHueso`). Materiales `MI_Caballero_*` con la paleta medida (ojos y filo emisivos). `LongitudHoja` a la hoja nueva. Vistas previas de Blender junto a REF-01 y captura en la cámara del juego.
86b. [x] **Protagonista desde las vistas de REF-01** (dueño: "el modelo tal cual las imágenes"): forma y color salen del dibujo (`reconstruir_desde_vistas.py` → malla densa → horneado en `T_Caballero.png` → atado a los 25 huesos con articulaciones medidas; `Generar-Caballero.ps1`, `pipeline-assets.md`). Espada y armas siguen de piezas. 132/132 tests, humo (48,8 fps) y recorrido SUPERADOS.
87. [x] Pose de reposo y de carrera según REF-04 (espada baja en diagonal hacia delante en reposo; cuerpo inclinado y espada atrás al correr), para que la hoja larga no atraviese el suelo.
88. [ ] Look 3D: ajustar `M_PP_Dibujado` y los materiales a `ARTE_SAEVUS.md` (contorno más fino, bandas suaves, sombra fría, filo de luz, niebla azulada, grano discreto), comparado en la cámara del juego con REF-01/REF-07.
89. [~] (n.º 4–11, commit 94a2c7f; n.º 12–15 y capa en cadena con `capa_3`, 2026-10-07; faltan guardia, parry, golpe, aturdido y muerte según REF-08 F/H/I) Animación con las poses objetivo de REF-04 (los 15 números) y la capa de REF-02: Idle, Run, Sprint, Jump, DoubleJump, Fall, Dash (4 direcciones + diagonal), WallRun, WallJump, Land, Slash 1–3, AirSlash, DashSlash, ChargedSlash, Guard, Parry, Hit, Stagger, Death; anticipación, pose clara, seguimiento de capa. Más huesos de capa si hace falta (tiras).
90. [ ] Laboratorio de combate (`L_LaboratorioCombate`, fuera del recorrido principal): cápsula del jugador, hurtbox, hitbox, trayectoria de la hoja, fases (preparación/activa/recuperación) visibles, muñecos.
91. [ ] Golpe por trazas continuas de la hoja (barridos entre fotogramas) además del abanico; parámetros por ataque: daño, alcance, arco, preparación, activa, recuperación, parada de golpe, retroceso, aturdimiento, sacudida, partículas, sonido. Corregir alcance corto, impactos sin peso y tajos demasiado rápidos.
92. [~] Goblin base según REF-05 (arriba 1) — v2 (2026-10-07): extremidades con volumen, cabeza ×1,25, facetas pintadas con la paleta del dibujo (`paleta_desde_referencia.py`, `T_Goblin_Paleta`); modelo con esqueleto y animación desde la IA hechos (aviso, ataque, recuperación, aturdido, muerte); falta la muerte en fragmentos y pulir en juego —: especie (orejas, dientes, piel de hueso, trapos, garras, hoz); un goblin excelente antes que variantes. Animación: reposo, andar, correr, preparación, ataque, reacción, aturdido, muerte (se deshace en fragmentos, REF-08 I).
93. [~] (guerrero y lanzador hechos e importados, commit e4f9d13) Variantes de goblin (tabla provisional de `ARTE_SAEVUS.md`): guerrero, asesino, lanzador, élite; escudero y pesado derivados con decisión mínima.
94. [~] (cuerpo desde las vistas de REF-06 hecho: `vistas/rey_goblin.json`, `T_ReyGoblin`, `USAEVUSDatosEnemigo::TexturasPiezas`, guja de piezas; faltan ataques de arena, fase 2 roja y muerte) Rey Goblin según REF-06 (2,9 m, corona de púas, capa larga, broche de anillo, guadaña de asta): modelo, materiales, esqueleto, ataques que usan la arena (barrido, golpe al suelo, salto, embestida), aturdido, fase 2 roja (REF-06 abajo, REF-08 J/K), muerte.
95. [~] (muros, suelos y columnas vestidos con el kit en tutorial y mazmorra, commit e4f9d13; faltan arcos, estandartes, braseros, escombros y la arena octogonal con sello rojo) Mazmorra con kit modular según REF-07 (suelo, pared, pared de carrera, esquina, columna, arco, escalera, balcón, plataforma, puerta, marco, techo, escombros, estandartes); primero layout funcional, después arte; arena octogonal con sello rojo.
96. [ ] VFX con el lenguaje de REF-08 D–L (dash, pared, aterrizaje, tajos, impactos, bloqueo/parry, sangre estilizada, muertes, telegrafiado rojo, ambiente), versiones baratas por escalabilidad.
97. [~] (HUD hecho, commit 453ca1f; faltan menús, muerte, victoria, notificaciones e iconos) UI según REF-08 A/C (HUD con rombo y barras finas, barra del jefe con nombre y fase, minimapa circular, menús, muerte, victoria, notificaciones, iconos en rombo).
98. [ ] Tipografía según REF-08 B: Cinzel (OFL) para títulos cuando el dueño ponga los `.ttf` (tarea 78), Roboto para información; licencias en `interfaz.md`.
99. [ ] Audio fundamental por capas: tajo, impacto, parry, bloqueo, dash, pared, aterrizaje, daño, muerte, UI, jefe.
100. [ ] Vertical slice: tutorial corto + sala de goblins + Rey Goblin en la mazmorra nueva, todo con el arte nuevo; empaquetado y prueba del .exe.

### Fase "Iteración integral" (dueño, 2026-10-07) — manda sobre el orden de la de arriba
Revisión externa del código (otro chat de Claude; respuesta punto por punto en `Documentos/REVISION_CODIGO.md`) y petición del dueño: corregir todo, quitar la rigidez (giro por capas, peso, respiración, reacción, dash con identidad, un gesto por golpe), animar enemigos y jefes, música, efectos, HUD, y dirección artística cercana a Elden Ring en 2D dibujado. **Los personajes los rehace el dueño** (no tocar sus modelos).
101. [x] Errores E1–E12 de la revisión, con tests (`SAEVUS.Combate.{EscudoSinDanio, ZonaNoEsGolpe, FotogramaLargo, ParadaEscalaCero}`, `SAEVUS.Movimiento.EscalasVelocidad`, `SAEVUS.Tutorial.PuertaEnemigoDesaparece`).
102. [x] Rendimiento R1–R5, R7, R8 (huesos de una vez, guardado en otro hilo, cachés de la IA y del sonido, interfaz sin trabajo de más, partículas y estela). Pendiente R6 (mapa de pausa).
103. [x] Duplicados D1–D8 y malas prácticas M1, M4–M8. Pendientes M2 (delegado estático → subsistema, con la red), M3 (renombrar el HUD de depuración con redirecciones), M9 (bajas del sonido).
104. [x] Vida del cuerpo: muelles, capa secundaria (giro por capas, inercia, rebote, respiración con cansancio, reposo vivo con gestos), locomoción viva, capa con muelles, suavizado por tramos (`animacion.md`).
105. [x] Un gesto por ataque (nueve, por datos) con amago, corte y seguimiento; combo que enlaza; rebote de la hoja al acertar.
106. [x] Dash en tres tiempos y por dirección + ecos (`USAEVUSComponenteEcoDash`, `M_EcoDash`).
107. [x] Goblins y Rey Goblin: gesto por ataque, aviso con tensión, recuperación, aturdido, rugido de fase, peso del jefe.
108. [x] Música generada (exploración, combate, jefe) con fundidos según la pelea (`audio.md`).
109. [x] HUD: rastro del daño, destello rojo, brillo del alma, muerte con banda, título de zona con rayas, tecla real (`interfaz.md`).
110. [x] Atmósfera: gradación (sombras frías, luces de oro viejo, negros hondos, resplandor suave) y motas de polvo en el aire.
111. [ ] Ajuste a ojo en el juego de gestos, muelles, música y gradación (los valores son de primera pasada); capturas comparadas con REF-04.
112. [ ] Dirección artística en los mapas: luz de rayos entre columnas, niebla por salas, braseros con brasas, estandartes; versiones baratas para la Vega 8.
113. [ ] Sonido por capas para los gestos (cada golpe con su sonido) y pasos; transiciones de música a compás.

## Estado del turno
- Completado: tareas 101–110 (iteración integral; ver `REVISION_CODIGO.md` y `REGISTRO_SESIONES.md`). Verificación completa con juego: 150/150 tests, assets SUPERADA, humo SUPERADA (31 fps en frío), recorrido Mazmorra → tutorial SUPERADO.
- A medias: fuego de los braseros (`ASAEVUSFuegoBrasero`, sin verificar ni en el commit; script de cambios preparado) y sonido por gesto con pasos y aterrizaje (tarea 113, script preparado).
- Siguiente paso exacto: aplicar 112 (braseros) y 113 (sonido), verificar con juego; luego 111 (ajuste a ojo con capturas de la prueba de humo).
