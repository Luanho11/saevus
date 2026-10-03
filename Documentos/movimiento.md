# Movimiento de SAEVUS

Estado: movimiento avanzado hecho; falta el ajuste por sensación. Caminar, correr, esprint, salto, control aéreo, tiempo coyote, buffer de salto, dash, salto de pared, carrera por pared, escalada. Construido sobre `UCharacterMovementComponent` (CMC).

## Por qué CMC y no Mover

CMC es maduro: tiene predicción en el cliente, corrección del servidor, root motion y modos de movimiento personalizados, y es lo que usan la mayoría de juegos de acción publicados en UE. El plugin Mover puede ser mejor opción más adelante; cambiar necesita un motivo concreto (un límite de CMC que bloquee una habilidad planeada).

## Paso

`ESAEVUSPaso` = Caminar / Correr / Esprint. El jugador *pide* esprint y caminar (teclas mantenidas). `ObtenerPasoActual()` resuelve las peticiones:

- esprint pedido → **Esprint** (manda sobre caminar);
- caminar pedido → **Caminar**;
- ninguno → **Correr** (por defecto).

`GetMaxSpeed()` devuelve la velocidad del paso al caminar **y al caer**, así un salto de esprint conserva su impulso. En el aire el tope usa el **paso del despegue** (`ObtenerPasoAire()`) mientras `bBloquearPasoEnAire` esté activo (por defecto): pulsar o soltar esprint en el aire no cambia nada hasta aterrizar. `MaxWalkSpeed` refleja la velocidad de carrera para los sistemas que no conocen el paso (IA, navegación).

## Ajustes

Todos los valores viven en `Source/SAEVUS/Movimiento/SAEVUSAjustesMovimiento.h`. Los inicializadores son los valores por defecto; `DA_AjustesMovimiento_Base` (un `USAEVUSAjustesMovimiento`) los sobrescribe por personaje. `AplicarAjustes()` los pasa al CMC en `InitializeComponent` y en `FijarAjustesMovimiento()`.

| Valor | Por defecto | Notas |
|---|---|---|
| VelocidadCaminar / VelocidadCorrer / VelocidadEsprint | 250 / 550 / 850 cm/s | |
| AceleracionMaxima | 2400 cm/s² | |
| DeceleracionFrenado | 2000 cm/s² | sin entrada en el suelo |
| FriccionSuelo | 8 | sequedad del cambio de dirección |
| VelocidadGiro | 720 grados/s | orientarse al movimiento |
| VelocidadSalto | 700 cm/s | cima = v² / 2g = 700² / (2 × 980 × 1,75) ≈ 143 cm |
| TiempoMaximoMantenerSalto | 0 s | salto de altura fija |
| EscalaGravedad | 1,75 | arco más pesado y seco que el 1,0 del motor |
| ControlAire | 0,35 | fracción de la aceleración de suelo en el aire |
| MultiplicadorImpulsoControlAire / Umbral | 2 / 25 cm/s | cambio de dirección más fácil desde un salto casi vertical |
| FriccionLateralCaida / DeceleracionFrenadoCaida | 0 / 0 | se conserva el impulso del despegue |
| bBloquearPasoEnAire | cierto | tope en el aire = paso del despegue |
| TiempoCoyote | 0,12 s | el salto de suelo sigue permitido tras salir caminando de un borde |
| TiempoBufferSalto | 0,12 s | un salto pulsado hasta este tiempo antes de aterrizar se ejecuta al tocar suelo |

Valores de primera pasada, sin ajustar por sensación. Se ajustan en el editor sobre el Data Asset, no en código.

## Asistencias de salto

Técnicas genéricas de plataformas (no sacadas de GunZ), en `USAEVUSComponenteMovimiento` y ajustables por Data Asset:

