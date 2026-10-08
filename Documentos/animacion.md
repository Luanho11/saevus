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
5. Se suma la capa secundaria y se escribe de una vez (transformados locales), **conservando la escala de reposo** de cada hueso. La espada y la pistola se muestran u ocultan escalando su hueso a 0 o a 1.

Ejes del espacio de la malla: Adelante = +Y, Derecha = −X, Arriba = +Z.

Además, durante la invulnerabilidad tras un golpe el cuerpo parpadea (se oculta y se muestra a `FrecuenciaParpadeo`), y al recibir el golpe el torso y la cabeza se echan atrás (`DuracionReaccionGolpe`, `InclinacionReaccionGolpe`).

**Mirada:** si ninguna pose ocupa la cabeza, se gira hacia el enemigo vivo más cercano dentro de `RadioMirada` (15 m), con `GiroMaximoCabeza` (60°) e `InclinacionMaximaCabeza` (30°); detrás del todo no se gira. Lee mejor de dónde viene el peligro.

**Estela del arma:** `ObtenerHoja()` da la base y la punta del aguijón en el mundo (hueso `arma` + `LongitudHoja` en su dirección actual). `USAEVUSComponenteEfectosCombate` reparte partículas finas a lo largo de la hoja en cada fotograma de la fase activa del tajo, más densas hacia la punta: dibujan el arco del golpe.

## Vida del cuerpo (iteración integral, 2026-10-07)

El dueño pidió que el personaje deje de sentirse rígido: que no gire como una sola pieza, que tenga peso, respiración, balanceo y reacción. Se añadieron cinco piezas:

**Muelles (`Animacion/SAEVUSMuelle.h`).** `TSAEVUSMuelle<T>`: muelle amortiguado de segundo orden (frecuencia en Hz, amortiguación adimensional), integrado en subpasos fijos (estable con fotogramas largos). Con amortiguación baja se pasa del objetivo y se asienta: eso es lo que da peso.

**Capa secundaria (`Animacion/SAEVUSCapaSecundaria.{h,cpp}`, ajustes `FSAEVUSAjusteSecundario` dentro de `FSAEVUSAjusteAnimacion::Secundario`).** Independiente del esqueleto; recibe la guiñada y la velocidad del actor y devuelve desplazamientos:
- **Giro por capas:** al girar (la cámara manda el cuerpo, GunZ), la cadera se queda donde estaba y un muelle la trae (`FrecuenciaCadera`, `MaxRetrasoCadera`); el pecho lleva una parte (`FraccionRetrasoPecho`) y la cabeza **se adelanta** al giro (`AdelantoCabeza`). Quieto y muy torcido, los pies dan un **paso de giro**. Corriendo, el cuerpo se inclina hacia dentro de las curvas (`LadeoCurva`) y hacia donde corre de lado (`LadeoLateral`). En una acción (ataque, dash) el retraso es pequeño: el golpe sale hacia la cámara.
- **Inercia del tronco:** cada cambio de velocidad empuja el tronco al revés (péndulo, `EscalaInercia`): al frenar sigue hacia delante y vuelve; al arrancar se queda atrás. La cabeza compensa casi todo (`EstabilizacionCabeza`): la mirada no bota.
- **Rebote:** aterrizar o recibir un golpe hunde la cadera con un muelle; las rodillas se doblan lo justo para que los pies no se hundan en el suelo.
- **Respiración con cansancio:** esprintar, atacar y hacer dash cansan; cansado respira más rápido y hondo (pecho y hombros), y se le pasa en calma.
- **Reposo vivo:** quieto, el peso pasa de un pie a otro (la rodilla libre se dobla), la cabeza vaga con ruido suave y cada pocos segundos sale un **gesto de reposo** por turno: mirar atrás por encima de los hombros, alzar y girar la hoja, estirar los hombros.

