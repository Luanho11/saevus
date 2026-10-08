# Revisión de código externa (2026-10-07) y qué se hizo

El dueño pasó el código (`Source.zip`, idéntico al `Source/` de ese momento) por otro chat de Claude, que hizo un análisis con 12 errores (E), 8 de rendimiento (R), 8 de código duplicado (D) y 9 de malas prácticas (M). Aquí está cada punto **comprobado contra el código**: si era cierto, y qué se cambió. Las rutas son relativas a `Source/SAEVUS/`.

**Resultado:** el análisis era correcto en lo esencial. Todos los errores (E1–E12) eran reales; casi todos estaban latentes porque las armas de fuego y los enemigos se habían retirado de los mapas, o porque solo aparecían en casos raros, pero había que corregirlos. Los dos más graves eran E1 (golpes que no hacían daño contaban como golpe y daban alma) y E2 (morir y volver al punto de control desde la pausa duplicaba al jugador).

## Errores

| Punto | ¿Cierto? | Arreglo | Test nuevo |
|---|---|---|---|
| **E1** Un tajo cuenta como golpe aunque no haga daño | Sí, las tres partes | `TakeDamage` del personaje y del enemigo devuelven la vida perdida de verdad (`USAEVUSComponenteVida::MedirDanio`); solo cuenta como golpe lo que tiene vida o es un blanco de práctica (`SAEVUSCombate::EtiquetaBlancoPractica`, el muñeco); la hoja ignora lo que no bloquea el canal `Arma` (zonas de aviso, banco, pasos) | `SAEVUS.Combate.EscudoSinDanio`, `SAEVUS.Combate.ZonaNoEsGolpe` |
| **E2** Reaparición doble | Sí | El modo de juego guarda el temporizador y `ReiniciarJugador` lo cancela; además `ReaparecerJugador` no toca un pawn vivo | — (necesita modo de juego; probado a mano) |
| **E3** El punto de control casi nunca se restaura | Sí | Punto de control **por mapa** (`PuntosControl`) y el último mapa; "Continuar" abre ese mapa con `?continuar` y aparece en su punto; el aviso del banco dice lo que de verdad hace | `SAEVUS.Guardado.*` (existentes) |
| **E4** Tres sistemas pisan la misma escala de velocidad | Sí | Una escala por fuente (`ESAEVUSFuenteEscalaVelocidad`: ataque, curación, guardia); manda la más baja | `SAEVUS.Movimiento.EscalasVelocidad` |
| **E5** Ataque cargado de espada con el arma de fuego | Sí | `CancelarCarga` al cambiar de arma y al soltar con arma de fuego | — |
| **E6** Los jefes reciben el empuje entero mientras atacan | Sí | Los saltos propios de la IA van por `ASAEVUSEnemigo::LanzarPropio`; todo lo demás se escala siempre | — |
| **E7** Parada de golpe infinita con escala 0 | Sí (latente) | La parada cuenta tiempo real (o del mundo con escala 0) y devuelve la dilatación que había | `SAEVUS.Combate.ParadaEscalaCero` |
| **E8** La puerta no se abre si un enemigo desaparece sin morir | Sí | El enemigo avisa también al destruirse sin morir; la puerta no cuenta los que se están destruyendo | `SAEVUS.Tutorial.PuertaEnemigoDesaparece` |
| **E9** Los proyectiles atraviesan puertas y paredes | Sí | El proyectil bloquea también `WorldDynamic`; es `PhysicsBody` para que los de un abanico no choquen entre sí, y bloquea el canal del arma (se sigue pudiendo devolver) | `SAEVUS.Enemigo.*` (existentes) |
| **E10** Un tirón salta la fase activa | Sí | Se golpea si el paso de tiempo toca la ventana activa, no solo si cae dentro | `SAEVUS.Combate.FotogramaLargo` |
| **E11** Fragmentos sin identificador | Sí | Sin identificador se usa uno estable (mapa + nombre del actor) y se avisa en el log; la cuenta sale del guardado | — |
| **E12** El ataque sigue tras morir; sentarse en el aire | Sí | Morir termina ataque, carga, guardia y curación; solo se sienta de pie en el suelo y sin atacar | — |

