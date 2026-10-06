# Animación de SAEVUS (prototipo procedural)

Fase de bocetos: el caballero se anima **por código** sobre su esqueleto propio, sin clips. Así la pose sale siempre del estado real del juego (movimiento, línea de tiempo del ataque, armas, vida) y nunca se desincroniza. Las animaciones hechas a mano la sustituirán cuando haya diseño final.

## Piezas

| Pieza | Dónde | Qué hace |
|---|---|---|
| Protagonista (nombre interno "Caballero") | `Herramientas/Arte/blender_caballero.py` (+ ayudantes `blender_piezas.py`) → `ArteFuente/Caballero/SK_Caballero.{fbx,blend}` | Modelo v3 según REF-01/02/03 (`ARTE_SAEVUS.md`): 1,92 m hasta las púas, casco de placas en hoja con aletas, máscara de hueso en V con ojos cian, capa con capucha y tiras dentadas, broche de anillo, correas en X, faldón, brazales de hueso, pies de garra, espada de 0,88 m con guarda de anillo; ≈ 7 200 triángulos, 9 ranuras de color. Piezas rígidas salvo capa y faldón (pesos repartidos por altura). Mismos 25 huesos. Se regenera con `Generar-Caballero.ps1`; `blender_vistas_comparacion.py` saca frente/perfil/espalda/juego para compararlo con las referencias. |
| Importación | `saevus_crear_assets.py` → `importar_caballero()` | Importa el FBX a `/Game/SAEVUS/Arte/Personajes/Caballero` y lo asigna al componente `Cuerpo` del `BP_SAEVUSPersonaje`. |
| Cuerpo | `ASAEVUSPersonaje::Cuerpo` (`UPoseableMeshComponent`) | Malla visible, colgada de la malla del personaje (hereda pies en Z = −96 y la corrección de −90°). La malla del maniquí queda vacía. |
| Animación | `USAEVUSComponenteAnimacionProcedural` (`Source/SAEVUS/Animacion/`) | Construye la pose objetivo cada fotograma y la aplica con suavizado. |

## Esqueleto

`raiz → pelvis → columna → pecho → cuello → cabeza`; `pecho → capa_1 → capa_2 → capa_3` (`capa_3` no se anima: sigue a `capa_2`); `pecho → hombro_{d,i} → brazo → antebrazo → mano`; `mano_d → arma, arma_fuego`; `pelvis → muslo_{d,i} → pierna → pie`. Enum `ESAEVUSHueso`.

## Cómo se calcula la pose

1. **Pose objetivo por estado** (por prioridad): muerte → escalada → carrera por pared → dash → aire (con recogida de piernas en el doble salto) → sentado en un banco (raíz baja, muslos adelante, manos en las rodillas) → curación → locomoción; encima, el ataque en curso (por fase: preparación, activa, recuperación, con trayectoria por índice de combo o tajo aéreo) o el disparo (retroceso por el evento `AlDisparar`); la capa siempre, según la velocidad.
2. Cada hueso tiene un **objetivo**: apuntar a una dirección del espacio de la malla, o un giro relativo.
3. **FK propio** sobre la pose de reposo: `Q_hueso = (Q_padre_final · Q_padre_reposo⁻¹) · Giro · Q_hueso_reposo`; al apuntar, el giro mínimo entre la dirección de reposo heredada y la deseada.
4. **Suavizado exponencial** de las rotaciones finales (`Suavizado`, más seco en acciones con `SuavizadoAccion`, más lento en la capa para que ondee). Las posiciones se recalculan desde las rotaciones suavizadas, así los huesos conservan su longitud.
5. Se escribe en espacio de componente con `SetBoneTransformByName`, **conservando la escala de reposo** de cada hueso. La espada y la pistola se muestran u ocultan escalando su hueso a 0 o a 1.

Ejes del espacio de la malla: Adelante = +Y, Derecha = −X, Arriba = +Z.

Además, durante la invulnerabilidad tras un golpe el cuerpo parpadea (se oculta y se muestra a `FrecuenciaParpadeo`), y al recibir el golpe el torso y la cabeza se echan atrás (`DuracionReaccionGolpe`, `InclinacionReaccionGolpe`).

**Mirada:** si ninguna pose ocupa la cabeza, se gira hacia el enemigo vivo más cercano dentro de `RadioMirada` (15 m), con `GiroMaximoCabeza` (60°) e `InclinacionMaximaCabeza` (30°); detrás del todo no se gira. Lee mejor de dónde viene el peligro.

**Estela del arma:** `ObtenerHoja()` da la base y la punta del aguijón en el mundo (hueso `arma` + `LongitudHoja` en su dirección actual). `USAEVUSComponenteEfectosCombate` reparte partículas finas a lo largo de la hoja en cada fotograma de la fase activa del tajo, más densas hacia la punta: dibujan el arco del golpe.

## Animación direccional (GunZ, tarea 68)

