# Referencias de arte de SAEVUS

Las imágenes de `assets/` son la **referencia visual canónica** (decisión del dueño, 2026-10-06). No se borran, no se renombran y no se sobrescriben: este documento les da un identificador estable (`REF-xx`) para citarlas en código, commits y documentos.

Prioridad de decisión: 1) la referencia; 2) decisiones explícitas del dueño; 3) arquitectura de SAEVUS; 4) escalabilidad; 5) rendimiento; 6) criterio técnico. La dirección que se deriva de ellas está en `ARTE_SAEVUS.md`.

## Tabla

| Id | Archivo (`assets/`) | Qué representa | Uso | Prioridad | Asset del proyecto relacionado |
|---|---|---|---|---|---|
| REF-01 | `52f674a4-06a6-4c96-936d-6a4060734f48.png` | **Protagonista, hoja principal**: pose heroica 3/4 con la espada, cabeza en primer plano (frente, perfil, 3/4), espalda, vistas frente/perfil/espalda abajo, espada aislada, detalles de guarda con anillo, broche y brazal. | Diseño canónico del protagonista: silueta, cabeza/máscara, capa, colores, espada. | **Máxima** | `Herramientas/Arte/blender_caballero.py` → `SK_Caballero`; `MI_Caballero_*`; `LongitudHoja` |
| REF-02 | `50bcbc56-82f1-46ab-9b3f-089d7aa7c9b2.png` | **Protagonista, hoja de construcción**: desglose de 14 componentes, tipo de pieza (deformable / rígida / separada), malla, esqueleto de referencia, detalle por zonas, articulaciones, silueta y proporciones en cuatro vistas, movimiento de capa y ropa, geometría recomendada para Blender. | Qué piezas van separadas, qué es rígido y qué deforma; proporciones; esqueleto; cómo se mueve la capa. | **Máxima** | `blender_caballero.py` (piezas, huesos); `ESAEVUSHueso`; `USAEVUSComponenteAnimacionProcedural` (capa) |
| REF-03 | `c5ca5c0b-f694-42d4-beea-3b09db0e9b04.png` | **Protagonista, primeros planos y poses**: diez encuadres de busto y cuerpo (reposo, mano en la espada, de frente con broche, capa cerrada, herido con sangre en el brazo, agachado en el suelo, de pie sobre una roca con la espada clavada, de espaldas envuelto en la capa). | Lectura de la cabeza desde muchos ángulos (cámara de juego), capa cerrada y abierta, poses de reposo, herida y caída. | Alta | `blender_caballero.py` (cabeza, capa); poses de reposo, golpe recibido y muerte |
| REF-04 | `79ad8a06-e9cd-4af1-bb8d-91d8739af6b0.png` | **Protagonista, hoja de movimiento**: 15 poses numeradas — 1 reposo, 2 carrera, 3 sprint, 4 salto, 5 doble salto, 6 dash adelante, 7 dash atrás, 8 dash lateral, 9 wall-run, 10 salto desde pared, 11 aterrizaje fuerte, 12 ataque horizontal, 13 ataque diagonal, 14 ataque durante dash, 15 ataque aéreo. | **Poses objetivo** de la animación; comprobar que el diseño permite cada pose (capa, piernas, espada). | **Máxima** (animación) | `USAEVUSComponenteAnimacionProcedural`, `FSAEVUSAjusteAnimacion` |
| REF-05 | `d69d0a8c-b84d-409c-9200-2a28f663987c.png` | **Goblins**: seis goblins de la misma especie con equipo distinto (ver `ARTE_SAEVUS.md`, Goblins). | Diseño canónico de la especie y base de las variantes. | **Máxima** (enemigos) | `blender_enemigos.py` → `SM_Goblin*`; `DA_Enemigo_Goblin*` |
| REF-06 | `417ae206-df5e-404e-ab3b-932faf992720.png` | **Rey Goblin**: vistas frente/3/4/perfil/espalda (fase 1), cuatro armas de asta, detalles de corona, broche y faldón; fila de poses de combate (golpe bajo, carrera, salto con arma alzada, barrido con la guadaña, golpe al suelo); fila de fase 2 (telas y acentos rojos, ojos rojos) con detalles de cabeza, corona y guadaña. | Diseño canónico del jefe, sus armas, poses de ataque y su fase 2. | **Máxima** (jefe) | `SM_Jefe_ReyGoblin`; `DA_Enemigo_ReyGoblin` |
| REF-07 | `b7f5680d-5a24-4d6f-adcd-d92a5b6bd61a.png` | **Dungeon**: plano de planta con rutas por colores y leyenda, vista isométrica del conjunto, tres alzados laterales (niveles), seis salas con su icono y su pieza de planta (entrada, plataformas, sala de columnas, pozo vertical, pasillo, arena octogonal), **kit modular** (suelos, plataformas, muros, columnas, arcos, puertas, escaleras, escombros, estandartes, marcos), diagramas de movilidad (pozo con cornisas, carrera y salto de pared, saltos entre plataformas, escalada). | Estructura y kit modular de la mazmorra; principios de verticalidad y rutas. | **Máxima** (escenario) | `blender_escenario.py`, `saevus_crear_assets.py` (`L_Mazmorra`, `L_Tutorial`) |
| REF-08 | `bbdd90ce-19ff-43ad-a668-91fcfbf70db6.png` | **Hoja maestra de juego**: A HUD/UI (vida, recursos, jefe con "Fase 2", minimapa, interacción, menú principal, opciones, pausa, muerte, victoria, notificaciones); B tipografía; C iconografía; D dash/movimiento; E espada/estelas; F impactos; G sangre; H parry/bloqueo; I muerte (goblin, goblin élite, jefe); J jefe fase 1 y 2; K telegrafiados; L ambiente; tira de escenarios; comparación de escala protagonista / Rey Goblin. | Lenguaje de VFX, UI, tipografía, iconos, telegrafiado, ambiente y escala. | **Máxima** (VFX/UI) | `USAEVUSComponenteEstelaDash`, `USAEVUSComponenteEfectosCombate`, `USAEVUSComponentePolvoMovimiento`, `USAEVUSWidgetInterfaz`, `USAEVUSWidgetPausa`, `M_PP_Dibujado` |

