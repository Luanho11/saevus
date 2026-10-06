# Dirección artística de SAEVUS

_Creado 2026-10-06 a partir de las referencias canónicas de `assets/` (índice en `REFERENCIAS_ARTE.md`)._

Este documento **no inventa** una dirección: describe lo que muestran las referencias y lo traduce a reglas de producción para el motor. Cuando una referencia no permite decidir un detalle, se dice ("sin determinar") y se elige lo más simple y reversible (sección Decisiones). Manda sobre lo que decían antes `pipeline-assets.md` y `animacion.md` acerca del aspecto (máscara blanca redonda con cuernos, "aguijón", capa en tubo): aquel caballero era un boceto previo a las referencias.

## Resumen en una frase

Fantasía oscura **estilizada y pintada**, casi monocroma (negro azulado, pizarra, hueso cálido, verde apagado), con **siluetas afiladas de hoja rota** y dos únicos acentos: **azul frío** para el jugador y el movimiento, **rojo oscuro** para el peligro, la sangre y el jefe.

## Paleta (medida en las referencias)

Valores sRGB tomados por cuantización y muestreo de las imágenes (no a ojo). En el motor se usan en lineal.

| Papel | Color | De dónde |
|---|---|---|
| Fondo / vacío | `#141517` – `#17181A` (negro azulado) | todas |
| Cuerpo oscuro, sombras del personaje | `#1A1B1D` – `#232226` | REF-01 |
| Púas del casco | `#232226` (luz: `#3E3B41`) | REF-01 cabeza |
| Hueso (máscara, brazales, escamas del pecho) | `#9C897F` luz · `#877771` medio · `#7C6C64` sombra | REF-01 |
| Interior de las aletas / orejas | `#4F3832` (marrón cálido) | REF-01 cabeza |
| Capa y faldón (verde pizarra apagado) | `#2E3133` medio · `#485555` luz · `#171819` pliegue | REF-01, REF-02 |
| Cuero (cinturón, correas, vendas) | `#322A2A` · `#4F3832` | REF-01 |
| Pantalón | `#302C2E` | REF-01 |
| Ojos del protagonista (cian) | `#458195` medio, núcleo claro | REF-01 |
| Filo de la espada (azul acero) | `#537B96`, brillo `#A8BBD4` | REF-01, REF-04 |
| VFX del jugador (dash, estelas, parry) | `#243C4D` · `#354E62` · `#6F8A97` · brillo `#BBC6E1` | REF-08 D, E, H |
| Sangre | `#502226` · `#69322E` | REF-08 G |
| Jefe fase 2 / telegrafiado | `#5A2529` · `#6D342F`; telegrafiado vivo `#AE5852` | REF-06, REF-08 K |
| Piel de goblin | `#5F534E` luz · `#352F2E` sombra | REF-05 |
| Piedra de la mazmorra (pizarra azul) | `#263E54` · `#303A44` · `#3C4855`; gris `#5C5D63` | REF-07 |
| Fuego de antorchas y braseros | `#E0A97E` · `#CDA773` | REF-07 |
| Estandartes | rojo apagado `#88503C` | REF-07 |
| UI: panel | `#0E1920` (azul noche) · borde `#40464D` | REF-08 A |
| UI: vida / recurso / texto | vida `#66262B`→`#964843`; recurso `#2B4A62`; texto hueso `#DFC6AD` | REF-08 A, B |

Reglas de paleta:
- **Saturación baja** en todo lo grande (personajes, escenario): los acentos saturados son pequeños y tienen significado.
- **Azul = jugador** (ojos, filo, estelas, dash, parry). **Rojo = peligro** (sangre, telegrafiado, jefe en fase 2, vida en la interfaz). **Naranja cálido = luz del mundo** (antorchas). No mezclar papeles.
- Lo más claro de un personaje es el **hueso** (máscara, brazales): ahí va la lectura de la cara y de las manos.

## Forma y silueta