Como el cuerpo mira siempre a la cámara (`movimiento.md`, Orientación), la animación lee la dirección del paso en el espacio de la malla (`DireccionMalla`):
- **Locomoción en ocho direcciones:** los muslos oscilan en la dirección del paso (hacia atrás, de espaldas); de lado, la pelvis gira `GiroCaderaLateral` (35°) hacia el paso (al revés si va hacia atrás) y la columna lo compensa, así el pecho sigue de frente; hacia atrás la inclinación se reduce a `FraccionInclinacionAtras` y se echa atrás. La carrera por pared usa lo mismo: las piernas corren a lo largo de la pared.
- **Dash direccional:** estocada inclinada hacia la dirección del dash (REF-04). La voltereta de antes (`GradosVolteretaDash` > 0, a los lados o hacia atrás por debajo de `UmbralDashFrontal`) sigue disponible por datos, apagada.
- **Salto de pared:** el movimiento emite `AlSaltoPared` con la normal de la pared; el cuerpo empuja con la mano más cercana durante `DuracionGiroSaltoPared` (0,4 s). El giro de antes (`GradosGiroSaltoPared` > 0) sigue disponible por datos, apagado.
- **Aterrizaje según la caída:** la flexión se escala con la velocidad de caída (`VelocidadCaidaAterrizaje`, 900 cm/s = flexión completa; entre 0,35 y 1,3).
- Test `SAEVUS.Animacion.Direccional`: correr de lado mueve los pies de lado y el dash lateral inclina el cuerpo hacia su lado.

## Poses de REF-04 (producción artística, tareas 87 y 89)

Las 15 poses numeradas de REF-04 (`REFERENCIAS_ARTE.md`) son el objetivo; donde contradecían a la animación anterior, manda la referencia:
- **1 Reposo:** la espada baja en diagonal, adelantada y abierta (`GradosHojaReposoAdelante`, `GradosHojaReposoAfuera`), rodillas algo dobladas (`FlexionRodillaReposo`).
- **2–3 Carrera y sprint:** tronco inclinado (`InclinacionCarrera`, 18° a la velocidad de referencia, más al esprintar), cadera baja (`BajadaCarrera`), el brazo de la espada atrás con la hoja casi horizontal (`GradosBrazoArmaCarrera`, `GradosHojaCarrera`) y el brazo libre adelante y doblado.
- **4–5 Salto y doble salto:** rodillas recogidas, la espada abajo y atrás, el brazo libre adelante.
- **6–8 Dash adelante, atrás y lateral:** la misma estocada baja inclinada hacia donde va, con zancada en esa dirección y la espada atrás. **Sin voltereta** (`GradosVolteretaDash` = 0): la referencia no la muestra (antes la ponía la tarea 68).
- **9 Carrera por pared:** la mano del lado de la pared la toca; la espada atrás, a lo largo de la carrera.
- **10 Salto desde pared:** empuje con la mano contra la pared y el cuerpo abierto hacia fuera durante `DuracionGiroSaltoPared`; **sin giro de 360°** (`GradosGiroSaltoPared` = 0).
- **11 Aterrizaje fuerte:** desde `FuerzaAterrizajeFuerte`, cuclillas abiertas, la mano libre al suelo y la espada afuera.
- **12–15 Ataques:** los tajos por línea de tiempo de siempre; su pulido va con el combate (tarea 91).

## Polvo del movimiento (tarea 74)

`USAEVUSComponentePolvoMovimiento` (`Efectos/`, partículas por CPU en mallas instanciadas con `M_ParticulaPolvo`, como la estela del dash): anillo de polvo a los pies al aterrizar, más grande cuanto más rápida la caída (`VelocidadCaidaReferencia`; por debajo de `VelocidadCaidaMinima` nada, así bajar un escalón no levanta polvo); rastro donde los pies pisan la pared al correr por ella o escalar; estallido contra la pared en el salto de pared (el evento `AlSaltoPared` lleva ahora la normal de la pared); polvo a ras de suelo, hacia atrás, en el dash por el suelo. Todo en `FSAEVUSAjustePolvoMovimiento`. Test `SAEVUS.Efectos.PolvoAterrizaje`.

## Ajustes

`FSAEVUSAjusteAnimacion` (en el componente): longitud de ciclo, amplitudes de muslo, rodilla y brazo, inclinación al correr, en el dash y en la pared, respiración, suavizados, aterrizaje y retroceso. Todo editable sin compilar.

## Lección aprendida: escala de reposo

El FBX de Blender llega con **escala 100 en la raíz** (unidades del armazón). Al escribir escala 1, todo el cuerpo se encogía al 1 % y quedaba invisible, aunque las posiciones de los huesos (lo que miden los tests) eran correctas. Ahora se conserva la escala de reposo. Lo vigilan el test `SAEVUS.Animacion.EscalaReposo` y la comprobación "cuerpo animado con tamaño de personaje" de la prueba de humo.

## Tests

`SAEVUS.Animacion.{Esqueleto, EscalaReposo, Correr, Ataque, TajoArriba, EstelaArma, MiradaEnemigo, Dash, ArmaVisible, Direccional}`. La prueba de humo guarda las capturas `SAEVUS_PruebaHumo_{Correr,Dash,Ataque}.png` en `Saved/Screenshots/WindowsEditor/`.

## Pendiente

- ~~Los materiales `M_Caballero_*` no sustituyen a los que crea Interchange~~: resuelto en la tarea 5 con *overrides* `MI_Caballero_*` en el componente (`pipeline-assets.md`, Sombreado).
- La escala 0 del arma oculta no la saca de los límites de física.
- Pulido: anticipación y estela de los tajos, transiciones de cancelación más marcadas, cara mirando a la cámara en el reposo.
