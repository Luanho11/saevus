# Investigación de Hollow Knight → objetivos de diseño de SAEVUS

Estado: borrador (2026-10-03). Fuentes públicas y conocimiento general del juego; sin cifras, código ni recursos de Hollow Knight. Las herramientas web se cortaron a mitad de la investigación (límite de sesión): ampliar con más fuentes cuando vuelvan.

## Fuentes

- Wiki de Hollow Knight (hollowknight.wiki): *Combat (Hollow Knight)* — SOUL, contraataque (parry) con breve invulnerabilidad, aturdimiento de jefes por número de golpes.
- Fextralife, *Nail* y *Combat* (fragmentos de búsqueda): rebote con el ataque hacia abajo ("pogo"), retroceso al golpear.
- PC Gamer, reseña de Hollow Knight: golpes "con peso", retroceso en cada impacto, pausa breve y destello al recibir daño.
- Resumen de habilidades de movimiento (varias reseñas): doble salto, agarre y salto de pared, dash, súper dash.

## Qué hace el jugador

| Mecánica | Qué es | Qué aporta |
|---|---|---|
| Ataque con aguijón en 4 direcciones | Tajo lateral; hacia arriba; hacia abajo en el aire | Atacar donde está el enemigo sin girar la cámara |
| Rebote (pogo) | Golpear hacia abajo a un enemigo, pinchos u objetos rebota al personaje hacia arriba | Une plataformas y combate; encadena rebotes |
| Retroceso al golpear | Cada golpe empuja un poco al jugador (y al enemigo) | Los golpes se sienten con peso; controla distancias |
| Daño recibido | Pierde una "máscara", pausa breve, destello, salir despedido, unos instantes invulnerable | Castigo claro y legible; evita daño encadenado injusto |
| SOUL | Se gana al golpear enemigos | Recurso que premia la agresión |
| Concentrar (curar) | Mantener un botón gasta SOUL y cura tras un tiempo; un golpe lo interrumpe | Riesgo/recompensa: curarse requiere espacio |
| Hechizos | Gastan SOUL: proyectil, caída en picado, explosión hacia arriba | Daño a distancia/área a cambio de no curarse |
| Dash | Ráfaga horizontal con enfriamiento; una versión mejorada atraviesa enemigos invulnerable | Esquiva y movilidad |
| Doble salto | Segundo salto en el aire | Movilidad vertical |
| Agarre de pared | Se pega a paredes, desliza y salta | Escalar huecos verticales |
| Súper dash | Cargar y salir disparado en línea recta | Cruzar grandes distancias |
| Artes del aguijón | Mantener ataque para cargar un golpe especial | Golpes fuertes con preparación |
| Contraataque (parry) | Golpear un ataque enemigo en el momento justo lo anula | Recompensa la precisión |
| Amuletos | Modificadores equipables | Personalización (fuera de alcance por ahora: inventario) |

## Por qué se siente bien

- **Respuesta táctil.** Golpes con peso, retroceso, pausa de impacto, destellos: cada contacto se *siente*.
- **Combate simple pero refinado.** Pocas acciones, todas precisas; la profundidad está en la movilidad y en usar bien la espada.
- **Riesgo/recompensa de la curación.** Curarse cuesta tiempo y recursos ganados atacando.
- **Enemigos y jefes legibles**, con ataques telegrafiados que se aprenden.
- **Tono:** oscuro, melancólico, dibujado a mano; siluetas claras (personaje negro con máscara blanca).

## Qué toma SAEVUS (diseño propio)

1. **Ataques direccionales:** tajo hacia arriba y hacia abajo en el aire, además del frontal.
2. **Rebote (pogo):** el tajo hacia abajo que golpea a un enemigo u objeto rebotable lanza al personaje hacia arriba y recarga el dash aéreo. Encaja con el estilo aéreo de GunZ.
3. **Respuesta al golpe:** retroceso del jugador al golpear, pausa de impacto breve (hit-stop), destello y partículas; al recibir daño: retroceso, destello e invulnerabilidad breve.
4. **Alma (recurso):** se gana golpeando; se gasta en curarse (concentrar, interrumpible) y más adelante en hechizos.
5. **Doble salto** (además del salto de pared de GunZ).
6. **Dash con invulnerabilidad breve** al inicio (esquiva tipo "dash sombra"), ajustable.
7. **Jefes legibles:** telegrafiado claro (destello, postura) antes de cada ataque.
8. **Estética:** sombreado oscuro con aspecto dibujado, silueta oscura con máscara clara (decisión del dueño).

Qué no se toma ahora: amuletos e inventario, mapa con cartógrafo, economía.