- **Borde de hoja rota**: capas, faldones, telas y púas acaban en tiras dentadas e irregulares (como hojas secas o plumas), nunca en un borde recto. Es el rasgo más reconocible de todo el reparto (protagonista, goblins, Rey Goblin).
- **Placas superpuestas** en lugar de superficies lisas: casco de placas en forma de hoja, brazales y espinilleras segmentadas, escamas en el pecho.
- **Miembros finos y largos**, cintura estrecha, **pies con tres garras** (protagonista, goblins y jefe).
- **Anillo metálico** como motivo: broche de la capa, guarda de la espada, hebillas, broche del Rey Goblin.
- Volúmenes grandes y claros, detalle controlado (REF-02, "Geometría recomendada para Blender: formas simples y claras, pocos polígonos, grandes volúmenes, detalles controlados, facilita el rig y la animación").

## Protagonista (REF-01, REF-02, REF-03)

**Silueta:** humanoide esbelto, cabeza grande coronada de púas hacia arriba y dos **aletas largas barridas hacia atrás y afuera**; capa hecha jirones que ensancha la silueta por los hombros y cae hasta media pierna; espada larga y fina que llega casi al suelo.

**Proporciones** medidas en la vista frontal casi ortogonal de REF-01 (abajo a la izquierda), como fracción de la altura total hasta lo alto de las púas (H). La pose heroica grande de REF-01 está en contrapicado y agranda la cabeza: no sirve para medir.

| Medida (desde el suelo) | Fracción de H | En el modelo (H = 1,92 m, la cápsula) |
|---|---|---|
| Lo alto de las púas y las aletas | 1,00 | 1,92 m |
| Barbilla (pico de la máscara) | 0,88 | 1,69 m |
| Hombros | 0,80 | 1,54 m |
| Cinturón | 0,67 | 1,28 m |
| Muñecas (brazos caídos) | 0,53 | 1,02 m |
| Entrepierna | 0,49 | 0,94 m |
| Rodillas | 0,27 | 0,52 m |
| Tobillos | 0,09 | 0,17 m |
| Anchura de hombros con la capa | 0,29 | 0,55 m |
| Envergadura de las aletas | 0,26 | 0,50 m |

Cabeza pequeña (≈ 1/8 de H de la barbilla a las púas; REF-02, silueta y proporciones, da ≈ 1/9), torso y piernas largos: **no es un personaje cabezón**. REF-04 dibuja la cabeza algo mayor (estilización de las poses): manda REF-02.

**Cabeza y máscara:** casco de **placas oscuras con forma de hoja** (`#232226`) que suben y se abren hacia atrás como una cresta; dos **aletas grandes** a los lados, barridas hacia atrás y arriba, con el interior marrón cálido; en la cara, una **máscara de hueso** en V: una placa central en rombo que baja por la frente hasta una **barbilla afilada** (pico), y dos placas de mejilla que enmarcan los **ojos cian**, rasgados y algo hundidos. No hay boca visible. Cuello alto envuelto por el cuello de la capa.

**Torso:** oscuro y estrecho, con **escamas de hueso** visibles en una abertura en V en el pecho; **correas cruzadas en X** sobre el pecho; **cinturón ancho** de cuero con hebilla y una segunda correa baja.

**Faldón:** dos paneles delanteros (y traseros) de tela verde pizarra que cuelgan del cinturón hasta la rodilla, con los bajos rotos.

**Brazos y manos:** finos; **brazales de hueso** segmentados en el antebrazo (lo claro de los brazos); manos oscuras con dedos en garra.

**Piernas y pies:** pantalón oscuro y holgado hasta la rodilla; espinillas finas envueltas en vendas y placas de cuero; **pies de tres garras** grandes hacia delante, con talón marcado.

**Capa:** verde pizarra oscura, **con capucha/cuello alto** y cerrada en el pecho con un **broche de anillo**; cae por la espalda hasta media pierna en **tiras dentadas**. En movimiento se abre en abanico hacia atrás (REF-02, movimiento de capa; REF-04).