- **Tiempo coyote.** Salir caminando de un borde (pasar de `Walking` a `Falling` sin pulsar salto) abre una ventana de `TiempoCoyote`. `ASAEVUSPersonaje::CanJumpWhileFalling()` (el gancho del motor para "primer salto al caer") devuelve cierto dentro de ella, así se usa el camino normal de salto. Un despegue con salto nunca abre la ventana.
- **Buffer de salto.** `ASAEVUSPersonaje::PedirSalto()` es la única entrada de salto. Pulsado en el aire y fuera de la ventana coyote, guarda un temporizador de `TiempoBufferSalto`. Cuando el modo pasa a `Walking` con el temporizador activo, el componente llama a `Jump()` después de que el motor reinicie el estado de salto, y el salto ocurre en la siguiente actualización.
- Los temporizadores bajan en `OnMovementUpdated`: siguen el tiempo de simulación (también en las repeticiones), no el reloj.

## Dash

Primera habilidad avanzada (objetivos en `investigacion/gunz.md`). En teclado: **doble toque** de una dirección (WASD); en mando: `IA_Dash` (botón derecho). Lo resuelve `USAEVUSComponenteMovimiento`:

- **Inicio.** `PedirDash()` deja una petición que se consume en la siguiente actualización de movimiento (`UpdateCharacterStateBeforeMovement`). Si entonces no se puede (ya en dash, enfriamiento, agachado, sin dashes aéreos), la pulsación se descarta.
- **Dirección.** La dirección de la entrada de movimiento en esa actualización (`Acceleration`, que viaja con el movimiento), o la orientación si no hay entrada. Fija durante todo el dash. Con `bGirarHaciaDash` el cuerpo gira hacia ella.
- **Durante.** `CalcVelocity` mantiene la velocidad horizontal en `Velocidad`: sin dirigir, sin fricción, sin frenado. En el aire, `GetGravityZ()` devuelve 0 mientras `bSuspenderGravedadEnDash` esté activo, así se mantiene la velocidad vertical fijada al inicio (`VelocidadVerticalDashAereo`, 0 = plano). Salir de un borde en pleno dash sigue cruzando el hueco.
- **Fin.** Tras `Duracion`, o cuando un salto cancela un dash de suelo: la velocidad horizontal se limita a `VelocidadSalida` y empieza el `Enfriamiento`.
- **Presupuesto aéreo.** `MaxDashesAereos` por estancia en el aire; se recarga al aterrizar y (por defecto) con un salto de pared, una carrera por pared o una escalada.
- **Cancelación diseñada.** Saltar durante un dash de suelo: termina el dash y el salto sale a `VelocidadSalida`.

| Valor (`FSAEVUSAjusteDash`) | Por defecto | Notas |
|---|---|---|
| Velocidad | 1600 cm/s | |
| Duracion | 0,18 s | unos 2,9 m por dash |
| VelocidadSalida | 700 cm/s | entre correr y esprint |
| Enfriamiento | 0,3 s | desde el final del dash |
| bGirarHaciaDash | cierto | |
| MaxDashesAereos | 1 | 0 desactiva el dash aéreo |
| VelocidadVerticalDashAereo | 0 cm/s | |
| bSuspenderGravedadEnDash | cierto | |

## Salto de pared

Sin entrada nueva: es el salto pulsado en el aire junto a una pared.

- **Disparo.** `UpdateCharacterStateBeforeMovement` → `IntentarSaltoPared()`: el salto se pulsó en esta actualización (`bPressedJump` con `JumpKeyHoldTime` aún en 0, y la pulsación no hizo ya un salto normal o coyote), el personaje cae o está en una pared, y quedan saltos de pared. El tiempo coyote tiene prioridad, y mantener el salto no lo repite.
- **Detección.** `BuscarNormalSaltoPared()`: ocho barridos horizontales de cápsula de `DistanciaDeteccion`. Un contacto cuenta si su normal es casi horizontal (`|Z| <= MaxNormalZPared`) y toca el lado recto de la cápsula, no la semiesfera inferior: así el borde del que se acaba de bajar no cuenta como pared (lo descubrió el test de coyote). Gana la pared más cercana.
- **Impulso.** Velocidad = normal × `VelocidadAlejarse` + velocidad paralela a la pared × `FraccionVelocidadTangencial`; vertical = `VelocidadSubida`. Termina un dash en curso, borra el buffer de salto y gira el cuerpo (`bGirarHaciaSalto`).
- **Presupuesto.** `MaxSaltosPared` por estancia en el aire, recargado al aterrizar. Con `bRecargaDashesAereos` devuelve los dashes aéreos: salto → dash → salto de pared → dash.

