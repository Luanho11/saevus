# Enemigos de SAEVUS

Prototipo de enemigos para el tutorial: mini enemigos y un mini jefe, todo por datos. Estilo Hollow Knight (contacto que hace daño, avisos claros, castigo tras atacar), con números propios.

## Piezas

| Pieza | Dónde | Qué hace |
|---|---|---|
| `ASAEVUSEnemigo` | `Source/SAEVUS/Enemigos/` | `ACharacter` con vida, IA y un cuerpo boceto de formas del motor (cuerpo, máscara, dos cuernos) coloreado por su tipo con instancias dinámicas de `M_SAEVUS_PersonajeDibujado`. Sin controlador: el movimiento simula igual (`bRunPhysicsWithNoController`) y no cae por bordes. |
| `USAEVUSComponenteIAEnemigo` | `Enemigos/` | Máquina de estados; ver abajo. |
| `FSAEVUSAjusteEnemigo` / `USAEVUSDatosEnemigo` | `Enemigos/SAEVUSTiposEnemigo.h` | Tipo de enemigo: vida, cápsula, colores, velocidad, sentidos, contacto, aturdimiento, empuje recibido, muerte, ataques y fases. Sin Data Asset: el rastrero por defecto. |
| Cuerpos generados | `Herramientas/Arte/blender_enemigos.py` (`Generar-Enemigos.ps1` → `ArteFuente/Enemigos/`) | Una malla estática por tipo (`SM_Enemigo_Rastrero/Saltador/Escupidor/Revoloteador`, `SM_Jefe_Guardian`, unos 1 000 triángulos), a la medida de su cápsula y mirando a +X; ranura 0 cuerpo, ranura 1 máscara. `USAEVUSDatosEnemigo::MallaCuerpo`; sin ella, formas del motor. El aspecto procedural (agacharse, estirarse, máscara que brilla, encogerse al morir) funciona igual. |
| `DA_Enemigo_Rastrero`, `DA_Enemigo_Saltador`, `DA_Enemigo_Escupidor`, `DA_Enemigo_Revoloteador`, `DA_Jefe_Guardian` | `/Game/SAEVUS/Datos/Enemigos/` | Los tipos, creados por `saevus_crear_assets.py` (tabla `ENEMIGOS`). |

## Estados

```
Reposo ──ve al objetivo──▶ Perseguir ──ataque a distancia──▶ Telegrafiar ──▶ Atacar ──▶ Recuperarse ──▶ Perseguir
   ▲                          │                                                                  
   └──────pierde al objetivo──┘          cualquier estado ──golpe recibido──▶ Aturdido ──▶ Perseguir
                                         cualquier estado ──vida 0──▶ Muerto
```

- **Reposo:** patrulla de ida y vuelta (`RadioPatrulla`) a media velocidad; pasa a perseguir dentro de `RadioDeteccion`. **Reposo lejano** (más allá de `RadioDeteccion × FactorDistanciaReposoLejos`): se queda quieto y la IA y el movimiento (si está de pie) se actualizan cada `IntervaloReposoLejos` en vez de cada fotograma (`rendimiento.md`).
- **Perseguir:** va en línea recta hacia el objetivo (sin navegación: arenas planas). Pierde al objetivo más allá de `RadioPerdida`.
- **Elegir ataque:** el siguiente en rotación cuya franja `DistanciaMinima..DistanciaMaxima` contenga la distancia y que la fase permita. Predecible a propósito: se aprende y se prueba.
- **Telegrafiar:** quieto, encarando al objetivo, durante `TiempoTelegrafiar`. El cuerpo se agacha y la máscara se enciende: es la lectura del jugador. La dirección del ataque se fija al acabar el aviso, así moverse a tiempo lo esquiva.
- **Atacar:**
  - `Embestida`: carrera recta a `Velocidad` durante `Duracion`.
  - `Salto`: lanzamiento en arco (`Velocidad` horizontal, `VelocidadVertical`); termina al aterrizar.
  - `Golpe`: esfera de `RadioGolpe` a `AlcanceGolpe` delante.
  - `Proyectil`: lanza un `ASAEVUSProyectil` hacia el objetivo a `Velocidad`, y se queda quieto `Duracion`.
  - `Picado` (voladores): línea recta en 3D hacia donde estaba el objetivo al acabar el aviso, a `Velocidad` durante `Duracion`.
- **Recuperarse:** `TiempoRecuperacion` sin actuar: la ventana de castigo.
- **Aturdido:** cada golpe recibido corta lo que hace durante `TiempoAturdido` (los jefes no se aturden).
- **Contacto:** tocar al enemigo quita `DanioContacto`; durante una embestida o un salto, el daño del ataque. La invulnerabilidad del jugador (1 s tras un golpe, inicio del dash) evita el daño continuo.

## Fases (jefes)

`Fases` ordenadas de mayor a menor `FraccionVida`; al bajar la vida se pasa a la siguiente (nunca se vuelve). Cada fase escala velocidad y tiempos y limita los ataques disponibles. El Guardián: fase 1 con golpe y embestida; a media vida, fase 2 más rápida (×1,3), avisos más cortos (×0,7) y añade el salto.

## Valores de primera pasada