Encontrado además en la partida del dueño (log de las 18:45): el proyectil creaba una instancia dinámica de material sobre otra (aviso `not a valid parent`). Corregido: parte del material padre.

## Rendimiento

| Punto | ¿Cierto? | Arreglo |
|---|---|---|
| **R1** Animación: 25 `SetBoneTransformByName` por fotograma | Sí: cada llamada reconstruye la pose entera (lo comprobé en el código del motor) | Se escriben los transformados locales de una vez (`EscribirEnMalla`); los nombres de hueso son `FName` creados una vez; los enemigos que no se ven se animan a 4 Hz |
| **R2** Guardado síncrono jugando | Sí | Guardado en otro hilo (`AsyncSaveGameToSlot`); al cerrar el juego, síncrono |
| **R3** Trabajo repetido por enemigo | Sí | El pawn objetivo se busca una vez por fotograma y su vida mientras sea el mismo; el aspecto (escala, brillo) solo se escribe si cambia |
| **R4** Interfaz recalculada cada fotograma | Sí | La zona se busca una vez y el título deja de calcularse al apagarse; la vida se busca una vez; las opacidades y escalas solo se escriben si cambian |
| **R5** Sonidos sintetizados por enemigo | Sí | Caché de síntesis por huella del sonido (todos los del mismo tipo comparten las muestras) |
| **R6** Mapa de pausa con un widget por rectángulo | Sí | **Pendiente**: solo cuesta al abrir la pausa; se mirará cuando el mapa crezca |
| **R7** Partículas y estela | Sí | El emisor sin partículas vivas no hace nada; la estela de la espada actualiza su malla en vez de rehacerla |
| **R8** Menores | Sí | La mirada busca enemigo cada 0,2 s; la exploración solo marca celdas al cambiar de celda. Los barridos de pared se quedan (son la mecánica) |

## Duplicados y malas prácticas

- **D1** `ObtenerMovimiento()` copiado: ahora `SAEVUSMovimiento::DelActor` en un sitio.
- **D2** Rama de reaparición repetida: `ReaparecerEnPuntoControl`.
- **D3** Aterrizaje detectado dos veces: evento `AlAterrizar` del movimiento.
- **D4** Acumulador de emisión ×4: `USAEVUSComponenteParticulas::ConsumirEmision`.
- **D5** `ShouldTakeDamage` idéntico: `USAEVUSComponenteVida::PuedeRecibirDanio`.
- **D6** Zona de solape ×3 y rutas de formas ×9: `SAEVUSColision::ConfigurarZonaJugador` y `SAEVUSFormasMotor.h`.
- **D7** Reinicio de estado ×4 en el combate: `FijarContexto`.
- **D8** `VidaPorMascara` ×2: la interfaz la lee del personaje (la suya queda de respaldo sin personaje).
- **M1** Punteros crudos y `AtaqueActual` dentro de un Data Asset: `TObjectPtr` y copia del ataque en curso.
- **M2** Delegado estático global: se mantiene, documentado (la puerta solo atiende a sus propios enemigos). Pasará a un subsistema de mundo cuando haya red.
- **M3** La interfaz la crea el "HUD de depuración": **pendiente** (renombrar rompe referencias de Blueprint; se hará con redirecciones).
- **M4** Tecla `[E]` fija: el aviso usa la tecla asignada de verdad. Los mapas como texto quedan (pendiente).
- **M5** Nulos: guardado, IA y menú principal comprobados.
- **M6** Comentarios desfasados: corregidos.
- **M7** Aleatoriedad: la dispersión de las armas se siembra en cada partida; los efectos usan un solo flujo.
- **M8** `FijarVidaMaxima` avisa del cambio y no cura a mitad de combate; la sensibilidad se acota como el volumen.
- **M9** Baja de eventos del sonido: **pendiente** (funciona; es legibilidad).

## Lo que no era un error

Nada del análisis resultó falso. Dos matices:
- E3 era en parte intencional (un solo punto de control), pero la interfaz prometía más de lo que hacía; ahora hace lo que promete.
- E7 no pasaba con los datos actuales (escala 0,05), pero el valor 0 estaba documentado como válido.