**Espada (REF-01 aislada, REF-02):** larga, recta con una ligera curva al final, **un filo**, hoja estrecha que se afila hasta una punta larga; **guarda con un anillo** (con un brillo azul en el filo y en el anillo); puño largo oscuro envuelto y pomo en punta. Hoja ≈ 0,45–0,47 de H (≈ 0,88 m; medido en REF-02, silueta, y en REF-04 n.º 1) más el puño (≈ 0,20 m). Se lleva en la **mano derecha**, en reposo baja y en diagonal hacia delante y afuera, con la punta cerca del suelo (REF-01, REF-04 n.º 1).

**Piezas (REF-02, tipo de pieza):** deformables con el esqueleto: cuello, torso, brazos, manos, piernas, pies, cinturón (ligero), capa (movimiento secundario, rigidez baja). Rígidas: cabeza, máscara/casco, broche, espada. Separadas como objeto en Blender: máscara/casco, capa, broche, cinturón, espada, accesorios.

**Esqueleto de referencia (REF-02):** cuello, hombro, codo, muñeca, cadera, rodilla, tobillo; dedos articulados (ver Decisiones: en este prototipo no).

## Movimiento y poses (REF-04, REF-02, REF-08 D)

Rasgos comunes de las 15 poses de REF-04:
- **Cuerpo muy inclinado hacia delante** al correr, en el sprint y en los dashes; la cabeza adelantada, mirando al frente.
- **Espada atrás y abajo** en carrera, sprint y dash (brazo derecho extendido hacia atrás, hoja paralela al suelo o en diagonal); el brazo izquierdo adelantado, mano abierta o en garra.
- **Capa abierta hacia atrás en abanico** en todo movimiento rápido; se cierra en reposo.
- **Zancadas largas y bajas** (rodillas muy flexionadas, piernas abiertas), sin rebotes verticales grandes.
- Salto y doble salto: **rodillas recogidas** al pecho, espada abajo y atrás.
- Wall-run: el cuerpo de lado contra la pared, mano izquierda en la pared, espada abajo y atrás. Salto desde la pared: empuje con la mano, cuerpo abierto.
- Aterrizaje fuerte: **en cuclillas**, piernas muy abiertas, una mano cerca del suelo, polvo a los lados.
- Ataque horizontal: piernas muy abiertas y bajas, brazo extendido, la hoja dibuja un **arco ancho** a la altura del pecho. Ataque diagonal: de arriba a abajo, brazo alto. Ataque durante el dash: estocada horizontal con todo el cuerpo extendido. Ataque aéreo: tajo con el cuerpo en horizontal, piernas recogidas.

Regla: **exagerado y legible**; cada pose se reconoce solo por su silueta.

## Goblins (REF-05)

Especie común:
- **Delgados, nervudos, encorvados**; brazos largos; piernas finas con **pies de tres garras**; más o menos de la altura del protagonista, más bajos por la postura.
- **Orejas largas y puntiagudas** con interior marrón; cara de hueso con **boca de dientes afilados** en sonrisa; ojos pequeños y oscuros.
- Piel de hueso grisáceo (`#5F534E`) con aspecto de placas, **trapos verde pizarra rotos** como los del protagonista, correas y brazales de cuero.
- **Armas curvas y toscas**: hoces, cuchillas, cuchillos.

Las seis siluetas de REF-05 (izquierda a derecha, arriba y abajo) y la variante que mejor encaja con cada una. La asignación es **provisional** (la referencia no pone nombres):

| Posición | Lo que se ve | Variante propuesta |
|---|---|---|
| Arriba 1 | Ágil, agachado, capucha, hoz larga, cara tipo máscara con ojos claros | **Goblin base** (canónico) / rápido |
| Arriba 2 | Más grande, sonrisa de dientes, carga a la espalda con un aro, cuchilla grande | Guerrero / pesado |
| Arriba 3 | Alto y fino, casco con púas, cuchillas en los dos antebrazos | Asesino |
| Abajo 1 | Agachado, casco redondo con correa, armazón (ballesta) a la espalda, hoz | Lanzador |
| Abajo 2 | Capucha puntiaguda, sonrisa, cadena con colgante y cuchillo | (chamán / lanzador; sin determinar) |
| Abajo 3 | Encorvado, bufanda **roja**, casco grande, cuchilla curva | Élite (el rojo marca peligro) |