**Gestos de ataque (`Animacion/SAEVUSGestosAtaque.{h,cpp}`).** Cada ataque tiene su gesto en los datos (`FSAEVUSDefinicionAtaque::Gesto`, `ESAEVUSGestoAtaque`): tajo de derecha a izquierda, revés (pie izquierdo delante), remate a dos manos, estocada a fondo (tajo en carrera), tajo hacia arriba, y en el aire diagonal, revés con las piernas al lado contrario y remate con todo el cuerpo volcado; el ataque cargado es un giro entero. Un gesto son claves en "tiempo de fases" (0–1 preparación, 1–2 activa, 2–3 recuperación, valga lo que valga cada fase en los datos) con brazo, hoja, brazo libre, codos, giro de pecho y cadera, inclinación, ladeo, vuelco, giro entero, postura y bajada; se interpolan en arco con curva suave o de corte rápido. Todos tienen **amago** (la hoja va atrás antes de salir), **corte** rápido y **seguimiento** que se pasa del final. Cada tajo de la cadena empieza donde acabó el anterior, así el combo fluye. La cabeza compensa el giro del tronco (mira al frente) y cada gesto pisa a su manera (`FSAEVUSPisadaGesto`). Al acertar, la hoja **rebota** un instante (`AlImpactar` del combate → muelle `RetrocesoImpacto`). El sentido del arco dibujado sale del gesto (`SAEVUSCombate::SentidoBarrido`), así arco y hoja coinciden.

**Dash con identidad.** En tres tiempos: **arranque** (se encoge y sale disparado, la cadera se hunde), **vuelo** (estirado, la pierna de atrás tendida) y **frenada** (la de delante se adelanta y el tronco se endereza; la inercia lo balancea al acabar). De frente, estocada baja con la espada atrás; hacia atrás, el tronco atrás y la espada en guardia; de lado, ladeado hacia el lado con el brazo libre abierto de contrapeso. La capa sale despedida. Además, los **ecos** (`USAEVUSComponenteEcoDash`): siluetas de luz fría que copian la pose a lo largo del dash y se apagan (material `M_EcoDash`, aditivo con borde de Fresnel; un grupo fijo de mallas reutilizado).

**Locomoción viva.** La cadera gira con la zancada y cae hacia la pierna que se levanta; el pecho gira al revés; los pies ruedan (talón al apoyar, punta al despegar); los codos se doblan más al ir el brazo delante; al esprintar, más inclinación y rodillas más altas; en el aire las piernas pedalean un poco y las puntas bajan al caer. **Capa con muelles** en sus tres tramos (cada uno más lento): atrás con la velocidad, de lado al correr de lado y hacia fuera de las curvas; al frenar se pasa hacia delante y vuelve.

**Suavizado por tramos.** El tronco llega primero; brazos, manos y hoja, y cabeza después (acción superpuesta); las piernas, más rápidas (pisan).

**Golpes recibidos y muerte.** El golpe empuja el tronco lejos de quien golpea (capa secundaria) y suelta los brazos. Al morir, ceden primero las rodillas y luego el cuerpo cae **lejos del golpe** que lo mató.

## Enemigos (`Animacion/SAEVUSAnimacionEnemigo.cpp`)

Los goblins y el Rey Goblin usan la misma locomoción y capa secundaria, encorvados (la cabeza compensa) y nerviosos (miran a su alrededor a tirones). Cada ataque tiene su gesto (`FSAEVUSAtaqueEnemigo::Gesto`, `ESAEVUSGestoEnemigo`; en automático, según el tipo): tajo, mazazo a dos manos, manotazo con la garra, estocada, lanzamiento, embestida y salto (encogido, arriba con el arma alzada y mazazo al caer). El **aviso** es una anticipación que tiembla de tensión al final; el **golpe** sale disparado; la **recuperación** se queda abajo jadeando; **aturdido**, se tambalea con un bamboleo que se apaga. El peso (`FSAEVUSAjusteEnemigo::PesoAnimacion`: goblin 1, guerrero 1,4, Rey Goblin 2,2) hace los muelles más lentos y los pasos del jefe hunden la cadera. Al cambiar de fase, el jefe **ruge** (brazos abiertos, cabeza atrás).

## Rendimiento

Los huesos se escriben de una vez en `BoneSpaceTransforms` de la malla poseable (antes, `SetBoneTransformByName` reconstruía la pose entera en cada llamada: 25 veces por personaje y fotograma). Los nombres de hueso son `FName` creados una vez. Los enemigos que no se ven se animan cada `IntervaloFueraDePantalla` (0,25 s). La mirada busca enemigo cada `IntervaloBusquedaMirada` (0,2 s).

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
- **12–13 Ataque horizontal y diagonal:** zancada ancha y baja que crece con la preparación y se asienta en la recuperación (`ZancadaAtaqueDelante`/`Atras`, `AperturaAtaque`, `FlexionRodillaAtaque*`, `BajadaAtaque`), brazo libre adelantado de contrapeso. El primer tajo es el horizontal; el segundo, el **diagonal** desde encima del hombro derecho hasta abajo a la izquierda (antes un revés horizontal); el tercero, el remate vertical.
- **14 Ataque durante el dash:** el tajo en carrera suma `InclinacionTajoCarrera` al cuerpo.
- **15 Ataque aéreo:** en el aire, las rodillas recogidas bajo el cuerpo (`RecogidaPiernasAtaqueAereo`).
- Test `SAEVUS.Animacion.ZancadaTajo`; el alcance y el peso de los golpes van con el combate (tarea 91).

