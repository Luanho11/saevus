# Pipeline de assets de SAEVUS

Estado: decidido (2026-10-03) con las respuestas del dueño; todo cambiable. Lo **verificado** en esta máquina se marca así.

## Restricciones

- Aspecto: estilizado, dibujado, oscuro y postapocalíptico; **sombreado tipo Hollow Knight** (decisión del dueño). No fotorrealista.
- Hardware: la máquina de desarrollo es la referencia (Vega 8, 7 GB RAM). Objetivo: 60 fps en Media a 720p en niveles reales (`rendimiento.md`).
- Sin Lumen, Nanite, ray tracing, Virtual Shadow Maps, MegaLights ni shaders pesados.
- **Sin fuentes externas** por ahora (decisión de Claude, cambiable): todo se genera — mallas por script de Blender, efectos y animación por código, contenido del motor. Si algún día se aceptan, cada una con aprobación del dueño y registro de licencia.
- Fase de **prototipos y bocetos**: se pule animación y efectos, no el diseño final.

## Sombreado

Default Lit + colores planos casi sin especular (rugosidad alta), paleta oscura con acentos claros (silueta negra, máscara blanca), niebla de altura para la profundidad, y un **contorno por post-proceso** barato (detección de bordes por profundidad). Materiales maestros pocos y con instancias; ≤ 4 muestras de textura por material opaco. Sin translucidez grande. Un modelo de sombreado propio (rampa toon) exigiría compilar el motor: descartado.

**Hecho (tarea 5)**, todo en `/Game/SAEVUS/Arte/Estilo/` y creado por `saevus_crear_assets.py`:
- `M_SAEVUS_PersonajeDibujado`: maestro de personaje. Parámetros `Color`, `Rugosidad` (1), `Especular` (0,15), `ColorBorde` + `IntensidadBorde` + `ExponenteBorde` (borde de luz fría por Fresnel, separa la silueta oscura del fondo oscuro) y `AutoIluminacion` (la máscara brilla un poco). Sin texturas.
- `MI_Caballero_{Cuerpo,Mascara,Capa,Metal,Ojos}`: instancias por pieza, aplicadas como *override* del componente `Cuerpo` (Interchange rehace los materiales del asset al reimportar).
- `M_PP_Dibujado` (tarea 69, sustituye a `M_PP_Contorno`): un solo post-proceso después del DOF (color lineal, antes del tonemapper), 10 lecturas de textura:
  - **Bandas de luz:** la luz recibida (color / albedo difuso del GBuffer) se cuantiza en `Bandas` (3) planas con bordes suaves (`SuavidadBandas`), en raíz cuadrada; las bandas bajas se enfrían hacia `ColorSombra` (azul). Peso según el albedo (lo negro no se toca) y la distancia.
  - **Niebla en capas:** desde `InicioNiebla` (9 m) a lo largo de `RangoNiebla` (50 m), en `CapasNiebla` (4) capas planas que se aclaran hacia `ColorNiebla`: lo cercano queda en silueta oscura sobre fondos más claros, como los fondos pintados.
  - **Tinta:** siluetas por laplaciano de la **inversa** de la profundidad (una superficie plana da 0, así el suelo no se raya) y pliegues por cambio de normal, a `Grosor` (2) píxeles, en `ColorTinta`; se aclara con la niebla.
  - **Grano de papel** fijo en pantalla (`Grano`, 6 %).
  - Todo son parámetros del material (`PARAMETROS_DIBUJADO` y `COLORES_DIBUJADO` en el script).
- `M_SAEVUS_PersonajeDibujado`: especular bajado a 0,04 (sin brillos de luz).
- La cámara lo aplica: `FSAEVUSAjusteCamara` → `MaterialDibujado` (nulo = sin él), `IntensidadVineta` (0,55) y `Saturacion` (0,8), en `DA_AjustesCamara_Base`. Test `SAEVUS.Estilo.PostProcesoCamara`. Prueba de humo con él: 47 fps de media en Media a 720p (antes 44).
- Coste medido: prueba de humo a 720p en Media, media de 63 fps (la línea base era 51–68).
- Pendiente: iluminación oscura de los mapas (llega con `L_Tutorial`); bajar el contorno por escalabilidad si hace falta.

## Presupuestos (LOD0)

| Clase | Triángulos | Texturas | Tamaño máx. | Materiales |
|---|---|---|---|---|
| Personaje jugador | 20–30 k | 1–2 juegos | 2048 | ≤ 3 |
| Enemigo normal | 10–20 k | 1 | 2048 | ≤ 2 |
| Jefe | 40–60 k | 2 | 2048 | ≤ 4 |
| Arma | 2–6 k | 1 | 1024 | 1 |
| Objeto pequeño / grande | 0,5–5 k | atlas compartidos | 1024 / 2048 | 1–2 |
| Pieza modular de escenario | 0,2–4 k | 1–2 por kit | 2048 | 1–2 |

Escena en Media a 720p: < 1.500 draw calls y < 1,5 M triángulos visibles; pool de streaming de 512 MB. Los bocetos generados están muy por debajo.

## LOD y descarte

Personajes: 3 LOD (100/50/25 %) con la reducción del motor. Objetos: 2–3 LOD según tamaño y distancia de descarte para los pequeños.

## Métricas de juego para escenarios