"Escudero" no aparece en REF-05: si se hace, se deriva del guerrero con un escudo tosco de tablas y placas, con los mismos materiales (decisión mínima, a confirmar).

## Rey Goblin (REF-06, REF-08 J, K, I)

- **Escala:** ≈ 1,6 veces el cuerpo del protagonista, ≈ 1,9 con la corona (comparación de REF-08): unos **2,9 m** de cuerpo.
- Figura alta y ancha de hombros, cubierta de **placas con púas** superpuestas; **corona de púas** alta; máscara de hueso con ojos pequeños; **capa larga hasta el suelo** en tiras dentadas, verde pizarra; **broche de anillo** en el pecho (mismo motivo que el protagonista); cinturón con anillo; pies de garra.
- **Arma:** asta larga con **hoja de guadaña/cuchilla curva** (cuatro variantes de asta en REF-06; la de las poses es una guadaña de hoja ancha). Es el arma del jefe; no se cambia.
- **Poses de combate (REF-06, fila central):** golpe bajo con el asta, carrera agachada con la capa al viento, salto con el arma alzada, **barrido ancho con la guadaña**, golpe al suelo con impacto.
- **Fase 2 (REF-06 abajo, REF-08 J):** las telas se vuelven **rojas** en tiras, ojos rojos, acentos rojos en corona y guadaña; más agresivo. Telegrafiado rojo en arcos y círculos en el suelo (REF-08 K).

## Mazmorra (REF-07)

- **Original** (no se copia la geometría de GunZ): toma los principios espaciales — salas cerradas, verticalidad, balcones, escaleras, columnas, muros de movilidad, rutas alternativas.
- **Estructura:** salas unidas por pasillos y escaleras sobre un **abismo oscuro** con agujas de roca al fondo; varias alturas (tres alzados en REF-07); **arena octogonal** con un **sello circular rojo** en el suelo para el jefe; sala de columnas central; pozo vertical; pasillos largos con puerta al fondo.
- **Materiales:** sillería de **piedra pizarra azulada** (bloques grandes, juntas marcadas, aristas gastadas), suelos de losas, madera oscura en plataformas.
- **Elementos:** columnas altas, **arcos góticos**, puertas con marco, **estandartes rojos colgantes**, antorchas y braseros cálidos, escombros.
- **Kit modular** de REF-07: suelos, plataformas (mesa), muros (rectos, curvos), columnas, arcos, puertas, marcos, escaleras, escombros, estandartes. Rejilla común.
- **Movilidad** (diagramas de REF-07): pozo vertical con cornisas alternas, muros para carrera y salto de pared, huecos para saltar entre plataformas, paredes para escalar.
- Orden: primero el layout funcional (métricas en `pipeline-assets.md`), después el arte.

## Iluminación

- **Oscura y fría** de base (luz de luna azul pizarra), con **islas cálidas** de antorchas y braseros (REF-07 salas, REF-08 tira de escenarios).
- Fondos que se aclaran con la distancia en **niebla azulada** (perspectiva atmosférica), primer plano casi en silueta.
- Personajes leídos por **luz de borde fría** sobre fondo oscuro y por el hueso claro de la cara.
- La arena del jefe tiene su propio acento: sello rojo en el suelo, estandartes rojos.

## Sombreado, contornos y textura

Las referencias son renders **pintados**: modelos 3D con luz suave en pocas masas, sombras frías, bordes oscuros y textura de pincel. El aspecto dibujado sale del render y los materiales, no de la geometría:
- **Bandas de luz** suaves (2–3), con la sombra que se **enfría hacia el azul**.
- **Contorno** oscuro y fino en siluetas y pliegues, más fino que en un cómic: las referencias no tienen línea negra gruesa; el borde se lee por contraste y por un **filo de luz** fría.
- **Materiales mate**: casi sin especular; solo el filo de la espada, los anillos metálicos y los ojos brillan.
- **Textura pictórica sutil** y **grano** muy discreto: manchas de pincel en la tela y la piedra, sin ruido fotográfico.
- **Niebla** en capas.

