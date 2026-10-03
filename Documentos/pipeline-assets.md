# Pipeline de assets de SAEVUS

Estado: decidido (2026-10-03) con las respuestas del dueño; todo cambiable. Lo **verificado** en esta máquina se marca así.

## Restricciones

- Aspecto: estilizado, dibujado, oscuro y postapocalíptico; **sombreado tipo Hollow Knight** (decisión del dueño). No fotorrealista.
- Hardware: la máquina de desarrollo es la referencia (Vega 8, 7 GB RAM). Objetivo: 60 fps en Media a 720p en niveles reales (`rendimiento.md`).
- Sin Lumen, Nanite, ray tracing, Virtual Shadow Maps, MegaLights ni shaders pesados.
- **Sin fuentes externas** por ahora (decisión de Claude, cambiable): todo se genera — mallas por script de Blender, efectos y animación por código, contenido del motor. Si algún día se aceptan, cada una con aprobación del dueño y registro de licencia.
- Fase de **prototipos y bocetos**: se pule animación y efectos, no el diseño final.

## Sombreado

Default Lit + colores planos casi sin especular (rugosidad alta), paleta oscura con acentos claros (silueta negra, máscara blanca), niebla de altura para la profundidad, y un **contorno por post-proceso** barato (detección de bordes por profundidad), activable por escalabilidad. Materiales maestros pocos y con instancias; ≤ 4 muestras de textura por material opaco. Sin translucidez grande. Un modelo de sombreado propio (rampa toon) exigiría compilar el motor: descartado.

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
- **Esqueleto personalizado desde el principio** (decisión del dueño): básico y modificable, nombres de huesos en español, sin huesos hoja, solo huesos de deformación, armature a escala 1. Pendiente de una sonda esquelética antes del primer personaje.
- Origen en el centro de los pies (personajes, objetos) o en el punto de rejilla (piezas modulares). Aplicar transformaciones antes de exportar.
- Archivos fuente (`.blend`, `.psd`, `.png`) en `ArteFuente/`, fuera de `Content/`, con LFS.

## Nombres y carpetas

Carpeta por asset: `/Game/SAEVUS/Arte/<Categoría>/<Nombre>/` (`Personajes`, `Enemigos`, `Armas`, `Objetos`, `Escenario/<Kit>`). Prefijos: `SM_`, `SK_`, `SKEL_`, `PHYS_`, `ABP_`, `AM_`, `BS_`, `M_`, `MI_`, `MF_`, `T_`, `PP_`. Sufijos de textura: `_BC` (color base, sRGB), `_N` (normal, BC5), `_ORM` (oclusión/rugosidad/metálico, lineal), `_M` (máscaras), `_E` (emisivo).

## Animación (prototipo)

- Locomoción en el sitio, dirigida por CMC (sin root motion).
- Fase de bocetos: **animación procedural en C++** sobre el esqueleto personalizado, derivada del estado de movimiento y de la línea de tiempo de cada ataque, así nunca se desincroniza y se ajusta por datos (`animacion.md`). Las animaciones hechas a mano en Blender la sustituirán cuando haya diseño final.

## Validación

- `Ejecutar-SondaPipeline.ps1` (hecho); falta la sonda esquelética.
- Más adelante: reglas de validación de datos (prefijos, presupuestos, colisión).
