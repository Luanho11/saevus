# Equilibrio (primera pasada)

Las tablas se calculan con los Data Assets reales (`/Game/SAEVUS/Datos/`) en los tests `SAEVUS.Equilibrio.Tecnicas` y `SAEVUS.Equilibrio.Enemigos` (`Source/SAEVUS/Pruebas/SAEVUSPruebasEquilibrio.cpp`). Cada ejecución las escribe en `Saved/Logs/Verificacion/pruebas.log` con el prefijo `[SAEVUS equilibrio]`, ya en formato de tabla Markdown. Para regenerarlas:

```
powershell -ExecutionPolicy Bypass -File Herramientas\Ejecutar-Pruebas.ps1 -Filtro SAEVUS.Equilibrio
```

Son cálculos sobre los datos, sin simular el juego: se encadena en cuanto el ataque lo permite (`EncadenarDesde`) y no se cuentan desplazamientos, paradas de golpe ni fallos.

## Reglas que se comprueban

| Regla | Valor |
|---|---|
| Aviso mínimo de cualquier ataque enemigo, en su fase más rápida | 0,25 s |
| Golpes del mismo ataque o contacto que hacen falta para tumbar al jugador | ≥ 4 (aguanta tres) |
| Combos de suelo para un enemigo menor | ≤ 2 |
| Combos de suelo para un jefe | ≥ 4 |
| Tajos que caben en la recuperación de cada ataque enemigo | ≥ 1 |
| El giro cargado pega más que cualquier tajo del combo; el tajo en carrera, más que el primer tajo | — |

Si un cambio de datos rompe una regla, el test falla. Para cambiar el diseño, se cambia la regla (constantes con nombre al principio del archivo).

## Técnicas del jugador (2026-10-04)

| Técnica | Daño por ciclo | Ciclo (s) | Daño/s |
|---|---|---|---|
| Combo de suelo | 47 | 1,11 | 42,3 |
| Combo aéreo | 42 | 0,89 | 47,2 |
| Tajo hacia arriba | 12 | 0,43 | 27,9 |
| Tajo hacia abajo (sin contar el rebote) | 12 | 0,43 | 27,9 |
| Tajo en carrera (sin contar el dash) | 16 | 0,49 | 32,7 |
| Ataque cargado (tajo + carga + giro) | 40 | 1,73 | 23,1 |
| Pistola (ráfaga) | 20 | 0,25 | 80,0 |
| Pistola (sostenido, con recarga) | 240 | 4,20 | 57,1 |
| Rifle (ráfaga) | 9 | 0,10 | 90,0 |
| Rifle (sostenido, con recarga) | 270 | 4,80 | 56,2 |
| Escopeta (ráfaga, todos los perdigones) | 56 | 0,80 | 70,0 |
| Escopeta (sostenido, con recarga) | 336 | 6,80 | 49,4 |

### Munición (tarea 36)

| Arma | Cargador | Reserva | Daño con todo lleno | Golpes de espada para llenar la reserva |
|---|---|---|---|---|
| Pistola | 12 | 24 | 720 | 12 |
| Rifle | 30 | 60 | 810 | 12 |
| Escopeta | 6 | 12 | 1008 | 12 |

Con todo lleno, cada arma tiene daño para unos tres Guardianes. La reserva vuelve con la espada (doce golpes la llenan) y en los puntos de control. Se comprueba que ninguna arma tenga reserva infinita.

## Enemigos (vida del jugador: 100)

| Enemigo | Vida | Primeros tajos | Combos de suelo | Tiempo con el combo (s) |
|---|---|---|---|---|
| Rastrero | 30 | 3 | 1 | 0,7 |
| Saltador | 25 | 3 | 1 | 0,6 |
| Escupidor | 20 | 2 | 1 | 0,5 |
| Guardián | 250 | 25 | 6 | 5,9 |

| Enemigo | Ataque | Aviso más corto (s) | Daño | Golpes para tumbar al jugador | Castigo (s) | Tajos que caben |
|---|---|---|---|---|---|---|
| Rastrero | contacto | — | 10 | 10 | — | — |
| Rastrero | Embestida | 0,55 | 15 | 7 | 0,80 | 3 |
| Saltador | Salto | 0,45 | 15 | 7 | 0,70 | 3 |
| Escupidor | Escupitajo | 0,60 | 15 | 7 | 1,10 | 3 |
| Guardián | contacto | — | 15 | 7 | — | — |
| Guardián | Golpe | 0,42 | 25 | 4 | 0,49 | 2 |
| Guardián | Embestida | 0,49 | 25 | 4 | 0,70 | 3 |
| Guardián | Salto | 0,42 | 30 | 4 | 0,63 | 3 |

(Contacto de 10 en rastrero, saltador y escupidor.)

## Lectura

- **Las armas de fuego superan al cuerpo a cuerpo** en daño/s, incluso contando la recarga (≈ 50–57 frente a 42), sin riesgo y a distancia. Ni en GunZ ni en Hollow Knight la opción segura es la más rentable. Solución adoptada (tarea 36, cambiable): munición de reserva limitada que se rellena con golpes de espada. El daño de las armas no se toca, pero cuesta acercarse.
- **El ataque cargado** tiene el menor daño/s a un solo objetivo. Es aceptable porque golpea todo alrededor (radio 170) y empuja, pero no compensa contra el Guardián.
- **El Guardián** cae en unos 6 s de combo puro. Contando esquivas y castigos, son de 30 a 40 s reales: suficiente para un mini jefe de tutorial, poco para un jefe de verdad.
- **Castigos:** todos los ataques enemigos dejan meter al menos dos tajos, y la mayoría el combo entero. Es generoso, a propósito para el tutorial.
- **Amenaza:** el Guardián tumba en 4 golpes y los demás en 7. La curación (alma) amplía ese margen.
