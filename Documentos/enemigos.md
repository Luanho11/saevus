# Enemigos de SAEVUS

Enemigos por datos sobre una IA de estados común. **Desde el 2026-10-04 en los mapas solo hay goblins** (estilo GunZ, ver abajo). Los enemigos de antes (rastrero, saltador, escupidor, revoloteador, acorazado, Guardián, Madre Alada) siguen en los datos y en los tests, fuera de los mapas (`ENEMIGOS_EN_MAPAS` en el script de assets es la lista de tipos que se colocan).

## Goblins (tareas 75 y 76)

Como los del modo misión de GunZ, con diseño propio. El dueño pidió que no castiguen solo por acercarse, que se puedan golpear con claridad y que tengan ataques variados:
- **sin daño por contacto** (`DanioContacto` 0): solo hacen daño sus ataques (en la embestida y el salto, el cuerpo es el ataque);
- **avisos legibles:** el arma y los ojos van en la ranura de la máscara, así que al avisar (se agachan y la máscara brilla) **se enciende el arma** que va a golpear;
- recuperaciones en las que cabe castigar (lo comprueba `SAEVUS.Equilibrio.Enemigos`, que ahora los incluye);
- cuerpos generados en Blender (`blender_enemigos.py`: `SM_Goblin`, `SM_Goblin_Maza`, `SM_Goblin_Hondero`, `SM_Jefe_ReyGoblin`, unos 1 000–1 200 triángulos); render de presentación en `Documentos/capturas/Goblins.png` (`Herramientas/Arte/blender_render_enemigos.py`).

| Tipo | Vida | Cápsula (r × media altura) | Ataques |
|---|---|---|---|
| Goblin (cuchillo) | 30 | 38 × 60 | Tajo delante (0–190 cm, aviso 0,45 s, 10) y estocada en carrera (250–550 cm, aviso 0,55 s, 12) |
| Goblin de maza | 60 | 50 × 75 | Mazazo amplio (0–260 cm, aviso 0,8 s, 22) y salto con la maza (300–800 cm, aviso 0,7 s, 20); recibe la mitad de empuje |
| Goblin hondero | 20 | 35 × 55 | Piedra con la honda (450–1300 cm, aviso 0,6 s, 10; se para con la guardia y se devuelve con un tajo), manotazo de cerca (8); se aleja si te acercas |
| **Rey Goblin** (jefe) | 320 | 90 × 130 | Cuchillazo amplio (25), embestida (25), salto aplastante (25) y, a media vida (más rápido, avisos ×0,75), lluvia de cinco piedras en abanico de 70° |

Dónde están: tres en el gran salón del tutorial (uno de cada clase, con el aviso "Goblins"); en la Mazmorra, cuatro en el salón central (uno en el balcón), dos en la galería y el Rey en la sala del trono, al sur del final de la galería; en `L_Pruebas`, uno de cada para la prueba de humo (comprueba que reaccionan al jugador).

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
  - `Proyectil`: lanza un `ASAEVUSProyectil` hacia el objetivo a `Velocidad`, y se queda quieto `Duracion`. Con `NumeroProyectiles` > 1 salen a la vez en abanico horizontal de `AperturaAbanico` grados; el del centro va al objetivo.
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
| Acorazado (escudo de 120°) | 40 | 45 × 60 | 200 | 900 | 10 | Escudazo 0–220 cm: aviso 0,6 s, golpe de 20 (radio 140 a 120 cm), recuperación 1 s; bloquea de frente, se rompe con 28 de daño (giro cargado) durante 2,5 s |
| Madre Alada (jefe de la Zona 1, vuela a 3,2 m) | 300 | 70 × 70 | 300 | 1600 | 15 | Picado 0–700 cm (25, 1100 cm/s), Abanico de 5 a 60° 300–1600 cm (20), Abanico amplio de 7 a 90° (20, solo fase 2); fase 2 a media vida: × 1,25 velocidad, avisos × 0,75 |
| Guardián (jefe) | 250 | 90 × 120 | 260 | 1500 | 15 | Golpe 0–260 cm (25), Embestida 300–1200 cm a 1300 cm/s (25), Salto 200–900 cm (30, solo fase 2) |

El Guardián apenas se mueve con los golpes (`EscalaEmpujeRecibido` 0,15) y tarda 1,5 s en deshacerse al morir.

## Escudo

`USAEVUSComponenteEscudo` (en todos los enemigos; solo actúa si el tipo tiene `AnguloEscudo`). Se engancha al filtro de daño de la vida, como la guardia del jugador. Un golpe que llega de frente, dentro de la mitad del ángulo a cada lado, se **bloquea**: no hace daño, no da alma, empuja al atacante (`EmpujeEscudo`), saca chispas y suena un choque metálico. Hay tres formas de vencerlo:

- **rodearlo:** por detrás o de lado el golpe pasa;
- **saltarle encima:** desde arriba no hay dirección, así que el tajo hacia abajo (pogo) pasa;
- **romperlo:** un golpe de al menos `DanioRompeEscudo` (28, el giro cargado) lo rompe durante `TiempoEscudoRoto` (2,5 s) y pasa entero.

El Acorazado está en la planta alta y en la arena de la Zona 1.

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
- `Escudo`: de frente bloquea (un bloqueo, sin daño); por detrás pasa; un golpe fuerte lo rompe y pasa, y roto no bloquea; recuperado, desde encima pasa.
- `Volador`: vuela sin caer, se pone a su altura sobre el jugador, cae en picado hacia él y, aturdido, cae y vuelve a volar.
- `ReposoLejano`: lejos, la IA y el movimiento se actualizan a intervalos; al acercarse despierta y vuelve a cada fotograma.
- `AvisoYEmbestida`: quieto durante el aviso; embiste hacia el jugador; se recupera.
- `DanioContacto`: daño, invulnerabilidad y empuje del jugador.
- `Aturdido`: el tajo aturde, quita vida y da alma; el aturdimiento pasa.
- `Muerte`: estado, aviso único, vida limitada, sin colisión, sin daño de contacto.
- `FasesJefe`: cambio de fase al 50 %, el jefe no se aturde, velocidad de fase.
- `Salto`: despega hacia el jugador y se recupera al aterrizar.
- `Proyectil`: el escupidor dispara y el proyectil alcanza al jugador.
- `Abanico`: cinco proyectiles a la vez, repartidos en 60°, el del centro hacia el jugador. Además `SAEVUS.Guardia.BloqueaProyectil` y `SAEVUS.Combate.DevolverProyectil`.

La prueba de humo lleva al jugador junto al primer enemigo del mapa y comprueba que reacciona (captura `SAEVUS_PruebaHumo_Enemigos.png`). En `L_Pruebas` hay una zona de enemigos en Y ≈ −4300 (rastrero, saltador y Guardián), lejos de la ruta de la prueba.

## Pendiente

- Navegación (navmesh) si los mapas dejan de ser arenas.
- Voladores; proyectiles en arco y en abanico.
- Barra de vida del jefe (tarea de interfaz) y música/sonido.
- Red: la IA corre solo en el servidor; el aspecto se derivará del estado replicado.
