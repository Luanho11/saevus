# Controles y técnicas de SAEVUS (boceto jugable)

Abrir el proyecto en el editor y pulsar Play: arranca en el tutorial (`L_Tutorial`) con el menú principal (Tutorial, Mazmorra, Opciones, Salir). Todas las teclas están en `IMC_SAEVUS` y se pueden cambiar.

## Teclas

| Acción | Teclado y ratón | Mando |
|---|---|---|
| Moverse / mirar | WASD / ratón | stick izquierdo / derecho |
| Esprintar / caminar | Mayús / Alt | clic del stick izquierdo |
| Saltar (doble salto en el aire) | Espacio | A |
| Dash | doble toque de una dirección (W W, A A...) | B |
| Atacar | clic izquierdo | X |
| Guardia (con el aguijón) | clic derecho (mantener) | gatillo izquierdo |
| Curar (gasta alma) | Q (mantener, en el suelo) | botón superior izquierdo |
| Interactuar: sentarse en un banco (cura y guarda) | E | Cruceta arriba |
| Pausa (opciones) | P | Start |
| Menús: moverse y confirmar | Flechas e Intro (o ratón) | Cruceta/stick y botón inferior |
| Datos de depuración | F6 | — |
| Números de daño (consola, tecla `º`/`~`) | `SAEVUS.Depuracion.NumerosDanio 1` | — |

## Técnicas

- **Tajo hacia arriba:** mira hacia arriba al atacar. (El tajo hacia abajo y el rebote se retiraron el 2026-10-04: con doble salto no aportaban.)
- **Contraataque:** levanta la guardia justo cuando llega el golpe: el enemigo queda aturdido y ganas alma.
- **Tajo → guardia:** con la guardia pulsada, cada tajo vuelve a la guardia en cuanto su ventana lo permite (la "mariposa" de GunZ, a la manera de SAEVUS).
- **Ataque cargado:** mantén el clic izquierdo (las partículas se juntan en la hoja) y suelta: giro que golpea todo alrededor.
- **Tajo en carrera:** ataca durante el dash: sigues lanzado en su dirección y el golpe llega más lejos y más fuerte.
- **Combo aéreo:** tres tajos en el aire; los dos primeros frenan la caída y te dejan flotando un momento.
- **Cancelaciones:** dash, salto y guardia cortan un tajo desde su ventana (los tajos rápidos pronto; el remate tarde). Un ataque durante un dash lo termina.
- **Paredes:** salta contra una pared para rebotar; corre de frente para escalarla; llega en diagonal para correr por ella. Un salto de pared devuelve el doble salto y el dash aéreo.
- **Salto en buffer:** pulsar salto justo antes de aterrizar salta al tocar el suelo (y conserva el doble salto).
- **Cámara y cuerpo (GunZ):** la cámara está siempre detrás en tercera persona y el cuerpo mira siempre hacia donde mira ella; te mueves en ocho direcciones sin girarte (andar hacia atrás es andar de espaldas, y el dash hacia atrás no te da la vuelta). Los tajos salen hacia donde miras.
- **Dash (como en GunZ):** sin espera entre dashes ni límite por salto; se encadenan sin pausa. La gravedad sigue actuando, así que encadenarlos no te hace volar. Invulnerable un instante al empezar; la cámara se queda fija mientras dura.
- **Alma:** cada golpe a un enemigo o muñeco la llena (orbe arriba a la izquierda); mantén Q en el suelo para curarte una máscara de vida. Un golpe, saltar, atacar o el dash interrumpen la curación.
- **Recibir un golpe:** sales despedido y eres invulnerable un segundo (el cuerpo parpadea).

## Enemigos

Retirados por ahora (decisión del dueño, 2026-10-04). Volverán como goblins y jefes al estilo GunZ: avisos claros, sin daño por contacto, fáciles de golpear (tareas 75 y 76 de `PROGRESS.md`). Las armas de fuego también están retiradas; su código sigue en el proyecto, inactivo.