## Índice por elemento

| Elemento | Referencias (por prioridad) |
|---|---|
| Protagonista | REF-01, REF-02, REF-03 |
| Vistas del protagonista | REF-01 (frente, perfil, espalda), REF-02 (silueta y proporciones, cuatro vistas) |
| Construcción del protagonista | REF-02 |
| Movimiento / animación | REF-04; REF-02 (movimiento de capa: reposo, carrera, salto, doble salto, aterrizaje); REF-08 D |
| Espada | REF-01 (espada aislada y guarda), REF-02 (espada) |
| Goblins | REF-05; REF-08 I (muerte de goblin) |
| Variantes de goblins | REF-05 (seis siluetas); REF-08 I (goblin élite) |
| Rey Goblin | REF-06; REF-08 J (fase 1 y 2), K (ataque importante), I (muerte del jefe), escala |
| Dungeon | REF-07; REF-08 tira de escenarios y L |
| Módulos de Dungeon | REF-07 (kit modular, piezas de planta por sala) |
| VFX | REF-08 D, E, F, G, H, I, K, L |
| Partículas | REF-08 L (polvo, ceniza, fragmentos, humo, partículas flotantes), D (estelas) |
| Dash | REF-08 D (frontal, trasero, lateral, diagonal, doble); REF-04 6–8 |
| Wall-run | REF-08 D; REF-04 9–10; REF-07 (diagramas de movilidad) |
| Impactos | REF-08 F (normal, fuerte, crítico, bloqueo, parry, aturdido, recibe daño) |
| Sangre | REF-08 G (gotas, salpicadura, mancha, proyección ligera/media/fuerte); REF-03 (herida en el brazo) |
| UI | REF-08 A |
| Tipografía | REF-08 B |
| Iconografía | REF-08 C |
| Paleta | todas; medida en `ARTE_SAEVUS.md`, Paleta |
| Iluminación | REF-07 (salas), REF-08 L (luz ambiental) y tira de escenarios |

## Notas

- Son imágenes generadas, no planos técnicos: entre vistas hay pequeñas diferencias (número de púas, largo de la capa, placas de las piernas). Ante una diferencia manda REF-01 para el protagonista, REF-06 para el jefe y REF-05 para los goblins, y se elige la versión más simple (`ARTE_SAEVUS.md`, Decisiones).
- Los textos de las hojas están en español y forman parte de la referencia ("Ruinas de Varden", "Rey Goblin", "Has muerto", "Victoria", "Continuar").
- `assets/` está en Git LFS (los `.png` del repositorio van por LFS, `.gitattributes`).