| Valor (`FSAEVUSAjusteSaltoPared`) | Por defecto | Notas |
|---|---|---|
| VelocidadAlejarse | 650 cm/s | |
| VelocidadSubida | 750 cm/s | algo más que el salto de suelo (700) |
| FraccionVelocidadTangencial | 0,5 | |
| MaxSaltosPared | 3 | 0 los desactiva |
| DistanciaDeteccion | 30 cm | hueco entre cápsula y pared |
| MaxNormalZPared | 0,3 | unos 72 grados; más empinado es pared |
| bRecargaDashesAereos | cierto | |
| bGirarHaciaSalto | cierto | |

## Carrera por pared

Primer modo de movimiento personalizado: `MOVE_Custom` + `ESAEVUSModoMovimientoPersonalizado::CarreraPared`. Código en `SAEVUSComponenteMovimientoParedes.cpp`. Sin entrada nueva.

- **Entrada** (`IntentarIniciarCarreraPared`, cada actualización al caer, salvo en dash o en la actualización de un salto de pared): hay entrada de movimiento y no apunta lejos de la pared (`UmbralSoltarEntrada`); una pared a menos de `DistanciaDeteccion` al lado del movimiento o en diagonal por delante (4 barridos, mismas reglas de pared); velocidad paralela de al menos `VelocidadMinimaEntrada`; no cayendo más rápido que `MaxVelocidadCaidaEntrada`; no es la última pared recorrida.
- **Al entrar.** Dirección = velocidad paralela a la pared. Se anula la caída y la subida se limita a `MaxVelocidadSubidaEntrada`. El cuerpo mira a lo largo de la pared. Se recargan los dashes aéreos.
- **Corriendo** (`FisicaCarreraPared`): velocidad paralela fija en `Velocidad`; gravedad del personaje × `EscalaGravedad`; la pared se vuelve a sondear cada actualización y se sigue su normal (paredes curvas); la cápsula se acerca a 1 cm de ella.
- **Salidas.** Tiempo (`DuracionMaxima`) → cae conservando la velocidad. La pared termina o se pierde → cae. Entrada lejos de la pared → cae. Llega al suelo → aterriza. Algo bloquea → cae. Salto → salto de pared desde esa pared.
- **Misma pared.** La última pared recorrida no se puede volver a recorrer hasta aterrizar o recorrer otra: alternar entre dos paredes enfrentadas funciona; subir una pared para siempre, no.

| Valor (`FSAEVUSAjusteCarreraPared`) | Por defecto | Notas |
|---|---|---|
| Velocidad | 750 cm/s | |
| DuracionMaxima | 1,2 s | 0 la desactiva |
| EscalaGravedad | 0,3 | cae unos 3,7 m en una carrera completa desde reposo |
| VelocidadMinimaEntrada | 300 cm/s | paralela a la pared |
| MaxVelocidadSubidaEntrada | 300 cm/s | |
| MaxVelocidadCaidaEntrada | 600 cm/s | |
| DistanciaDeteccion | 40 cm | |
| MaxNormalZPared | 0,3 | |
| UmbralSoltarEntrada | 0,7 | |
| bRecargaDashesAereos | cierto | |

## Escalada

`MOVE_Custom` + `ESAEVUSModoMovimientoPersonalizado::EscaladaPared`, en `SAEVUSComponenteMovimientoParedes.cpp`. Sin entrada nueva.