## Polvo del movimiento (tarea 74)

`USAEVUSComponentePolvoMovimiento` (`Efectos/`, partículas por CPU en mallas instanciadas con `M_ParticulaPolvo`, como la estela del dash): anillo de polvo a los pies al aterrizar, más grande cuanto más rápida la caída (`VelocidadCaidaReferencia`; por debajo de `VelocidadCaidaMinima` nada, así bajar un escalón no levanta polvo); rastro donde los pies pisan la pared al correr por ella o escalar; estallido contra la pared en el salto de pared (el evento `AlSaltoPared` lleva ahora la normal de la pared); polvo a ras de suelo, hacia atrás, en el dash por el suelo. Todo en `FSAEVUSAjustePolvoMovimiento`. Test `SAEVUS.Efectos.PolvoAterrizaje`.

## Ajustes

`FSAEVUSAjusteAnimacion` (en el componente): longitud de ciclo, amplitudes de muslo, rodilla y brazo, inclinación al correr, en el dash y en la pared, respiración, suavizados, aterrizaje y retroceso. Todo editable sin compilar.

## Lección aprendida: escala de reposo

El FBX de Blender llega con **escala 100 en la raíz** (unidades del armazón). Al escribir escala 1, todo el cuerpo se encogía al 1 % y quedaba invisible, aunque las posiciones de los huesos (lo que miden los tests) eran correctas. Ahora se conserva la escala de reposo. Lo vigilan el test `SAEVUS.Animacion.EscalaReposo` y la comprobación "cuerpo animado con tamaño de personaje" de la prueba de humo.

## Tests

`SAEVUS.Animacion.{Esqueleto, EscalaReposo, Correr, Ataque, ZancadaTajo, TajoArriba, EstelaArma, MiradaEnemigo, Dash, ArmaVisible, Direccional, PuntaCapa, Muelle, GiroPorCapas, InerciaFrenada, Cansancio, ReposoVivo, GestosDistintos}`, `SAEVUS.Efectos.EstelaEspada`, `SAEVUS.Enemigo.Gestos`. La prueba de humo guarda las capturas `SAEVUS_PruebaHumo_{Correr,Dash,Ataque}.png` en `Saved/Screenshots/WindowsEditor/`.

## Pendiente

- ~~Los materiales `M_Caballero_*` no sustituyen a los que crea Interchange~~: resuelto en la tarea 5 con *overrides* `MI_Caballero_*` en el componente (`pipeline-assets.md`, Sombreado).
- La escala 0 del arma oculta no la saca de los límites de física.
- Ajuste fino a ojo de los gestos y muelles en el juego (los valores son de primera pasada).
- Los personajes los rehace el dueño: los gestos dependen solo de los 25 huesos, así sirven igual para los modelos nuevos.

## Auditoría v2 (2026-10-08)

- **Muelles estables:** `TSAEVUSMuelle` integra con Euler implícito (el semiimplícito explotaba a NaN con la cadera a ~44 Hz y un tirón de 0,1 s: el protagonista desaparecía). Frecuencia ≤ 20 Hz, fotograma recortado a 0,1 s, muelle roto reiniciado al objetivo y huesos con NaN devueltos al reposo (`Animacion.MuelleEstable`, `Animacion.TironNoRompe`).
- **Zancada sin patinar:** la longitud del ciclo es proporcional a la zancada (velocidad × apertura del muslo), así el pie en apoyo recorre lo mismo que el cuerpo.
- **Reacciones:** el golpe recibido mueve el cuerpo en proporción al daño (media a doble fuerza). El parpadeo tras un golpe va a 4 Hz y se puede quitar en Opciones. Una rotación forzada (sentarse, portal) mayor de 100° en un fotograma no deja la cadera retorcida. Límites de la malla poseable ×1,6.
- **Muerte de enemigos:** el cuerpo con esqueleto cae entero y solo se encoge en el último 30 % de su vida (1,5 s).