Ya existe `M_PP_Dibujado` (bandas, sombra fría, tinta, niebla, grano; `pipeline-assets.md`): se **ajusta** a esta paleta y a este contorno más fino, no se rehace.

## VFX (REF-08 D–L)

| Grupo | Lenguaje |
|---|---|
| Dash | **Estelas azul frío** horizontales detrás del cuerpo, con jirones oscuros de tela/humo; doble dash con dos estelas |
| Wall-run / wall-jump / salto / aterrizaje | estela azul fina y **polvo gris** contra la pared o el suelo; aterrizaje con anillo de polvo |
| Tajos y estelas de espada | **media luna blanca azulada** que sigue al filo (horizontal, vertical, diagonal, aérea, en dash, cargada más ancha y brillante) |
| Impactos | chispas y astillas **blancas** en estrella (normal), más grandes (fuerte), con **rojo** (crítico) |
| Bloqueo / parry | **destello azul** y chispas en el punto de contacto; el parry, más grande y brillante |
| Daño / sangre | **sangre estilizada** rojo oscuro: gotas, salpicadura y proyección en abanico en la dirección del golpe (ligera / media / fuerte); **moderada**, sin gore |
| Muerte | goblin y élite: se **deshacen en fragmentos oscuros y polvo**; jefe: estallido mayor con fragmentos |
| Jefe fase 2 / telegrafiado | **arcos y anillos rojos** en el suelo antes del ataque (horizontal, vertical, embestida, de área, importante) |
| Ambiente | polvo, ceniza, fragmentos flotantes, humo, niebla, partículas suspendidas, luz ambiental, grietas |

Todo parametrizado y reutilizable sobre los componentes que ya hay (`USAEVUSComponenteParticulas`, estela del dash, efectos de combate, polvo), con versiones baratas para la Vega 8.

## UI (REF-08 A, C)

- **Paneles azul noche** semitransparentes con **marco fino** de metal gris y esquinas en ángulo; mucho espacio negativo; nada de cajas pesadas.
- HUD arriba a la izquierda: **emblema en rombo** con el retrato/icono, **barra de vida roja** larga y fina, barra de recurso azul debajo, pequeños iconos en rombo y contador.
- Jefe: icono en rombo, **nombre en la tipografía de títulos**, barra roja larga, indicador "Fase 2".
- Minimapa **circular** con marco fino; aviso de interacción con icono en rombo y tecla en un recuadro pequeño.
- Menús: listas verticales centradas con separadores finos; el elegido, con marco y brillo frío. "HAS MUERTO" en **rojo**; "VICTORIA" en hueso claro con recompensas en rombos.
- **Iconografía (C):** iconos de línea fina dentro de **rombos** con doble marco: vida (gota roja), ataque, defensa, dash, wall-run, parry, interacción, mapa, punto de control, jefe, objeto, estado negativo.
- Minimalista: no tapa el juego, sin aspecto de MMORPG.

## Tipografía (REF-08 B)

- **Principal (títulos):** romana clásica en **mayúsculas con versalitas**, remates finos y cincelados, aspecto antiguo y elegante ("SAEVUS", "RUINAS DE VARDEN", "REY GOBLIN", "CONFIGURACIÓN"), color **hueso** `#DFC6AD`. La que más se parece con licencia libre es **Cinzel** (OFL), ya pedida al dueño (tarea 78, sin los `.ttf` todavía).
- **Secundaria (información):** sans **humanista** limpia, legible a tamaño pequeño, blanco azulado. Mientras no haya otra, **Roboto** del motor.
- Botones: texto en mayúsculas de la secundaria dentro de un marco fino con esquinas cortadas.
- Licencias: toda fuente nueva se registra en `Documentos/interfaz.md` con su licencia antes de entrar en el proyecto.

