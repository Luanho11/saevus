# Animación de SAEVUS (prototipo procedural)

Fase de bocetos: el caballero se anima **por código** sobre su esqueleto propio, sin clips. Así la pose sale siempre del estado real del juego (movimiento, línea de tiempo del ataque, armas, vida) y nunca se desincroniza. Las animaciones hechas a mano la sustituirán cuando haya diseño final.

## Piezas

| Pieza | Dónde | Qué hace |
|---|---|---|
| Caballero boceto | `Herramientas/Arte/blender_caballero.py` → `ArteFuente/Caballero/SK_Caballero.{fbx,blend}` | Malla de piezas rígidas (peso 1 por hueso) + esqueleto de 24 huesos con nombres en español. Se regenera con `Herramientas/Arte/Generar-Caballero.ps1`. |
| Importación | `saevus_crear_assets.py` → `importar_caballero()` | Importa el FBX a `/Game/SAEVUS/Arte/Personajes/Caballero` y lo asigna al componente `Cuerpo` del `BP_SAEVUSPersonaje`. |
| Cuerpo | `ASAEVUSPersonaje::Cuerpo` (`UPoseableMeshComponent`) | Malla visible, colgada de la malla del personaje (hereda pies en Z = −96 y la corrección de −90°). La malla del maniquí queda vacía. |
| Animación | `USAEVUSComponenteAnimacionProcedural` (`Source/SAEVUS/Animacion/`) | Construye la pose objetivo cada fotograma y la aplica con suavizado. |

## Esqueleto

`raiz → pelvis → columna → pecho → cuello → cabeza`; `pecho → capa_1 → capa_2`; `pecho → hombro_{d,i} → brazo → antebrazo → mano`; `mano_d → arma, arma_fuego`; `pelvis → muslo_{d,i} → pierna → pie`. Enum `ESAEVUSHueso`.

## Cómo se calcula la pose

1. **Pose objetivo por estado** (por prioridad): muerte → escalada → carrera por pared → dash → aire (con recogida de piernas en el doble salto) → curación → locomoción; encima, el ataque en curso (por fase: preparación, activa, recuperación, con trayectoria por índice de combo o tajo aéreo) o el disparo (retroceso por el evento `AlDisparar`); la capa siempre, según la velocidad.
2. Cada hueso tiene un **objetivo**: apuntar a una dirección del espacio de la malla, o un giro relativo.
3. **FK propio** sobre la pose de reposo: `Q_hueso = (Q_padre_final · Q_padre_reposo⁻¹) · Giro · Q_hueso_reposo`; al apuntar, el giro mínimo entre la dirección de reposo heredada y la deseada.
4. **Suavizado exponencial** de las rotaciones finales (`Suavizado`, más seco en acciones con `SuavizadoAccion`, más lento en la capa para que ondee). Las posiciones se recalculan desde las rotaciones suavizadas, así los huesos conservan su longitud.
5. Se escribe en espacio de componente con `SetBoneTransformByName`, **conservando la escala de reposo** de cada hueso. La espada y la pistola se muestran u ocultan escalando su hueso a 0 o a 1.

Ejes del espacio de la malla: Adelante = +Y, Derecha = −X, Arriba = +Z.

Además, durante la invulnerabilidad tras un golpe el cuerpo parpadea (se oculta y se muestra a `FrecuenciaParpadeo`), y al recibir el golpe el torso y la cabeza se echan atrás (`DuracionReaccionGolpe`, `InclinacionReaccionGolpe`).

**Estela del arma:** `ObtenerHoja()` da la base y la punta del aguijón en el mundo (hueso `arma` + `LongitudHoja` en su dirección actual). `USAEVUSComponenteEfectosCombate` reparte partículas finas a lo largo de la hoja en cada fotograma de la fase activa del tajo, más densas hacia la punta: dibujan el arco del golpe.

## Ajustes

`FSAEVUSAjusteAnimacion` (en el componente): longitud de ciclo, amplitudes de muslo, rodilla y brazo, inclinación al correr, en el dash y en la pared, respiración, suavizados, aterrizaje y retroceso. Todo editable sin compilar.

## Lección aprendida: escala de reposo

El FBX de Blender llega con **escala 100 en la raíz** (unidades del armazón). Al escribir escala 1, todo el cuerpo se encogía al 1 % y quedaba invisible, aunque las posiciones de los huesos (lo que miden los tests) eran correctas. Ahora se conserva la escala de reposo. Lo vigilan el test `SAEVUS.Animacion.EscalaReposo` y la comprobación "cuerpo animado con tamaño de personaje" de la prueba de humo.

## Tests

`SAEVUS.Animacion.{Esqueleto, EscalaReposo, Correr, Ataque, TajoArriba, EstelaArma, Dash, ArmaVisible}`. La prueba de humo guarda las capturas `SAEVUS_PruebaHumo_{Correr,Dash,Ataque}.png` en `Saved/Screenshots/WindowsEditor/`.

## Pendiente

- ~~Los materiales `M_Caballero_*` no sustituyen a los que crea Interchange~~: resuelto en la tarea 5 con *overrides* `MI_Caballero_*` en el componente (`pipeline-assets.md`, Sombreado).
- La escala 0 del arma oculta no la saca de los límites de física.
- Pulido: anticipación y estela de los tajos, transiciones de cancelación más marcadas, cara mirando a la cámara en el reposo.