| Métrica | Valor |
|---|---|
| Cima del salto | 1,43 m |
| Escalón (`MaxStepHeight`) | 0,45 m |
| Pendiente caminable | < 44,8° |
| Paredes para correr / saltar / escalar | planas, más de ~72° (normal Z ≤ 0,3) |
| Carrera por pared completa | ~9 m |
| Escalada | ~3,4 m (más el salto previo; una pared de 3,5 m se corona) |
| Hueco de chimenea de salto de pared | ~3 m |

Colisión simple (cajas / envolventes convexas) en todo lo que toca el personaje; las paredes para movimiento, con una sola cara plana de colisión.

## Blender → Unreal

Verificado con `Herramientas/Pipeline/Ejecutar-SondaPipeline.ps1` (Blender 4.2 → UE 5.8 por Interchange):
- Escena métrica, escala 1,0, modelar a tamaño real en metros. **Verificado:** 1,8 m en Blender = 180 cm en UE, de pie en Z = 0.
- FBX con ejes por defecto de Blender (adelante −Z, arriba Y), *Apply Unit*, *FBX Units Scale None*. glTF también funciona igual.
- **Orientación verificada:** un personaje que mira a −Y en Blender mira a **+Y** en UE, como el maniquí; la corrección de −90° del personaje sirve tal cual.
- **Formato (decisión):** FBX para mallas esqueléticas y animación; FBX o glTF para estáticas.
- **Esqueleto personalizado desde el principio** (decisión del dueño): básico y modificable, nombres de huesos en español, sin huesos hoja, solo huesos de deformación. Primer personaje: el caballero boceto (`animacion.md`). El armazón se llama `Armature`, así UE no lo añade como hueso. **Verificado:** la raíz llega con **escala 100** en la pose de reposo; quien escriba poses debe conservarla.
- Origen en el centro de los pies (personajes, objetos) o en el punto de rejilla (piezas modulares). Aplicar transformaciones antes de exportar.
- Archivos fuente (`.blend`, `.psd`, `.png`) en `ArteFuente/`, fuera de `Content/`, con LFS.

## Personajes desde las vistas de las referencias (2026-10-06)

Cuando una referencia tiene vistas de frente, perfil y espalda (REF-01 protagonista, REF-06 Rey Goblin), la forma y el color salen del dibujo, no de primitivas a mano:
1. `reconstruir_desde_vistas.py` (Python del sistema: numpy, scipy, PIL) con `Herramientas/Arte/vistas/<personaje>.json`: recorte de cada vista, registro en metros (pies en Z = 0, mira a −Y), casco visual por pieza (frente y perfil; la espalda solo colorea, porque intersecar dos dibujos que no coinciden del todo come volumen), elipses por tramo para que brazos y piernas sean redondos y color de las vistas que ven cada vóxel. Cabeza tallada aparte con los primeros planos, a más resolución.
2. `blender_malla_desde_puntos.py`: puntos → malla densa con color por vértice (intermedio en `Saved/ArteIntermedio/`, fuera del repo).
3. `blender_hornear_color.py`: copia reducida (≈ 30 000 triángulos) con UV automáticas y la pintura horneada en `T_<personaje>.png` (Cycles).
4. `blender_piezas.atar_reconstruccion`: esqueleto de 25 huesos con las articulaciones **medidas** en la reconstrucción (tramos de la silueta por altura), pesos automáticos (los vértices sin peso, al hueso más cercano) y cada hueso llevado a la dirección de reposo que espera la animación procedural; esa pose pasa a ser el reposo. Las armas siguen siendo piezas (`colocar_armas`), con su ranura propia.
En UE la ranura `Referencia` usa `M_SAEVUS_PersonajeDibujado` con el parámetro `Textura` (blanca por defecto en las demás ranuras). Los goblins de REF-05 solo tienen vistas de tres cuartos posadas: siguen con el modelo por piezas.

## Escenario (boceto)

`Herramientas/Arte/blender_escenario.py` genera piezas modulares estáticas (FBX por pieza, origen en la base, metros, semilla fija) que `saevus_crear_assets.py` importa a `/Game/SAEVUS/Arte/Escenario/` con el material de escenario. Presupuesto del boceto: menos de 100 triángulos por pieza.

## Nombres y carpetas

Carpeta por asset: `/Game/SAEVUS/Arte/<Categoría>/<Nombre>/` (`Personajes`, `Enemigos`, `Armas`, `Objetos`, `Escenario/<Kit>`). Prefijos: `SM_`, `SK_`, `SKEL_`, `PHYS_`, `ABP_`, `AM_`, `BS_`, `M_`, `MI_`, `MF_`, `T_`, `PP_`. Sufijos de textura: `_BC` (color base, sRGB), `_N` (normal, BC5), `_ORM` (oclusión/rugosidad/metálico, lineal), `_M` (máscaras), `_E` (emisivo).

## Animación (prototipo)

- Locomoción en el sitio, dirigida por CMC (sin root motion).
- Fase de bocetos: **animación procedural en C++** sobre el esqueleto personalizado, derivada del estado de movimiento y de la línea de tiempo de cada ataque, así nunca se desincroniza y se ajusta por datos (`animacion.md`). Las animaciones hechas a mano en Blender la sustituirán cuando haya diseño final.

## Validación

- `Ejecutar-SondaPipeline.ps1` (hecho). La importación esquelética se valida con el caballero: tests `SAEVUS.Animacion.*` y `saevus_verificar_assets.py`.
- Más adelante: reglas de validación de datos (prefijos, presupuestos, colisión).