- **Entrada** (`IntentarIniciarEscalada`, antes que la carrera por pared: llegar de frente escala, en ángulo corre): cayendo, sin dash, con escaladas restantes; una pared justo delante a menos de `DistanciaDeteccion`; movimiento y entrada de frente (`UmbralFrontal`); velocidad hacia la pared de al menos `VelocidadMinimaEntrada`; no cayendo más rápido que `MaxVelocidadCaidaEntrada`. En la práctica: correr hacia una pared, saltar y seguir empujando hacia ella.
- **Escalando** (`FisicaEscalada`): `Velocidad` constante hacia arriba, sin gravedad, mirando a la pared, pegado a 1 cm (mismas piezas que la carrera: `SondearParedActual`, `MoverJuntoAPared`).
- **Salidas.** Tiempo (`DuracionMaxima`) → voltereta hacia atrás (`VelocidadVolteretaAlejarse`, `VelocidadVolteretaSubida`, mirando hacia fuera). Llega arriba → avanza a `VelocidadAvanceCornisa` sin dejar de subir y aterriza encima. Entrada lejos → se suelta. Techo → cae. Salto → salto de pared.
- **Presupuesto.** `MaxEscaladas` por estancia en el aire (1), recargado al aterrizar. Empezar una escalada recarga los dashes aéreos.

| Valor (`FSAEVUSAjusteEscaladaPared`) | Por defecto | Notas |
|---|---|---|
| Velocidad | 650 cm/s | |
| DuracionMaxima | 0,55 s | unos 3,4 m medidos (3,6 nominales; el último paso es la voltereta) |
| MaxEscaladas | 1 | |
| VelocidadMinimaEntrada | 200 cm/s | hacia la pared |
| UmbralFrontal | 0,85 | unos 32 grados |
| MaxVelocidadCaidaEntrada | 600 cm/s | |
| DistanciaDeteccion | 40 cm | |
| MaxNormalZPared | 0,3 | |
| UmbralSoltarEntrada | 0,7 | |
| VelocidadVolteretaAlejarse / VelocidadVolteretaSubida | 350 / 250 cm/s | |
| VelocidadAvanceCornisa | 300 cm/s | |
| bRecargaDashesAereos | cierto | |

## Preparado para red

`FSAEVUSMovimientoGuardado` guarda las peticiones de esprint y caminar, que viajan como `FLAG_Custom_0` (esprint) y `FLAG_Custom_1` (caminar) y se restauran en `UpdateFromCompressedFlags`. La petición de dash viaja como `FLAG_Custom_2`; el movimiento guardado también guarda el estado del dash al inicio (dirección, tiempo, enfriamiento, dashes aéreos usados) y lo restaura en `PrepMoveFor`, así una repetición tras una corrección simula el dash igual. El salto de pared no necesita bandera: lo mueve la pulsación de salto replicada (`FLAG_JumpPressed`; el servidor reinicia `JumpKeyHoldTime` en cada pulsación nueva). El estado de pared (normal, dirección, tiempo, última pared, escaladas) se guarda y restaura igual; el modo de movimiento lo replica el motor. `FLAG_Custom_3` sigue libre; las habilidades con más estado deberían usar la API de datos de red de CMC en vez de banderas.

Límite conocido: el paso en el aire y los temporizadores coyote/buffer son estado local derivado de transiciones, fuera del movimiento guardado; tras una corrección pueden desfasarse una transición. El salto sigue el camino normal de `bPressedJump`, así el servidor manda.

## Preguntas abiertas

- ~~Esprint en el aire~~ — decidido: se bloquea el paso del despegue (`bBloquearPasoEnAire`, desactivable).
- **Dirección del esprint:** con orientarse al movimiento el cuerpo siempre mira hacia donde corre; revisar si llega un modo de apuntado.
- **Recuperación al aterrizar:** pendiente de las animaciones.

## Tests