## Decisiones (lo que la referencia no fija)

| Tema | Decisión mínima y reversible | Por qué |
|---|---|---|
| Nombre interno del protagonista | Se sigue llamando `Caballero` (`SK_Caballero`, `MI_Caballero_*`) | Evita renombrar código, tests y assets; el nombre no se ve en el juego |
| Dedos articulados (REF-02) | Mano de una pieza con garras, sin huesos de dedos | Animación procedural sin manos detalladas; la cámara está detrás y lejos |
| Capa | Piezas rígidas en tiras sobre los huesos de capa existentes; simulación más adelante | REF-02 pide "simulación simple"; hoy no hay física de tela barata |
| Detalle de placas, vendas, escamas | Geometría solo en lo que cambia la silueta (púas, aletas, bordes de la capa); el resto por color de material | REF-02, "detalles controlados"; Vega 8 |
| Texturas | Sin texturas pintadas por ahora: colores por pieza + post-proceso | Pipeline actual sin texturas; se añaden si el material no basta |
| Variantes de goblin | Tabla provisional de arriba; escudero derivado del guerrero | REF-05 no nombra las variantes |
| Fuentes | Cinzel (OFL) cuando el dueño ponga los `.ttf`; Roboto mientras tanto | REF-08 B; sin descargas no autorizadas |

## Rendimiento

Presupuestos de `pipeline-assets.md` (protagonista 20–30 k triángulos, ≤ 3 materiales como objetivo; hoy por ranura de color: se acepta más ranuras en el boceto y se reduce a color por vértice cuando el aspecto esté fijado). Nada de Lumen, Nanite, ray tracing ni sombras virtuales.

## Cómo se valida

Cada asset se compara con su referencia **en la cámara real del juego** (capturas de la prueba de humo), no solo en Blender. La comparación y lo que falta se apuntan en `REGISTRO_SESIONES.md`.

## Estado de producción

Qué está ya hecho según la referencia y qué falta. Se actualiza en cada bloque.

| Elemento | Referencia | Estado | Asset / código |
|---|---|---|---|
| Protagonista (modelo) | REF-01/02/03 | **v3 hecho** (silueta, cabeza, capa, espada, paleta); falta pulir dedos y placas | `blender_caballero.py`, `SK_Caballero`, `MI_Caballero_*` |
| Protagonista (poses) | REF-04 | reposo y carrera en curso (tarea 87); el resto, tarea 89 | `SAEVUSComponenteAnimacionProcedural` |
| Goblin base | REF-05 arriba 1 | modelo hecho; animación desde la IA en curso (tarea 92) | `blender_goblins.py`, `SK_Goblin` |
| Guerrero, lanzador | REF-05 arriba 2, abajo 1 | modelos hechos, sin importar | `SK_Goblin_Guerrero`, `SK_Goblin_Lanzador` |
| Asesino, élite, chamán | REF-05 arriba 3, abajo 3, abajo 2 | pendiente (no tienen papel en el juego todavía) | — |
| Rey Goblin | REF-06 | modelo fase 1 hecho; fase 2 (rojo) por datos pendiente | `SK_ReyGoblin` |
| Mazmorra (kit) | REF-07 | 19 piezas generadas, sin importar ni colocar | `blender_kit_mazmorra.py` |
| VFX (formas) | REF-08 D–L | 8 formas generadas, sin integrar | `blender_vfx.py` |
| HUD | REF-08 A | en preparación (emblema en rombo, barras finas, jefe con fase) | `SAEVUSWidgetInterfaz` |
| Tipografía | REF-08 B | pendiente de los `.ttf` de Cinzel | `SAEVUSEstiloInterfaz` |
| Iluminación y sombreado | REF-07, REF-08 L | borde frío rebajado en la ropa del protagonista; ajuste de `M_PP_Dibujado` pendiente (tarea 88) | `M_PP_Dibujado`, `M_SAEVUS_PersonajeDibujado` |