| Tipo | Vida | Cápsula (r × h/2) | Velocidad | Detección | Contacto | Ataques |
|---|---|---|---|---|---|---|
| Rastrero | 30 | 40 × 40 | 280 | 900 | 10 | Embestida 0–450 cm: aviso 0,55 s, 0,4 s a 950 cm/s, 15 de daño, recuperación 0,8 s |
| Saltador | 25 | 35 × 50 | 220 | 1100 | 10 | Salto 150–700 cm: aviso 0,45 s, 500 + 750 cm/s, 15 de daño, recuperación 0,7 s |
| Escupidor | 20 | 38 × 55 | 200 | 1300 | 10 | Escupitajo 400–1200 cm: aviso 0,6 s, proyectil a 900 cm/s, 15 de daño, recuperación 1,1 s; retrocede si el jugador está a menos de 4 m |
| Revoloteador (vuela) | 15 | 35 × 35 | 260 | 1000 | 10 | Picado 0–450 cm: aviso 0,6 s, 0,55 s a 950 cm/s, 15 de daño, recuperación 0,9 s; vuela a 2,2 m sobre el jugador |
| Guardián (jefe) | 250 | 90 × 120 | 260 | 1500 | 15 | Golpe 0–260 cm (25), Embestida 300–1200 cm a 1300 cm/s (25), Salto 200–900 cm (30, solo fase 2) |

El Guardián apenas se mueve con los golpes (`EscalaEmpujeRecibido` 0,15) y tarda 1,5 s en deshacerse al morir.

## Voladores

`bVolador` (Revoloteador): el movimiento pasa a `MOVE_Flying` (sin gravedad, `DefaultLandMovementMode` también de vuelo, frenada `FrenadaVuelo`). En reposo patrulla a su altura de origen; al perseguir busca `AlturaVuelo` por encima del objetivo (la diferencia de altura pesa el doble que avanzar, así se coloca antes de llegar). Ataca con `Picado`. Un golpe lo deja caer mientras está aturdido (el empuje lo pasa a caída) y después vuelve a volar (`MantenerVuelo`). El cuerpo sube y baja con un aleteo (`AmplitudAleteo`, `FrecuenciaAleteo`, desfasado por enemigo; solo aspecto). Pensado para Hollow Knight: se le da saltando con el tajo hacia arriba o rebotando encima con el tajo hacia abajo. Está en la arena S7 del tutorial y en la zona de enemigos de `L_Pruebas`.

## Proyectiles

`ASAEVUSProyectil` (`Enemigos/`): esfera brillante que vuela recto (barrido en su tick, sin físicas), se deshace contra el escenario y hace daño al jugador al tocarlo, con el proyectil como causante: **la guardia lo para** si llega de frente (y cuenta para el contraataque). **Un tajo lo devuelve:** invierte su dirección hacia el lanzador, va 1,6 veces más rápido y entonces solo daña a enemigos. No da alma. Los enemigos con `DistanciaRetirada` retroceden si el jugador se les echa encima, en vez de acercarse.

## Golpes

- El jugador golpea a los enemigos con el combate normal: daño, alma, empuje (escalado por el enemigo), parada de golpe y rebote con el tajo hacia abajo.
- El jugador recibe golpes por `ApplyDamage`: pierde vida, queda invulnerable 1 s parpadeando y sale despedido lejos del causante (`FSAEVUSAjusteGolpeRecibido`: 550 cm/s en horizontal, 350 cm/s hacia arriba).
- Jugador y enemigos aceptan daño sin modo de juego (`ShouldTakeDamage` solo pide autoridad).

## Destello

Al recibir daño, cuerpo y máscara se pintan casi en blanco y brillan 0,12 s (`ASAEVUSEnemigo::AplicarDestello`): el golpe se lee aunque el enemigo no se aturda (jefes).

## Muerte

Al morir: sin colisión ni movimiento, ráfaga de partículas, se encoge durante `DuracionMuerte` y desaparece. `ASAEVUSEnemigo::AlMorirEnemigo` (estático) avisa a quien escuche (puertas y recuentos del tutorial).

## Tests

`SAEVUS.Enemigo.*`:
- `Deteccion`: patrulla en reposo, persigue al ver y se acerca.
- `Volador`: vuela sin caer, se pone a su altura sobre el jugador, cae en picado hacia él y, aturdido, cae y vuelve a volar.
- `ReposoLejano`: lejos, la IA y el movimiento se actualizan a intervalos; al acercarse despierta y vuelve a cada fotograma.
- `AvisoYEmbestida`: quieto durante el aviso; embiste hacia el jugador; se recupera.
- `DanioContacto`: daño, invulnerabilidad y empuje del jugador.
- `Aturdido`: el tajo aturde, quita vida y da alma; el aturdimiento pasa.
- `Muerte`: estado, aviso único, vida limitada, sin colisión, sin daño de contacto.
- `FasesJefe`: cambio de fase al 50 %, el jefe no se aturde, velocidad de fase.
- `Salto`: despega hacia el jugador y se recupera al aterrizar.
- `Proyectil`: el escupidor dispara y el proyectil alcanza al jugador. Además `SAEVUS.Guardia.BloqueaProyectil` y `SAEVUS.Combate.DevolverProyectil`.

La prueba de humo lleva al jugador junto al primer enemigo del mapa y comprueba que reacciona (captura `SAEVUS_PruebaHumo_Enemigos.png`). En `L_Pruebas` hay una zona de enemigos en Y ≈ −4300 (rastrero, saltador y Guardián), lejos de la ruta de la prueba.

## Pendiente

- Navegación (navmesh) si los mapas dejan de ser arenas.
- Voladores; proyectiles en arco y en abanico.
- Barra de vida del jefe (tarea de interfaz) y música/sonido.
- Red: la IA corre solo en el servidor; el aspecto se derivará del estado replicado.
