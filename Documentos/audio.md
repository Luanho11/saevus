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

## Tests

`SAEVUS.Audio.Sintesis` (longitud, no silencio, pico dentro del volumen, envolvente que se apaga, determinismo) y `SAEVUS.Audio.Eventos` (un tajo que acierta suena dos veces; el dash suena).

## Pendiente

- Sonidos de enemigos (aviso, embestida, muerte), pasos, aterrizaje, música ambiental.
- Posicionales (3D) para lo que no sea del jugador; mezcla y atenuación.
- Sustituir por sonidos diseñados a mano cuando haya dirección de audio.