`Source/SAEVUS/Pruebas/` (filtro `SAEVUS.Movimiento`):

| Test | Comprueba |
|---|---|
| `Paso.Prioridad` | corre por defecto; camina; esprint gana a caminar; soltar vuelve a correr |
| `Paso.Velocidades` | velocidades del ajuste; caminar < correr < esprint |
| `Ajustes.Aplicar` | un Data Asset sobrescribe valores del CMC; nulo restaura |
| `Aire.CimaSalto` | cima simulada a menos del 2 % de v² / 2g |
| `Aire.ControlAire` | aceleración aérea ≈ AceleracionMaxima × ControlAire (±10 %); 0 no acelera |
| `AsistenciaSalto.TiempoCoyote` | saltar justo tras salir de un borde funciona; con 0 se rechaza |
| `AsistenciaSalto.BufferSalto` | salto pulsado a 15 cm del suelo se ejecuta al aterrizar; con 0 se pierde |
| `AsistenciaSalto.BloqueoPasoAire` | esprint en el aire no sube el tope si está bloqueado; sí si no; un despegue con salto no abre coyote |
| `Dash.Suelo` / `Direccion` / `Enfriamiento` / `PresupuestoAereo` / `CancelarConSalto` | velocidad, dirección, distancia (293 vs 288 cm), enfriamiento, presupuesto aéreo con gravedad suspendida, cancelación con salto |
| `SaltoPared.Basico` / `SinPared` / `NoDesdeBorde` / `Tangencial` / `Presupuesto` / `RecargaDashAereo` | impulso, ausencia de pared, el borde no es pared, fracción tangencial, presupuesto, recarga |
| `CarreraPared.Entrar` / `CondicionesEntrada` / `Duracion` / `Aterrizaje` / `FinPared` / `Saltar` / `Soltar` | entrada, rechazos, 72 pasos = 1,2 s, misma pared, aterrizaje, final de pared, salto, soltar |
| `Escalada.Entrar` / `CondicionesEntrada` / `Voltereta` / `Cornisa` / `UnaPorEstanciaAerea` / `Saltar` / `Soltar` | entrada, rechazos, altura (340 vs 358 cm) y voltereta, subir a una cornisa de 3,5 m, una por estancia, salto, soltar |

Los tests simulan un `ASAEVUSPersonaje` real en un mundo desechable (`Pruebas/SAEVUSMundoPrueba.h`: sin mapa ni controlador, `bRunPhysicsWithNoController`, plataformas de cubos del motor). El fixture actualiza los componentes a mano y avanza el tiempo del mundo desde 10 s: en el editor CMC no empieza a caer en el primer segundo.

Prueba en el juego: `Herramientas\Ejecutar-PruebaHumo.ps1` corre, esprinta, salta, hace dash, ataca, dispara, recibe daño, muere y reaparece solo; escribe `[SAEVUS humo] SUPERADA/FALLIDA`, tiempos por fotograma y una captura (`rendimiento.md`).

## Mapa de pruebas (`/Game/SAEVUS/Mapas/L_Pruebas`)

Suelo de rejilla de 100 × 100 m, más:

- escalones de 50–250 cm para la altura de salto;
- rampas de 20°, 35° y 50° (el límite caminable por defecto es unos 44,8°);
- plataformas de 200 cm con huecos de 2–6 m para distancia de salto y control aéreo;
- una pared de 30 m × 6 m para la carrera por pared;
- una chimenea de salto de pared en X = -30 m: dos paredes de 7 m con 3 m de hueco y una cornisa de 4,5 m al fondo;
- un pasillo de carrera por pared en Y = -30 m: dos paredes de 20 m × 6 m a 5 m;
- bloques de escalada en X = 30 m: uno de 3,5 m al que subirse y una pared de 10 m que acaba cada escalada en voltereta;
- muñecos de entrenamiento y una galería de tiro (`combate.md`).

F6 muestra paso, velocidad, modo, dash, paredes, ataque, arma y vida.
