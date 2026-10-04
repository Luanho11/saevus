# Controles y técnicas de SAEVUS (boceto jugable)

Abrir el proyecto en el editor y pulsar Play: arranca en el tutorial (`L_Tutorial`). Todas las teclas están en `IMC_SAEVUS` y se pueden cambiar.

## Teclas

| Acción | Teclado y ratón | Mando |
|---|---|---|
| Moverse / mirar | WASD / ratón | stick izquierdo / derecho |
| Esprintar / caminar | Mayús / Alt | clic del stick izquierdo |
| Saltar (doble salto en el aire) | Espacio | A |
| Dash | doble toque de una dirección (W W, A A...) | B |
| Atacar / disparar | clic izquierdo | X |
| Guardia (con el aguijón) | clic derecho (mantener) | gatillo izquierdo |
| Curar (gasta alma) | Q (mantener, en el suelo) | botón superior izquierdo |
| Recargar (de la reserva; la espada la rellena) | R | Y |
| Armas | 1–4, rueda del ratón | botón superior derecho |
| Pausa (opciones) | P | Start |
| Datos de depuración | F6 | — |
| Números de daño (consola, tecla `º`/`~`) | `SAEVUS.Depuracion.NumerosDanio 1` | — |

## Técnicas

- **Tajos en tres direcciones:** mira hacia arriba al atacar para el tajo hacia arriba; en el aire, mira hacia abajo para el tajo hacia abajo.
- **Rebote (pogo):** el tajo hacia abajo que golpea algo (enemigos, muñecos, pinchos) te lanza hacia arriba y recupera el dash y el doble salto. Se puede encadenar indefinidamente.
- **Proyectiles:** la guardia los para; un tajo los devuelve contra quien los lanzó.
- **Contraataque:** levanta la guardia justo cuando llega el golpe: el enemigo queda aturdido y ganas alma.
- **Tajo → guardia:** con la guardia pulsada, cada tajo vuelve a la guardia en cuanto su ventana lo permite (la "mariposa" de GunZ, a la manera de SAEVUS).
- **Ataque cargado:** mantén el clic izquierdo (las partículas se juntan en la hoja) y suelta: giro que golpea todo alrededor.
- **Tajo en carrera:** ataca durante el dash: sigues lanzado en su dirección y el golpe llega más lejos y más fuerte.
- **Combo aéreo:** tres tajos en el aire; los dos primeros frenan la caída y te dejan flotando un momento.
- **Cancelaciones:** dash, salto, guardia y cambio de arma cortan un tajo desde su ventana (los tajos rápidos pronto; el remate tarde). Un ataque durante un dash lo termina.
- **Paredes:** salta contra una pared para rebotar; corre de frente para escalarla; llega en diagonal para correr por ella. Un salto de pared devuelve el doble salto y el dash aéreo.
- **Salto en buffer:** pulsar salto justo antes de aterrizar salta al tocar el suelo (y conserva el doble salto).
- **Dash:** invulnerable un instante al empezar; la cámara se queda fija mientras dura.
- **Alma:** cada golpe a un enemigo o muñeco la llena (orbe arriba a la izquierda); mantén Q en el suelo para curarte una máscara de vida. Un golpe, saltar, atacar o el dash interrumpen la curación.
- **Recibir un golpe:** sales despedido y eres invulnerable un segundo (el cuerpo parpadea).

## Enemigos

Avisan antes de atacar (se agachan y la máscara se enciende), atacan y quedan un momento quietos: es el momento de castigar. Tocarlos hace daño. El Guardián (mini jefe) se enfada a media vida.
