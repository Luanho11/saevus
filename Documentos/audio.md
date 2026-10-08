# Sonido de SAEVUS (boceto)

Sin archivos de audio: los efectos se **sintetizan por código** al empezar la partida y se reproducen con ondas procedurales. Es un boceto para que las acciones se oigan; los sonidos hechos a mano lo sustituirán.

## Piezas (`Source/SAEVUS/Audio/`)

| Pieza | Qué hace |
|---|---|
| `FSAEVUSSonidoSintetizado` | Un sonido descrito por datos: duración, tono que barre de `FrecuenciaInicio` a `FrecuenciaFin`, `Aspereza` (seno → casi cuadrada), `MezclaRuido` y `FiltroRuido` (ruido de paso bajo), envolvente `Ataque`/`Caida`, `Volumen`, `Semilla` (siempre suena igual). |
| `SAEVUSSintesis::Sintetizar` | Función pura: PCM de 16 bits mono a 22 050 Hz. |
| `USAEVUSComponenteSonido` | En el personaje: sintetiza cada efecto una vez y lo reproduce (2D) al llegar su evento. |
| `FSAEVUSAjusteSonidos` | Los siete sonidos y los volúmenes general, de combate y de movimiento. |

## Efectos y eventos

| Sonido | Evento | Carácter |
|---|---|---|
| Tajo | `USAEVUSComponenteCombate::AlIniciarAtaque` | siseo de aire |
| Golpe | `AlGolpear` | impacto sordo y seco |
| Disparo | `USAEVUSComponenteArmas::AlDisparar` | chasquido con cuerpo grave |
| Daño | `USAEVUSComponenteVida::AlCambiarVida` (negativo) | zumbido áspero que baja |
| Curación | `USAEVUSComponenteAlma::AlCurar` | campanilla que sube |
| Dash | `AlIniciarDash` | soplo |
| Doble salto | `AlSaltoAereo` | bocanada suave |
| Gatillo en seco | `USAEVUSComponenteArmas::AlGatilloEnSeco` (sin reserva) | clic metálico flojo |

## Enemigos

`USAEVUSComponenteSonidoEnemigo` (en cada `ASAEVUSEnemigo`, `FSAEVUSAjusteSonidosEnemigo`): gruñido que sube al **avisar**, arrastre al **atacar** y chillido que cae al **morir**, disparados por `USAEVUSComponenteIAEnemigo::AlCambiarEstado` y reproducidos en la posición del enemigo. El aviso sonoro refuerza la lectura del ataque. Los **voladores** además aletean: un soplo grave y suave por ciclo de aleteo (`FrecuenciaAleteo`), solo con su objetivo a menos de `DistanciaAleteo` (12 m); el componente solo tiene tick en los voladores (`ActualizarTipo`, también al cambiar los datos).

## Mundo

`USAEVUSSubsistemaSonidoMundo` (`FSAEVUSAjusteSonidosMundo`): sonidos de objetos, sintetizados la primera vez que suenan y guardados. **Palanca** (golpe metálico seco, `ASAEVUSPalanca::Accionar`) y **Puerta** (retumbo largo y grave de piedra, `ASAEVUSPuertaArena::Abrir`, también las compuertas). Se llama con `USAEVUSSubsistemaSonidoMundo::Reproducir(Contexto, Sonido, Posicion)`.

`SAEVUSSintesis::SintetizarBytes` y `CrearOnda` son las piezas compartidas por los componentes y el subsistema. `SintetizarBytes` guarda lo sintetizado por huella del sonido: los enemigos del mismo tipo no repiten la síntesis.

## Música (`SAEVUSMusica`, `USAEVUSSubsistemaMusica`)

Música **generada por código** (iteración integral, 2026-10-07), sin archivos: tres bucles que se sintetizan una vez en otro hilo al empezar la partida (`SAEVUSMusica::GenerarPista`, funciones puras) y suenan a la vez; el subsistema de mundo los **funde** según lo que pasa (`FSAEVUSAjusteMusica`):

| Pista | Cuándo | Qué suena |
|---|---|---|
| Exploración (32 s) | Calma | Bordón grave de re con su quinta que respira, acordes lentos re m – si♭ – sol m – la (la sensible da la tensión), una voz de coro a ratos, campanas lejanas inarmónicas en sitios fijos (semilla) y viento grave en ráfagas. Reverberación de sala de piedra grande. |
| Combate (120 ppm, 8 compases) | Un enemigo cerca persigue o ataca | Tambores graves (taiko) sincopados, ostinato de cuerdas graves en corcheas sobre los mismos acordes, colchón de tensión y golpe de metales graves en cada cambio de acorde. |
| Jefe (140 ppm, 8 compases) | Un jefe despierto | Más tambores, re frigio (mi♭: lo más oscuro del modo), coro arriba y metales a contratiempo. |

La calma no se apaga en la pelea: queda de fondo (`CalmaEnPelea`). La pelea sigue sonando un rato tras el último enemigo (`EsperaFinPelea`); entrar es más rápido que salir. Las pistas cierran el bucle sumando la cola de la reverberación al principio, así enlazan sin salto. Suenan en los menús (son sonido de interfaz) y no se crean sin dispositivo de audio (tests, `-NoSound`). El volumen general de las opciones va encima.

## Tests

`SAEVUS.Audio.Sintesis` (longitud, no silencio, pico dentro del volumen, envolvente que se apaga, determinismo) y `SAEVUS.Audio.Eventos` (un tajo que acierta suena dos veces; el dash suena); `SAEVUS.Enemigo.AvisoYEmbestida` comprueba que el aviso y el ataque del enemigo suenan. `SAEVUS.Audio.SonidosMundo` (la puerta suena al abrirse, la palanca una sola vez) y `SAEVUS.Audio.AleteoVolador` (lejos no se oye; cerca, unos 3 batidos por segundo).

## Pendiente

- Pasos y aterrizaje.
- Música: un tema propio por zona y transiciones a compás (ahora los fundidos son libres).
- Posicionales (3D) para lo que no sea del jugador; mezcla y atenuación.
- Sustituir por sonidos diseñados a mano cuando haya dirección de audio.
