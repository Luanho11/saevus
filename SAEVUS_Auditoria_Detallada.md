# SAEVUS v2 — Auditoría detallada por áreas (anexo del Master Audit Report)

Rutas relativas a Source/SAEVUS/ de la v2. Cuatro áreas + inventario de textos.


---

# Auditoría AAA SAEVUS v2: área GAMEPLAY

Alcance: `Combate/`, `Armas/`, `Movimiento/`, `Personaje/`, `Nucleo/`, `Mundo/`, `Tutorial/` y sus pruebas (incluidas `Pruebas/SAEVUSPruebasRevision.cpp` y `Pruebas/SAEVUSMundoPrueba.h`). He leído el código de v2 completo. Además he contrastado el diff, sin tener en cuenta los cambios de fin de línea (CRLF): Guardia, Armas, Alma y ModoJuego aparecen en `v1_a_v2.diff` como reescritos enteros, pero su cambio real es pequeño. Todas las rutas son relativas a `Source/SAEVUS/` de v2.

## 1. Resumen del área

- **Estado.** El código de gameplay es limpio, legible y está muy orientado a datos. v2 corrige con cuidado casi toda la revisión anterior: copia del ataque en curso, escalas de velocidad por fuente, parada de golpe restaurable, golpe «real» medido en vida, puerta de arena a prueba de enemigos destruidos y punto de control por mapa. Compilando de memoria el código nuevo de v2 no he encontrado errores probables. Hay una trampa latente en el inicializador de `EscalasVelocidadAccion` (GAMEPLAY-21).
- **Fortalezas que conservar:**
  - Ataques como líneas de tiempo de datos, con ventanas de cancelación por acción.
  - El ataque empieza en el fotograma de la pulsación.
  - Tiempo coyote, buffer de salto y «buffer gana al doble salto».
  - Parada de golpe en tiempo real.
  - Alma y munición que se alimentan de la espada: una economía con buena intención.
  - Pruebas deterministas con `FMundoPrueba`.
  - Tablas de equilibrio automáticas.
- **Debilidades.** El diseño tiene estrategias dominantes:
  - Dash sin enfriamiento, con 83 % de invulnerabilidad y dashes aéreos ilimitados que permiten planear.
  - Guardia gratuita al 100 % con parry que se puede machacar.

  Además, las pulsaciones fuera de ventana se pierden. El mundo no se persiste (puertas, palancas, paredes, jefes, enemigos), así que muerte y punto de control casi no castigan, y el jefe no se reinicia.
- **Mayor riesgo.** Ahora mismo el juego se gana spameando dash y tajo en carrera, y los fosos del nivel no obligan a usar la mecánica que enseñan. Cuando haya más contenido, la falta de persistencia del estado del mundo produce bloqueos al hacer «Continuar» y deja la progresión rota. Caer fuera del mapa deja al jugador sin personaje y sin reaparición.

## 2. Puntuaciones sugeridas (0–100)

| Categoría | Nota | Justificación |
|---|---|---|
| Movimiento: control y sensación | 72 | Kit GunZ completo y bien asistido (coyote, buffer, pared). El dash ilimitado y el salto de altura fija le quitan peso y precisión. |
| Combate cuerpo a cuerpo: estructura y peso | 66 | Líneas de tiempo, abanico visible, parada de golpe, retroceso y nueve gestos. Lo rompe el bucle dash, tajo en carrera y dash. |
| Defensa: guardia y parry | 42 | Bloquea el 100 % sin coste, el parry se puede machacar y su respuesta es pobre. |
| Latencia y entrada | 60 | Ataque inmediato con buffer de 0,2 s. Salto, dash, cambio de arma y ataque durante el cambio se descartan en silencio. |
| Economía de recursos (alma, munición, vida, fragmentos) | 52 | Bien planteada. Se rompe con el parry farmeable, el banco infinito y la curación al cambiar de mapa o reiniciar. |
| Progresión | 30 | Solo hay fragmentos de máscara: todas las habilidades están desde el minuto 1 y no se persiste nada del mundo. |
| Muerte, puntos de control y castigo | 35 | Solo cuesta el alma y la caminata. El jefe conserva el daño recibido. Al caer fuera del mapa no hay reaparición. |
| Onboarding (sistema; los textos están en los mapas: DESCONOCIDO) | 45 | Avisos de texto en cajas de un solo uso, sin cola y sin «aprender haciendo». Los fosos se saltan con dashes. |
| Exploración y secretos | 45 | Mapa explorado, fragmentos y paredes falsas. Hay pocos incentivos y nada persiste salvo los fragmentos. |
| Guardado | 45 | Async y tolerante a nulos, pero sin versión, sin copia de seguridad, con escrituras concurrentes y sin estado del mundo. |
| Robustez del código v2 (compila y sin bugs) | 78 | Pocos fallos, casi todos de borde. Ver GAMEPLAY-3, 13, 19 y 21. |
| Pruebas de gameplay | 70 | Amplias y deterministas. No cubren E2, E3, E5 ni E12 (ataque), ni los campos nuevos del guardado, y el equilibrio no modela las cancelaciones. |
| Orientado a datos | 82 | Todo es ajustable por Data Asset. Los valores por defecto del código son los que se prueban. |

## 3. Estado de la revisión anterior (IDs del área)

| ID | Estado | Evidencia (v2) |
|---|---|---|
| E1 | RESUELTO | Se filtra por respuesta a `TrazaArma` (`Combate/SAEVUSComponenteCombate.cpp:413-418`). «Golpe» exige `Hecho>0` y vida o etiqueta de práctica (`:24-27`, `:440-444`). Los personajes y el enemigo devuelven la vida perdida de verdad gracias a `MedirDanio` (`Combate/SAEVUSComponenteVida.cpp:118-123`, `Personaje/SAEVUSPersonaje.cpp:320-323`). Hay pruebas (`Pruebas/SAEVUSPruebasRevision.cpp:89-142`). Efecto colateral: `ASAEVUSPeligro` ya no se golpea y su `TakeDamage` es código muerto (GAMEPLAY-29). |
| E2 | RESUELTO (sin prueba) | El manejador se guarda en `FTimerHandle TemporizadorReaparicion` (`Nucleo/SAEVUSModoJuego.cpp:91`). `ReiniciarJugador` lo limpia (`:122`) y `ReaparecerJugador` sale si el pawn está vivo (`:104-108`). |
| E3 | RESUELTO (sin prueba) | `TMap PuntosControl` (`Nucleo/SAEVUSGuardado.h:49-50`). «Continuar» abre el mapa del último punto con `?continuar` (`Nucleo/SAEVUSControladorJugador.cpp:139-161`) y el modo de juego lo aplica (`Nucleo/SAEVUSModoJuego.cpp:58-65`). Sigue siendo una decisión a confirmar: llegar por un paso pisa el punto de ese mapa (`:53-56`). |
| E4 | RESUELTO | Escalas por fuente (`Movimiento/SAEVUSTiposMovimiento.h:21-28`, `Movimiento/SAEVUSComponenteMovimiento.h:112-115`), con prueba. La gravedad sigue teniendo un solo dueño (Combate). Ver la trampa de GAMEPLAY-21. |
| E5 | RESUELTO (sin prueba) | `CancelarCarga` al aceptar un cambio de arma y al soltar con arma de fuego o durante un cambio (`Personaje/SAEVUSPersonaje.cpp:549-563`). |
| E7 | RESUELTO (alternativa) | La escala 0 está permitida y el tick usa el delta del mundo cuando la dilatación es ≈0 (`Combate/SAEVUSComponenteCombate.cpp:143-144`). Se restaura la dilatación previa (`Combate/SAEVUSParadaGolpe.h:31,58`). Hay prueba. |
| E8 | RESUELTO | `ASAEVUSEnemigo::EndPlay` emite `AlMorirEnemigo` si lo destruyen sin morir (`Enemigos/SAEVUSEnemigo.cpp:352-358`) y la puerta ignora los que se están destruyendo (`Tutorial/SAEVUSPuertaArena.cpp:58`). Hay prueba. Excepción: si el enemigo se descarga por streaming (`RemovedFromWorld`) no se emite nada. |
| E10 | RESUELTO | `TiempoAtaque >= InicioActivo && Antes < FinActivo` (`Combate/SAEVUSComponenteCombate.cpp:158-166`), con prueba. |
| E11 | RESUELTO | Identificador de respaldo `Mapa.NombreActor` y aviso en el log (`Mundo/SAEVUSFragmentoMascara.cpp:39-52`). No se cuenta dos veces (`Personaje/SAEVUSPersonaje.cpp:214-221`). Hay una incompatibilidad con partidas antiguas (GAMEPLAY-26). |
| E12 | RESUELTO (en parte, sin prueba de ataque) | La muerte corta ataque, carga, guardia, curación y banco (`Personaje/SAEVUSPersonaje.cpp:166-172`). El banco exige `PuedeSentarse` (`Mundo/SAEVUSBanco.cpp:95`). Queda: guardia o golpe recibido estando sentado (GAMEPLAY-20). |
| R2 | PARCIAL | Ahora el guardado es asíncrono y solo descubre al cambiar de celda, pero `IntervaloGuardado` sigue en 5 s (`Mundo/SAEVUSExploracion.h:19`). Tampoco guarda al pausar. |
| D1 | RESUELTO | `SAEVUSMovimiento::DelActor` (`Movimiento/SAEVUSComponenteMovimiento.cpp:683-687`), usado en todos los componentes. |
| D2 | RESUELTO | `ReaparecerEnPuntoControl` (`Nucleo/SAEVUSModoJuego.cpp:131-141`). |
| D5 | RESUELTO | `USAEVUSComponenteVida::PuedeRecibirDanio` (`Combate/SAEVUSComponenteVida.cpp:125-128`). |
| D6 | RESUELTO | `SAEVUSFormas` (`Nucleo/SAEVUSFormasMotor.h`) y `SAEVUSColision::ConfigurarZonaJugador` (`Nucleo/SAEVUSCanalesColision.h:22-29`). Solo quedan rutas escritas a mano en las pruebas. |
| D7 | RESUELTO | `FijarContexto` (`Combate/SAEVUSComponenteCombate.cpp:243-250`). |
| D8 | PARCIAL | El dueño es `FSAEVUSAjusteProgreso::VidaPorMascara` (`Personaje/SAEVUSPersonaje.h:55`). El widget lo lee, pero mantiene un respaldo duplicado de 20 (`Interfaz/SAEVUSWidgetInterfaz.h:31`, `.cpp:554`). |
| M1 (AtaqueActual) | RESUELTO | Copia `AtaqueEnCurso` y `bAtacando` (`Combate/SAEVUSComponenteCombate.h:145-147`). |
| M4 | RESUELTO | La tecla real sale de `QueryKeysMappedToAction` (`Interfaz/SAEVUSWidgetInterfaz.cpp:467-477`). Las rutas de mapa son `UPROPERTY` (`Nucleo/SAEVUSModoJuego.h:64-70`). |
| M5 | RESUELTO (área) | `Partida` nunca es nula (`Nucleo/SAEVUSGuardado.cpp:70-74`) y hay comprobación del `MenuPrincipal` (`Nucleo/SAEVUSControladorJugador.cpp:96`). |
| M7 | RESUELTO | Semilla desde `FPlatformTime::Cycles()` y `VRandCone` del flujo (`Armas/SAEVUSComponenteArmas.cpp:31,298`). |
| M8 | PARCIAL | `FijarVidaMaxima` ya documenta que cura y revive (`Combate/SAEVUSComponenteVida.h:56`) y la sensibilidad se acota. Pero `FijarSensibilidad` guarda el valor sin acotar (GAMEPLAY-31). |

## 4. Hallazgos

### GAMEPLAY-1 Dash sin enfriamiento con 83 % de invulnerabilidad y planeo infinito: estrategia dominante y fosos sin función
- Estado: CONFIRMADO (valores por defecto del código y prueba `ComoGunZ`). Los valores del DA de movimiento son DESCONOCIDOS · Prioridad: P0 · Severidad 5 · Impacto jugador 5 · Coste S · Riesgo regresión medio
- Dónde:
  - `Movimiento/SAEVUSAjustesMovimiento.h:138` (Enfriamiento 0), `:146` (MaxDashesAereos −1), `:150` (VelocidadVerticalDashAereo 0), `:165` (DuracionInvulnerable 0,15).
  - `Movimiento/SAEVUSComponenteMovimiento.cpp:303-309`.
  - `Personaje/SAEVUSPersonaje.cpp:130-133`.
  - `Pruebas/SAEVUSPruebasDash.cpp:206-241`.
- Qué pasa y por qué:
  1. Cada dash da 0,15 s de invulnerabilidad sobre 0,18 s de duración y no tiene enfriamiento. Pulsando dash a unos 5,5 Hz, el jugador es invulnerable el 83 % del tiempo, y el dash cancela incluso la curación y la guardia.
  2. En el aire, cada dash pone `Velocity.Z = 0` (`:306`). Con gravedad 1,75 g solo se caen unos 28 cm por dash mientras se avanzan unos 288 cm: planeo 10:1 ilimitado.
  3. Bucle de daño: dash, ataque (tajo en carrera, 16 de daño) y otra vez dash a los 0,19 s (`CancelarConDashDesde = EncadenarDesde`, `Combate/SAEVUSTiposCombate.cpp:130-131`). Da unos 80 de daño por segundo con invulnerabilidad casi continua, frente a unos 42/s del combo de suelo. La tabla de `SAEVUS.Equilibrio` no lo detecta porque mide el tajo en carrera «suelto» (`Pruebas/SAEVUSPruebasEquilibrio.cpp:157`).
- Por qué importa:
  - Jugador: no hace falta leer los telegrafiados. Los fosos del tutorial (800 y 900 cm, `Pruebas/SAEVUSPruebasObstaculos.cpp:31-36`) se cruzan sin la mecánica que pretenden enseñar, y los límites del nivel no contienen al jugador (ver GAMEPLAY-3).
  - Producción: obliga a diseñar cada sala contra el vuelo.
- Solución propuesta (incremental):
  - Invulnerabilidad solo en los primeros 0,08–0,10 s del dash y nunca en un dash encadenado antes de X s (contador de «dash en racha»).
  - Enfriamiento de suelo de 0,15–0,25 s, o bien encadenar sin espera pero sin invulnerabilidad.
  - `MaxDashesAereos = 1` (que el salto de pared y la carrera por pared ya recargan) o `VelocidadVerticalDashAereo` negativa.
  - Que el tajo en carrera exija un dash de al menos 0,1 s y añada 0,1 s más a su ventana de cancelación por dash.
  - Si se quiere el «modo GunZ», que sea un perfil de DA aparte.
- Validación: añadir a `Equilibrio` una técnica «bucle dash y tajo en carrera» y la regla «ningún bucle supera 1,3× el daño por segundo del combo». Prueba «dashes aéreos encadenados: altura perdida por metro ≥ X». Prueba «fracción invulnerable en 3 s de dash machacado ≤ 40 %».

### GAMEPLAY-2 El estado del mundo no se guarda: puertas, palancas, paredes, jefes y enemigos vuelven con cada carga
- Estado: CONFIRMADO (código). El riesgo de bloqueo depende del diseño de nivel (DESCONOCIDO) · Prioridad: P1 · Severidad 4 · Impacto 5 · Coste M · Riesgo medio
- Dónde: `Nucleo/SAEVUSGuardado.h:24-59` (solo opciones, puntos, fragmentos y exploración); `Mundo/SAEVUSPalanca.h:69`; `Mundo/SAEVUSParedRompible.h:46-47`; `Tutorial/SAEVUSPuertaArena.h:61`; enemigos y jefes sin persistencia.
- Qué pasa: «Continuar» u «OpenLevel» recargan el mapa con todo cerrado y vivo, pero el jugador aparece en su punto de control, que puede estar detrás de una puerta de arena. Secuencia de bloqueo: el jugador limpia la arena A y la puerta se abre. Descansa en un banco de la sala B y sale del juego. Al hacer «Continuar», la puerta A está cerrada y sus enemigos vivos al otro lado. Si hay que volver atrás, la puerta solo se abre cuando mueren esos enemigos, a los que el jugador no llega. Lo mismo ocurre con los atajos de palanca (`Mundo/SAEVUSPalanca.h:14-16`) y con el jefe derrotado, que revive.
- Por qué importa: rompe la progresión tipo metroidvania y el «Victoria/recompensa» del concepto. Además, a ×10 de contenido cada actor nuevo necesitaría su propio parche.
- Solución: añadir `TSet<FName> EstadoMundo` (o `TMap<FName,uint8>`) en `USAEVUSPartidaGuardada`, con un identificador estable por actor (el mismo patrón que `ObtenerIdentificador` de los fragmentos) y una interfaz `ISAEVUSPersistente { FName Id(); void Restaurar(); }`. Palanca, puerta (cuando se abre), pared rota y jefe derrotado se registran al cambiar y se restauran en `BeginPlay`. Los enemigos menores no se persisten: reaparecen al descansar, como en Hollow Knight.
- Validación: prueba «puerta abierta y recarga del mundo de prueba con el mismo guardado ⇒ `EstaAbierta()`». Recorrido automático: Continuar y volver atrás.

### GAMEPLAY-3 Caer fuera del mundo destruye al jugador sin reaparición
- Estado: CONFIRMADO (no hay manejador). El comportamiento del motor es INFERIDO · Prioridad: P1 · Severidad 4 · Impacto 4 · Coste S · Riesgo bajo
- Dónde: `Personaje/SAEVUSPersonaje.h` (no sobrescribe `FellOutOfWorld`); `Nucleo/SAEVUSModoJuego.cpp:85-93` (solo `NotificarMuerteJugador` programa la reaparición).
- Qué pasa: `AActor::FellOutOfWorld` oculta y destruye el pawn sin pasar por `AlMorir`, así que no hay temporizador de reaparición. Secuencia: con dashes aéreos ilimitados (GAMEPLAY-1) el jugador sale por un borde sin `VolumenCaida` y cae hasta KillZ. KillZ es DESCONOCIDO en los mapas; con el valor por defecto son minutos de caída. Al final se queda con la cámara sin pawn, sin «Has caído», y solo lo salva la pausa con «Volver al punto de control».
- Solución: `ASAEVUSPersonaje::FellOutOfWorld` que llame a `Vida->AplicarDanio(VidaMax, nullptr)` (muerte normal) o que haga como `VolumenCaida`. Además, que `ModoJuego` detecte un controlador sin pawn y lo reaparezca. KillZ razonable por mapa en `WorldSettings`.
- Validación: prueba que llama a `FellOutOfWorld` en el mundo de prueba con modo de juego y espera que el controlador vuelva a tener pawn.

### GAMEPLAY-4 Parry que se puede machacar y guardia gratuita al 100 %: la defensa no tiene coste
- Estado: CONFIRMADO · Prioridad: P1 · Severidad 4 · Impacto 5 · Coste S/M · Riesgo medio
- Dónde: `Combate/SAEVUSComponenteGuardia.h:28` (FraccionBloqueada 1), `:50` (Ventana 0,15), `:53` (22 de alma); `Combate/SAEVUSComponenteGuardia.cpp:45-52,68-75,135-149,107-111`.
- Qué pasa:
  - Cada pulsación de guardia reinicia `InicioGuardia`. Soltar y pulsar a unos 6 Hz deja la ventana de parry abierta más del 75 % del tiempo y no hay castigo por fallar.
  - Cada parry da 22 de alma (dos tercios de una curación), aturde y da 0,3 s de invulnerabilidad.
  - La guardia mantenida bloquea todo el daño frontal, proyectiles incluidos, sin estamina ni rotura.
  - La «mariposa» (tajo, guardia por la ventana `Guardia` a los 0,18 s, tajo…) reabre una ventana de parry tras cada tajo y hace más daño por segundo que el combo.
  - También se hace parry al daño por contacto del cuerpo enemigo.
- Por qué importa: la decisión de atacar o defenderse se vuelve trivial y el alma se farmea sin riesgo. Esto contradice el pilar Hollow Knight (curar cuesta exposición).
- Solución:
  - Bloqueo parcial: `FraccionBloqueada` 0,6–0,8 en el suelo, con «daño gris» recuperable al golpear.
  - O bien una barra de postura, o retroceso que crece con golpes seguidos.
  - Parry con enfriamiento y castigo al fallar: si no hubo contraataque en la ventana, no se puede volver a hacer parry hasta 0,4 s después de soltar.
  - Excluir el contacto y los proyectiles del parry, o darles una reflexión propia.
  - Alma del parry 11.
- Validación: prueba «machacar guardia 2 s contra golpes cada 0,3 s ⇒ como mucho un parry». Prueba «guardia mantenida 10 golpes ⇒ hay coste».

### GAMEPLAY-5 Las pulsaciones fuera de la ventana de cancelación se pierden en silencio (sin buffer de cancelación)
- Estado: CONFIRMADO · Prioridad: P1 · Severidad 3 · Impacto 5 · Coste S · Riesgo medio (hay que tocar `SAEVUS.Combate.Cancelaciones`)
- Dónde: `Personaje/SAEVUSPersonaje.cpp:302-305` (salto), `:483`, `:493` (dash), `:533` (ataque durante un cambio de arma), `:560` (ranura); `Combate/SAEVUSTiposCombate.cpp:49` (Remate sin cancelación por salto, 0,75 s); `Pruebas/SAEVUSPruebasCombate.cpp:196-201`.
- Qué pasa: el ataque tiene un buffer de 0,2 s, pero dash, salto y cambio de arma se descartan si llegan 1–2 fotogramas antes de la ventana. Secuencia: Tajo1 y dash pulsado a 0,15 s ⇒ no pasa nada. El jugador percibe «el juego no responde», que es lo peor para el game feel.
- Solución: buffer genérico de acción en `USAEVUSComponenteCombate` (`AccionPendiente` + tiempo, 0,15 s). `IntentarCancelarPor` guarda la petición y `TickComponent` la ejecuta en cuanto se abre la ventana. El ataque pulsado durante `EstaCambiando` se guarda hasta que termina el cambio.
- Validación: modificar la prueba de cancelaciones: «dash pulsado a 0,15 s ⇒ dash a 0,18 s».

### GAMEPLAY-6 Morir no castiga y el jefe no se reinicia: combates de desgaste y curación gratis desde la pausa
- Estado: CONFIRMADO · Prioridad: P1 · Severidad 4 · Impacto 4 · Coste M · Riesgo medio
- Dónde: `Nucleo/SAEVUSModoJuego.cpp:95-129`; `Nucleo/SAEVUSControladorJugador.cpp:163-173`; en `Enemigos/` no hay ningún `RestablecerVida`; `Mundo/SAEVUSBanco.cpp:92-108`.
- Qué pasa:
  - Al morir solo se pierde el alma y el tiempo de vuelta. Los enemigos muertos siguen muertos y el jefe conserva su vida y su fase.
  - «Volver al punto de control» desde la pausa es una curación total gratuita en cualquier momento, también a mitad de jefe. Secuencia: golpear al jefe hasta el 50 %, pausa, reiniciar con vida llena, volver; el jefe sigue al 50 %.
  - El banco cura, rellena munición y fija el punto de forma ilimitada sin repoblar el mundo.
  - Cambiar de mapa cura del todo, porque el pawn es nuevo (GAMEPLAY-11).
- Solución:
  - Que el jefe se reinicie al morir o reiniciar el jugador: `ASAEVUSEnemigo::ReiniciarCombate()` restaura la vida, la fase y la posición inicial cuando cambia el pawn objetivo.
  - Puerta de jefe que se cierra al entrar: `PuertaArena` con disparador de entrada.
  - El banco repuebla a los enemigos menores (lista de los que se pueden reaparecer).
  - «Volver al punto de control» cuenta como muerte, o se desactiva en combate.
  - Valorar una sombra o pérdida de un recurso al morir.
- Validación: prueba «jugador muere ⇒ el jefe vuelve a su vida máxima». Prueba «reiniciar desde la pausa con un enemigo en combate ⇒ cuenta como muerte».

### GAMEPLAY-7 Guardado: escrituras asíncronas concurrentes sobre la misma ranura, sin copia de seguridad ni versión; una lectura fallida borra el progreso
- Estado: INFERIDO (concurrencia según la implementación de `AsyncSaveGameToSlot`). CONFIRMADO: no hay copia de seguridad ni versión · Prioridad: P1 · Severidad 5 · Impacto 4 · Coste S · Riesgo bajo
- Dónde: `Nucleo/SAEVUSGuardado.cpp:17-28,36-50,78-85,151-157`; `Mundo/SAEVUSExploracion.cpp:110-114`.
- Qué pasa:
  - Cada cambio (punto, fragmento, exploración cada 5 s, cada clic de volumen o sensibilidad) lanza un `AsyncSaveGameToSlot` independiente. Dos en el mismo fotograma, o una escritura asíncrona en vuelo más la síncrona de `Deinitialize`, escriben el mismo archivo desde hilos distintos sin orden garantizado.
  - Si la escritura queda truncada (corte de luz o cierre), `Cargar` registra «se empieza con una nueva» y el siguiente `Guardar` sobrescribe el archivo: pérdida total.
  - No hay campo de versión para migrar.
- Solución:
  - Serializar las escrituras: un único guardado en vuelo con una bandera de «sucio» que relanza al terminar.
  - Escribir en `SAEVUS_tmp` y rotar a `SAEVUS` y `SAEVUS_bak`.
  - Al cargar, si falla la principal, probar con `_bak` y no sobrescribir hasta tener un guardado válido.
  - Añadir `UPROPERTY() int32 Version` y la migración en `Cargar`.
  - Agrupar (debounce) volumen y sensibilidad.
- Validación: prueba que corrompe el archivo (escribe bytes basura) y comprueba que se carga `_bak`. Prueba de ida y vuelta con `PuntosControl`, `Fragmentos` y `Exploracion` (hoy no existe: `Pruebas/SAEVUSPruebasGuardado.cpp:19-51`).

### GAMEPLAY-8 El tutorial no obliga a usar la mecánica que enseña y queda el rastro de un foso de rebote sin rebote
- Estado: INFERIDO (las medidas del mapa salen de las constantes de las pruebas). Los textos y el contenido del mapa son DESCONOCIDOS · Prioridad: P2 · Severidad 3 · Impacto 4 · Coste S · Riesgo bajo
- Dónde: `Pruebas/SAEVUSPruebasObstaculos.cpp:31-47` (constantes `AnchoFosoRebote`, `XMunecosRebote` y pinchos sin prueba que las use); `Tutorial/SAEVUSPeligro.h:11-15`; `Combate/SAEVUSTiposCombate.h:22-28` (no hay tajo hacia abajo).
- Qué pasa:
  - El foso de dash (800) y el de carrera por pared (900) se cruzan encadenando dashes aéreos (GAMEPLAY-1).
  - Las pruebas describen un «foso de rebote» de 1200 con muñecos sobre pinchos, pensado para un tajo hacia abajo que no existe. Si el mapa lo conserva, enseña algo inexistente y además regala alma (los muñecos cuentan como golpe).
  - Los avisos son texto en cajas de un solo uso, sin comprobar que el jugador hizo la acción, y un aviso nuevo pisa al anterior (GAMEPLAY-23).
- Por qué importa: el primer minuto enseña «clase» en lugar de «haciendo», y las salas no validan lo aprendido.
- Solución:
  - Retocar los fosos para que solo la mecánica objetivo los resuelva (con GAMEPLAY-1).
  - Quitar el foso de rebote o implementar el pogo con `ESAEVUSDireccionAtaque::Abajo` (inclinación de cámara ≤ −X en el aire, con rebote vertical al impactar con algo que tenga `TrazaArma` Block, incluidos los pinchos).
  - Los volúmenes de aviso pueden exigir «acción realizada» (evento del componente) antes de abrir el paso.
- Validación: añadir a `PruebasObstaculos` las pruebas negativas «sin carrera por pared no se cruza el foso de carrera» y «sin dash no se cruza el foso de dash».

### GAMEPLAY-9 El parry y el bloqueo apenas se sienten: sin parada de golpe, sin cámara y sin efecto propio en el parry
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto 4 · Coste S · Riesgo bajo
- Dónde: `Combate/SAEVUSComponenteGuardia.cpp:151-169`; `Camara/SAEVUSComponenteCamara.cpp:60,77` (solo `AlBloquear`); `Efectos/SAEVUSComponenteEfectosCombate.cpp:45` (solo bloqueo); `Audio/SAEVUSComponenteSonido.cpp:196` (el parry solo tiene sonido).
- Qué pasa: el parry, el momento cumbre de la hoja de dirección de arte (sección H), se ve igual que un bloqueo, con un sonido distinto. No hay parada de golpe, ralentización, destello ni postura del enemigo «abierto» legible.
- Solución: en `Contraatacar`, una `FSAEVUSParadaGolpe` de 0,1 s sobre el jugador y el atacante, una sacudida propia, chispas azules (concepto) y un pulso de color en la interfaz. Reutilizar `ParadaGolpe` (hay que exponerla o duplicar la instancia en la guardia).
- Validación: prueba «contraataque ⇒ el atacante tiene `CustomTimeDilation` < 1 durante X».

### GAMEPLAY-10 El tajo hacia arriba lo decide la inclinación de la cámara: entrada oculta que rompe combos
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Personaje/SAEVUSPersonaje.cpp:541-542`; `Combate/SAEVUSTiposCombate.h:254-255` (30°); `Combate/SAEVUSComponenteCombate.cpp:215-223`.
- Qué pasa: subir la cámara 30° para mirar a un volador o a un jefe alto convierte cada tajo en `TajoArriba`, que tiene alcance corto (esfera de 70 cm sobre la cabeza) y no encadena. El jugador ve combos «que no salen» sin entender por qué.
- Solución: dirección por stick o tecla (mantener arriba más ataque), o un umbral alto (≥45°) con histéresis más un aviso visual (retícula o flecha). Opción de accesibilidad.
- Validación: la prueba `Tajos.Inclinacion` se amplía con los casos de histéresis.

### GAMEPLAY-11 El estado del jugador no cruza mapas ni reapariciones: el alma se pierde y la vida se rellena
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 3 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Mundo/SAEVUSVolumenCambioMapa.cpp:7-23`; `Personaje/SAEVUSPersonaje.cpp:194-208` (solo recupera fragmentos); `Combate/SAEVUSComponenteAlma.h:111`.
- Qué pasa: cada paso de mapa crea un pawn nuevo con vida llena, alma 0 y munición llena. Pasar de zona sirve para curarse (exploit) y castiga al que llega con alma. Además, el corte entre mapas es seco (`OpenLevel` sin transición, INFERIDO).
- Solución: `FSAEVUSEstadoJugador {Vida, Alma, Cargadores, Reservas, Ranura}` en el subsistema de guardado (no hace falta persistirlo a disco), aplicado en `BeginPlay` cuando se llega por un paso. Al reaparecer tras morir se mantiene la regla actual.
- Validación: prueba de paso: vida y alma iguales antes y después.

### GAMEPLAY-12 Los ataques aéreos no alcanzan a enemigos bajos y no hay ataque hacia abajo
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto 3 · Coste M · Riesgo bajo
- Dónde: `Combate/SAEVUSTiposCombate.cpp:64` (TajoAereo con `AlturaGolpe` 0); `Combate/SAEVUSTiposCombate.h:112` (`MitadAlturaAbanico` 70); `Combate/SAEVUSComponenteCombate.cpp:332-335`.
- Qué pasa: un objetivo con su punto más cercano a más de 70 cm por debajo del centro del jugador queda fuera del abanico. Un rastrero no se puede castigar desde el salto, y el combo aéreo flotante no tiene uso contra el suelo.
- Solución: un abanico aéreo con un desplazamiento vertical negativo, o el ataque hacia abajo de GAMEPLAY-8 (pogo), que además da profundidad sin complejidad artificial.

### GAMEPLAY-13 El punto de control copia la transformación del volumen, escala incluida
- Estado: INFERIDO (`RestartPlayerAtTransform` usa la transformación completa) · Prioridad: P2 · Severidad 3 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Tutorial/SAEVUSPuntoControl.cpp:13`; `Nucleo/SAEVUSModoJuego.cpp:133-136`.
- Qué pasa: si un diseñador escala la caja del `ASAEVUSPuntoControl` para cubrir un pasillo (lo habitual), el pawn reaparece escalado o con alabeo. Si el centro del volumen queda a ras de suelo, aparece enterrado o no aparece (DESCONOCIDO). El banco lo hace bien (`Mundo/SAEVUSBanco.cpp:86-90`).
- Solución: `FTransform(FRotator(0, Yaw, 0), PuntoEnSuelo)` con una traza hacia abajo y la media altura de la cápsula. Opcional: un `PuntoReaparicion` editable (MakeEditWidget), como el de `VolumenCaida`.
- Validación: prueba con el volumen a escala 3 ⇒ el personaje reaparece con escala 1.

### GAMEPLAY-14 La tabla de equilibrio no modela las cancelaciones y da por buenos bucles dominantes
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto 3 (indirecto) · Coste S · Riesgo bajo
- Dónde: `Pruebas/SAEVUSPruebasEquilibrio.cpp:64-90,153-159`.
- Qué pasa: solo mide cadenas enteras y ataques sueltos. Le faltan «dash, tajo en carrera y cancelación por dash», «mariposa con guardia» y «Tajo1 cancelado por dash, en bucle», que son las estrategias reales. Tampoco hay ninguna regla sobre invulnerabilidad, alma por segundo ni curaciones por combate.
- Solución: generar las técnicas a partir de las ventanas (`CancelarCon*Desde`), aplicar la regla de dominancia (GAMEPLAY-1) y medir «alma por segundo bloqueando».

### GAMEPLAY-15 Faltan pruebas de las correcciones E2, E3, E5 y E12 (ataque) y del guardado nuevo
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: no hay pruebas de `ModoJuego` salvo `ChoosePlayerStart` (`Pruebas/SAEVUSPruebasMapas.cpp`); `Pruebas/SAEVUSPruebasVida.cpp:76-100` no prueba que un muerto deje de golpear; `Pruebas/SAEVUSPruebasGuardado.cpp` no cubre `PuntosControl`, `Fragmentos` ni `Exploracion`; en las pruebas de armas no hay ningún `CancelarCarga`. En `Pruebas/SAEVUSPruebasRevision.cpp:133-140`, el segundo bloque no comprueba `bListo`.
- Solución: pruebas con modo de juego en el mundo de prueba: doble reaparición, Continuar con `?continuar`, carga y cambio a pistola y soltar ⇒ sin `AtaqueCargado`, muerte a mitad de la fase activa ⇒ el muñeco no recibe golpes.

### GAMEPLAY-16 Una fracción del tiempo de invulnerabilidad se consume durante la propia parada de golpe
- Estado: CONFIRMADO (mecánica); impacto INFERIDO · Prioridad: P3 · Severidad 2 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Combate/SAEVUSComponenteVida.cpp:92-104` (tiempo del mundo); `Combate/SAEVUSParadaGolpe.h:20-36`.
- Qué pasa: el jugador se congela a 0,05 y su invulnerabilidad (dash, tras golpe, parry) sigue corriendo en tiempo de mundo. Un tajo en carrera que acierta gasta 0,06 s de los 0,15 de invulnerabilidad del dash con el personaje quieto. Es inconsistente con el resto de temporizadores, que son dilatados.
- Solución: medir la invulnerabilidad con el tiempo dilatado del dueño (acumulador en `Tick`) o alargarla con la duración de la parada.

### GAMEPLAY-17 Retroceso al recibir golpe fuerte (550/350) sin tener en cuenta los fosos: muertes baratas
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Personaje/SAEVUSPersonaje.h:37-40`; `Personaje/SAEVUSPersonaje.cpp:154-160`.
- Qué pasa: cada golpe lanza al jugador con `LaunchCharacter`, sustituyendo XY y Z, unos 2–3 m. Junto a un foso suma el daño de caída al del golpe. Durante una estocada, `CalcVelocity` anula el retroceso (`Movimiento/SAEVUSComponenteMovimiento.cpp:441-446`), así que el golpe se siente distinto según cuándo llegue.
- Solución: reducir el retroceso horizontal (unos 300), cortar la estocada al recibir un golpe (`TerminarEstocada`) y dejar el retroceso por datos según el tipo de ataque enemigo.

### GAMEPLAY-18 Salto de altura fija (`TiempoMaximoMantenerSalto = 0`)
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 3 · Coste S · Riesgo medio (tests de obstáculos)
- Dónde: `Movimiento/SAEVUSAjustesMovimiento.h:64-66`.
- Qué pasa: sin salto variable, el plataformeo pierde precisión; es el control más expresivo de Hollow Knight. Con gravedad 1,75 el salto es seco, pero no se puede «pellizcar».
- Solución: corte de salto al soltar (multiplicar `Velocity.Z` por 0,5 si sube) con `bCortarSaltoAlSoltar`. Volver a validar `PruebasObstaculos`.

### GAMEPLAY-19 La cadena del combo no comprueba si se está en el aire o en el suelo y arrastra la escala de gravedad
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Combate/SAEVUSComponenteCombate.cpp:226-230,304-312`.
- Qué pasa: si el jugador aterriza a mitad del combo aéreo, el buffer encadena `TajoAereo2` en el suelo, con el gesto de piernas recogidas. `PrepararAtaque` solo cambia la gravedad si `IsFalling`, así que el 0,35 del primer tajo aéreo se mantiene hasta `TerminarAtaque`. Si el jugador sale de un borde durante ese ataque, cae flotando.
- Solución: si `bEnAire != bCadenaAerea`, reiniciar la cadena del medio actual (`IniciarAtaque(bEnAire, 0)`) y poner siempre la gravedad (1 en el suelo).

### GAMEPLAY-20 Sentado: la guardia y los golpes recibidos no levantan al jugador
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Personaje/SAEVUSPersonaje.cpp:148-161,501-514`; la animación lee `EstaSentado` (`Animacion/SAEVUSComponenteAnimacionProcedural.cpp:408`).
- Qué pasa: si un enemigo golpea al jugador sentado, este sale despedido con la pose de sentado (en el aire). Levantar la guardia entra en guardia sin levantarse. Curar estando sentado también funciona.
- Solución: `Levantarse()` en `ManejarCambioVida` (con Delta < 0), en `FijarGuardia(true)` y en `FijarCuracion(true)`.

### GAMEPLAY-21 Trampa en `EscalasVelocidadAccion`: una fuente nueva arranca a 0 y deja al jugador inmóvil
- Estado: CONFIRMADO (latente) · Prioridad: P3 · Severidad 4 si se dispara · Impacto 1 hoy · Coste S · Riesgo bajo
- Dónde: `Movimiento/SAEVUSComponenteMovimiento.h:259`; `Movimiento/SAEVUSTiposMovimiento.h:21-28`.
- Qué pasa: `float Escalas[Total] = { 1.f, 1.f, 1.f };` inicializa a cero los elementos que sobran. Si alguien añade `ESAEVUSFuenteEscalaVelocidad::Aturdido`, esa escala vale 0 y `GetMaxSpeed` en el suelo es 0. Además, `FijarEscalaVelocidadAccion` no comprueba el índice (pasar `Total` escribe fuera del array).
- Solución: rellenar el array con 1 en el constructor (`for` o `TStaticArray` más `Fill`) y añadir `check(Fuente < Total)`.

### GAMEPLAY-22 Recursos fáciles de farmear: los muñecos de práctica dan alma y munición sin fin
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Combate/SAEVUSComponenteCombate.cpp:24-27`; `Mundo/SAEVUSMunecoEntrenamiento.cpp:34,65-75`; `Armas/SAEVUSComponenteArmas.cpp:73-83`.
- Qué pasa: cualquier muñeco colocado fuera del tutorial (o en el foso de rebote) es una fuente infinita de alma y de munición de reserva.
- Solución: la etiqueta de práctica da el efecto visual de golpe pero solo alma hasta un tope (p. ej. 33) o nada fuera de `L_Tutorial`. Separar `AlGolpear` (feedback) de `AlGolpearRecurso`.

### GAMEPLAY-23 Avisos de tutorial: un solo hueco sin prioridad ni cola
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Tutorial/SAEVUSSubsistemaTutorial.cpp:6-14`; quien lo pisa: `Personaje/SAEVUSPersonaje.cpp:233-239`, `Mundo/SAEVUSBanco.cpp:110-114`, `Mundo/SAEVUSPalanca.cpp:86-92`, `Tutorial/SAEVUSPuertaArena.cpp:85-91`.
- Qué pasa: recoger un fragmento o sentarse borra a mitad de lectura la instrucción de tutorial activa (9 s). Los `bUsado` no se guardan, así que el tutorial se repite entero en cada carga.
- Solución: una cola con prioridad (Tutorial > Sistema > Notificación) y duración mínima. Registrar los avisos vistos en el guardado (como `EstadoMundo`) y ofrecer un menú «Recordar controles».

### GAMEPLAY-24 Atacar a la vez que se usa el doble toque y el bloqueo de cámara en el dash degradan el control con teclado y ratón
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Personaje/SAEVUSPersonaje.cpp:447-455,458-463`; `Personaje/SAEVUSConfigEntrada.h:49-58`; `Movimiento/SAEVUSAjustesMovimiento.h:161`.
- Qué pasa: los reajustes de posición con WW junto a un foso disparan dashes accidentales, y la opción solo está en el Data Asset, no en Opciones. Durante cada dash se descarta la entrada del ratón; con el dash machacado (GAMEPLAY-1) la cámara «se pega» la mayor parte del tiempo.
- Solución: opciones de jugador «Dash con doble toque» y «Bloquear cámara en el dash». En vez de descartar el giro de cámara, acumularlo y aplicarlo al terminar.

### GAMEPLAY-25 El mapa y la exploración se guardan cada 5 s y no al pausar (R2)
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 1 · Impacto 1 · Coste S · Riesgo bajo
- Dónde: `Mundo/SAEVUSExploracion.h:19`; `Mundo/SAEVUSExploracion.cpp:110-114`.
- Solución: 30 s, más al fijar el punto de control y al pausar (desde `AlternarPausa`). Combinar con la serialización de GAMEPLAY-7.

### GAMEPLAY-26 Partidas de v1 con fragmentos sin identificador: el mismo fragmento puede contar dos veces
- Estado: INFERIDO (depende de si había fragmentos sin `Identificador` en los mapas: DESCONOCIDO) · Prioridad: P3 · Severidad 2 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Mundo/SAEVUSFragmentoMascara.cpp:39-44,55`; `Nucleo/SAEVUSGuardado.h:100-101`.
- Qué pasa: en v1, recoger un fragmento sin identificador guardaba `None`. En v2 su identificador pasa a ser `Mapa.Actor`, así que el fragmento reaparece, se recoge otra vez y `ObtenerNumeroFragmentos` cuenta `None` y el nuevo: máscara regalada.
- Solución: en la migración (versión de GAMEPLAY-7), descartar `None` de `FragmentosMascara`.

### GAMEPLAY-27 Plataforma móvil (y titileo del brasero) con tiempo `float` que crece sin límite
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 2 (sesiones de más de 24 h) · Coste S · Riesgo bajo
- Dónde: `Mundo/SAEVUSPlataformaMovil.h:55`, `.cpp:61-66`; `Mundo/SAEVUSFuegoBrasero.cpp:65,76-77`.
- Qué pasa: hacia los 131 000 s (unas 36 h) la precisión de `float` es de 0,0156 s, aproximadamente un fotograma: la plataforma avanza a saltos y el jugador encima tiembla. El titileo del brasero se vuelve escalonado.
- Solución: `Tiempo = FMath::Fmod(Tiempo + Dt, Ciclo)` en la plataforma y envolver el tiempo del brasero.

### GAMEPLAY-28 Los golpes contra el escenario (puerta, palanca ya accionada) tienen parada de golpe pero ni sonido ni chispa
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 1 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Combate/SAEVUSComponenteCombate.cpp:438-449` (`AlImpactar` sin `AlGolpear`); `Mundo/SAEVUSPalanca.cpp:69-74`; la puerta no tiene `TakeDamage`.
- Qué pasa: el golpe «muerto» contra una puerta se nota en la mano pero suena a nada. Es inmersión táctil a medias.
- Solución: un evento `AlImpactarEscenario(Superficie)` que el sonido y los efectos usen para la chispa o el sonido metálico, con material por etiqueta.

### GAMEPLAY-29 Código y comentarios muertos de mecánicas eliminadas (pogo, tajo hacia abajo, rebote)
- Estado: CONFIRMADO · Prioridad: P4 · Severidad 1 · Impacto 1 · Coste S · Riesgo bajo
- Dónde:
  - `Tutorial/SAEVUSPeligro.h:11-15` y `Tutorial/SAEVUSPeligro.cpp:20-21,30-34`: «el tajo hacia abajo los golpea»; la caja ignora `TrazaArma`, así que nunca se golpea.
  - `Combate/SAEVUSComponenteCombate.h:38` («abajo en el suelo es frontal») y `:137` («rebote»).
  - `Pruebas/SAEVUSPruebasObstaculos.cpp:33-47`: constantes de rebote sin uso.
- Solución: decidir entre implementar el pogo (GAMEPLAY-8 y 12) o borrar el código y alinear los comentarios.

### GAMEPLAY-30 La puerta de arena no reacciona a enemigos descargados por streaming
- Estado: CONFIRMADO (código) · Prioridad: P4 · Severidad 3 si ocurre · Impacto 1 hoy (no hay streaming) · Coste S · Riesgo bajo
- Dónde: `Enemigos/SAEVUSEnemigo.cpp:352-358` (solo con `EEndPlayReason::Destroyed`).
- Solución: emitir también con `RemovedFromWorld`, o que la puerta compruebe en un temporizador de 1 s mientras siga cerrada.

### GAMEPLAY-31 Detalles menores
- Estado: CONFIRMADO · Prioridad: P4 · Severidad 1 · Impacto 1 · Coste S · Riesgo bajo
- Dónde y qué:
  - `Nucleo/SAEVUSControladorJugador.cpp:247`: `GuardarSensibilidad(Nueva)` guarda el valor sin acotar. Debería pasar `Sensibilidad`.
  - `Personaje/SAEVUSPersonaje.cpp:224`: sin subsistema de guardado (mundos de prueba), `++FragmentosMascara` ya no evita duplicados.
  - `Mundo/SAEVUSFragmentoMascara.cpp:84`: usa el sonido `Banco` para recoger el fragmento, cuando necesita uno propio que dé sensación de recompensa.
  - En PIE se escribe en la ranura real `SAEVUS` (`Nucleo/SAEVUSGuardado.cpp:13`): con `WITH_EDITOR` debería usarse una ranura distinta.

## 5. Inmersión (elementos del área que la rompen)

- **CRITICAL:** el dash machacado con invulnerabilidad y el planeo infinito (GAMEPLAY-1). El mundo deja de pesar: ni los fosos ni los enemigos suponen una amenaza.
- **HIGH:**
  - El jefe con la vida a medias tras morir, y curarse reiniciando desde la pausa (GAMEPLAY-6).
  - Puertas y jefes que «resucitan» al continuar (GAMEPLAY-2).
  - Caer al vacío varios minutos sin muerte (GAMEPLAY-3).
- **HIGH:** pulsaciones ignoradas (GAMEPLAY-5): se rompe la sensación de presencia y control.
- **MEDIUM:**
  - Parry sin respuesta propia (GAMEPLAY-9).
  - Tajo hacia arriba involuntario (GAMEPLAY-10).
  - Curación y pérdida de alma al cruzar de mapa, con corte seco (GAMEPLAY-11).
  - Reaparecer escalado (GAMEPLAY-13).
  - Pose de sentado en el aire (GAMEPLAY-20).
  - Avisos que se pisan y tutorial que se repite (GAMEPLAY-23).
- **LOW:**
  - Golpe «mudo» contra puertas (GAMEPLAY-28).
  - Plataforma que tiembla en sesiones largas (GAMEPLAY-27).
  - Sonido de banco al recoger un fragmento (GAMEPLAY-31).

## 6. Escala ×10

- **Persistencia:** sin un sistema genérico de estado del mundo (GAMEPLAY-2), 10× mapas significa 10× parches a mano y bloqueos combinatorios. Es lo primero que se rompe.
- **Guardado:** se reescribe la partida entera (todo `Exploracion` de todos los mapas) en cada cambio y cada 5 s. Con 10× mapas son unos cientos de KB por escritura, con escrituras concurrentes (GAMEPLAY-7). Hace falta serializar y agrupar las escrituras, y quizá trocear por mapa.
- **Avisos y notificaciones:** con un solo hueco, a ×10 de contenido (objetos, códice, rutas del concepto) se pisan constantemente.
- **Puertas:** `ASAEVUSEnemigo::AlMorirEnemigo` es global (M2). Cada puerta recibe cada muerte. Con 10× puertas sigue siendo barato, pero acopla todo el mundo a un estático. A largo plazo conviene un «encuentro» que posea a sus enemigos.
- **Diseño de nivel:** cada sala nueva tiene que diseñarse contra el planeo con dash. Si no se corrige GAMEPLAY-1, el coste de nivel crece de forma lineal o peor.
- **Equilibrio:** las tablas automáticas escalan bien a 10× enemigos. Les faltan las reglas de dominancia (GAMEPLAY-14).
- **Rendimiento:** la carrera por pared y la escalada solo sondean el personaje del jugador (los enemigos usan el CMC estándar). La detección de golpes es un `Overlap` por fotograma activo. Ambas escalan bien.

## 7. Oportunidades (mayor impacto)

1. Reajustar el dash (GAMEPLAY-1): invulnerabilidad corta al inicio, un dash aéreo recargable con la pared y un tajo en carrera con compromiso real. Es lo que más mejora el combate y el diseño de nivel a la vez.
2. Buffer de acciones de cancelación (GAMEPLAY-5): unas 40 líneas, mejora inmediata de respuesta.
3. Sistema `EstadoMundo` persistente más reinicio de jefes y repoblado al descansar (GAMEPLAY-2 y 6): convierte el mapa en metroidvania y la muerte en aprendizaje.
4. Defensa con coste: postura o daño gris, más parry con castigo y respuesta de primer nivel (GAMEPLAY-4 y 9). Da el «momento Sekiro» propio del concepto.
5. Ataque hacia abajo o pogo y abanico aéreo bajo (GAMEPLAY-8 y 12): verticalidad con identidad Hollow Knight, sin complejidad artificial.
6. Progresión por habilidades: bloquear al principio dash aéreo, carrera por pared, escalada y doble salto (ya son booleanos o enteros en los datos) y desbloquearlos con ítems persistentes. Así la exploración tiene motivo, como en el «Has desbloqueado una ruta» del concepto.
7. Tutorial «haciendo»: fosos que exigen cada mecánica, avisos condicionados a la acción y una cola de avisos (GAMEPLAY-8 y 23).
8. Guardado robusto: versión, copia de seguridad y escritura única (GAMEPLAY-7).
9. Arrastrar el estado del jugador entre mapas, con una transición en fundido (GAMEPLAY-11).
10. Salto variable y opciones de control (GAMEPLAY-18 y 24): precisión y accesibilidad.

## 8. Contraste con los conceptos

- **Hoja de dirección de arte (UI y notificaciones):** pide «Objeto obtenido», «Nueva entrada en el códice», «Punto de control activado», «Has desbloqueado una ruta» y una pantalla de «Victoria» con recompensas. No existen inventario, objetos, códice, rutas persistentes ni estado de «jefe derrotado».
  - Cambio técnico: `EstadoMundo` y persistencia (GAMEPLAY-2), una cola de notificaciones con tipos (GAMEPLAY-23) y un evento `AlDerrotarJefe` con tabla de recompensas en datos.
- **«Has muerto → Reintentar / Volver al menú»:** hoy hay reaparición automática a los 3 s, sin elección. Hay que exponer `NotificarMuerteJugador` a la interfaz y dejar que sea la pantalla quien pida `ReaparecerJugador`.
- **Parry y bloqueo (sección H) e impactos «crítico» y «aturdido» (sección F):**
  - El parry no tiene respuesta diferenciada (GAMEPLAY-9).
  - No existe ningún concepto de crítico. Haría falta un multiplicador por ventana, por ejemplo contra un enemigo aturdido o por la espalda, en `FSAEVUSDefinicionAtaque` y en la confirmación del golpe.
- **Hoja de poses** (dash adelante, atrás y lateral; carrera por pared; salto de pared; aterrizaje fuerte; ataque durante el dash; aéreo):
  - Todo está cubierto por el sistema. Las ocho direcciones de dash están implementadas; las variantes atrás y lateral dependen de la animación procedural, DESCONOCIDO.
  - Falta un ataque aéreo hacia abajo. No está en el concepto, pero lo pide el diseño (GAMEPLAY-12).
- **Hoja de dirección de arte, dash (sección D):** muestra «Doble dash», lo que sugiere un presupuesto de dashes visible (el HUD tiene indicadores tipo «3/12»). Encaja con limitar los dashes aéreos (GAMEPLAY-1) y mostrar las cargas en el HUD.


---

# Auditoría AAA SAEVUS v2 — Animación, VFX y Cámara (`animacion_vfx`)

Rutas relativas a `Source/SAEVUS/`. Líneas de la v2. Leídos enteros: `Animacion/*` (8 archivos), `Efectos/*` (14), `Camara/*` (3),
`Pruebas/SAEVUSPruebasAnimacion.cpp`, `SAEVUSPruebasEstilo.cpp`, `SAEVUSPruebasPolvo.cpp`, `SAEVUSPruebasDash.cpp` (solo índice: son de movimiento),
y la parte de animación/efectos de `SAEVUSPruebasRevision.cpp` (221–450). Contexto leído de Combate, IA/Enemigo, Personaje, ParadaGolpe y las 8 imágenes.
Simulación numérica de `TSAEVUSMuelle` (réplica exacta en Python del integrador) en `simulación del integrador (Python)`.

---

## 1. Resumen del área

- **Estado**: la v2 sí ataca la queja del dueño en lo conceptual. Hay capa secundaria seria (giro por capas cadera→pecho→cabeza, inercia por ΔV independiente de fps, rebote, respiración con cansancio, vaivén de peso, gestos de reposo, desfase por actor), 9 gestos de ataque con claves propias, amago, seguimiento y encadenado continuo (con prueba), dash en tres tiempos (arranque/vuelo/frenada) con ecos de silueta, capa con 3 muelles en cadena y enemigos con esqueleto y 7 gestos. Es, con diferencia, la mejor versión del sistema.
- **Fortalezas que conservar**: `FSAEVUSCapaSecundaria` como módulo puro y testeable; los gestos con continuidad entre golpes del combo; la estela de cinta en espacio de mundo con subdivisión angular; el hit-stop por `CustomTimeDilation` con la cámara fuera de él; R1 bien resuelto.
- **Debilidades**: (a) el integrador de muelles **no es estable** con los fotogramas largos ni con el ×9 de frecuencia de cadera en «aire + acción», y produce **NaN permanentes** (prota invisible hasta morir, enemigos rotos); (b) no hay contacto con el suelo (ni IK, ni ciclo ajustado a la zancada), así que los pies patinan; (c) los enemigos dan el golpe **antes** de que se vea el barrido; (d) el lenguaje de VFX es un confeti de cubos opacos con una sola malla y un solo material para todo, y el arco del tajo es siempre horizontal aunque el gesto sea vertical o una estocada; (e) el esqueleto está cableado (enum de 25, máscara de ≤32 huesos), así que el concepto (digitígrado, cresta, tiras, guadaña a dos manos) no cabe.
- **Mayor riesgo**: ANIMVFX-1 (NaN). Se dispara jugando normal en el hardware objetivo, porque el código ya habla de una Vega 8. Arreglarlo es S.
- **Compilación**: no veo errores probables en el código nuevo del área. Los inicializadores designados de `SAEVUSGestosAtaque.cpp:73–199` están en el orden de declaración de `FClave` (comprobado clave a clave; necesitan C++20, que es lo que usa UE 5.8 por defecto). No hay sombras de variables. Las conversiones double→float son explícitas.

## 2. Puntuaciones sugeridas (0–100)

| Categoría | Nota | Justificación |
|---|---|---|
| Animación del protagonista (expresividad) | 55 | Capas y gestos con intención; sin contacto con el suelo, sin giro axial controlado, transiciones todas iguales (filtro exponencial). |
| Distinción de golpes (cada golpe su animación) | 70 | 9 gestos realmente distintos, testados y con encadenado; fallan el punto de contacto (velocidad 0 a mitad del corte) y que la reacción del que recibe sea igual para todos. |
| Movimiento secundario / peso | 62 | Bien diseñado; la respiración y el ondeo son mecánicos (senos fijos) y la capa es de una sola tira. |
| Robustez numérica | 25 | NaN permanente con dt > 66 ms en aire+acción y con enemigos fuera de pantalla; inestabilidad marginal a 60/30 fps. |
| Animación de enemigos y jefe | 38 | Dos poses por gesto, golpe desincronizado del daño, el mismo set para goblin y Rey, sin agarre a dos manos, muerte encogiéndose. |
| Transiciones / integración entre acciones | 50 | Sin «pops», pero todo es el mismo low-pass; patinaje en ataques con desplazamiento; sin poses de carga, bloqueo ni parry. |
| Contacto con el suelo / IK | 10 | No existe. |
| Escalabilidad del pipeline de animación | 25 | Claves en C++, esqueleto en un enum, ≤32 huesos, todo en el hilo de juego. |
| VFX: intención y legibilidad | 40 | La cinta de la hoja y los ecos están bien; el arco horizontal, los cubos y un material único para todo restan; faltan sangre, parry e impactos por tipo. |
| VFX: coste y tecnología | 50 | Barato, pero recorre y sube el pool entero cada frame; las motas lo mantienen siempre activo; desborda en el giratorio. |
| Cámara: sensación | 55 | Modelo de trauma correcto y hit-stop sin congelar la cámara; golpe dado casi imperceptible, sin reacciones a aterrizaje, carga ni jefe. |
| Cámara: confort y accesibilidad | 30 | Sin opciones de sacudida, FOV ni parpadeo; el FOV bombea con cada dash; roll en la sacudida. |
| Pruebas del área | 50 | Muchas y con intención; todas a paso fijo de 1/60, sin dt variable, NaN ni sincronía golpe–pose. |

## 3. Estado de la revisión anterior (IDs del área)

| ID | Estado | Evidencia v2 |
|---|---|---|
| R1 | **RESUELTO** | `FName` creados una vez (`Animacion/SAEVUSComponenteAnimacionProcedural.cpp:101–113`); escritura directa en `BoneSpaceTransforms` sin buscar nombres (`:1341–1362`); enemigos sin renderizar a 4 Hz en `TickComponent`, nunca en `Actualizar` (`:343–355`, `Ajuste.IntervaloFueraDePantalla` `.h:369–370`). Queda pendiente: no hay LOD por distancia para enemigos visibles y lejanos (ver ANIMVFX-17). |
| R7 | **PARCIAL** | Sale pronto si no hay nada vivo (`Efectos/SAEVUSComponenteParticulas.cpp:90–93`), pero en cuanto hay una sola partícula recorre el pool entero y sube todas las instancias (`:96–118`, `:122–144`, `BatchUpdateInstancesTransforms` con `bMarkRenderStateDirty`). Las motas mantienen su pool siempre vivo (`SAEVUSComponenteMotasAmbiente.cpp:34–46`). La estela rehace la sección cada vez que cambia el número de tramos (`SAEVUSComponenteEstelaEspada.cpp:114–120`), es decir, casi todos los fotogramas en que está activa. |
| R8 (animación) | **RESUELTO** | `PoseMirada` busca cada `IntervaloBusquedaMirada` = 0,2 s (`.cpp:1141–1156`; `.h:372–374`). |
| D3 | **RESUELTO** | Animación (`.cpp:231, 1198–1206`) y polvo (`SAEVUSComponentePolvoMovimiento.cpp:51, 65`) usan `AlAterrizar`. Los enemigos detectan su aterrizaje aparte (`SAEVUSAnimacionEnemigo.cpp:150–155`), lo cual está justificado porque no usan el movimiento propio. |
| D4 | **RESUELTO** | `ConsumirEmision` único (`SAEVUSComponenteParticulas.h:79–85`), usado en `EstelaDash.cpp:60`, `PolvoMovimiento.cpp:95` y `EfectosCombate.cpp:144, 160`. |
| M1 | **RESUELTO** | Hermanos con `UPROPERTY(Transient) TObjectPtr` (`.h:515–535`); `ObjetivoMirada` como `TWeakObjectPtr` (`.h:578`). Fuera del área: `AtaqueActual` ahora es una copia (`Combate/SAEVUSComponenteCombate.h:76, 146`). |

## 4. Hallazgos

### ANIMVFX-1 Muelles explotan a NaN: protagonista invisible hasta morir y enemigos rotos
- Estado: CONFIRMADO (análisis del integrador y simulación) · Prioridad: **P0** · Severidad 5 · Impacto jugador 5 · Coste S · Riesgo regresión bajo
- Dónde: `Animacion/SAEVUSMuelle.h:39–46, 61–62`; `Animacion/SAEVUSCapaSecundaria.cpp:84, 92–102`; `Animacion/SAEVUSComponenteAnimacionProcedural.cpp:343–355, 1306`
- Qué pasa y por qué:
  - El muelle usa Euler semiimplícito con subpasos de 1/240 s, pero **como mucho 16**. Con dt > 66,7 ms el subpaso crece (dt/16).
  - La frecuencia de la cadera se multiplica: `FrecuenciaCadera` 2,2 Hz × (1+1,2·avance) × 3 (acción) × 3 (en el aire) = **43,6 Hz** (`CapaSecundaria.cpp:92–101`).
  - Ese esquema solo es estable si (ωh)² + 4ζωh < 4. Con ω = 274 rad/s:
    - Con h = 1/240 da 4,04: ya es inestable (|λ| = 1,026 por subpaso, ×1,11 por fotograma a 60, 30, 20 o 15 fps exactos).
    - Con un tirón de 0,1 s da λ ≈ −14 por subpaso, es decir, **10¹⁸ en un solo fotograma**.
  - Simulación (`simulación del integrador (Python)`):
    - aire+acción con tirón de 0,1 s → 2·10²⁵ en 3 fotogramas;
    - acción en suelo con 0,25 s → 7·10²⁰;
    - enemigo fuera de pantalla (dt = 0,25 s por la limitación a 4 Hz, `.cpp:348`), en acción y moviéndose → 1,8·10⁴¹, que en float es inf;
    - aire+acción a 60 fps exactos → la desviación crece de 5 a 62 en medio segundo en vez de asentarse.
  - Cuando `Velocidad` llega a inf o NaN, `Clamp` de `:84` devuelve el máximo, pero la velocidad sigue siendo NaN, así que `Valor` vuelve a ser NaN en cada fotograma. Luego `GirarTramo(Pelvis)` mete NaN en todo el cuerpo y `Slerp(RotacionActual, …)` (`:1306`) ya no se recupera nunca.
  - Solo `PrepararEsqueleto` reinicia el estado: el jugador sigue así hasta morir y reaparecer (`Nucleo/SAEVUSModoJuego.cpp:126–139`), y el enemigo hasta que desaparece (también su muerte).
- Secuencias concretas:
  1. Jugador: salto → dash aéreo (`bGirarHaciaDash` gira el actor de golpe y deja `RetrasoCadera` en ±55°) o tajo aéreo, y en ese momento un tirón ≥ 67 ms (compilación de shader, GC, streaming, alt-tab; UE deja pasar hasta 0,4 s). El caballero desaparece o se deforma, y la estela y las partículas de la hoja (`ObtenerHoja`) salen en NaN.
  2. Goblin con `Embestida` (1000 cm/s) o `Salto` que ataca desde detrás de la cámara (fuera de pantalla). A 4 Hz, una o dos actualizaciones bastan para que al entrar en pantalla esté roto para siempre.
  3. Sin tirones, a 60 Hz con VSync: en las cadenas aéreas, que pueden durar más de 1 s gracias a `EscalaGravedadEnAire` 0,35, la cadera cabecea cada vez más en lugar de asentarse.
- Por qué importa: el protagonista desaparece en mitad del combate. Es una regresión nueva de la v2 (los muelles no existían en la v1), y el comentario del archivo («estable con fotogramas largos», `SAEVUSMuelle.h:20`) es falso.
- Solución propuesta (incremental):
  1. Que el número de subpasos dependa de ω: `Pasos = ceil(dt·ω/0.5)` con un tope alto (64), o mejor un integrador incondicionalmente estable (la solución analítica del muelle amortiguado para un objetivo constante, o Euler implícito con amortiguación implícita).
  2. Limitar el dt de entrada en `Actualizar` (p. ej. `Min(dt, 1/20)`) y repartir la acumulación de fuera de pantalla en trozos de 1/30.
  3. Limitar el producto de frecuencias de la cadera (≤ 15 Hz).
  4. Guarda en `Avanzar`: si `!FMath::IsFinite`, fijar al objetivo. Guarda en `SuavizarYAplicar`: si `RotacionActual[i].ContainsNaN()`, volver al reposo.
- Validación: prueba automática que recorre todos los muelles reales (cadera en sus 4 combinaciones, inercia, rebote, capa, mirada, retroceso) con dt ∈ {1/240, 1/144, 1/60, 1/30, 1/15, 0,1, 0,25, 0,4} y peso ∈ {0,3; 1; 4}, sembrando 55°/1000°·s⁻¹, y comprueba que la magnitud decae y es finita. Prueba de integración: el personaje en el aire con un tajo y un `Actualizar(0.3f)`, que ningún hueso tenga NaN.

### ANIMVFX-2 El golpe del enemigo hace daño antes de verse el barrido
- Estado: CONFIRMADO · Prioridad: **P1** · Severidad 4 · Impacto jugador 5 · Coste S · Riesgo bajo
- Dónde: `Enemigos/SAEVUSComponenteIAEnemigo.cpp:411–416` (daño en el primer tick de `Atacar`), `:344` (proyectil al empezar); `Animacion/SAEVUSAnimacionEnemigo.cpp:215–229` (barrido de `Aviso` a `Golpe` en `Duracion·0,5` = 0,225 s con salida rápida), más el suavizado de 31–70 ms (`.cpp:1290–1299`).
- Qué pasa: al acabar el aviso, `ActualizarAtacar` aplica el daño de inmediato (`bGolpeAplicado`), cuando la pose sigue en `Aviso` (U = 0) y el filtro la retrasa todavía más. El jugador recibe el golpe con el arma del goblin aún levantada. El proyectil sale con el brazo atrás.
- Por qué importa: en un combate tipo Hollow Knight/Souls la lectura es «lo que veo me golpea». Esto se percibe como injusto y desmiente el aviso. El parry, sincronizado con el aviso visual, se vuelve una apuesta.
- Solución: añadir `TiempoImpacto` a `FSAEVUSAtaqueEnemigo` (por defecto ≈ 0,3·`Duracion`, que es donde `SalidaRapida` ya pasa del 85 %) y aplicar el daño o lanzar el proyectil al cruzarlo, con la misma protección contra fotogramas largos que E10. Alternativa barata: que el barrido llegue a `Golpe` en ≤ 2 fotogramas (`U` con duración 0,04 s) y saltarse el suavizado en ese tramo.
- Validación: prueba que avanza un goblin con un `Tajo` y comprueba que el daño ocurre cuando el ángulo de la mano supera el 80 % del recorrido entre `Aviso` y `Golpe`.

### ANIMVFX-3 Sin contacto con el suelo: pies que patinan, se hunden o flotan
- Estado: CONFIRMADO (estructural); magnitud INFERIDA (depende de la malla) · Prioridad: **P1** · Severidad 3 · Impacto jugador 4 · Coste M · Riesgo medio
- Dónde: `.cpp:476` (fase por distancia con `LongitudCiclo` fijo de 190 cm), `:498` (amplitud de la zancada proporcional a la velocidad); bajadas de cadera fijas en `:547, 561, 571` (carrera −7, aterrizaje −10/−18), `:806` (ataque hasta −16), `:856` (curación −14), `:902` (guardia −6), `:832` (muerte); la única compensación analítica está en el rebote (`:1014–1030`).
- Qué pasa:
  - La fase del paso avanza a 190 cm por ciclo, pero el ángulo del muslo escala con la velocidad. A 100 cm/s (stick a medias) el muslo abre unos 7° (paso de unos 20 cm con una pierna de unos 85 cm) mientras el ciclo pide 95 cm por paso: los pies se deslizan como en una «moonwalk».
  - En ataques con desplazamiento (`VelocidadEstocada` 450–1100 cm/s, `EscalaVelocidadSuelo` 0,25), la pose de piernas es fija mientras la cápsula avanza 50–175 cm: patina.
  - Ninguna bajada de cadera, salvo el rebote, recalcula las rodillas: los pies atraviesan el suelo o quedan en el aire. Tampoco se hacen trazas al suelo: escaleras y pendientes sin apoyo.
- Por qué importa: es la causa número 1 de «se ve flotante y barato» en tercera persona. El concepto (aterrizaje fuerte con la mano en el suelo, zancadas de ataque) exige apoyos firmes.
- Solución:
  1. Inmediato (S): longitud de zancada en función de la velocidad (`LongitudCiclo = f(v)`, calibrado con la pierna real de `ReposoComponente`).
  2. M: IK analítica de dos huesos por pierna sobre el propio FK (ya hay ley de cosenos en `:1018`), con objetivo = traza al suelo, bloqueo del pie en la fase de apoyo y pelvis bajada al pie más bajo.
  3. En ataques: plantar el pie delantero al final de la estocada.
- Validación: prueba que corre a 100/300/550/850 cm/s y mide el deslizamiento del pie en apoyo (< 3 cm por paso); prueba de rampa a 30° sin penetración.

### ANIMVFX-4 El arco del tajo es siempre horizontal y se suman tres capas: ruido y contradicción
- Estado: CONFIRMADO · Prioridad: **P1** · Severidad 3 · Impacto jugador 4 · Coste S · Riesgo bajo
- Dónde: `Efectos/SAEVUSComponenteEfectosCombate.cpp:101–131` (arco sobre el abanico horizontal de la hitbox), `:85–99` (5 partículas por fotograma en la hoja), `SAEVUSComponenteEstelaEspada.cpp:38–82` (cinta); `Combate/SAEVUSComponenteCombate.cpp:351–364`.
- Qué pasa:
  - El arco recorre el abanico horizontal de la hitbox para **todos** los gestos que tienen abanico: el remate (hoja de arriba abajo), la estocada (recta), el remate aéreo (volcado vertical) y el giratorio.
  - A la vez se dibujan la cinta, que sigue la hoja de verdad, y las partículas sobre la hoja.
  - Cuentas: `Pasos = ceil(|Δ°|·0,45)` × 5 partículas da unas 340 por tajo de 150° y unas **810 en el giratorio** (360°), más de las 512 del pool. El búfer circular (`Particulas.cpp:56–71`) se come el principio del propio arco mientras se dibuja, y un combo de 3 golpes hace saltar partículas a mitad de vida.
- Por qué importa: la promesa «lo que se ve es lo que golpea» se rompe justo en los golpes más vistosos (un remate vertical con un abanico horizontal). La legibilidad se ensucia y el concepto pide una única media luna limpia (hoja de arte, apartado E).
- Solución: que la cinta sea la única firma visual del corte. Desactivar el arco por defecto (`ParticulasArcoPorPaso` = 0) o, si se quiere enseñar la hitbox, solo en un modo de depuración/accesibilidad y con 1 partícula por paso. Quitar `EmitirEstelaArma`, o reducirla a chispas que salen de la punta al contactar.
- Validación: prueba de que ningún ataque del repertorio por defecto emite más de N partículas; una captura por gesto en la sala de pruebas.

### ANIMVFX-5 Un solo emisor, una malla y un material para todo el combate; impacto sin punto de contacto
- Estado: CONFIRMADO (código); DESCONOCIDO si algún Blueprint asigna otra malla o material · Prioridad: **P1** · Severidad 3 · Impacto jugador 4 · Coste M · Riesgo bajo
- Dónde: `Efectos/SAEVUSComponenteEfectosCombate.cpp:69–78, 80–84, 85–174` (golpe, curación, bloqueo, carga, arco y hoja en el mismo `UInstancedStaticMeshComponent`); `SAEVUSComponenteParticulas.cpp:11–33` (cubo del motor como malla por defecto, sin material propio); `:77` (chispas en `GetActorLocation()` del golpeado).
- Qué pasa:
  - Chispas de impacto, alma que converge, destello de curación, chispas de bloqueo y carga del ataque comparten tamaño (9 cm), forma (cubo opaco) y material.
  - El impacto nace en el centro de la cápsula del enemigo y sale «alejándose del atacante», no en la dirección del tajo.
  - No hay sangre (apartado G del concepto), ni destello de parry (H), ni impacto normal/fuerte/crítico diferenciado (F), ni pulso de aturdido.
- Por qué importa: el jugador no distingue «he curado» de «me han bloqueado» ni de «he golpeado» con la vista periférica. El peso del golpe se pierde.
- Solución (incremental):
  1. Un emisor por familia (golpe, alma, guardia), cada uno con su malla (`SM_VFX_Chispa` ya está previsto en `Particulas.h:45`) y material aditivo.
  2. Punto de contacto = el punto más cercano del segmento de la hoja (`ObtenerHoja`) al golpeado, con la dirección del barrido.
  3. Familias nuevas: «sangre» (salpicadura direccional más calcomanía) y «parry» (estallido azul con un fotograma blanco).
  4. Intensidad según el daño del ataque (`Danio`, `DuracionParadaGolpe`).
- Validación: prueba de que cada evento usa su emisor y de que el punto de impacto está a menos de X cm de la hoja.

### ANIMVFX-6 Esqueleto cableado: ≤ 32 huesos, enum fijo y el mismo esqueleto obligatorio para todas las especies
- Estado: CONFIRMADO · Prioridad: **P1** (producción) · Severidad 4 · Impacto jugador 3 · Coste L · Riesgo medio
- Dónde: `Animacion/SAEVUSComponenteAnimacionProcedural.h:25–34` (enum de 25), `:542` (`uint32 Subarbol`), `.cpp:27` (`static_assert(NumHuesos <= 32)`), `:302–312` (si falta un solo hueso, sin animación), `:59–98` (tabla de nombres y ejes); `SAEVUSAnimacionEnemigo.cpp` (enemigos sobre el mismo enum).
- Qué pasa: añadir un hueso exige tocar el enum, la tabla y la lógica, y el tope absoluto son 7 huesos más. Un goblin o el Rey deben tener `capa_1..3` y `arma_fuego`; si no, se quedan en la pose de referencia con un aviso en el log. No hay correspondencia de huesos por especie.
- Por qué importa: el concepto necesita del orden de 40–90 huesos (ver §8). Ni el jefe con guadaña ni los goblins con dos armas caben.
- Solución: corto plazo, `TBitArray` o `uint64` y una tabla de «roles» por especie (data asset que asocia rol → nombre de hueso, con roles opcionales). Medio plazo: modelo híbrido (§8), donde los huesos extra (tiras, orejas, dedos) los mueven nodos genéricos (cadenas físicas o IK) sin tocar código.
- Validación: cargar un esqueleto sin `arma_fuego` ni capa y comprobar que anima.

### ANIMVFX-7 Enemigos y jefe: dos poses por gesto, el mismo set para todos y sin agarre a dos manos
- Estado: CONFIRMADO · Prioridad: **P1** · Severidad 3 · Impacto jugador 4 · Coste M–L · Riesgo bajo
- Dónde: `Animacion/SAEVUSAnimacionEnemigo.cpp:78–120` (7 gestos = par `Aviso`/`Golpe`), `:215–256` (mezcla lineal Aviso→Golpe, mantener, Golpe→Lista); `Enemigos/SAEVUSTiposEnemigo.h:293` (solo `PesoAnimacion` distingue al Rey).
- Qué pasa: cada ataque enemigo es una interpolación entre dos poses, sin seguimiento que se pase del final, sin peso en la recuperación y sin claves intermedias. El Rey Goblin usa los mismos 7 gestos que un goblin, solo más lentos. La guadaña o alabarda del concepto (dos manos, arcos largos, golpes al suelo) no tiene representación: la mano libre nunca se ancla al mango.
- Por qué importa: el jefe es el clímax de inmersión. «El mismo golpe pero más lento» no da presencia.
- Solución: llevar el sistema de claves de `SAEVUSGestosAtaque` a los enemigos (la misma `FClave`, 4–6 claves). Añadir un rol `mano_apoyo` con IK de dos huesos hacia un hueso o socket del arma. Repertorio propio del jefe por fase (data asset).
- Validación: prueba al estilo `GestosDistintos` para enemigos; prueba de que la mano de apoyo queda a menos de 5 cm del mango.

### ANIMVFX-8 Las claves de los gestos viven en C++: iterar exige programador y recompilar
- Estado: CONFIRMADO · Prioridad: **P2** · Severidad 3 · Impacto jugador 2 (indirecto) · Coste M · Riesgo bajo
- Dónde: `Animacion/SAEVUSGestosAtaque.cpp:66–202`; `Combate/SAEVUSTiposCombate.h:34–55` (el enum es lo único que se puede tocar desde datos).
- Qué pasa: hay unas 50 claves con vectores escritos a mano en ejes del cuerpo. No hay vista previa, ni curvas editables, ni eventos (contacto, pasos o sonidos ligados a la pose). Además las fases se normalizan (0–1–2–3, `:268–281`): si el diseñador cambia `TiempoPreparacion`, el amago se estira o se encoge con él, sin un límite que lo controle.
- Por qué importa: el ajuste fino de la animación (lo que pide el dueño) depende del cuello de botella de un programador. Con 10× contenido es inviable.
- Solución: un `UDataAsset` con `FClave` como `USTRUCT` (o `UCurveVector` por canal) y gestos asignados por referencia en lugar de por enum, más una herramienta de vista previa en el editor (componente que evalúa `Evaluar` en un actor de edición). Es un paso intermedio, válido también en el modelo híbrido.

### ANIMVFX-9 El corte frena a cero justo en el contacto y el filtro le quita el «chasquido»
- Estado: CONFIRMADO (matemática de curvas) · Prioridad: P2 · Severidad 2 · Impacto jugador 3 · Coste S · Riesgo bajo
- Dónde: `SAEVUSGestosAtaque.cpp:301` (`SalidaRapida` por tramo, derivada 3 → 0), claves en 1,0 → 1,5 → 2,0 de todos los tajos; `.cpp:1290–1306` (low-pass de 32/s sobre la hoja también en la fase activa).
- Qué pasa: cada tramo «Rápida» sale a 3× la velocidad media y llega a 0. En la clave 1,5, que es la hoja al frente, el instante de contacto, la velocidad cae a 0 y vuelve a 3×: el corte se parte en dos golpes. Encima, el suavizado exponencial (τ ≈ 31 ms) retrasa y aplana el pico.
- Solución: interpolar la fase activa con una spline continua en velocidad (Catmull-Rom esférica o Hermite con tangentes entre claves), o un único tramo con un punto de paso; y alfa = 1 (sin filtro) para brazo, mano y arma durante la fase activa.
- Validación: prueba que mide que la velocidad angular de la hoja es máxima cerca de la fase 1,5.

### ANIMVFX-10 Reacciones: el retroceso del arma siempre hacia arriba y el golpe recibido igual para todo
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto jugador 3 · Coste S · Riesgo bajo
- Dónde: `.cpp:757–761` (`Girar(Adelante, Arriba, RetrocesoImpacto)`), `:1232–1236`; `:1092–1112` (`ImpulsoGolpeRecibido` constante, empuje desde la posición del instigador); `:1114–1130` (solo brazo libre y cabeza).
- Qué pasa: en un tajo horizontal la hoja rebota hacia arriba en vez de contra el barrido. Un tajo de 10 y un remate de 25 producen la misma reacción en el golpeado, siempre «hacia atrás desde el atacante» y no en la dirección del corte. No hay tambaleo, ni derribo, ni variante pesada, ni reacción del jugador al bloquear.
- Por qué importa: «cada golpe su animación» también va del que lo recibe. Es lo que da peso al golpe.
- Solución: añadir `Delta`/`Danio` y la dirección del barrido al evento (ya existe `SentidoBarrido`): impulso = f(daño) sobre el eje del tajo; retroceso del arma contra la velocidad de la hoja; umbrales para tambaleo y derribo (poses o clips).

### ANIMVFX-11 Sin control del giro axial: el filo y el plano del codo quedan indeterminados
- Estado: INFERIDO · Prioridad: P2 · Severidad 2 · Impacto jugador 2 · Coste M · Riesgo medio
- Dónde: `.cpp:1269–1274` (`FindBetweenNormals`: solo el giro mínimo), `:760` (codo con eje fijo de reposo).
- Qué pasa: los huesos «apuntados» toman la rotación mínima desde la dirección heredada. La torsión (hacia dónde mira el filo, el plano del codo) depende del camino recorrido, sin vector polar. En la estocada (brazo atrás 146° respecto a la hoja adelante) y en poses altas el filo puede girar entre fotogramas o el codo doblarse en un plano raro. Con la espada «larga y fina» del concepto, el filo debería liderar el corte.
- Solución: «apuntar + vector polar» (giro alrededor del eje para alinear un segundo eje) en brazo y arma; una clave `FiloHacia` en `FClave`.
- Validación: prueba que muestrea cada gesto a 240 Hz y comprueba que la torsión del hueso `arma` no salta más de 15° entre muestras.

### ANIMVFX-12 Parpadeo de invulnerabilidad: oculta al personaje a 12 Hz y sin opción
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto jugador 3 · Coste S · Riesgo bajo
- Dónde: `.cpp:905–917`; `.h:265–267` (`FrecuenciaParpadeo` = 12).
- Qué pasa: tras un golpe, el personaje desaparece la mitad del tiempo, 12 veces por segundo, justo cuando el jugador más necesita leer su posición y su pose. Es un parpadeo de alta frecuencia sin interruptor (riesgo de fotosensibilidad y fatiga). Además, `SetVisibility` en vez de un material es tosco para el estilo del concepto.
- Solución: destello o borde por parámetro de material (como `AplicarDestello` de los enemigos) a ≤ 4 Hz, y una opción de accesibilidad para quitarlo.

### ANIMVFX-13 Cámara sin confort ni accesibilidad: el FOV bombea con cada dash
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto jugador 3 · Coste S · Riesgo bajo
- Dónde: `Camara/SAEVUSComponenteCamara.cpp:154–170` (FOV por velocidad real: el dash va a 1600 cm/s, por encima del esprint de 850, y sube el FOV hacia 100°), `:90–106` (sacudida con roll, `GiroSacudida.Roll`), `:41`; las opciones de pausa solo tienen calidad, sensibilidad y volumen (`Interfaz/SAEVUSWidgetPausa.h:14, 44`).
- Qué pasa: los dashes encadenados (estilo GunZ) hacen oscilar el FOV entre 90° y unos 97° continuamente. No se puede escalar ni quitar la sacudida, el FOV dinámico, el parpadeo ni el viñeteado.
- Solución: una sección «Accesibilidad / Confort» con multiplicador de sacudida (0–1), FOV dinámico sí/no, sin roll y parpadeo alternativo. Que el FOV dependa del paso (esprint) y no del dash, o un golpe corto de FOV propio del dash con el interruptor.

### ANIMVFX-14 Cámara de combate plana: golpe dado casi imperceptible y sin reacción a los grandes momentos
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto jugador 3 · Coste S · Riesgo bajo
- Dónde: `Camara/SAEVUSAjustesCamara.h:113–119`; `SAEVUSComponenteCamara.cpp:64–80` (solo golpe dado, golpe recibido y bloqueo). En la v2 la cámara solo ganó gradación y bloom (diff 6474–6560).
- Qué pasa: un golpe dado suma trauma 0,3, cuyo cuadrado es 0,09: unos 0,9 cm y 0,18°, imperceptible. No hay reacción al aterrizaje fuerte, al ataque cargado, al rugido de fase del jefe, al parry ni a la muerte. La sacudida es ruido, no un «empujón» direccional en el sentido del tajo.
- Solución: golpe direccional (desplazamiento o giro corto en la dirección del barrido, 60–90 ms), escalado por `Danio` y `DuracionParadaGolpe`; eventos para aterrizaje fuerte (`AlAterrizar` con `FuerzaAterrizajeFuerte`), cargado, rugido (`AlCambiarFase`) y parry. Todo detrás del multiplicador de ANIMVFX-13.

### ANIMVFX-15 Sin animación de carga, de bloqueo con impacto ni de parry o contraataque
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto jugador 3 · Coste M · Riesgo bajo
- Dónde: `PoseGuardia` es estática (`.cpp:891–903`); el componente de animación no consulta `ObtenerProgresoCarga` ni `EstaCargado` (`Combate/SAEVUSComponenteCombate.h:50–53`); `AlBloquear` solo dispara VFX y cámara; `VentanaContraataque` (`Combate/SAEVUSComponenteGuardia.h:50`) no tiene señal visual.
- Qué pasa: mantener el ataque no cambia el cuerpo (solo convergen partículas). Bloquear no empuja la guardia hacia atrás. El parry, protagonista en la hoja de arte (H), no tiene gesto.
- Solución: pose de carga con temblor creciente (reutilizando la tensión del aviso enemigo), retroceso de guardia por muelle al bloquear y un gesto de parry de 3 claves con fotograma de brillo.

### ANIMVFX-16 Capa: una sola cadena de 3 huesos, sin colisión y con ondeo senoidal perpetuo
- Estado: CONFIRMADO (código); penetración con las piernas INFERIDA · Prioridad: P2 · Severidad 2 · Impacto jugador 3 · Coste M · Riesgo medio
- Dónde: `.cpp:919–947`.
- Qué pasa: toda la capa es una lámina. El ondeo (`4+4·tramo`° con seno a 3,1 rad/s más Perlin) existe siempre, haya viento o no, y se lee mecánico al quedarse quieto. No hay colisión con muslos ni tronco: en zancadas de ataque (muslo 38° adelante), al correr hacia atrás (`AtrasCapa` hasta −0,3) o en el giratorio puede atravesar el cuerpo. El concepto exige «muchas tiras desgarradas» con movimiento independiente.
- Solución: 5–8 cadenas de 3–4 huesos con verlet o muelles y colisión contra 3–4 cápsulas (pelvis, muslos, tronco), o en el modelo híbrido AnimDynamics / Rigid Body o Chaos Cloth. Ondeo modulado por un volumen de viento.

### ANIMVFX-17 Coste en el hilo de juego, sin LOD por distancia
- Estado: INFERIDO (no se puede perfilar) · Prioridad: P2 · Severidad 2 · Impacto jugador 2 · Coste M · Riesgo bajo
- Dónde: `.cpp:343–375` (`TG_PostPhysics`, serie); `:949–970` (`GirarTramo` recorre 25 huesos y se llama unas 10 veces por fotograma); `:1255–1339`; `:1361` (`MarkRefreshTransformDirty` siempre).
- Qué pasa: por personaje se hacen unos 25 `FindBetweenNormals`, 25 slerps y 25 relativizaciones, unos 250 pasos de `GirarTramo`, más el refresco de la malla poseable. Mi estimación es 20–40 µs por personaje, unos 1–1,5 ms con 30 enemigos. Un enemigo visible pero lejano se anima a tasa completa: el único ahorro es la limitación fuera de pantalla. Nada va en hilos de trabajo (al contrario que una AnimInstance).
- Solución: LOD por distancia o tamaño en pantalla (30/15/7,5 Hz con acumulación, ya preparada en `TiempoPendiente`), sin secundaria ni capa a distancia; presupuesto con un significance manager; mover el cálculo a un `ParallelFor` o a la AnimInstance (§8).

### ANIMVFX-18 La muerte del enemigo se encoge antes de terminar de caer
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto jugador 3 · Coste S · Riesgo bajo
- Dónde: `Enemigos/SAEVUSEnemigo.cpp:348, 386–392` (escala a 0 en `DuracionMuerte` = 0,6 s y `SetLifeSpan`); `Animacion/SAEVUSComponenteAnimacionProcedural.cpp:822–848` (la caída dura 0,15 + 0,55 = 0,7 s × `Peso`).
- Qué pasa: el cuerpo con esqueleto se encoge como un globo mientras se derrumba y desaparece antes de tocar el suelo (antes aún en el jefe, con peso > 1). El concepto (I. Muerte) muestra cuerpos que caen y se quedan.
- Solución: con esqueleto, sin encogido. El cadáver se queda N s, se disuelve (material) o se convierte en ceniza (VFX) al terminar la caída, y `DuracionMuerte` ≥ duración de la caída × `Peso`.

### ANIMVFX-19 Las pruebas no cubren lo que se rompe
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto jugador 1 · Coste S · Riesgo bajo
- Dónde: `Pruebas/SAEVUSMundoPrueba.h:38` (todo a 1/60), `SAEVUSPruebasRevision.cpp:221–249` (el test de «fotograma largo» usa 8 Hz y ζ 0,2, combinación estable que no existe en el juego); `SAEVUSPruebasAnimacion.cpp:54–60` (`AddError` si falta `SK_Caballero`, así que una CI sin contenido sale en rojo en vez de omitir).
- Qué falta: dt variable y largo con las frecuencias reales (ANIMVFX-1), NaN, sincronía daño–pose en enemigos (ANIMVFX-2), deslizamiento de pies (ANIMVFX-3), presupuesto de partículas por ataque (ANIMVFX-4).
- Solución: las pruebas propuestas en cada hallazgo, y un `SAEVUSPruebas::PasoVariable` con secuencias de dt aleatorias pero con semilla.

### ANIMVFX-20 Motas de ambiente: el aire se vacía al moverse y atraviesan paredes
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 1 · Impacto jugador 2 · Coste S · Riesgo bajo
- Dónde: `Efectos/SAEVUSComponenteMotasAmbiente.cpp:34–46` (solo repone cuando muere una mota; viven 7 s en el mundo).
- Qué pasa: tras un dash (1600 cm/s) el jugador sale de la caja de 9 m en medio segundo y las motas se quedan atrás hasta 7 s. Nacen dentro de muros. Están en todos los mapas por igual (ligadas al jugador, no a la luz o la zona), y mantienen el pool de partículas siempre activo (R7).
- Solución: reciclar (envolver) las motas que salen de la caja; densidad por volumen o zona (`InfoZona`); material aditivo con brillo solo dentro de haces de luz.

### ANIMVFX-21 Eco del dash: creación en caliente y aspecto dependiente de un material sin garantizar
- Estado: CONFIRMADO / DESCONOCIDO (el asset) · Prioridad: P3 · Severidad 1 · Impacto jugador 2 · Coste S · Riesgo bajo
- Dónde: `Efectos/SAEVUSComponenteEcoDash.cpp:74–101` (`NewObject` + `RegisterComponent` + `SetSkinnedAssetAndUpdate` de 4 mallas en el primer dash), `:91–99` (sin `MaterialEco`, clones con el material normal).
- Qué pasa: en el primer dash de la partida se registran 4 mallas con esqueleto, lo que puede dar un tirón (que a su vez dispara ANIMVFX-1 si el dash es aéreo). Sin `MaterialEco` asignado, los ecos son 4 caballeros opacos.
- Solución: crear el grupo en `BeginPlay`; si falta el material, avisar en el log y no emitir.

### ANIMVFX-22 Restos de R7 y arrastre que depende de los fps
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 1 · Impacto jugador 1 · Coste S · Riesgo bajo
- Dónde: `SAEVUSComponenteParticulas.cpp:95, 110` (`1 − Arrastre·dt`, que a 15 fps frena 2–4× distinto que a 144); `:122–144`; `SAEVUSComponenteEstelaEspada.cpp:114–120`.
- Solución: `Frenado = Exp(−Arrastre·dt)`; recorrer solo un rango de vivas (lista compacta o índice alto); subir solo `[min, max]` sucio; mantener la sección de la estela con capacidad fija (`MaxTramos`) y degenerar los tramos sobrantes en vez de llamar a `CreateMeshSection`.

### ANIMVFX-23 Cadera retorcida tras rotaciones forzadas (banco, portal, teletransporte)
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 1 · Impacto jugador 2 · Coste S · Riesgo bajo
- Dónde: `SAEVUSCapaSecundaria.cpp:84` (`RetrasoCadera -= DeltaGuinada`); `CapaSecundaria.Reiniciar` solo en `PrepararEsqueleto` (`.cpp:284`).
- Qué pasa: un `SetActorRotation` de 180° (sentarse, salir de un portal) deja la cadera a 55° y luego se «desenrosca», con un paso de giro incluido.
- Solución: un `Reiniciar` público (también de `MuellesCapa`, `GiroMirada` y `RetrocesoImpacto`) llamado al sentarse, al teletransportar y al reaparecer, o ignorar saltos de guiñada mayores de 90° en un fotograma.

### ANIMVFX-24 Repetición perceptible del reposo y sin estados emocionales
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 1 · Impacto jugador 2 · Coste S · Riesgo bajo
- Dónde: `SAEVUSCapaSecundaria.cpp:161–176` (gestos en turno fijo cada 6 s exactos), `.cpp:939` (ondeo de la capa con seno fijo).
- Qué pasa: en 100 horas el jugador ve el mismo ciclo MirarAtrás → GirarHoja → Estirar con el mismo tempo. Faltan los estados del concepto de expresiones: herido (mano en la herida, con poca vida), cresta erizada en combate, orejas caídas.
- Solución: intervalos aleatorios con semilla por actor y ponderados por contexto (vida, cansancio, zona); una pose «herido» a partir de `Vida` < 30 %.

### ANIMVFX-25 Límites de la malla poseable con poses que salen del reposo
- Estado: INFERIDO · Prioridad: P3 · Severidad 1 · Impacto jugador 2 · Coste S · Riesgo bajo
- Dónde: `.cpp:348` (`WasRecentlyRendered`), `:1336`.
- Qué pasa: los límites de la malla se calculan con la pose de referencia. La espada extendida (90 cm), el giratorio, la muerte o el volteo del remate aéreo pueden salirse de ellos: posible desaparición en el borde de pantalla, y enemigos que se dan por «no renderizados» cuando sí se ven parcialmente (animados a 4 Hz).
- Solución: `SetBoundsScale(1.5f)` en las mallas animadas, o límites a partir de las posiciones de los huesos.

### ANIMVFX-26 Ocultar el arma escalando su hueso a 0
- Estado: CONFIRMADO / INFERIDO · Prioridad: P4 · Severidad 1 · Impacto jugador 1 · Coste S · Riesgo bajo
- Dónde: `.cpp:457–458, 1309–1310, 1327–1336, 1359`.
- Qué pasa: al cambiar de arma, la espada «se infla» durante unos 70 ms. Si la malla tuviera huesos hijos de `arma` (punta, vaina), `GetRelativeTransform` contra un padre de escala 0 daría inf o NaN en ellos.
- Solución: corte a escala 0 sin interpolar, o visibilidad por sección o material; nunca relativizar contra escala 0.

### ANIMVFX-27 La sacudida se aplica después de la colisión del brazo de cámara
- Estado: CONFIRMADO · Prioridad: P4 · Severidad 1 · Impacto jugador 1 · Coste S · Riesgo bajo
- Dónde: `Camara/SAEVUSComponenteCamara.cpp:33–43`.
- Qué pasa: junto a una pared, la sacudida (hasta 10 cm) puede meter la cámara en la geometría.
- Solución: restar la sonda (`ProbeSize`) del desplazamiento, o aplicar la sacudida solo como giro cuando el brazo está recortado.

## 5. Inmersión

| Elemento | Nivel |
|---|---|
| Protagonista desaparece o explota tras un tirón en el aire; enemigos rotos al entrar en pantalla (ANIMVFX-1) | CRITICAL |
| El enemigo golpea antes de mover el arma (ANIMVFX-2) | HIGH |
| Pies que patinan, se hunden o flotan; sin apoyo en escaleras ni pendientes (ANIMVFX-3) | HIGH |
| Arco horizontal en golpes verticales y estocadas; confeti de cubos (ANIMVFX-4/5) | HIGH |
| Jefe con los gestos del goblin, sin agarre a dos manos (ANIMVFX-7) | HIGH |
| Enemigo muerto que se encoge como un globo (ANIMVFX-18) | MEDIUM |
| Personaje que parpadea en la invulnerabilidad (ANIMVFX-12) | MEDIUM |
| Capa de una sola lámina que atraviesa las piernas y ondea sin viento (ANIMVFX-16) | MEDIUM |
| Golpe dado sin respuesta de cámara; reacción idéntica a todo golpe (ANIMVFX-10/14) | MEDIUM |
| FOV que bombea al encadenar dashes (ANIMVFX-13) | MEDIUM |
| Gesto de reposo cada 6 s exactos y en el mismo orden (ANIMVFX-24) | LOW |
| Motas que se quedan atrás o salen de los muros (ANIMVFX-20) | LOW |
| Cadera que se desenrosca tras sentarse o atravesar un portal (ANIMVFX-23) | LOW |

## 6. Escala ×10

- **10× enemigos**: unos 300 personajes animados en serie en el hilo de juego. A 20–40 µs cada uno son 6–12 ms solo de pose, sin LOD por distancia (ANIMVFX-17). Más enemigos atacando fuera de pantalla multiplican los NaN de ANIMVFX-1. Cada enemigo tiene su `USAEVUSComponenteParticulas` de 96 instancias y su pase de BatchUpdate.
- **10× ataques / armas**: cada gesto nuevo son unas 6 claves escritas en C++ y un valor nuevo en un `UENUM` (ANIMVFX-8). El arco de 5 partículas por paso agota pools de 512 (ANIMVFX-4).
- **10× especies o jefes**: imposible sin rediseñar el esqueleto (≤ 32 huesos, enum único, capa obligatoria; ANIMVFX-6). Cada especie hereda el ajuste del caballero (`FSAEVUSAjusteAnimacion` por componente, solo `PesoAnimacion` por datos).
- **10× mapas**: las motas son únicas y globales; no hay VFX por zona (niebla, ceniza, brasas, grietas del apartado L) ni volúmenes de viento para la capa.
- **10× contenido de animación hecho a mano**: con el sistema actual no hay dónde enchufar un clip (malla poseable, sin AnimInstance).

## 7. Oportunidades (máx. 10)

1. Arreglar los muelles (ANIMVFX-1): el máximo impacto por el mínimo coste. Una tarde y una prueba.
2. Sincronizar el contacto en enemigos (`TiempoImpacto`) y en el jugador (pico de velocidad en el contacto, sin filtro en la fase activa) (ANIMVFX-2/9).
3. IK de pies y zancada en función de la velocidad (ANIMVFX-3): el salto de calidad más visible.
4. VFX con vocabulario: cinta como única firma del corte, impacto en el punto de contacto, sangre, parry y familias de emisor (ANIMVFX-4/5).
5. Cámara con intención: golpe direccional escalado por daño, aterrizaje fuerte, rugido del jefe, y opciones de confort (ANIMVFX-13/14).
6. Gestos como datos con vista previa (ANIMVFX-8): permite que un animador ajuste sin programador.
7. Reacciones por peso del golpe (ligero, pesado, derribo) y por dirección del tajo (ANIMVFX-10).
8. Capa de tiras con colisión, y orejas y cresta como cadenas secundarias (ANIMVFX-16, §8).
9. Repertorio y agarre a dos manos del jefe por fase (ANIMVFX-7).
10. LOD de animación por distancia y significance manager (ANIMVFX-17).

## 8. Contraste con los conceptos y decisión procedural vs. híbrido

### Lo que el concepto pide y el sistema actual no puede representar

| Exigencia del concepto | Por qué no cabe hoy | Qué haría falta |
|---|---|---|
| **Piernas digitígradas** (hoja de rig, poses de carrera y aterrizaje) | La cadena es muslo→pierna→pie (plantígrada, `.h:25–34`); la zancada y el aterrizaje están pensados con 2 segmentos (`.cpp:1018`, `FraccionMusloPierna`) | Hueso `metatarso` y `dedos` por pierna (+4); IK de 3 segmentos o IK de 2 huesos con ángulo de tobillo fijado por pose; apoyo en dedos |
| **Cresta y orejas** expresivas (hoja de expresiones: erizada, caída) | Solo `cabeza`; tope de 32 huesos | 2–4 cadenas cortas (+6–10 huesos) con muelle (cadena física) y un «estado emocional» que mueve sus objetivos |
| **Capa de muchas tiras desgarradas** (hoja de rig: «movimiento de capa y ropa») | 1 cadena de 3 huesos (`capa_1..3`) | 5–8 cadenas × 3–4 huesos (+15–30) con verlet o muelles y colisión de cápsulas, o Chaos Cloth / AnimDynamics |
| **Faldón y tiras del cinturón** | No hay huesos | 4–6 cadenas cortas (+8–12) |
| **Manos-garra articuladas** («dedos articulados») | Sin dedos | +10–15 huesos o al menos poses de mano (abierta, puño, agarre) por blend |
| **Espada larga y fina, filo que lidera** | Sin control de giro axial (ANIMVFX-11) | Apuntar con vector polar; `LongitudHoja` ya existe |
| **Aterrizaje fuerte con la mano al suelo**, apoyo en pared con la mano | Sin IK ni trazas | IK de mano hacia una traza (suelo o pared) |
| **Jefe con guadaña/alabarda a dos manos**, 2 fases | Una sola arma en `mano_d`, mano izquierda libre, gestos de goblin | Hueso o socket del arma con un punto de agarre, IK de la mano de apoyo, arcos largos con claves, repertorio por fase |
| **Goblins con dos armas o cadenas** | Un solo hueso `arma` | `arma_i` y cadena física para el mayal |
| **VFX**: sangre, parry, impactos por intensidad, telegrafiados en el suelo (anillo rojo para ataque de área), aura de fase 2, ceniza y niebla | Un solo ISM de cubos por actor y sin decals | Niagara (recomendado, ya en el motor) o como mínimo emisores por familia con mallas y materiales, decals de sangre y telegrafiado |

Cuenta aproximada para el protagonista con el concepto: 25 + 4 (digitígrado) + 8 (cresta/orejas) + 20 (tiras de capa) + 10 (faldón) + 12 (dedos) ≈ **75–80 huesos**, frente al tope actual de 32.

### ¿Seguir 100 % procedural o pasar a híbrido?

**Recomendación: modelo híbrido por etapas** (clips para el gesto principal, capas procedurales para la vida), sin tirar lo hecho.

- **A. Seguir 100 % procedural**
  - Coste a corto: bajo.
  - Techo: bajo. Cada movimiento nuevo es trabajo de programador; los enemigos y el jefe seguirán siendo «dos poses»; no hay animación facial, de dedos ni de tela. El riesgo de no llegar a la calidad AAA pedida es **alto**.
  - Solo es razonable para un prototipo o un juego muy estilizado.
- **B. Híbrido (recomendado)**
  - Técnica:
    - Pasar de `UPoseableMeshComponent` a `USkeletalMeshComponent` + `UAnimInstance` en C++, con la pose actual como un nodo propio en una primera fase (adaptador).
    - Los clips (Blender → FBX) cubren locomoción en 8 direcciones (blendspace, arranques, frenadas, giros), ataques (montajes con eventos de contacto y cancelación que se cruzan con `FSAEVUSDefinicionAtaque`), reacciones (4 direcciones × ligera/pesada, derribo), muertes, carga, parry y bloqueo.
    - Se mantienen como capas procedurales: `FSAEVUSCapaSecundaria` (aditiva), mirada, IK de pies y manos (Control Rig o Two Bone IK), cadenas físicas (AnimDynamics o Rigid Body para tiras, orejas y cresta), retrocesos con muelle, hit-stop por velocidad de reproducción y ecos.
  - Coste:
    - Programación: unas 3–5 semanas para una persona (adaptador, nodo secundario, IK, cadenas, montajes y eventos, migración de pruebas).
    - Contenido: protagonista 70–90 clips; goblin 25–35 por arquetipo; jefe 40–60 (×2 fases). Del orden de 4–8 meses de un animador, o captura de movimiento más limpieza.
  - Riesgo: medio. Se mitiga con la fase de adaptador: se comparan transformaciones de huesos frente a la versión actual en pruebas, y la jugabilidad no depende de la animación porque la línea de tiempo ya está en los datos.
  - Ganancias: animación en hilos de trabajo (escala ×10), LOD de animación del motor y un pipeline de animadores.
- **C. Intermedio barato** (válido como paso previo de B)
  - Muelles estables, gestos como datos con vista previa, máscara de 64 huesos o `TBitArray` con roles por especie, IK de pies en el FK propio, capa en varias cadenas.
  - Coste M, riesgo bajo.
  - Sube la calidad de forma clara, pero sigue sin clips ni pipeline de animador. Todo lo que se haga aquí (secundaria, IK, roles, gestos como datos) se reutiliza como capa en B.

**Orden propuesto**:
1. ANIMVFX-1/2/9 (días).
2. C: gestos como datos, IK de pies, tiras de capa (2–4 semanas).
3. Decisión de producción sobre el animador.
4. B con adaptador.
5. Sustituir por clips primero la locomoción y los ataques del jugador, y luego jefe y enemigos.

Sin un animador en plantilla, quedarse en C es lo «terminable». Con animador, B es lo que permite llegar al concepto.


---

# Auditoría AAA SAEVUS v2: área `enemigos_audio` (IA, enemigos, jefes, SFX, música)

Rutas relativas a `Source/SAEVUS/`. Leídos enteros: `Enemigos/*`, `Audio/*`, `Animacion/SAEVUSAnimacionEnemigo.cpp`, `Pruebas/SAEVUSPruebas{Enemigos,Proyectiles,Audio,Equilibrio}.cpp` y lo relativo a enemigos y audio de `SAEVUSPruebasRevision.cpp`. Para entender cómo encajan con lo demás he leído `Combate/SAEVUSComponenteGuardia.cpp`, `Combate/SAEVUSComponenteCombate.cpp:420-450`, `Nucleo/SAEVUSModoJuego.cpp:85-140`, `Interfaz/SAEVUSEstadoInterfaz.cpp:81-97`, `Tutorial/SAEVUSPuertaArena.cpp` y `Animacion/SAEVUSComponenteAnimacionProcedural.cpp:206-375`. También he mirado los conceptos de goblins, del jefe, la hoja de dirección de arte y la hoja de la protagonista.

## 1. Resumen del área

El núcleo es limpio y está bien pensado para crecer con datos. La IA es una máquina de estados en un componente (Reposo → Perseguir → Telegrafiar → Atacar → Recuperarse, más Aturdido y Muerto). Cada tipo es un `UPrimaryDataAsset` con ataques, fases y gestos, y hay 5 primitivas de ataque. Los tiempos son explícitos: hay aviso, ventana de castigo y hasta pruebas de equilibrio con reglas de diseño. Conviene **conservar** cuatro cosas: el telegrafiado como estado propio (el arma brilla y hay pose de aviso), el escudo con sus 3 counters, la devolución de proyectiles y el `LanzarPropio`.

Las debilidades son de fondo:
- **La IA no navega ni ve.** Detecta por radio 2D, atraviesa paredes y suelos, va en línea recta y patrulla solo por el eje X del mundo.
- **No tiene memoria ni coordinación de grupo.** Elige el ataque por rotación fija y por distancia.
- **El daño del `Golpe` llega antes que su animación.**
- **Los jefes son enemigos con barra.** No tienen introducción, ni transición de fase audible o visible (salvo el rugido de los que tienen esqueleto), ni payoff, y no se reinician cuando muere el jugador.

En audio, los SFX suenan **sin atenuación** (2D a volumen pleno en todo el mapa), son idénticos para todos los tipos de enemigo y no hay forma de poner assets reales. La música dinámica es una buena base: 3 capas con fundidos y generación en otro hilo. Pero tiene bucles muy cortos, armonías que chocan al superponerse, un escalón de volumen en el bucle de exploración, un solo tema para los 3 jefes y ningún stinger.

**Mayor riesgo:** la experiencia de combate depende de la legibilidad, y hoy fallan a la vez la espacialidad del sonido, la sincronía golpe-animación, la coordinación de grupos y la navegación. Por eso los goblins en grupo y el Rey Goblin del concepto **no se pueden construir sobre este sistema sin las ampliaciones de la sección 8**.

## 2. Puntuaciones sugeridas (0–100)

| Categoría | Nota | Justificación |
|---|---|---|
| Arquitectura enemigos (data-driven) | 68 | Ajustes, ataques, fases y gestos en datos y FSM pequeña y probada. Faltan validación, condiciones y extensibilidad (AoE, invocar). |
| IA: percepción, navegación, memoria | 22 | Radio 2D sin línea de visión, sin navmesh, sin búsqueda; patrulla en el eje X; no reacciona al disparo lejano. |
| IA: comportamiento de grupo | 10 | Nada: sin turnos de ataque, sin separación, todos atacan a la vez. |
| Identidad de tipos de enemigo | 45 | Las 5 primitivas y el escudo dan variedad base (INFERIDO de nombres y datos). La rotación determinista los hace robóticos. |
| Telegrafiado y legibilidad | 50 | Buena intención (pose, brillo, gruñido), pero el golpe va por delante de la pose, el giro es instantáneo, no hay telegrafiado de área y el sonido no da dirección. |
| Jefes como experiencia | 22 | Fases = escalas + subconjunto de ataques. Sin intro, sin bloqueo de arena, sin transición, sin tema propio ni payoff, sin reinicio. |
| SFX (comunicación de peligro, distancia y tamaño) | 25 | Sin atenuación, sin sonido por tipo, sin pasos enemigos, sin sonido al detectar, aturdirse o desviar. |
| Mezcla y espacialización | 15 | Solo volumen maestro; sin SoundClass, Submix, concurrencia ni reverb de sala para los SFX. |
| Música (composición y sistema dinámico) | 45 | Capas con fundido y generación asíncrona correctas. Bucles de 13–32 s, choques armónicos, sin leitmotiv ni stingers. |
| Síntesis procedural como producto comercial | 20 | Seno con barrido + ruido de 1 polo a 22 kHz mono: placeholder honesto, no comercial. No se puede sustituir por assets sin tocar código. |
| Rendimiento y hilos del audio | 60 | Caché de PCM y generación fuera del hilo de juego. Pero copia el PCM por enemigo, crea una onda por reproducción, regenera la música en cada mundo y desenlaza con riesgo de carrera. |
| Rendimiento de enemigos ×10 | 58 | El reposo lejano y la caché de objetivo ayudan. El tick y el aspecto no se recortan fuera de pantalla; cada enemigo lleva unos 12 componentes. |
| Pruebas del área | 55 | Buena cobertura de la FSM, el escudo, los proyectiles y el equilibrio. Sin pruebas de E6/E9, música, grupo, sincronía golpe-animación ni el caso real de pogo. |

## 3. Estado de la revisión anterior

| ID | Estado | Evidencia v2 |
|---|---|---|
| E6 | **RESUELTO** (sin prueba) | `Enemigos/SAEVUSEnemigo.cpp:328-338` (`bLanzamientoPropio` con `TGuardValue`); `Enemigos/SAEVUSComponenteIAEnemigo.cpp:358` usa `LanzarPropio`. Ninguna prueba verifica que un empuje recibido durante `Atacar` se escale. |
| E9 | **RESUELTO** (sin prueba) | `Enemigos/SAEVUSProyectil.cpp:38-41` bloquea WorldDynamic; el objeto es PhysicsBody y las zonas (`Nucleo/SAEVUSCanalesColision.h` `ConfigurarZonaJugador`) ignoran todo salvo Pawn, así que no lo paran. El tajo lo encuentra por `TrazaArma`. No hay prueba con puerta o pared rompible. |
| R3 | **PARCIAL** | Caché del pawn por fotograma y de su vida: `SAEVUSComponenteIAEnemigo.cpp:58-87`. Reposo lejano: `:284-294`. Escrituras de escala y brillo solo si cambian: `SAEVUSEnemigo.cpp:404-423`. **Falta:** `ASAEVUSEnemigo::Tick` (`:362-367`) sigue corriendo cada fotograma sin comprobar si se renderiza (aleteo, destello, escala). `ActualizarFase` hace `FindComponentByClass` en cada cambio de vida (`:158`, menor). |
| R5 | **PARCIAL** | Caché compartida de PCM: `Audio/SAEVUSSintesis.cpp:57-77`. Pero **devuelve copia** (`return *Guardado`, `:67`), así que cada enemigo guarda unos 60 KB propios (`SAEVUSComponenteSonidoEnemigo.cpp:67-69`). Sigue habiendo una `USoundWaveProcedural` nueva por reproducción (`SAEVUSSintesis.cpp:79-89`, `SAEVUSComponenteSonido.cpp:247`, `SAEVUSComponenteSonidoEnemigo.cpp:146`, `SAEVUSSubsistemaSonidoMundo.cpp:94`). |
| M2 | **NO** (mantenido por decisión) | `SAEVUSEnemigo.h:60` sigue igual y ahora también se emite al destruirse sin morir (`SAEVUSEnemigo.cpp:352-360`, correcto). No hay delegados globales nuevos. Sí hay un estado global estático nuevo: la caché `static TMap` con cerrojo en `SAEVUSSintesis.cpp:60-61` (aceptable, pero crece sin límite y sobrevive entre sesiones de PIE). |
| M5 (IA) | **RESUELTO** | `ObtenerAjuste` tiene respaldo: `SAEVUSComponenteIAEnemigo.cpp:46-50`. `Actualizar` sale si no hay enemigo (`:189`). Los accesos posteriores (`MantenerVuelo`, `ComprobarContacto`) solo se llaman desde ahí. |
| M9 | **NO** | `SAEVUSComponenteSonido.cpp:209-235` sigue con la cadena de `Cast` por tipo de emisor, y para `Guardia` quita los dos manejadores con cada par. Los delegados dinámicos (`:178,182-187`) no se desenlazan (es inofensivo, pero el criterio es desigual). |

## 4. Hallazgos

### EA-1 La IA no navega ni ve: detección 2D por radio, línea recta y patrulla en el eje X
- Estado: CONFIRMADO (código); el impacto en mapas concretos es DESCONOCIDO · Prioridad: **P1** · Severidad 4 · Impacto jugador 5 · Coste L · Riesgo regresión medio
- Dónde: `Enemigos/SAEVUSComponenteIAEnemigo.cpp:89-93` (`Dist2D`), `:234-260` (detección y patrulla en `Origen.X`), `:296-315` (persigue en línea recta), `:460-476` (elige ataque por distancia 2D); `Enemigos/SAEVUSEnemigo.cpp:72` (`bCanWalkOffLedges=false`); comentario de diseño en `SAEVUSComponenteIAEnemigo.h:19`.
- Qué pasa y por qué:
  - **Detección:** basta con `Dist2D <= RadioDeteccion`. No mira altura, ni paredes, ni a dónde mira el enemigo.
  - **Movimiento:** `AddMovementInput` va recto hacia el jugador.
  - **Patrulla:** ida y vuelta por el eje X del mundo, y solo da la vuelta cuando el desplazamiento en X llega al radio.
  - Secuencias que lo rompen:
    - (a) El jugador sube a una cornisa 3 m por encima de un goblin. La distancia 2D es menor que `DistanciaMaxima` del tajo, así que el goblin avisa y golpea al aire en bucle mientras el jugador le dispara.
    - (b) Un enemigo en la sala de al lado (o en el piso de abajo) entra en Perseguir a través del muro, empuja contra la pared indefinidamente y además enciende la música de pelea (EA-10).
    - (c) En `L_Mazmorra`, con pasillos a lo largo de Y, un enemigo patrulla contra la pared de su pasillo: el desplazamiento en X nunca llega al radio, así que se queda andando sin moverse.
    - (d) Tras perseguir, vuelve a patrullar donde esté: nunca regresa a su `Origen` en Y.
- Por qué importa: es la diferencia entre «habitantes del mundo» y «entes esperando». En un juego con escalada y carrera por pared, los enemigos de suelo se anulan con cualquier cornisa. Y es la base de cualquier mapa que no sea un pasillo en X (INFERIDO: el tutorial está construido a lo largo de X, `Depuracion/SAEVUSSubsistemaRecorrido.cpp:614`).
- Solución propuesta, incremental y sin cambiar de arquitectura (el componente sigue mandando):
  1. Comprobar línea de visión con `LineTraceSingleByChannel(ECC_Visibility)` a 5 Hz, más un cono de visión y una tolerancia vertical (`AlturaMaximaDeteccion`).
  2. Elegir ataque solo con `|dz| <` un alcance vertical por ataque (campo nuevo).
  3. Patrulla relativa a la orientación inicial del actor (o con puntos en el editor), con vuelta al encontrar una pared (velocidad ~0 durante 0,3 s) y regreso a `Origen` en el estado nuevo `Volver`.
  4. Navegación: ruta con `UNavigationSystemV1::FindPathToLocationSynchronously` cada ~0,5 s, siguiendo puntos con `AddMovementInput` (sin `AAIController`). Si no hay navmesh, se mantiene la línea recta actual, así que los mundos de prueba no cambian.
- Validación: pruebas nuevas en `SAEVUSPruebasEnemigos.cpp`. (1) Jugador sobre un bloque a +300 cm: el enemigo no entra en Telegrafiar. (2) Muro entre los dos: sigue en Reposo. (3) Patrulla con pared en +X: la velocidad no se queda a 0 más de 0,5 s.

### EA-2 Sin memoria ni aggro por daño; los jefes no reaccionan al disparo lejano ni se reinician cuando muere el jugador
- Estado: CONFIRMADO (rutas de código); la explotabilidad real depende de radios y geometría (INFERIDO) · Prioridad: **P1** · Severidad 4 · Impacto 5 · Coste M · Riesgo medio
- Dónde: `SAEVUSComponenteIAEnemigo.cpp:131-139` (`Aturdir` no hace nada en jefes), `:147-154`, `:222-226` (Aturdido → Perseguir), `:296-302` (lo pierde más allá de `RadioPerdida`); `:173-178` (las fases solo avanzan); `Nucleo/SAEVUSModoJuego.cpp:131-140` (reaparecer = `RestartPlayer`, el mundo no se reinicia); `Interfaz/SAEVUSEstadoInterfaz.cpp:84` (la barra del jefe se oculta en Reposo).
- Qué pasa y por qué: recibir daño no da información sobre el atacante. No hay estado «Alerta» ni «Buscar».
  - (a) Un enemigo menor disparado con el rifle a más de 18 m se aturde, pasa a Perseguir, comprueba que la distancia supera `RadioPerdida` y vuelve a Reposo: no reacciona.
  - (b) Un jefe en Reposo que recibe disparos no hace **nada**: `Aturdir` sale sin efecto, no cambia de estado y su barra ni aparece. Se le puede bajar la vida desde fuera de su radio.
  - (c) Si el jugador muere contra el jefe, reaparece en el punto de control y el jefe conserva vida y fase. Basta con morir unas cuantas veces para ganarle por desgaste.
- Por qué importa: trivializa los jefes (el payoff emocional del juego) y rompe la inmersión: un enemigo al que acaban de disparar vuelve a pasear.
- Solución: en `AlCambiarVida`, si `Instigador` es un pawn, fijar `UltimaPosicionConocida` y pasar a Perseguir (o a un estado `Buscar` de unos 4 s que va a esa posición) sin mirar `RadioPerdida`, y despertar también a los jefes. Además, un reinicio de enemigos al morir el jugador: método `ASAEVUSEnemigo::Reiniciar()` (vida llena, `Fase=0`, posición `Origen`, Reposo), llamado desde un evento «al reaparecer» del modo de juego, al menos para `bEsJefe` y los enemigos de la arena activa. Decidir con el dueño si los enemigos menores reaparecen en el banco (estilo Souls/HK).
- Validación: (1) Prueba: dañar a un jefe en Reposo desde 3000 cm hace que su estado deje de ser Reposo. (2) Prueba: tras `ReiniciarJugador`, un jefe dañado vuelve a vida máxima y fase 0.

### EA-3 Los jefes no son una experiencia: sin intro, sin arena, sin transición de fase, sin tema propio y sin payoff
- Estado: CONFIRMADO lo que falta en código; el contenido de los mapas es DESCONOCIDO · Prioridad: **P1** · Severidad 4 · Impacto 5 · Coste L · Riesgo bajo
- Dónde:
  - `Enemigos/SAEVUSTiposEnemigo.h:150-171`: una fase = `EscalaVelocidad`, `EscalaTiempos` y lista de ataques.
  - `SAEVUSComponenteIAEnemigo.cpp:156-179`: `AlCambiarFase` solo lo escucha la animación (`Animacion/SAEVUSComponenteAnimacionProcedural.cpp:239`), que ruge si hay esqueleto (`Animacion/SAEVUSAnimacionEnemigo.cpp:277-287, 296-303`).
  - `Audio/SAEVUSMusica.h:17`: una sola pista `Jefe`.
  - `SAEVUSEnemigo.cpp:340-350` y `:388-395`: el jefe muere igual que un rastrero (se encoge en 0,6 s y suelta 40 partículas).
  - `Tutorial/SAEVUSPuertaArena.cpp:53-92`: la puerta solo se abre al ganar y no cierra la entrada.
- Qué pasa:
  - **Inicio del combate:** el jefe «empieza» cuando entra en Perseguir. La barra y la música aparecen a la vez, sin presentación ni nombre en pantalla (la barra lleva el nombre, `SAEVUSEstadoInterfaz.cpp:94`).
  - **Cambio de fase:** el jefe sigue atacando *mientras* ruge (la pose del rugido pisa la del brazo del arma en pleno golpe, `SAEVUSAnimacionEnemigo.cpp:283-284`). No suena nada, no cambia la música ni el color y no hay invulnerabilidad. Guardián y Madre Alada (INFERIDO: cuerpos de boceto) no tienen ninguna señal de cambio de fase.
  - **Victoria:** la música de jefe sigue 4 s más (`Audio/SAEVUSSubsistemaMusica.cpp:149-153`) y se apaga en ~3 s. No hay stinger, ni silencio, ni pantalla «Victoria» (la hoja de arte la pide), ni recompensa: no hay drops en el código (búsqueda sin resultados).
  - **Defensa:** la parada contra un jefe solo da alma (`Combate/SAEVUSComponenteGuardia.cpp:154-157`), porque `Aturdir` no hace nada. No existe «postura» ni «castigo largo».
- Por qué importa: la referencia de calidad es Elden Ring. Cada jefe tiene que ser un capítulo, y hoy es un enemigo con más vida.
- Solución incremental (datos primero):
  - Añadir a `FSAEVUSFaseEnemigo`: `DuracionTransicion` (un estado `Transicion` invulnerable que espera a terminar el ataque en curso), `SonidoTransicion`, `ColorMascara` y `ColoresPiezas` alternativos (fase 2 roja), `PistaMusica` o capa extra y `AtaqueApertura` (un ataque fijo al entrar en la fase).
  - Añadir a `FSAEVUSAjusteEnemigo`: `PistaMusicaJefe` y `EscalaAturdimiento`/«postura», un medidor que las paradas y los golpes fuertes llenan y que da un `Aturdido` largo también al jefe.
  - Un actor `ASAEVUSArenaJefe`: volumen que cierra la entrada, despierta al jefe, lanza el nombre y el stinger, y al ganar hace silencio, stinger, cartel y recompensa. La muerte del jefe necesita su propia secuencia (cámara lenta y cadáver).
- Validación: pruebas de la transición (invulnerable durante `DuracionTransicion`, sin atacar), de la postura (N paradas dan Aturdido aunque sea `bEsJefe`) y de la arena (al entrar, puerta cerrada y jefe en Perseguir; al morir, puerta abierta).

### EA-4 El `Golpe` hace daño en el primer fotograma de `Atacar`, antes de que la animación llegue al impacto
- Estado: CONFIRMADO · Prioridad: **P1** · Severidad 4 · Impacto 5 · Coste S · Riesgo bajo
- Dónde: `SAEVUSComponenteIAEnemigo.cpp:411-421` (se golpea si `!bGolpeAplicado`, es decir, en el primer `ActualizarAtacar`) frente a `Animacion/SAEVUSAnimacionEnemigo.cpp:218-229` (la pose va de Aviso a Golpe con `SalidaRapida` a lo largo de `Duracion*0.5`, y además pasa por los muelles de `SuavizarYAplicar`).
- Qué pasa: al acabar el aviso, el daño se aplica en el fotograma 0 de `Atacar`, cuando el arma del goblin aún está arriba (U=0). El tajo visible llega 75–150 ms después (INFERIDO por la curva y los muelles). La hitbox es una esfera puntual en un solo fotograma, sin barrido ni arco: no representa el área de una alabarda.
- Por qué importa: el jugador recibe el golpe antes de verlo, en el corazón del combate «tipo Hollow Knight». Parece injusto, desincroniza el esquive y el bloqueo, y hace inútil leer la pose.
- Solución: campo `TiempoImpacto` (s, escalado por fase) en `FSAEVUSAtaqueEnemigo`. El golpe se aplica cuando `TiempoEstado >= TiempoImpacto` (con la protección de E10 para fotogramas largos), y la animación usa ese mismo valor como pico de U. Opcional: una ventana activa con barrido de esfera en arco (`AnguloArco`) para armas largas.
- Validación: prueba que simule un ataque `Golpe` y compruebe que la vida del jugador no baja antes de `TiempoImpacto` y sí después. Prueba de animación: la mano del arma está en su punto más bajo dentro de ±1 fotograma de `TiempoImpacto`.

### EA-5 Los SFX del mundo y de los enemigos no se espacializan: se oyen igual en todo el mapa
- Estado: CONFIRMADO (no se pasa atenuación y la onda no tiene) + INFERIDO alto (comportamiento del motor: sin `AttenuationSettings` el sonido se reproduce 2D a volumen pleno) · Prioridad: **P1** · Severidad 4 · Impacto 5 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSComponenteSonidoEnemigo.cpp:146`, `Audio/SAEVUSSubsistemaSonidoMundo.cpp:94` (`PlaySoundAtLocation(..., Volumen)` sin atenuación); `Audio/SAEVUSSintesis.cpp:79-89` (la onda nace sin atenuación).
- Qué pasa: el gruñido de aviso, el ataque y la muerte de cualquier enemigo, y la puerta, la palanca, el banco y el escudo, suenan a volumen completo y centrados vengan de donde vengan. Con 4 goblins, 4 gruñidos idénticos no dicen cuál ataca ni de dónde. La única distancia que existe es un corte manual del aleteo a 12 m (`SAEVUSComponenteSonidoEnemigo.cpp:129-133`).
- Por qué importa: el sonido es el canal de amenaza fuera de cámara en tercera persona. Con un `AvisoMinimo` de 0,25 s (`Pruebas/SAEVUSPruebasEquilibrio.cpp:43`), un ataque desde atrás sin dirección sonora no se puede leer.
- Solución: un `USoundAttenuation` creado en código una vez (subsistema; atenuación por radio de 3–25 m, espacialización HRTF o panning, oclusión opcional) y pasarlo en `PlaySoundAtLocation(..., AttenuationSettings)`. Mejor aún, un activo de proyecto referenciado por `TSoftObjectPtr` en los ajustes (ver EA-7).
- Validación: prueba: la onda o la llamada lleva atenuación no nula. Prueba manual de escucha con un enemigo a 30 m.

### EA-6 Todos los enemigos suenan igual y les faltan los sonidos que comunican estado
- Estado: CONFIRMADO · Prioridad: **P1** · Severidad 3 · Impacto 4 · Coste M · Riesgo bajo
- Dónde: `Audio/SAEVUSComponenteSonidoEnemigo.h:12-38` (ajustes en el componente, no en `USAEVUSDatosEnemigo`, `Enemigos/SAEVUSTiposEnemigo.h:314-360`); `SAEVUSComponenteSonidoEnemigo.cpp:86-102` (solo Telegrafiar, Atacar y Muerto); `:11-21` (aviso fijo de 0,45 s).
- Qué pasa:
  - Rastrero, Rey Goblin y Madre Alada usan los mismos 3 PCM: el tamaño no se oye (el comentario de `PesoAnimacion`, «los pasos retumban», `SAEVUSTiposEnemigo.h:288-291`, solo existe en la animación).
  - No suena nada al detectar (sin «te he visto»), al recibir golpe ni aturdirse, al romperse el escudo (el bloqueo y la rotura usan **el mismo** `Escudo`, `Enemigos/SAEVUSComponenteEscudo.cpp:69` y `:82`, así que no se oye que se ha roto), con los pasos del enemigo, con el vuelo o impacto del proyectil, ni al devolverlo (`Combate/SAEVUSComponenteCombate.cpp:440-443`: el proyectil devuelve 0, no es «golpe» y la desviación es muda).
  - El aviso dura 0,45 s aunque el telegrafiado del jefe dure 1,2 s o el de un goblin 0,25 s, y no se corta al aturdirse.
- Por qué importa: el diseño sonoro debería contar el tamaño, la intención y el estado, y hoy solo dice «algo ataca en algún sitio».
- Solución: mover `FSAEVUSAjusteSonidosEnemigo` a `USAEVUSDatosEnemigo` (con el actual como valor por defecto) y añadir `Detectar`, `Herido`, `Paso` (por zancada de la animación o cada X cm), `EscudoRoto` y `Desviar`. Aviso en bucle o con la duración del telegrafiado (`Duracion = TiempoTelegrafiar*EscalaTiempos`, sintetizado por fase con caché por huella), detenido al cambiar de estado (guardar `UAudioComponent*` de `SpawnSoundAtLocation`). Variación de tono por tamaño (`PesoAnimacion`).
- Validación: pruebas de reproducción por evento (contador como en `SAEVUSPruebasAudio.cpp`): detectar, recibir golpe, romper escudo, desviar.

### EA-7 El audio no tiene camino hacia producción: sin assets, sin categorías, sin concurrencia y sin mezcla
- Estado: CONFIRMADO · Prioridad: **P1** · Severidad 3 · Impacto 4 · Coste M · Riesgo bajo
- Dónde: `Audio/SAEVUSSintesis.h:10-14` («lo sustituirán sonidos hechos a mano», pero ningún ajuste admite un `USoundBase`); `Nucleo/SAEVUSGuardado.cpp:57-63` (solo `SetTransientPrimaryVolume`); no hay `USoundClass`, `USoundMix`, `USoundSubmix` ni `USoundConcurrency` en todo el módulo (búsqueda).
- Qué pasa:
  - Para sustituir un sonido hay que tocar el C++ de cada componente.
  - No hay volúmenes de música, efectos y voces por separado (la hoja de arte pide opciones de «Audio»).
  - No hay concurrencia: 10 goblins avisando son 10 voces idénticas.
  - No baja la música en pausa ni en los golpes grandes, y no hay reverb de sala para los SFX: suenan secos y cercanos mientras la música lleva una reverb de catedral horneada (EA-9).
  - La síntesis (seno con barrido + ruido de 1 polo, 22 kHz mono, `SAEVUSSintesis.cpp:14-43`) es un boceto, no un timbre comercial.
- Por qué importa: es el cuello de botella entre «prototipo» y «producto». Sin categorías no hay accesibilidad de audio.
- Solución: añadir a cada `FSAEVUSSonidoSintetizado` un `TSoftObjectPtr<USoundBase> Activo` (si existe se usa; si no, la síntesis), así se sustituye pieza a pieza sin regresiones. Las SoundClass Master/Música/SFX/Ambiente/UI y la concurrencia por tipo de enemigo se crean como assets (DESCONOCIDO: no tenemos `Content/`) y se referencian desde un `UDeveloperSettings`. MetaSounds para la variación procedural en lugar de PCM de un solo uso.
- Validación: prueba: con `Activo` asignado se reproduce el activo y no la onda procedural. Prueba de opciones: el volumen de música no afecta a los SFX.

### EA-8 Sin coordinación de grupo: todos avisan y atacan a la vez, sin separación
- Estado: CONFIRMADO · Prioridad: **P1** · Severidad 4 · Impacto 5 · Coste M · Riesgo medio
- Dónde: `SAEVUSComponenteIAEnemigo.cpp:296-315` (cada IA decide sola); no existe ningún subsistema de combate ni de turnos.
- Qué pasa: 4 goblins en rango eligen ataque en el mismo fotograma y telegrafían a la vez. Los voladores convergen en el mismo punto `AlturaVuelo` sobre el jugador y se empujan entre ellos. Los enemigos de suelo se apilan en fila.
- Por qué importa: el concepto muestra 6 variantes de goblin pensadas para ir en grupo. Sin turnos de ataque el grupo es ilegible e injusto, sobre todo con el sonido sin dirección (EA-5).
- Solución: un `UWorldSubsystem` «director» con fichas de ataque (máx. 1–2 atacantes cuerpo a cuerpo y 1–2 a distancia, prioridad al más cercano o visible; los jefes no necesitan ficha). Quien no tiene ficha rodea al jugador a distancia (desplazamiento lateral según un ángulo asignado por slot) en vez de esperar quieto. Separación simple (repulsión entre enemigos a menos de 2 radios). El estado sigue en el componente; el director solo concede permisos.
- Validación: prueba con 4 enemigos a 300 cm: como mucho N están en Telegrafiar o Atacar a la vez durante 5 s simulados.

### EA-9 Música: bucles de 13–32 s, un tema para 3 jefes, sin leitmotiv, sin stingers y sin silencio
- Estado: CONFIRMADO · Prioridad: **P1** · Severidad 3 · Impacto 4 · Coste L · Riesgo bajo
- Dónde: `Audio/SAEVUSMusica.cpp:13-19` (exploración 32 s, combate 16 s, jefe 13,7 s), `:253-281`, `:285-337`; `Audio/SAEVUSMusica.h:9-19` (3 pistas); `Audio/SAEVUSSubsistemaMusica.cpp:124-158`.
- Qué pasa:
  - Un jefe de 3 minutos repite el mismo bucle de 13,7 s unas 13 veces. La exploración repite cada 32 s durante horas, con las campanas «siempre en los mismos sitios» (semilla fija, `:271-278`).
  - No hay tema por jefe ni por zona, ni intro, ni capa extra en fase 2, ni stinger de victoria, muerte, descubrimiento o banco, ni momentos de silencio: la exploración suena siempre, también al 0,3 bajo el combate.
  - En el menú suena la misma exploración (INFERIDO: el menú principal se muestra sobre un mapa de juego, `Nucleo/SAEVUSModoJuego.cpp:83`).
  - Las transiciones no esperan al compás.
- Por qué importa: la música es medio peso emocional de un jefe. La repetición corta produce fatiga y rompe la «catedral en ruinas».
- Solución incremental:
  - (1) Generación por secciones (A/B/C con variaciones de semilla y orquestación) encadenadas al azar en el `Underflow` (que ya pide más audio) en lugar de un único bucle.
  - (2) Pista por jefe y por zona en datos, más una capa «fase 2» que entra en la transición.
  - (3) Stingers de un disparo (victoria, muerte, banco).
  - (4) Silencio de verdad: la exploración solo en zonas marcadas, ambiente sin música en el resto.
  - (5) A medio plazo, música compuesta (asset) con el mismo subsistema de capas.
- Validación: prueba: la secuencia de secciones no repite la misma sección más de 2 veces seguidas. Prueba de `DecidirPista` con jefe vivo, jefe muerto (stinger) y fases.

### EA-10 La música de pelea se enciende a través de paredes y funciona como radar
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 3 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSSubsistemaMusica.cpp:21-26, 124-143` (cualquier enemigo en Perseguir a ≤25 m en 3D cuenta como pelea), unido a EA-1.
- Qué pasa: un enemigo del piso de abajo que «ve» al jugador en 2D enciende la pista de combate sin que haya pelea visible. El jugador aprende que la música le avisa de enemigos escondidos, y las emboscadas mueren.
- Solución: contar como pelea solo a los enemigos con línea de visión o en Telegrafiar/Atacar, o a los que ya han golpeado o recibido golpe en los últimos N s, y exigir que estén en la misma «zona» (`ASAEVUSInfoZona`).
- Validación: prueba con un enemigo en Perseguir tras un muro: la pista sigue en Exploración.

### EA-11 Choques armónicos: la exploración suena a 0,3 debajo del combate y del jefe, con otros acordes
- Estado: CONFIRMADO (cálculo sobre los datos) · Prioridad: P2 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSSubsistemaMusica.cpp:183-186` (`CalmaEnPelea` también con `Jefe`), `Audio/SAEVUSMusica.cpp:248-249, 293-294`.
- Qué pasa: las pistas arrancan juntas. La exploración cambia de acorde cada 8 s (Rem, Sib, Solm, La) y el combate cada 4 s con la misma progresión, así que se superponen Sib con La (si♭ contra la y do♯ contra re) en [12,16) s y La con Solm en [24,28) s. Son unos 8 s de choque cada 32 s. Contra el jefe (re frigio: Rem, Mib, Rem, Do) suenan La mayor contra Mib, un tritono. Además cada pista se normaliza por pico (`:236-245`), no por sonoridad, así que el equilibrio entre capas es arbitrario.
- Solución: bajo combate o jefe, quitar la capa de exploración (`CalmaEnPelea=0`) o dejar solo el bordón y el viento como pista aparte. Normalizar por RMS o LUFS aproximado.
- Validación: prueba: la suma de las 3 pistas al volumen de pelea no supera ~0,95 de pico; revisión de escucha.

### EA-12 El bucle de exploración tiene un escalón de volumen a los 5 s
- Estado: CONFIRMADO (lectura); magnitud INFERIDA · Prioridad: P2 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSMusica.cpp:149-165` (`Bordon`) y `:168-180` (`Viento`) escriben sobre `Lienzo.Muestras.Num()`, que **incluye** los 5 s de cola; `:227-233` (`Cerrar` suma esa cola al principio).
- Qué pasa: la cola está pensada para las notas que se pasan del final, pero el bordón continuo y el viento también la llenan. Al cerrar el bucle, los 5 primeros segundos llevan dos bordones superpuestos (el desfase de fase de unos 0,2 ciclos los suma: ~×1,6, unos +4 dB) y doble viento. A los 5 s el nivel cae de golpe, cada 32 s.
- Solución: limitar `Bordon` y `Viento` a `[0, Largo)` y cerrar su fase en bucle (frecuencias con ciclos enteros en `Largo`). Para la reverb, procesar el bucle dos veces seguidas y quedarse con la segunda pasada.
- Validación: prueba: el RMS en ventanas de 0,5 s alrededor de t=5 s varía menos de 1 dB (patrón de `ValorEficaz`, `Pruebas/SAEVUSPruebasAudio.cpp:24-32`).

### EA-13 La música se regenera en cada mundo: silencio al cargar, CPU repetida y bloqueo al cerrar
- Estado: CONFIRMADO (código) / INFERIDO (tiempos) · Prioridad: P2 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSSubsistemaMusica.cpp:34-56` (cada `OnWorldBeginPlay`), `:58-80` (`Generacion.Wait()`), `:28-32`.
- Qué pasa:
  - Cada carga de mapa (menú, tutorial, mazmorra, reaparición por `?continuar`) vuelve a generar las 3 pistas en un hilo (INFERIDO: 0,5–1,5 s, con `Exp` por muestra en `FPasoBajo::Filtrar`, `SAEVUSMusica.cpp:40-49`). Mientras tanto hay silencio y la música empieza de cero, sin continuidad entre mapas.
  - Si se sale antes de terminar, `Deinitialize` bloquea el hilo de juego hasta que acabe.
  - Los mundos de prueba son `EWorldType::Game` (`Pruebas/SAEVUSMundoPrueba.h:59-62`). Si el editor tiene dispositivo de audio, cada prueba genera las 3 pistas y **las reproduce**, y su destrucción espera a la generación. El comentario «sin dispositivo… tests» (`SAEVUSSubsistemaMusica.h:50`) solo es cierto con `-nosound`.
- Solución: caché de proceso (`static TSharedPtr<const TArray<uint8>>` por pista, como `SintetizarBytes`), generada una vez y reutilizada; mejor aún, hornear las pistas a assets con un commandlet. Excluir mundos sin `GameInstance` o con una bandera de prueba. Guardar la posición del bucle en un subsistema de `GameInstance` para no reiniciar al cambiar de mapa.
- Validación: prueba: dos mundos seguidos reutilizan los mismos bytes (comparar punteros). Prueba: en `FMundoPrueba` el subsistema no genera.

### EA-14 Carrera al desenlazar el `Underflow` y posible virtualización con volumen 0
- Estado: INFERIDO · Prioridad: P3 · Severidad 3 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSSubsistemaMusica.cpp:76` (desenlaza en el hilo de juego un delegado que ejecuta el hilo de render de audio, `:109-112`); `:114` (`CreateSound2D` con volumen 0) y `:191` (mínimo 0,001).
- Qué pasa: `Stop()` es asíncrono. El hilo de audio puede estar dentro de la lambda mientras `Unbind` modifica el delegado (carrera de datos; muy raro, pero puede romper al salir del mapa). La lambda solo captura un `TSharedPtr` (bien, no captura `this`). Arrancar a volumen 0 puede hacer que el motor virtualice o pare la fuente según la configuración.
- Solución: no desenlazar. Dejar que la onda muera con su dueño, o bien `Stop` y desenlazar tras la confirmación del hilo de audio (`FAudioThread::RunCommandOnAudioThread`). Arrancar con 0,001 en lugar de 0.
- Validación: soak test de cambios de mapa en bucle (100 veces) con ASan o TSan en Linux.

### EA-15 PCM copiado por enemigo y una `USoundWaveProcedural` nueva por reproducción (R5 parcial)
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSSintesis.cpp:57-77` (devuelve copia), `:79-89`; `Audio/SAEVUSComponenteSonidoEnemigo.h:70-73`.
- Qué pasa: unos 60 KB de PCM por enemigo (aviso 19,8 KB, ataque 13,2 KB, muerte 26,5 KB), así que 200 enemigos son 12 MB duplicados. Cada sonido (pasos a ~4 Hz, golpes, gruñidos) crea un UObject y copia los bytes, lo que produce basura para el GC en pleno combate. Las ondas procedurales no admiten caché del motor ni compresión.
- Solución: `SintetizarBytes` devuelve `TSharedRef<const TArray<uint8>>`. Medio plazo: generar `USoundWave` no procedurales una vez por huella (o hornearlas) y reutilizarlas, lo que permite atenuación y concurrencia en el asset.
- Validación: prueba: dos enemigos del mismo tipo comparten el mismo puntero de datos; `stat memory` con 100 enemigos.

### EA-16 La elección de ataque es una rotación determinista por distancia: patrones robóticos
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 3 · Impacto 4 · Coste M · Riesgo medio
- Dónde: `SAEVUSComponenteIAEnemigo.cpp:460-476`.
- Qué pasa: siempre el siguiente de la lista que encaje en la distancia. No tiene pesos, enfriamientos, condiciones (jugador en el aire, curándose, detrás, en guardia, huyendo), combos encadenados, fintas ni retrasos variables, ni castiga la curación (la mecánica central de alma). Además, `Telegrafiar` es la única ventana de lectura y siempre dura lo mismo, así que el ritmo es metronómico.
- Por qué importa: es legible pero no «intencional». A las 10 horas el jugador juega contra una lista, no contra un ser vivo.
- Solución: añadir a `FSAEVUSAtaqueEnemigo` `Peso`, `Enfriamiento`, `Condiciones` (bitmask: SueloJugador, AireJugador, JugadorCurando, JugadorDetras, JugadorEnGuardia), `SiguienteEnCombo` (índice y ventana) y `VariacionAviso` (±%). Elección por peso con `FRandomStream` sembrado (pruebas deterministas) y antirrepetición. Una regla global: si el jugador se cura dentro del alcance, prioridad a un ataque rápido.
- Validación: prueba con semilla fija para la secuencia; prueba de que, con el jugador curándose, se elige el ataque marcado.

### EA-17 Giro instantáneo y seguimiento perfecto al avisar; se retira de espaldas
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 3 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `SAEVUSComponenteIAEnemigo.cpp:304-307, 317-326, 518-524` (`SetActorRotation` directo cada fotograma); `:310-314` y `Enemigos/SAEVUSEnemigo.cpp:71` (`bOrientRotationToMovement`).
- Qué pasa:
  - Al elegir ataque el enemigo gira 180° en un fotograma (salto visual en el cuerpo con esqueleto) y sigue al jugador hasta el último fotograma del aviso.
  - El counter de «rodear» al Acorazado (`SAEVUSTiposEnemigo.h:249-253`) casi no funciona: en cuanto avisa, el escudo te mira.
  - El Escupidor y el Hondero se retiran dándote la espalda (orientación al movimiento) y escupen girando de golpe.
- Solución: `VelocidadGiro` (°/s) por tipo, seguimiento solo durante el primer X % del aviso (`FraccionSeguimiento`). Durante la retirada, `bOrientRotationToMovement=false` y encarar con la velocidad de giro (paso atrás).
- Validación: prueba: con el jugador detrás, a mitad de aviso el ángulo es menor que `VelocidadGiro·t`; el Acorazado avisando se puede rodear.

### EA-18 Escudo: el pogo solo pasa si el atacante está exactamente encima, y la rotura no se distingue
- Estado: CONFIRMADO (código y prueba) · Prioridad: P2 · Severidad 3 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Enemigos/SAEVUSComponenteEscudo.cpp:59-63` (dirección 2D normalizada; solo es nula si coinciden X e Y); prueba degenerada en `Pruebas/SAEVUSPruebasEnemigos.cpp:257-263` (jugador en `+Z` exacto); mismo sonido para bloquear y romper en `SAEVUSComponenteEscudo.cpp:69,82`.
- Qué pasa: en el pogo real el jugador está encima pero unos cm por delante, así que `GetSafeNormal2D` da un vector unitario hacia delante: **bloquea y lo empuja**. El counter documentado «se salta por encima (pogo)» falla casi siempre. El escudo no tiene malla propia en el boceto (INFERIDO; DESCONOCIDO con el asset) ni estado roto visible.
- Solución: pasar si `Causante.Z - Enemigo.Z > MediaAltura` o si el ataque del jugador es «Abajo» (usar `FDamageEvent` con un tipo de daño propio o la dirección de ataque). Sonido y VFX distintos para la rotura, y un brillo o color de escudo roto mientras dure.
- Validación: cambiar la prueba para que el pogo venga desde `(+30, 0, +150)`.

### EA-19 Las fases no tienen transición y la animación se desincroniza de la IA en fase 2
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Animacion/SAEVUSAnimacionEnemigo.cpp:246` (la recuperación usa `TiempoRecuperacion` sin `EscalaTiempos`) frente a `SAEVUSComponenteIAEnemigo.cpp:216` (escalado); `SAEVUSAnimacionEnemigo.cpp:277-287` (rugido encima del ataque).
- Qué pasa: con `EscalaTiempos=0,6` la IA sale de Recuperarse cuando la pose aún va por el 60 %, y la animación salta a «Lista». El rugido no detiene la IA.
- Solución: exponer en la IA `ObtenerProgresoRecuperacion()` como ya se hace con `ObtenerProgresoAviso()`. El estado `Transicion` de EA-3.
- Validación: prueba: en la fase 2, al salir de Recuperarse la pose está a menos de ε de «Lista».

### EA-20 No hay ataques de área: el «mazazo al caer» solo hace daño por contacto
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 3 · Impacto 4 · Coste M · Riesgo bajo
- Dónde: `SAEVUSComponenteIAEnemigo.cpp:404-407` (Salto: termina al aterrizar), `:498-501` (daño de cuerpo durante Atacar); `Animacion/SAEVUSAnimacionEnemigo.cpp:220-227` (pose de mazazo en el suelo); `SAEVUSTiposEnemigo.h:31-44` (5 tipos, ninguno de área).
- Qué pasa: el Saltador o el Rey Goblin caen con un golpe a dos manos, pero solo hiere el roce de cápsulas. Un jugador a 1 m del impacto no recibe nada. No hay onda, decal ni temblor ligados al golpe.
- Solución: tipo `Area` (centro en el enemigo o en el punto de aterrizaje, `Radio`, `TiempoImpacto`) y opción `OndaAlAterrizar` en `Salto`. Telegrafiado en el suelo con un decal o malla plana con material emisivo que crece durante el aviso (concepto K: «ataque de área», «ataque importante (jefe)»).
- Validación: prueba: jugador a `Radio-10` del aterrizaje recibe daño; a `Radio+10`, no.

### EA-21 Proyectiles: el solape a quemarropa se pierde, desviar es mudo y el proyectil no tiene presencia
- Estado: CONFIRMADO / INFERIDO (el primer punto) · Prioridad: P2 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Enemigos/SAEVUSProyectil.cpp:61` (enlace después del `SpawnActor`); `SAEVUSComponenteIAEnemigo.cpp:430-451` (salida a la altura del centro del actor, aunque el comentario dice «máscara» (M6); un MID por proyectil, `SAEVUSProyectil.cpp:66-75`); `Combate/SAEVUSComponenteCombate.cpp:440-443`.
- Qué pasa:
  - (a) Si el proyectil nace ya solapando al jugador (abanico del jefe a quemarropa), el solape inicial ocurre antes de `AddUniqueDynamic` y no genera un `BeginOverlap` posterior, así que el proyectil le atraviesa (INFERIDO: depende de cuándo actualiza el motor los solapes al spawnear).
  - (b) Desviar no da sonido ni alma.
  - (c) No suena al volar ni al chocar, y al tocar una pared desaparece sin impacto.
  - (d) Las balas del jugador (`TrazaArma`, `:41`) también los devuelven. Puede ser intencional; hay que confirmarlo con el dueño.
- Solución: enlazar en el constructor y, en `Lanzar`, revisar `GetOverlappingActors` una vez. Evento propio `AlDesviar` con sonido, chispa y hit-stop. Sonido en bucle atenuado mientras vuela y ráfaga más sonido al chocar. Material compartido por tipo en lugar de un MID por instancia.
- Validación: prueba: un proyectil creado dentro de la cápsula del jugador hace daño en el primer paso.

### EA-22 La muerte encoge el cuerpo hasta cero (también goblins y jefes) y no deja cadáver
- Estado: CONFIRMADO · Prioridad: P2 · Severidad 2 · Impacto 4 · Coste S · Riesgo bajo
- Dónde: `Enemigos/SAEVUSEnemigo.cpp:388-395` (escala → 0) y `:399-403` (la excepción de esqueleto excluye Muerto); `:348` (vida útil de 0,6 s por defecto).
- Qué pasa: el goblin hace `PoseMuerte` mientras se encoge y desaparece en 0,6 s, igual que el jefe. El concepto (hoja de arte, «I. Muerte») muestra cuerpos que caen y se quedan.
- Solución: con esqueleto, no encoger. Mantener la pose final N s (configurable, más larga para jefes) y luego disolver o hundir en el suelo. Presupuesto de cadáveres (los 8 más recientes).
- Validación: prueba: con esqueleto, la escala de `Piezas` es 1 durante la muerte.

### EA-23 La goblin y la protagonista comparten esqueleto, locomoción y lenguaje visual: riesgo de legibilidad
- Estado: CONFIRMADO (mismo rig y misma `PoseLocomocion`: `SAEVUSTiposEnemigo.h:334-340`, `Animacion/SAEVUSAnimacionEnemigo.cpp:157-166`) + INFERIDO (conceptos) · Prioridad: P2 · Severidad 3 · Impacto 4 · Coste M · Riesgo bajo
- Qué pasa: los conceptos de goblin y protagonista comparten capas de hojas rasgadas, máscara puntiaguda con orejas, patas digitígradas con garras, hebillas de anillo y paleta oscura verde y marrón. Además el código les da el mismo ciclo de paso y la misma capa secundaria, cambiando solo el encorvado (20°) y el tembleque. En una sala oscura, con 4 goblins y la cámara alejada, la silueta del jugador se pierde entre ellos.
- Solución: un código de color de amenaza fijo (los enemigos con acentos cálidos rojo y óxido y ojos que brillan; la protagonista con acento frío cian, ya presente en los ojos y la hoja del concepto), con contorno o borde de luz por facción. Proporciones distintas (los goblins más bajos, cabeza grande, brazos largos). Locomoción propia (paso corto, nervioso, a saltitos) con parámetros de `FSAEVUSAjusteEnemigo` en lugar de reutilizar el ciclo de la protagonista.
- Validación: prueba visual. Capturas en escala de grises y a 25 % de resolución: se identifica a la protagonista en menos de 0,5 s (prueba con 5 personas).

### EA-24 Exploit: el daño por contacto se puede parar, así que una guardia a tiempo da alma sin atacar
- Estado: INFERIDO (depende del enfriamiento de la guardia, área combate) · Prioridad: P2 · Severidad 2 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `SAEVUSComponenteIAEnemigo.cpp:504-511` (el contacto usa `ApplyDamage` con el enemigo como causante); `Combate/SAEVUSComponenteGuardia.cpp:135-161` (cualquier daño frontal con causante dentro de la ventana es contraataque: aturde y da `AlmaPorContraataque`=22).
- Secuencia: el jugador se pega a un Rastrero y pulsa guardia justo antes del contacto. Contraataque: el rastrero se aturde y el jugador gana 22 de alma. Repite al acabar el aturdimiento, sin atacar nunca. Es alma infinita para curarse.
- Solución: el daño de contacto se marca (tipo de daño `UDamageType` propio) y la guardia lo bloquea sin contraataque. O solo cuentan como contraatacables los `Atacar` del enemigo.
- Validación: prueba: guardia recién activada y contacto de un enemigo en Reposo o Perseguir no cuentan como contraataque.

### EA-25 Sin validación de datos de enemigos
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 2 · Coste S · Riesgo bajo
- Dónde: `Enemigos/SAEVUSTiposEnemigo.h:150-171, 308-311` (las fases «deben» ir ordenadas; `Ataques` son índices sin comprobar); `SAEVUSComponenteIAEnemigo.cpp:164-172`.
- Qué pasa: fases desordenadas dan una fase equivocada; índices fuera de rango dejan una fase sin ataques (el jefe solo persigue); `DistanciaMinima > DistanciaMaxima` hace que un ataque no salga nunca; un `Picado` sin `bVolador` hace cosas raras. `SAEVUSPruebasEquilibrio.cpp` solo vigila los 11 assets que conoce.
- Solución: `USAEVUSDatosEnemigo::IsDataValid` con esas reglas más las de equilibrio (aviso ≥ 0,25 s).
- Validación: prueba de `IsDataValid` con datos rotos.

### EA-26 Coste por enemigo que no se recorta fuera de pantalla ni lejos (R3 parcial, escala)
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 2 · Coste M · Riesgo bajo
- Dónde: `Enemigos/SAEVUSEnemigo.cpp:362-424` (tick, aleteo con `SetRelativeLocation` cada fotograma, destello); `:76-106` (≈12 componentes por enemigo, 4 mallas estáticas y una poseable aunque se oculten); la animación hace tick también sin esqueleto (`SAEVUSComponenteAnimacionProcedural.cpp:343-363`, sale enseguida).
- Solución: `SetActorTickInterval` según la distancia y `WasRecentlyRendered`, crear las piezas de boceto bajo demanda (o subclase con esqueleto sin ellas) y desactivar el tick de `Animacion` sin malla.
- Validación: `stat game` con 200 enemigos; prueba: un enemigo sin renderizar recientemente tiene intervalo de tick > 0.

### EA-27 Riesgo de compilación: `TFuture` en la cabecera sin su include
- Estado: INFERIDO · Prioridad: P3 · Severidad 3 · Impacto 1 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSSubsistemaMusica.h:79` (`TFuture<…>`; incluye `CoreMinimal.h` y `Subsystems/WorldSubsystem.h`, ninguno garantiza `Async/Future.h`).
- Qué pasa: con unity build probablemente compila por arrastre. Con `bUseUnity=false`, compilación adaptativa o IWYU estricto puede fallar.
- Solución: `#include "Async/Future.h"` en la cabecera (y `Templates/SharedPointer.h` está en `CoreMinimal`).
- Validación: compilar el módulo sin unity.

### EA-28 Las pruebas del área no cubren lo nuevo ni lo que más falla
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 2 · Impacto 2 · Coste M · Riesgo bajo
- Dónde: `Pruebas/` (no hay `Musica`, `LanzarPropio`, `WorldDynamic` ni pruebas con varios enemigos; búsqueda sin resultados).
- Qué pasa: E6 y E9 se arreglaron sin prueba. La música (nueva en v2, 560 líneas) no tiene ninguna. Las de equilibrio usan los datos y no la ejecución, así que no habrían visto EA-4. La caché del objetivo depende de `GFrameCounter` (`SAEVUSComponenteIAEnemigo.cpp:65-69`), que no avanza durante una prueba que da pasos a mano: una prueba que dependa del objetivo por defecto vería siempre el pawn del primer paso (hoy todas usan `FijarObjetivo`).
- Solución: pruebas mínimas: `SAEVUS.Enemigo.EmpujeDuranteAtaque`, `SAEVUS.Enemigo.ProyectilPuerta`, `SAEVUS.Audio.MusicaBucleContinuo` (EA-12), `SAEVUS.Audio.MusicaDecision` (pelea, jefe, fin), `SAEVUS.Enemigo.GrupoFichas` (EA-8), `SAEVUS.Enemigo.TiempoImpacto` (EA-4). Cambiar la caché a una clave por mundo y tiempo (`GetWorld()->TimeSeconds`).

### EA-29 Fatiga sonora: cada sonido es idéntico en cada reproducción
- Estado: CONFIRMADO · Prioridad: P3 · Severidad 1 · Impacto 3 · Coste S · Riesgo bajo
- Dónde: `Audio/SAEVUSComponenteSonido.cpp:238-311` (solo los pasos y el aterrizaje varían el tono; el tajo cambia por ataque), `Audio/SAEVUSComponenteSonidoEnemigo.cpp:139-147`.
- Solución: tono aleatorio de ±4–6 % en todo, 3–4 variantes por sonido (cambiando `Semilla`; la caché por huella ya lo admite) en rotación sin repetir, y volumen ligado al daño.
- Validación: prueba: N reproducciones de `Golpe` usan al menos 3 variantes.

## 5. Inmersión

| Elemento | Nivel |
|---|---|
| Golpe de enemigo que llega antes del tajo visible (EA-4) | **CRITICAL** |
| Sonido sin dirección ni distancia: se oye un gruñido de un enemigo al otro lado del mapa (EA-5) | **HIGH** |
| Enemigos que ven a través de muros y empujan paredes o bordes (EA-1) | **HIGH** |
| Jefe que «empieza», cambia de fase y muere sin ceremonia; conserva el daño si mueres (EA-2/3) | **HIGH** |
| Grupos que atacan todos a la vez (EA-8) | **HIGH** |
| Música de 13–16 s en bucle durante todo un jefe; acordes que chocan (EA-9/11) | MEDIUM |
| Enemigo disparado que vuelve a pasear (EA-2) | MEDIUM |
| Giro de 180° en un fotograma; el que se retira te da la espalda (EA-17) | MEDIUM |
| Muerte que encoge al goblin como un globo (EA-22) | MEDIUM |
| Mismo sonido para un rastrero y para el Rey Goblin; el escudo roto suena igual que el bloqueo (EA-6) | MEDIUM |
| Escalón de volumen cada 32 s en la exploración (EA-12) | MEDIUM |
| Enemigos «de boceto» (esferas y conos con cuernos) conviviendo con goblins pintados: dos lenguajes visuales en el mismo mundo (INFERIDO) | MEDIUM |
| SFX secos con la música en una catedral con reverb; sin ambiente por zona ni silencio (EA-7/9) | MEDIUM |
| Patrullas de ida y vuelta en X a media velocidad, sin rutinas (comer, vigilar, hablar): entes esperando, no habitantes | MEDIUM |
| Pogo bloqueado por el escudo cuando «debería» pasar (EA-18) | LOW |
| 1–2 s de silencio al cargar cada mapa y música que reinicia (EA-13) | LOW |

## 6. Escala ×10 (10× enemigos, mapas y contenido)

- **CPU por enemigo:** el CMC completo, `Tick` del actor sin recorte, contacto y búsqueda del pawn cada fotograma, animación procedural (R1, área animación). Cada sistema recorre todos los enemigos: el HUD cada fotograma (R4), `PoseMirada` de la protagonista y la música a 4 Hz. Con 500 enemigos en un mapa abierto hace falta un gestor de relevancia (significance) y una lista registrada de enemigos en lugar de `TActorIterator`.
- **Memoria de audio:** unos 60 KB de PCM duplicado por enemigo (EA-15), así que 500 enemigos son unos 30 MB. Las `USoundWaveProcedural` por reproducción generan basura para el GC en proporción al número de enemigos activos.
- **Voces:** sin concurrencia, 30 enemigos avisando son 30 voces 2D a volumen pleno. Se roban voces al azar (el límite por defecto es de unas 32) y desaparecen el golpe del jugador o la música.
- **Contenido:**
  - Los sonidos por tipo viven en código: 10× tipos es 10× C++.
  - Una sola pista de jefe y tres pistas en total: 10× jefes o zonas no caben en `ESAEVUSPistaMusica` (enum cerrado). Hay que pasar a pistas en datos.
  - Sin `IsDataValid`, 100 data assets acumularán errores silenciosos.
  - Sin navegación, cada mapa nuevo necesita diseñarse «para la línea recta».
- **Generación de música:** 10 zonas con su pista, generadas en cada mundo, son segundos de CPU en cada carga. Se impone la caché o el horneado.
- **Grupo:** sin director ni fichas, la dificultad crece de forma combinatoria con el número de enemigos.

## 7. Oportunidades (mayor impacto)

1. **`TiempoImpacto` + telegrafiado de área + `Transicion` de fase** (EA-4/20/19). Son tres campos de datos que convierten «leer el aviso» en la columna del combate y hacen posible el moveset del concepto.
2. **Atenuación y sonido por tipo de enemigo con assets sustituibles** (EA-5/6/7). Es el mayor salto de legibilidad por coste S/M.
3. **Director de combate con fichas** (EA-8). Así los goblins en grupo pasan a ser una coreografía legible.
4. **Percepción con línea de visión, aggro por daño, memoria y navmesh opcional** (EA-1/2). Los enemigos pasan a ser habitantes.
5. **Arena de jefe completa** (EA-3): bloqueo, nombre, stinger, fase 2 con color, música y rugido sonoro, postura, cadáver, silencio, «Victoria» y recompensa.
6. **Música por secciones encadenadas, tema por jefe y zona, stingers y silencio** (EA-9). Es una corrección barata con el `Underflow` que ya existe.
7. **Elección de ataque con pesos, enfriamientos, condiciones y castigo a la curación** (EA-16). Da «intención» sin coste de rendimiento.
8. **Reacciones y rutinas en reposo:** sobresalto al detectar (aviso de 0,3 s más el sonido de detección), puestos de guardia, goblins que se avisan entre ellos (propagación a 10 m), huida del Hondero cuando cae el grupo.
9. **Muerte con peso:** cadáver, dissolve y golpe final con hit-stop y sonido grave por tamaño (EA-22).
10. **Lenguaje de amenaza por color y silueta** que separe a los goblins de la protagonista (EA-23).

## 8. Contraste con los conceptos

**Goblins** (6 variantes: dagas dobles, cuchilla con mochila, hoz con brazo de garra, ballesta o «hondero» con casco, hondero encapuchado con cuerda y peso, encapuchado con hoz):

| El concepto exige | El sistema actual | Cambio técnico |
|---|---|---|
| 6 siluetas y proporciones distintas (encorvado bajo frente a alto con mochila) | Las 6 tendrían que usar los **25 huesos de la protagonista**, sus proporciones de referencia y su ciclo de paso (`SAEVUSTiposEnemigo.h:334-340`). Solo cambian el encorvado y el peso. | Rig de goblin propio (la animación procedural ya busca huesos por nombre: añadir el mapeo del esqueleto, `SAEVUSComponenteAnimacionProcedural.cpp:302-315`) o escalado por hueso en los datos. Parámetros de locomoción por tipo. |
| Armas distintas: 2 dagas, cuchilla, hoz, peso con cuerda, ballesta | Una sola ranura «Arma» que brilla y un hueso `Arma` en la mano derecha. Sin segunda arma, sin armas a distancia distintas: la piedra, el virote y el escupitajo son la misma esfera (`SAEVUSProyectil.cpp:44-50`). | Ranuras «Arma_I», brillo por ranura, `Malla` y `Gravedad` del proyectil por ataque (el hondero lanza en parábola), gesto «Doble». |
| Telas y hojas que ondean | Malla estática con piel; no hay simulación de tela ni huesos de tela. | Huesos de capa con muelles (la `CapaSecundaria` ya tiene muelles) o Chaos Cloth. |
| Garras y brazo libre que ataca (variante 3) | El gesto `Manotazo` existe (`SAEVUSAnimacionEnemigo.cpp:87-91`), pero el golpe es una esfera única. | Hitbox por hueso (mano o arma) con barrido en la ventana activa. |

**Jefe** (concepto del Rey Goblin según la hoja de arte: unas 2× la altura de la protagonista, capa de jirones, corona de púas, guadaña o alabarda en 4 variantes; fase 2 con jirones rojos y ojo rojo; ataques en las poses: barrido amplio de guadaña, golpe vertical con alabarda que levanta escombros, salto; telegrafiados de área en círculo rojo):

| El concepto exige | El sistema actual | Cambio técnico |
|---|---|---|
| **Fase 2 roja** (jirones rojos, ojo encendido, VFX) | `FSAEVUSFaseEnemigo` solo escala tiempos y velocidad (`SAEVUSTiposEnemigo.h:150-171`). Colores y malla fijos en `USAEVUSDatosEnemigo`. | Cambios por fase: `ColoresPiezas`, `TexturasPiezas`, emisivo, malla alternativa o piezas visibles, partículas en bucle, transición con rugido sonoro e invulnerable (EA-3/19). |
| Arma larga con barrido de 180° y alcance de 3–4 m | Esfera puntual a `AlcanceGolpe` en un fotograma (`SAEVUSComponenteIAEnemigo.cpp:411-421`). | Hitbox de sector (`AnguloArco`, `RadioInterior/Exterior`) o barrido por hueso del arma entre `TiempoImpacto` y `FinActivo`. |
| **Telegrafiados de área** (círculo rojo en el suelo, «ataque importante») | No existe ataque de área ni decal (EA-20). El aviso es la pose más el brillo del arma. | Tipo `Area` con decal o malla plana emisiva que crece con `ObtenerProgresoAviso()`. Color reservado para ataques imparables o de área. |
| Golpe al suelo con escombros y onda | El Salto solo hace daño por contacto; no hay temblor de cámara ni partículas ligadas al impacto del enemigo. | Evento `AlImpactoEnemigo` (posición, fuerza) que dispare partículas, temblor y sonido grave atenuado. |
| Capa de jirones en movimiento | Igual que los goblins: sin tela. | Huesos de capa con muelles o tela; además el rig propio de 2× altura (con 25 huesos humanos el resultado es una «protagonista grande y encorvada»). |
| Presentación, nombre «REY GOBLIN — Fase 2», «VICTORIA» | La barra del HUD muestra nombre y fase (`SAEVUSEstadoInterfaz.cpp:92-96`); no hay intro ni pantalla de victoria. | `ASAEVUSArenaJefe` (EA-3) con cartel, música propia y stinger. |
| Cadáveres (panel «Muerte»: goblin normal, élite, jefe) | Se encoge a 0 en 0,6 s (EA-22). | Pose final persistente y luego dissolve. |

**Madre Alada** (no hay concepto): como volador solo puede ser un cuerpo de boceto que sube y baja entero (`SAEVUSEnemigo.cpp:412-418`). El rig humano no tiene huesos de ala, así que no puede aletear. Si su nombre implica invocar crías, **no hay ataque de invocación** (`ESAEVUSTipoAtaqueEnemigo` no lo tiene); habría que añadir un tipo `Invocar` (clase y datos del enemigo, número, tope de vivos).

**Legibilidad protagonista frente a goblins:** ver EA-23. Comparten capas de hojas, máscaras con orejas puntiagudas, patas digitígradas con garras, hebillas de anillo y paleta. El código lo agrava porque comparten rig y locomoción. La separación debe venir de un código de color fijo (cálido = amenaza, frío = jugadora), de proporciones y de un paso propio, no solo del modelado.


---

# Auditoría AAA SAEVUS v2: interfaz y arquitectura

Ámbito: `Interfaz/`, `Depuracion/` y sus pruebas (`SAEVUSPruebasInterfaz.cpp`, `SAEVUSPruebasMapa.cpp`, `SAEVUSPruebasDepuracion.cpp`), además de la arquitectura global, el rendimiento global, la UI/UX, la accesibilidad, la localización y los textos. Todas las rutas son relativas a `Source/SAEVUS/` y los números de línea son de la v2. Los textos visibles para el jugador están en `textos.md`.

## 1. Resumen del área

La base técnica está sana y es disciplinada:
- composición por componentes;
- estado de interfaz separado del widget y probado sin pantalla (`Interfaz/SAEVUSEstadoInterfaz.h:15`);
- subsistemas por responsabilidad;
- ningún puntero crudo a UObject sin `UPROPERTY` en las cabeceras, comprobado con un grep sistemático;
- delegados desenganchados en `EndPlay`;
- unas 171 pruebas de automatización, una prueba de humo y un recorrido del tutorial en el juego real.

Conviene conservar la separación estado/widget, las pruebas puras (`OpacidadTituloZona`, `TrocearTeclas`, `Encuadrar`) y el uso de `TWeakObjectPtr` en las herramientas.

Las debilidades están en el **producto**:
- **Menús.** Los menús son de prototipo: tres opciones cíclicas y nada de accesibilidad.
- **HUD.** No transmite progreso: una máscara nueva no hace crecer la barra.
- **Notificaciones.** Todas comparten una sola ranura con el tutorial.
- **Pantalla de muerte.** No ofrece elección.
- **Guardado.** No tiene versión ni copia de respaldo, y ahora usa escrituras asíncronas que pueden solaparse.
- **Release.** No hay nada de release: ni CI visible, ni crash reporting, ni telemetría, y el código de depuración se compila en Shipping.

**Mayor riesgo:** perder la partida. Una partida ilegible se sustituye en silencio por una nueva, y la primera escritura posterior la pisa (`Nucleo/SAEVUSGuardado.cpp:17-28`). A eso se suma la carrera entre guardados asíncronos y el guardado síncrono de `Deinitialize`.

Lo nuevo de la v2 en mi área compila mentalmente sin errores evidentes (Enhanced Input `QueryKeysMappedToAction`, `FKey::GetDisplayName(false)`, lambdas sin sombras de variables).

## 2. Puntuaciones sugeridas (0–100)

| Categoría | Nota | Justificación |
|---|---|---|
| Arquitectura global | 68 | Composición clara y subsistemas bien elegidos. Pesan el módulo único, los ciclos `Interfaz↔Mundo` y `Nucleo↔Depuracion`, el HUD de juego dentro de «Depuración» y el componente de animación que hace de nudo central. |
| Clean code / legibilidad | 78 | Estilo coherente y comentarios que explican el porqué. Hay funciones de 180–370 líneas (`WidgetInterfaz::Construir`, `PruebaHumo::Tick`), comentarios obsoletos y finales de línea mezclados. |
| Ciclo de vida / GC | 82 | Sin punteros crudos sin `UPROPERTY` en las cabeceras, débiles donde toca y delegados retirados. Quedan un delegado estático global, una caché estática sin límite y desreferencias de débiles sin comprobar en las herramientas de QA. |
| Data-driven | 55 | Ajustes en structs `UPROPERTY` y data assets de enemigos y armas. Los colores de menús y mapa, los tamaños y las rutas están en `namespace`; los textos del tutorial llevan teclas escritas dentro; la UI no admite Blueprint. |
| Rendimiento global (hoy) | 72 | Sin tirones evidentes. Quedan barridos por fotograma en el HUD, un widget por rectángulo en el mapa, un `UObject` por sonido y un guardado cada 5 s. |
| Rendimiento ×10 | 55 | El coste por enemigo (movimiento, animación, IA, sonido y partículas) domina. La UI escala mal en el mapa y con varios jefes. |
| Testing | 72 | Unas 171 pruebas en 33 archivos, más humo y recorrido. No hay pruebas de widget ni de rendimiento con umbral, el guardado tiene una cobertura mínima y varias pruebas dependen de assets. |
| HUD | 58 | Legible y sobrio, con rastro de daño, destello y título de zona. No hay iconografía, minimapa, contador de curas ni progreso visible, y usa Roboto en vez de una serif. |
| Menús y ajustes | 35 | Tres opciones cíclicas. No hay «Nueva partida», remapeo, categorías ni confirmaciones. |
| Accesibilidad | 15 | Nada específico: no hay remapeo, tamaño de texto, daltonismo, interruptor de sacudidas ni de destellos, volumen por canales ni alternancia de las acciones que se mantienen pulsadas. |
| Localización | 50 | Los textos de interfaz usan `LOCTEXT` y los de mapa son `FText`. Los nombres de arma son `FName`, hay teclas incrustadas en los textos y no hay selector de idioma ni string tables. |
| Producción / release readiness | 35 | Sin CI visible, crash reporting ni telemetría. El guardado no tiene versión, el HUD de depuración llega a Shipping y los finales de línea están mezclados. |
| Robustez del guardado | 45 | Ranura única, sin versión ni copia de respaldo, con carreras asíncronas y sustitución silenciosa de una partida corrupta. |

## 3. Estado de la revisión anterior (IDs de mi área)

| ID | Estado | Evidencia v2 |
|---|---|---|
| R4 (HUD recalcula todo por fotograma) | **PARCIAL** | Resuelto: la zona se busca una sola vez (`Interfaz/SAEVUSWidgetInterfaz.cpp:322`) y los setters solo actúan si el valor cambia (`:57-95`). Sigue igual: cada fotograma hay 3 `FindComponentByClass` (`Interfaz/SAEVUSEstadoInterfaz.cpp:22,34,45`), un `TActorIterator<ASAEVUSEnemigo>` (`:81`) y 1–3 `FText::Format` (`:43,56-57`), además de `Calcular` en `WidgetInterfaz.cpp:555`. No se ha implementado el barrido a ~4 Hz. |
| R6 (mapa: un `UBorder` por rectángulo) | **NO** | `Interfaz/SAEVUSMapa.cpp` es idéntico a la v1 (`AnadirRectangulo:101`, `Refrescar:117-185`). `RecortarBloques` multiplica los rectángulos al trocearlos por celdas de 8 m. |
| M3 (la UI de juego la crea el «HUD de depuración»; depuración en Shipping) | **NO** (se decidió solo anotarlo) | `Nucleo/SAEVUSModoJuego.cpp:29` (`HUDClass = ASAEVUSHUDDepuracion`) y `Depuracion/SAEVUSHUDDepuracion.cpp:20-28`. Las CVars (`Depuracion/SAEVUSVariablesDepuracion.cpp:7-23`) y el enlace F6 (`Nucleo/SAEVUSControladorJugador.cpp:196-199`) no tienen `#if !UE_BUILD_SHIPPING`. Solo humo y recorrido se excluyen (`SAEVUSSubsistemaPruebaHumo.cpp:93`, `SAEVUSSubsistemaRecorrido.cpp:75`). |
| M4 (tecla `[E]` y rutas de mapa fijas) | **PARCIAL** | La tecla de interactuar se lee del mapeo (`WidgetInterfaz.cpp:465-485`). Problemas que quedan: descarta las teclas de mando (`:479`), conserva `"E"` por defecto (`WidgetInterfaz.h:304`, `EstadoInterfaz.h:49`) y los avisos del tutorial siguen con teclas escritas entre corchetes, que es la convención probada en `Pruebas/SAEVUSPruebasInterfaz.cpp:166`. Las rutas de mapa son `FString` editables pero con valor por defecto fijo (`Nucleo/SAEVUSModoJuego.h:66,70`, `ModoJuego.cpp:30`). |
| D8 (`VidaPorMascara` definido dos veces) | **RESUELTO** (queda un duplicado de respaldo) | Manda `FSAEVUSAjusteProgreso::VidaPorMascara` (`Personaje/SAEVUSPersonaje.h:55`), que lee `WidgetInterfaz.cpp:553-555`. `FSAEVUSAjusteInterfaz::VidaPorMascara` (`WidgetInterfaz.h:31`) solo se usa sin personaje; no se borra porque es una `UPROPERTY` serializada. |
| (otros) M1 (punteros crudos) | RESUELTO, comprobado con grep global | Ninguna cabecera fuera de `Pruebas/` declara miembros `U*/A*` crudos ni `TObjectPtr` sin `UPROPERTY`. Los miembros crudos de `Pruebas/SAEVUSMundoPrueba.h:257-268` son aceptables porque el mundo de prueba los mantiene vivos. |

## 4. Hallazgos

### IA-1 Partida corrupta sustituida en silencio y sobrescrita (sin versión ni copia de respaldo)
- **Estado:** CONFIRMADO
- **Prioridad:** P1
- **Severidad:** 5
- **Impacto en el jugador:** 5
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Nucleo/SAEVUSGuardado.cpp:17-28` (`Cargar`) y `Nucleo/SAEVUSGuardado.h:24-59` (sin campo de versión)
- **Qué pasa y por qué:** si la ranura existe pero `LoadGameFromSlot` devuelve nulo, se avisa en el log y se crea una partida vacía. Secuencia:
  1. Corte de luz o crash durante una escritura, lo que deja el `.sav` truncado.
  2. Al arrancar, la carga falla y se crea una partida nueva.
  3. A los 5 s, el subsistema de exploración (`Mundo/SAEVUSExploracion.cpp:111-114`) o cualquier cambio de opción llama a `Guardar()`.
  4. Las 30 horas de progreso desaparecen sin rastro.

  Además, `USAEVUSPartidaGuardada` no tiene número de versión, así que una migración futura no puede distinguir formatos.
- **Por qué importa:** es la peor experiencia posible en un juego de progreso. Es también un requisito de certificación en consola.
- **Solución propuesta (incremental):**
  1. Si existe y no se lee, copiar el archivo a `SAEVUS_corrupta_<fecha>` antes de seguir y mostrar un aviso.
  2. Añadir `UPROPERTY() int32 Version = 1` y una función `Migrar()` tras cargar.
  3. Guardar con rotación (`SAEVUS` y `SAEVUS_bak`): se escribe primero el respaldo y se intenta leer el respaldo si falla el principal.
- **Validación:** prueba automática que escribe bytes basura en `SAEVUS_Prueba`, llama a `Cargar` y comprueba que el archivo original sigue existiendo (copiado) y que el respaldo se usa.

### IA-2 Guardados asíncronos concurrentes sobre la misma ranura (+ guardado síncrono en `Deinitialize`)
- **Estado:** INFERIDO (comportamiento de `AsyncSaveGameToSlot` en UE5: serializa en el hilo de juego y escribe en una tarea)
- **Prioridad:** P1
- **Severidad:** 4
- **Impacto en el jugador:** 4
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Nucleo/SAEVUSGuardado.cpp:36-50` (`GuardarSinEsperar`), `:151-157` y `:78-85`. Este comportamiento es nuevo en la v2.
- **Qué pasa y por qué:** cada cambio lanza una escritura asíncrona independiente y no hay serialización entre ellas. Secuencia:
  1. Se recoge un fragmento (`GuardarFragmento`).
  2. En el mismo segundo se entra en una celda nueva y se toca un banco (`GuardarPuntoControl`).
  3. Salen dos o tres tareas que escriben el mismo archivo con `FFileHelper::SaveArrayToFile`, que no es atómico.
  4. El orden de finalización no está garantizado, así que la instantánea vieja puede quedar la última, o la escritura falla por violación de compartición.
  5. Si el jugador pulsa «Salir» justo después, `Deinitialize` escribe en síncrono mientras una tarea vieja sigue en vuelo, y esa tarea puede pisar el estado final.
- **Por qué importa:** pérdida intermitente de progreso, imposible de reproducir en QA.
- **Solución propuesta:**
  1. Un solo guardado en vuelo, con `bGuardandoAhora` y `bPendiente`; al terminar, si hay algo pendiente, se guarda otra vez.
  2. En `Deinitialize`, esperar la tarea en vuelo antes de la escritura síncrona.
  3. Agrupar cambios: guardar como mucho cada N segundos, salvo en puntos de control.
  4. Opcional: escribir en un temporal y renombrar.
- **Validación:** prueba que llama a `GuardarFragmento` y a `GuardarExploracion` diez veces seguidas, espera y comprueba que la ranura contiene el último estado. Log con un contador de guardados en vuelo.

### IA-3 Ajustes de prototipo: sin remapeo, categorías, volumen por canales ni opciones de accesibilidad
- **Estado:** CONFIRMADO
- **Prioridad:** P1
- **Severidad:** 4
- **Impacto en el jugador:** 5
- **Coste:** L
- **Riesgo de regresión:** bajo
- **Dónde:** `Interfaz/SAEVUSWidgetPausa.cpp:265-271` (solo Calidad, Sensibilidad y Volumen, y Volver), `Nucleo/SAEVUSControladorJugador.h:46-52` y `Nucleo/SAEVUSGuardado.h:30-35`
- **Qué pasa y por qué:** solo existen un nivel de escalabilidad global, una sensibilidad compartida entre ratón y stick (rotulada «del ratón», `WidgetPausa.cpp:286`) y un volumen maestro (`SetTransientPrimaryVolume`, `Guardado.cpp:57-63`).

  Falta todo esto:
  - remapeo (Enhanced Input User Settings);
  - invertir el eje Y;
  - sensibilidad separada para el mando;
  - volúmenes de música, efectos e interfaz (la música ya tiene `SOUNDGROUP_Music`, `SubsistemaMusica.cpp:102`);
  - resolución, modo de ventana, vsync, límite de fps, FOV y brillo;
  - tamaño de texto;
  - modo daltónico;
  - desactivar la sacudida de cámara (el ajuste existe, `Camara/SAEVUSAjustesCamara.h:95 bSacudida`, pero no se expone);
  - desactivar el destello rojo (`WidgetInterfaz.h:155-162`) y el hit-stop;
  - alternar en vez de mantener (guardia, curar, esprint, `Personaje/SAEVUSPersonaje.cpp:365-397`);
  - asistencias de dificultad;
  - idioma.

  Los valores se recorren pulsando, sin deslizadores.
- **Por qué importa:** en PC es lo mínimo esperable, y en un juego de reflejos con hit-stop y sacudidas también es un tema de salud (fotosensibilidad y mareo). La reseña lo castiga.
- **Solución propuesta:**
  1. Crear un `USAEVUSAjustesJuego : UGameUserSettings`, o ampliar `USAEVUSPartidaGuardada` con un bloque `Opciones`.
  2. Construir el menú por categorías (Juego, Gráficos, Audio, Controles, Accesibilidad, Idioma), como en el concepto, con filas «etiqueta + valor ◂ ▸» reutilizables.
  3. Usar `UEnhancedInputUserSettings` (UE 5.3+) para el remapeo.
  4. Crear 3 `USoundClass` y un mix con volumen por clase.
  5. Conectar `bSacudida` y el destello a opciones del jugador.

  Todo se puede hacer de forma incremental, una fila tras otra.
- **Validación:** pruebas puras de ida y vuelta de cada opción en la partida guardada. El recorrido abre Opciones y comprueba que cada fila cambia su valor.

### IA-4 Calidad «Épica» (valor por defecto del motor) rotulada «Alta», y pulsar la baja a «Baja»
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 3
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Interfaz/SAEVUSWidgetPausa.cpp:44-47` (`SiguienteCalidad`, `Clamp(...,0,2)`), `:68-79` (`NombreCalidad`, `default → "Alta"`) y `:357-366`
- **Qué pasa:** `GetOverallScalabilityLevel()` vale 3 (Épica) en una instalación nueva sin benchmark y 4 en Cinemática. Secuencia:
  1. El menú muestra «Calidad: Alta», cuando en realidad está en Épica.
  2. El jugador pulsa: `Clamp(3)=2`, y `(2+1)%3=0` lo lleva a Baja.
  3. El salto Épica→Baja es brusco, la etiqueta mentía y no hay forma de volver a Épica.
- **Solución propuesta:** usar 4 o 5 niveles (`Baja`…`Épica`), mostrar «Personalizada» con −1 y añadir una opción de autodetección (`RunHardwareBenchmark`).
- **Validación:** extender `SAEVUS.Interfaz.OpcionesPausa` con `SiguienteCalidad(3)` y `NombreCalidad(3)`.

### IA-5 Notificaciones, recompensas y tutorial comparten una única ranura que se pisa
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 4
- **Coste:** M
- **Riesgo de regresión:** medio
- **Dónde:** `Tutorial/SAEVUSSubsistemaTutorial.cpp:6-14` (un solo `Mensaje`; uno nuevo sustituye al anterior)
- **Quién la usa:** `Personaje/SAEVUSPersonaje.cpp:235`, `Mundo/SAEVUSBanco.cpp:112`, `Mundo/SAEVUSPalanca.cpp:90`, `Tutorial/SAEVUSPuertaArena.cpp:89` y `Tutorial/SAEVUSVolumenTutorial.cpp:11`
- **Qué pasa:**
  - Si se recoge un fragmento dentro de un volumen de tutorial, o se abre una puerta de arena justo al cruzar un aviso, el mensaje anterior desaparece sin haberse leído.
  - «Fragmento de máscara» se ve igual que un aviso de tutorial: mismo panel arriba al centro, sin icono ni sonido propio.
  - Un aviso con `Duracion = 0` se queda hasta el siguiente, aunque haya combate.
- **Por qué importa:** los descubrimientos (fragmento, ruta abierta) son los picos de recompensa del juego y ahora no se celebran ni se distinguen. El concepto pide un feed de notificaciones con iconos.
- **Solución propuesta:**
  - Separar en un `USAEVUSSubsistemaNotificaciones` con cola, prioridad, tipo (Objeto, PuntoControl, Ruta, Códice) e icono en rombo.
  - Dejar `SubsistemaTutorial` solo para la enseñanza y mantener su API.
  - El HUD muestra las notificaciones abajo a la derecha y apiladas.
- **Validación:** prueba con 3 notificaciones seguidas que comprueba que las 3 se muestran en orden y que un aviso de tutorial no las borra.

### IA-6 El progreso de vida (máscaras) es invisible: la barra tiene tamaño fijo
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 4
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:**
  - `Interfaz/SAEVUSWidgetInterfaz.h:35` (`TamanoBarraVida 380×11`, fijo);
  - `WidgetInterfaz.cpp:413-419`: las `Mascaras` del estado se suman en una sola fracción;
  - `Personaje/SAEVUSPersonaje.cpp:236`: el mensaje dice «una máscara de vida más».
- **Qué pasa:** al completar una máscara, la vida máxima sube de 100 a 120 y la barra sigue midiendo 380 px y llena. El jugador no ve nada distinto, porque el mismo golpe ahora quita menos barra. El array `Mascaras` (`EstadoInterfaz.h:18`) se calcula y no se dibuja.
- **Por qué importa:** la progresión es el bucle de recompensa de un metroidvania. Sin una señal visible, un fragmento no se siente como una recompensa.
- **Solución propuesta:** ancho de barra = `TamanoBarraVida.X * VidaMaxima / VidaMaximaBase`, con tope, o marcas finas cada `VidaPorMascara` dentro de la barra. Animar el crecimiento la primera vez.
- **Validación:** prueba de widget que llama a `Aplicar` con 5 y luego con 6 máscaras y comprueba el ancho del `SizeBox` o el número de marcas.

### IA-7 Muerte sin elección y sin pantalla de victoria (concepto: «HAS MUERTO / Reintentar / Volver al menú», «VICTORIA»)
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 4
- **Coste:** M
- **Riesgo de regresión:** medio
- **Dónde:** `Interfaz/SAEVUSWidgetInterfaz.cpp:341-364` y `:570-583` (texto «Has caído», `:357`), `Nucleo/SAEVUSModoJuego.cpp:91-92` (reaparición automática a los `RetardoReaparicion = 3 s`). Un grep de `Victoria|derrotad` no da nada fuera del log del recorrido.
- **Qué pasa:** al morir, a los 3 s se reaparece en el banco sin intervención. No hay «Reintentar» ni «Volver al menú». Al derrotar a un jefe no hay rótulo ni recompensa: solo desaparece la barra (`EstadoInterfaz.cpp:84`).
- **Por qué importa:** la muerte y la victoria contra un jefe son los dos momentos más memorables de un souls-like. Ahora pasan sin ceremonia. La reaparición forzada a los 3 s también impide un respiro después de una muerte frustrante.
- **Solución propuesta:**
  1. Que `ModoJuego` no reaparezca solo, sino que espere a «Reintentar» (o lo haga tras X s si así se configura).
  2. Botones en el velo, reutilizando `CrearBoton` de la pausa.
  3. Rótulo de jefe derrotado al recibir `AlMorirEnemigo` con `bEsJefe`: nombre y «derrotado»/«vencido» en serif con fundido, y un sonido grave.
- **Validación:** el humo comprueba que tras morir aparece «Reintentar» con el foco y que pulsarlo reaparece.

### IA-8 Foco del mando perdido en el menú principal: el jugador con solo mando queda atascado
- **Estado:** INFERIDO
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 4
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Nucleo/SAEVUSControladorJugador.cpp:44-63` (`FInputModeGameAndUI`), `:25-30` (la pausa no cierra el principal) e `Interfaz/SAEVUSWidgetPausa.cpp:139-152` (`NativeTick` solo cambia el estilo)
- **Qué pasa:**
  1. Steam Deck o mando en PC: un toque en la pantalla o un clic fuera de un botón, o volver con Alt-Tab, quita el foco del usuario al botón.
  2. Ningún código lo devuelve.
  3. La cruceta ya no navega, y la tecla de pausa no hace nada en el menú principal.
  4. Sin ratón, no hay salida.

  En Opciones tampoco hay «atrás» con B o Esc: Esc cierra la pausa entera (`AlternarPausa`) y en el principal no hace nada.
- **Solución propuesta:** en `NativeTick`, si `!TieneFocoAlgunBoton()` y se recibe entrada de mando (o no hay cursor), aplicar `ObtenerPrimerBoton()->SetUserFocus`. Añadir `NativeOnKeyDown` para `Gamepad_FaceButton_Right` y `Escape`: en Opciones vuelve atrás y en la pausa continúa.
- **Validación:** el humo quita el foco (`FSlateApplication::ClearUserFocus`), simula una cruceta y comprueba que vuelve el foco.

### IA-9 Teclas fijas: con mando se muestra «[E]» y los textos del tutorial llevan teclas escritas
- **Estado:** CONFIRMADO (código) / INFERIDO (contenido del mapa)
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 3
- **Coste:** M
- **Riesgo de regresión:** bajo
- **Dónde:** `WidgetInterfaz.cpp:477-484` (se salta `IsGamepadKey`), `WidgetInterfaz.h:304`, `EstadoInterfaz.h:49`, la convención `[W] [A]` de `Pruebas/SAEVUSPruebasInterfaz.cpp:166` y `Tutorial/SAEVUSVolumenTutorial.h:22-25`
- **Qué pasa:** con mando, el aviso de interacción sigue mostrando la tecla de teclado. Los avisos del tutorial, escritos en el mapa con teclas literales, no cambian con un remapeo ni con el mando.
- **Solución propuesta:**
  - Marcado por acción en vez de por tecla: `{Saltar}` y `{Dash}`, resueltos por `TrocearTeclas` mediante `QueryKeysMappedToAction` según el último dispositivo usado (hay que detectar el dispositivo activo).
  - Glifos de mando.
  - Los textos `[X]` antiguos siguen funcionando.
- **Validación:** prueba pura `ResolverAcciones("{Saltar}") → "Space" | "Gamepad_FaceButton_Bottom"`.

### IA-10 El HUD de depuración y sus atajos llegan a Shipping (M3)
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 2
- **Impacto en el jugador:** 2
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Depuracion/SAEVUSHUDDepuracion.cpp:31-42,71-154`, `Depuracion/SAEVUSVariablesDepuracion.cpp:7-23` (`ECVF_Default`, no `ECVF_Cheat`), `Nucleo/SAEVUSControladorJugador.cpp:196-199` (F6, si el IMC lo mapea, lo cual es DESCONOCIDO) y `Mundo/SAEVUSMunecoEntrenamiento.cpp:87-89`
- **Qué pasa:** en la versión final, un jugador con F6 mapeado (o una CVar en un `.ini`) ve texto verde de depuración y las lecturas de los muñecos. Toda la clase de la UI de juego vive en «Depuración».
- **Solución propuesta (incremental):**
  - Envolver `DibujarDepuracion`, `DibujarNumerosDanio` y el enlace de `AccionAlternarHUD` en `#if !UE_BUILD_SHIPPING`.
  - Usar el flag `ECVF_Cheat` en las CVars.
  - Más adelante, crear `Interfaz/SAEVUSHUD` (HUD de juego) y dejar `HUDDepuracion` como subclase solo de desarrollo. `HUDClass` es una propiedad del modo de juego, así que cambiarla no rompe assets si el BP no la fija.
- **Validación:** compilar en Shipping y comprobar con `stat`/`nm` que no hay símbolos de `DibujarDepuracion`. Probar a pulsar F6 en el build.

### IA-11 Barridos por fotograma en el HUD y la UI (R4 parcial)
- **Estado:** CONFIRMADO
- **Prioridad:** P3 (P2 a ×10)
- **Severidad:** 2
- **Impacto en el jugador:** 1
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `EstadoInterfaz.cpp:22,34,45` (`FindComponentByClass`), `:81-98` (`TActorIterator` sobre todos los enemigos), `:43,56-57` (`FText::Format` por fotograma) y `WidgetInterfaz.cpp:555`
- **Qué pasa:** con 20 enemigos el coste es despreciable, del orden de microsegundos. Con 200 son 200 desreferencias y una lista temporal del iterador cada fotograma, más asignaciones de `FText`.
- **Solución propuesta:**
  - Cachear `Vida`/`Alma`/`Armas` del pawn y renovar la caché en `OnPossessedPawnChanged`.
  - Un registro de jefes activos en un subsistema, alimentado por `AlCambiarEstado` de la IA. Ese mismo registro sirve a la música (`SubsistemaMusica.cpp:133`) y a la mirada (`ComponenteAnimacionProcedural.cpp:1147`).
  - Formatear la munición solo cuando cambien los números.
- **Validación:** `stat game`/Insights con 200 enemigos de prueba. Contador de llamadas en una prueba.

### IA-12 Mapa de pausa: widgets por rectángulo, recolectados del mundo, incompatible con World Partition (R6)
- **Estado:** CONFIRMADO (código) / INFERIDO (×10)
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 2
- **Coste:** M
- **Riesgo de regresión:** bajo
- **Dónde:** `Interfaz/SAEVUSMapa.cpp:36-56` (`TActorIterator<AStaticMeshActor>`), `:117-185` (`ClearChildren` y un `UBorder` por trozo) y `Mundo/SAEVUSExploracion.cpp:23-49`
- **Qué pasa:** al abrir la pausa se recorren todos los StaticMeshActor y se crea un widget por trozo de bloque y celda. En un mapa diez veces mayor son miles de `UBorder`, con un tirón al pausar y más memoria de Slate. Con streaming o World Partition, las celdas no cargadas no aparecen y el mapa sale incompleto.

  Funcionalmente:
  - no hay pisos (los niveles superpuestos se tapan por orden de altura, `:155`);
  - no hay leyenda, zoom ni orientación del jugador;
  - los bancos no se marcan (solo `ASAEVUSPuntoControl`, `:171`).
- **Solución propuesta:**
  1. Dibujar en `NativePaint` con `FSlateDrawElement::MakeBox` desde un array cacheado.
  2. Hornear la geometría del mapa en un data asset por mapa con el script de assets, y abandonar la recolección en tiempo de ejecución.
  3. Añadir capas por altura (pisos) y un icono de banco.
- **Validación:** prueba con 5000 bloques sintéticos que compruebe que `Refrescar` dura menos de 2 ms. Captura del recorrido.

### IA-13 Componente de animación como nudo central (God component) de jugador y enemigos
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 1
- **Coste:** L
- **Riesgo de regresión:** medio
- **Dónde:** `Animacion/SAEVUSComponenteAnimacionProcedural.cpp` (1367 líneas), `.h` (592 líneas, 95 `UPROPERTY`) y `Animacion/SAEVUSAnimacionEnemigo.cpp` (303 líneas, que implementa `ConstruirPoseEnemigo` de la misma clase). Busca 7 componentes ajenos (`.cpp:216-221`).
- **Qué pasa:** el mismo componente sirve al jugador y a los enemigos, observa movimiento, combate, armas, vida, alma, guardia e IA, y mezcla en una sola clase el solver, la capa secundaria y las poses. No es un problema de rendimiento (R1 se trató en su área), sino de cambio: cada nuevo tipo de enemigo o gesto toca el archivo más grande del proyecto. El personaje (575 líneas) es una raíz de composición razonable y **no** es un God object.
- **Solución propuesta (sin dogma, por pasos):**
  1. Crear un struct `FSAEVUSEntradaAnimacion` que el dueño rellena (velocidad, fase de ataque, estado de la IA…), siguiendo el patrón de `FSAEVUSEstadoInterfaz`, ya probado.
  2. Separar `ConstruirPoseJugador` y `ConstruirPoseEnemigo` en dos clases «constructor de pose» que comparten `FPose`, solver y muelles.
  3. Así se quitan los `FindComponentByClass` cruzados.
- **Validación:** las 13+7 pruebas de animación existentes deben seguir pasando sin cambios.

### IA-14 Ciclos de dependencia entre carpetas y un único módulo
- **Estado:** CONFIRMADO
- **Prioridad:** P3
- **Severidad:** 2
- **Impacto en el jugador:** 0
- **Coste:** S–M
- **Riesgo de regresión:** bajo
- **Dónde:**
  - `Mundo/SAEVUSExploracion.cpp:7` incluye `Interfaz/SAEVUSMapa.h`, y `Interfaz/SAEVUSMapa.cpp:16` incluye `Mundo/SAEVUSExploracion.h`, por `FSAEVUSBloqueMapa`.
  - `Nucleo/SAEVUSModoJuego.cpp:4` incluye `Depuracion/SAEVUSHUDDepuracion.h`, que incluye `Interfaz/`.
  - `Combate/SAEVUSComponenteGuardia.cpp:155` conoce `USAEVUSComponenteIAEnemigo`.
  - `SAEVUS.Build.cs:14-34` es un solo módulo, con las pruebas (6744 líneas, un 24 % del total) dentro.
- **Por qué importa:** con diez veces más contenido, los tiempos de compilación crecen y no se pueden aislar sistemas, por ejemplo un módulo de editor para herramientas de mapas.
- **Solución propuesta:**
  - Mover `FSAEVUSBloqueMapa` y `Encuadrar`/`RecogerBloques` a `Mundo/SAEVUSGeometriaMapa.h`; la Interfaz solo dibuja.
  - Mover el HUD de juego a `Interfaz/` (ver IA-10).
  - Más adelante, `SAEVUSEditor` (herramientas de mapa y horneado del minimapa) y quizá `SAEVUSPruebas` como módulo `DeveloperTool`.

  Ninguno de estos cambios es urgente.
- **Validación:** script que construye el grafo de `#include` por carpeta y falla si hay ciclos.

### IA-15 Herramientas de QA (humo y recorrido) frágiles: débiles sin comprobar y mapa lineal en +X
- **Estado:** CONFIRMADO
- **Prioridad:** P3
- **Severidad:** 3 (crash de la herramienta)
- **Impacto en el jugador:** 0
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:**
  - `Depuracion/SAEVUSSubsistemaRecorrido.cpp:303-307` (`Avisos[Indice].Get()->` sin `IsValid`), `:537-541` (`Palanca`), `:460-465` (`Pared`), `:616` (`Puertas[Indice - 1]->`), `:153-156`/`:168-191` (ordenación con `A->` dentro del predicado);
  - `SAEVUSSubsistemaPruebaHumo.cpp:249` (`ObtenerEstelaDash()->` sin nulo).
- **Qué pasa:**
  - Si una pared rota o una palanca se destruye (`SetLifeSpan`, streaming) o el jugador cae, el recorrido revienta con un acceso a nulo en vez de informar del FALLO.
  - El recorrido supone un nivel lineal en +X (ordena por X y coloca al jugador en `Y = 0`, `:594`), así que no sirve para la Mazmorra ni para mapas no lineales.
  - `PruebaHumo.cpp:320,466` dicen «Jugar», pero ese botón ya no existe (ahora es «Tutorial»).
- **Solución propuesta:** usar `if (!X.IsValid()) { Comprobar(..., false); ++Indice; return; }` en cada paso. Colocar al jugador junto a cada objetivo usando su transform, en vez de X/Y fijos. Actualizar los textos.
- **Validación:** ejecutar el recorrido en `L_Mazmorra` con `-SAEVUSRecorrido`.

### IA-16 Pruebas: huecos de cobertura y fragilidad ante assets
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 2
- **Coste:** M
- **Riesgo de regresión:** bajo
- **Dónde:** `Pruebas/` (171 `IMPLEMENT_SIMPLE_AUTOMATION_TEST`), `SAEVUSPruebasGuardado.cpp:19-51` (una sola prueba de 5 campos), `SAEVUSPruebasEquilibrio.cpp:21-38`, `SAEVUSPruebasEnemigos.cpp:446,497,541`, `SAEVUSPruebasAnimacion.cpp:16` y `SAEVUSPruebasRevision.cpp:422` (dependen de `/Game/...`), y `SAEVUSPruebasObstaculos.cpp:16-46` (copia a mano las medidas del mapa)
- **Qué falta:**

  | Sistema | Cobertura hoy | Lo que falta |
  |---|---|---|
  | Widgets (HUD y pausa) | Estado puro, sí. `Construir`/`Aplicar`/`AnimarBarras`/velo, no. | Una prueba que instancie el widget y llame a `Aplicar` con estados clave. |
  | Guardado | Ida y vuelta de 5 campos | `PuntosControl`, `Exploracion`, `Fragmentos`, partida corrupta, versión antigua, concurrencia (IA-1/2). |
  | Rendimiento | Solo informativo en humo y recorrido (`PruebaHumo.cpp:500-510`, `Recorrido.cpp:96-101`) | Umbral de ms/fotograma comparado con una línea base guardada, y un escenario de estrés con N enemigos. |
  | Regresión de contenido | Equilibrio y obstáculos | Las medidas de obstáculos duplican las del mapa y no se leen del mapa. Si un asset falta, falla con «créalo con Verificar-Arranque.ps1», que en CI sin Content es ruido. |
  | Localización | Nada | Prueba que recorre los `FText` de los data assets y detecta `FText::FromString` no localizables. |
  | Menús | Funciones puras | Navegación con mando y foco (IA-8). |

- **Solución propuesta:**
  - Separar las pruebas en dos filtros: `SAEVUS.Unit.*` (sin assets) y `SAEVUS.Contenido.*` (con assets, solo en CI con Content).
  - Leer las medidas de los obstáculos de etiquetas del mapa.
  - Añadir las pruebas de la tabla por orden de riesgo: guardado, widget, rendimiento.
- **Validación:** que la ejecución `Automation RunTests SAEVUS.Unit` pase en una máquina sin Content.

### IA-17 Sin CI/CD visible, crash reporting ni telemetría; logging mínimo
- **Estado:** DESCONOCIDO (fuera de `Source/`) / CONFIRMADO (en código)
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 2
- **Coste:** M
- **Riesgo de regresión:** bajo
- **Dónde:** se citan los scripts `Herramientas/Ejecutar-Recorrido.ps1` (`Depuracion/SAEVUSSubsistemaRecorrido.h:19`), `Verificar-Arranque.ps1` (`Pruebas/SAEVUSPruebasEnemigos.cpp:448`) y `Generar-Caballero.ps1` (`Pruebas/SAEVUSPruebasAnimacion.cpp:56`), pero no se ven. Hay una sola categoría de log, `LogSAEVUS` (`SAEVUS.cpp:8`), y 10 `UE_LOG` en código de juego. No hay `ensure`/`check`, el grep da 0.
- **Por qué importa:** sin reportes de fallos ni datos de dónde muere la gente, el equilibrado y el QA a 100 horas van a ciegas.
- **Solución propuesta:**
  1. Pipeline con BuildCookRun (Development y Shipping), las pruebas `SAEVUS.Unit`, el humo y el recorrido con `-nullrhi`/`-RenderOffscreen` y los artefactos de captura.
  2. Activar el CrashReportClient o un backend tipo Sentry.
  3. Telemetría local opcional en JSON por sesión (muertes por zona y causa, tiempo por arena, uso de curas), que sirve de base para el equilibrado.
  4. Categorías de log por sistema (`LogSAEVUSGuardado`, `…IA`) y `ensureMsgf` en invariantes como nulos de componentes obligatorios.
- **Validación:** el pipeline en verde; un crash provocado aparece en el backend.

### IA-18 Localización: nombres de arma no traducibles, namespaces incoherentes, texto con marcado, sin idioma
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 2
- **Impacto en el jugador:** 3
- **Coste:** M
- **Riesgo de regresión:** medio (cambiar `FName` a `FText` exige un campo nuevo)
- **Dónde:**
  - `Armas/SAEVUSTiposArma.h:74` (`FName NombreVisible`, mostrado con `FText::FromName` en `EstadoInterfaz.cpp:50`);
  - `WidgetInterfaz.cpp:461` (`NSLOCTEXT("SAEVUS", "FaseJefe", ...)`, frente a `"SAEVUSInterfaz"` en el resto del HUD, `:357` y `EstadoInterfaz.cpp:17`);
  - `EstadoInterfaz.cpp:43` (`"[{0}]  {1}"` reparsea corchetes: si un nombre traducido lleva `[`, se dibuja como tecla);
  - `Mundo/SAEVUSMunecoEntrenamiento.cpp:89` (`FString::Printf`, que es de depuración);
  - fuente Roboto (`Interfaz/SAEVUSEstiloInterfaz.cpp:54-72`).
- **Solución propuesta:**
  - Nuevo campo `FText NombreMostrado` en `USAEVUSDefinicionArma`; si está vacío, se usa el `FName`, sin borrar el campo actual.
  - Un namespace por carpeta, o una string table `ST_SAEVUS_Interfaz`.
  - Escapar los corchetes de los nombres o usar el marcado de acción (IA-9).
  - Selector de idioma en Opciones (`FInternationalization::SetCurrentCulture`).
  - Fuente serif propia con cobertura de los caracteres españoles (á é í ó ú ñ ¿ ¡ ∞).
- **Validación:** gather de localización sin advertencias y una pseudo-localización (`-culture=en-US-POSIX`/«ensanchada») para detectar cortes.

### IA-19 Identidad: terminología y números calcados de Hollow Knight visibles al jugador
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 2
- **Impacto en el jugador:** 3
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:**
  - `EstadoInterfaz.cpp:50`: «Aguijón» es la traducción oficial al español del *Nail*.
  - `Personaje/SAEVUSPersonaje.cpp:235-237`: máscaras y fragmentos de máscara.
  - `Mundo/SAEVUSBanco.cpp:83,112`: banco, «Sentarse», «Descansas».
  - `Combate/SAEVUSComponenteAlma.h:22,26,30`: Alma 99, 11 por golpe, 33 por cura, idénticos al Soul de HK.
  - Comentarios como `(Hollow Knight)` en `Mundo/SAEVUSExploracion.h:29` e `Interfaz/SAEVUSWidgetInterfaz.h:98`.
- **Por qué importa:** el dueño pide identidad propia y referencia de calidad «sin copiar». Un jugador de HK reconoce al instante «aguijón + máscaras + alma 33/99 + bancos», y la prensa también.
- **Solución propuesta:** decisión narrativa (ver el informe de narrativa). Por ejemplo, la espada con nombre propio del mundo de SAEVUS, las «máscaras» sustituidas por algo ligado al emblema o la máscara del protagonista con otro nombre, los bancos convertidos en braseros o hitos, y valores de alma propios. Solo afecta a textos y datos.
- **Validación:** revisión de `textos.md` frente a un glosario propio.

### IA-20 Sin «Nueva partida», «Continuar» no es el foco inicial y la ruta guardada no se valida
- **Estado:** CONFIRMADO
- **Prioridad:** P2
- **Severidad:** 3
- **Impacto en el jugador:** 3
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Interfaz/SAEVUSWidgetPausa.cpp:242-251` (Tutorial, Mazmorra, Continuar), `:154-163` (el primer visible es «Tutorial») y `Nucleo/SAEVUSControladorJugador.cpp:139-161`
- **Qué pasa:**
  - No hay forma de borrar el progreso (fragmentos, exploración, puntos) ni varias ranuras: un segundo jugador del mismo PC hereda la partida.
  - Con partida guardada, el foco inicial cae en «Tutorial», no en «Continuar», que en el concepto va primero.
  - Si el mapa del último punto se renombra entre versiones, `OpenLevel` va a un mapa inexistente.
  - «Menú principal» y «Salir» no piden confirmación.
- **Solución propuesta:** orden Continuar, Nueva partida (con confirmación y borrado de ranura), Tutorial/Mazmorra (mientras sea un vertical slice), Opciones, Créditos y Salir. Validar el mapa con `FPackageName::DoesPackageExist`.
- **Validación:** prueba pura de orden y visibilidad de botones con y sin partida.

### IA-21 UI sin zona segura, ultrapanorámico ni escala de texto
- **Estado:** CONFIRMADO (código) / DESCONOCIDO (curva DPI en Config)
- **Prioridad:** P2
- **Severidad:** 2
- **Impacto en el jugador:** 3
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `WidgetInterfaz.cpp:199-203,240-245` (anclas a esquinas con `Margen` fijo de 48×40), sin `USafeZone` (el grep da 0); los tamaños de letra son fijos (`WidgetInterfaz.h:79-86`, `WidgetPausa.cpp:28-35`).
- **Qué pasa:**
  - En 32:9, las vitales y el arma quedan en los extremos, fuera de la visión periférica.
  - En TV con overscan pueden cortarse.
  - Con letra de 18 px a 1080p, el texto es pequeño en una Steam Deck.
- **Solución propuesta:** envolver el lienzo en `USafeZone`; un ancho máximo de HUD de 16:9 centrado (opción «HUD centrado»); multiplicador de tamaño de texto en Opciones aplicado en `FuenteTexto`/`FuenteTitulo`.
- **Validación:** capturas del humo con `-ResX=3840 -ResY=1080` y 1280×800.

### IA-22 Barra de jefe: un solo jefe, rastro falso al reaparecer y sin presentación
- **Estado:** CONFIRMADO
- **Prioridad:** P3
- **Severidad:** 2
- **Impacto en el jugador:** 3
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `EstadoInterfaz.cpp:79-98` (el jefe activo más cercano) y `WidgetInterfaz.cpp:519-527` (el rastro se reinicia a 1.0 al ocultarse)
- **Qué pasa:**
  - Dos jefes a la vez (dúo o arena doble) muestran una sola barra que salta entre ellos según la distancia, con el rastro saltando también.
  - Si el jefe herido vuelve a reposo y se reactiva, la barra aparece llena en color rastro y se «vacía» hasta su vida real: un daño fantasma.
  - El nombre aparece sin fundido ni rótulo de entrada.
- **Solución propuesta:** una lista de jefes registrados (IA-11) con hasta 2 barras. Al mostrarla, inicializar `FraccionRastroJefe` con la fracción actual. Fundido del nombre en 0.6 s con un golpe de música (ya existe la pista del jefe).
- **Validación:** prueba de `AnimarBarras` con el jefe al 50 % que aparece y comprueba que el rastro es 0.5.

### IA-23 Sensibilidad guardada sin acotar; stick de cámara posiblemente dependiente del framerate
- **Estado:** CONFIRMADO (lo primero) / INFERIDO (lo segundo)
- **Prioridad:** P3
- **Severidad:** 2
- **Impacto en el jugador:** 2
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Nucleo/SAEVUSControladorJugador.cpp:241-249` (guarda `Nueva`, no `Sensibilidad`; M8 parcial) y `Personaje/SAEVUSPersonaje.cpp:458-467` (`AddControllerYawInput(Eje.X)` sin `DeltaTime`)
- **Qué pasa:** un valor fuera de rango se persiste (al cargar se recorta, así que no es grave). Si el IMC del stick no tiene el modificador «Scale By Delta Time» (DESCONOCIDO), la cámara gira el doble de rápido a 120 fps que a 60.
- **Solución propuesta:** guardar `Sensibilidad`. Escalar en código solo la acción del stick con `GetWorld()->GetDeltaSeconds() * VelocidadGiroStick`, con sensibilidad separada.
- **Validación:** prueba que simula 1 s de stick a 30 y a 120 fps y compara los grados girados.

### IA-24 Caché de síntesis y sonidos: estado estático sin límite y un `UObject` por reproducción
- **Estado:** CONFIRMADO
- **Prioridad:** P3
- **Severidad:** 2
- **Impacto en el jugador:** 1
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Audio/SAEVUSSintesis.cpp:58-77` (`static TMap<uint32, TArray<uint8>>` con clave de 32 bits y copia del array en cada consulta), `Audio/SAEVUSComponenteSonido.cpp:~249` (`CrearOnda` hace `NewObject<USoundWaveProcedural>` en cada reproducción)
- **Qué pasa:**
  - La caché vive todo el proceso, también entre sesiones PIE.
  - Una colisión de huella hace que suene otro sonido (poco probable).
  - Con diez veces más enemigos golpeando son decenas de `UObject` por segundo, lo que presiona al GC y causa microtirones.
- **Solución propuesta:** `TSharedPtr<const TArray<uint8>>` en la caché, con la huella combinada con `Duracion` en 64 bits. Un pool de `UAudioComponent` y ondas por efecto, reutilizado.
- **Validación:** `obj list class=SoundWaveProcedural` tras 60 s de combate.

### IA-25 Exploración guardada cada 5 s (serializa la partida entera en el hilo de juego)
- **Estado:** CONFIRMADO
- **Prioridad:** P3
- **Severidad:** 2
- **Impacto en el jugador:** 1
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Mundo/SAEVUSExploracion.h:19` (`IntervaloGuardado = 5.f`, R2 no aplicado) y `Mundo/SAEVUSExploracion.cpp:96-129` (`Celdas.Array()` completo)
- **Qué pasa:** la escritura es asíncrona, pero `SaveGameToMemory` de toda la partida ocurre en el hilo de juego cada 5 s mientras se explora. En mapas diez veces mayores son miles de celdas, cada vez, y alimenta la carrera de IA-2.
- **Solución propuesta:** 30 s, más al tocar un banco, pausar o salir (lo que ya se decidió en R2).
- **Validación:** Insights durante un sprint por un mapa grande.

### IA-26 Fuerte acoplamiento al «primer jugador» y a mundos de un solo jugador
- **Estado:** CONFIRMADO
- **Prioridad:** P4
- **Severidad:** 1
- **Impacto en el jugador:** 0
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `Mundo/SAEVUSExploracion.cpp:98`, `Audio/SAEVUSSubsistemaMusica.cpp:127` y `Enemigos/SAEVUSComponenteIAEnemigo.cpp:68` (`GetPlayerPawn(0)`/`GetFirstPlayerController`), y el delegado estático `ASAEVUSEnemigo::AlMorirEnemigo` (`Enemigos/SAEVUSEnemigo.h:60`, M2 aceptado)
- **Qué pasa:** no es un problema para un juego de un jugador. El delegado estático cruza mundos en PIE con varios clientes o con pruebas simultáneas; `PuertaArena` filtra por su lista (`Tutorial/SAEVUSPuertaArena.cpp:66-71`), así que hoy es inocuo.
- **Solución propuesta:** dejarlo, y no añadir más estado global. Si se hace un registro de enemigos (IA-11), mover el evento a un subsistema de mundo.

### IA-27 Velo de muerte: `OpacidadMuerte` no controla la opacidad final
- **Estado:** CONFIRMADO
- **Prioridad:** P4
- **Severidad:** 1
- **Impacto en el jugador:** 1
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** `WidgetInterfaz.cpp:344` (color con alfa 0.6 fijo), `:353` (banda con alfa 0.8) y `:577` (la opacidad se normaliza dividiendo por `OpacidadMuerte`)
- **Qué pasa:** el ajuste solo cambia la velocidad del fundido. Quien lo toque para oscurecer más no verá efecto.
- **Solución propuesta:** `VeloMuerte->SetBrushColor(FLinearColor(0,0,0,Ajuste.OpacidadMuerte))` y la opacidad de render de 0 a 1.

### IA-28 Finales de línea mezclados y deuda menor
- **Estado:** CONFIRMADO
- **Prioridad:** P3
- **Severidad:** 1
- **Impacto en el jugador:** 0
- **Coste:** S
- **Riesgo de regresión:** bajo
- **Dónde:** 105 archivos CRLF y 71 LF en `Source/` (el diff v1→v2 de `Depuracion/SAEVUSHUDDepuracion.cpp` es el archivo entero sin cambios reales).
- **Comentarios obsoletos:**
  - `EstadoInterfaz.h:20` («el orbe brilla», ya no hay orbe);
  - `EstadoInterfaz.h:27` («"E  Sentarse"»);
  - `PruebaHumo.cpp:320,466` («Jugar»);
  - `WidgetInterfaz.h:183-184` («Una subclase Blueprint puede sustituirlo», aunque `Construir` no deja hueco para un árbol de diseñador porque solo construye si `RootWidget` es nulo; lo permite si el BP tiene árbol, así que es correcto pero frágil).
- **Funciones largas:** `WidgetInterfaz::Construir` tiene 181 líneas y `PruebaHumo::Tick` 367.
- **Solución propuesta:** `.gitattributes` con `* text=auto eol=crlf` para `*.h`/`*.cpp`/`*.cs` y una normalización en un solo commit. Partir `Construir` en `ConstruirVitales`/`ConstruirJefe`/`ConstruirMuerte`/`ConstruirZona`.

## 5. Inmersión

| Elemento | Nivel | Dónde |
|---|---|---|
| Muerte sin ceremonia ni elección (reaparición a los 3 s) | HIGH | `Nucleo/SAEVUSModoJuego.cpp:91`, `WidgetInterfaz.cpp:341-364` |
| Victoria contra un jefe sin rótulo ni recompensa | HIGH | sin código (IA-7) |
| Texto del banco que habla del menú: «Si caes, vuelves aquí; "Continuar" también.» | MEDIUM | `Mundo/SAEVUSBanco.cpp:112` |
| Fragmentos y descubrimientos anunciados como si fueran tutorial | MEDIUM | `Personaje/SAEVUSPersonaje.cpp:235` (IA-5) |
| Tipografía Roboto del motor en lugar de la serif del concepto (títulos «grabados») | MEDIUM | `Interfaz/SAEVUSEstiloInterfaz.cpp:54-72` |
| Paleta de los menús azul-blanco fría (`0.85, 0.9, 1`) frente al hueso y oro viejo de la dirección de arte | MEDIUM | `WidgetPausa.cpp:38`, `WidgetInterfaz.h:73`, `EstiloInterfaz.cpp:10-12` |
| «Aguijón», «máscaras», «alma», «bancos»: terminología ajena que rompe la identidad | MEDIUM | IA-19 |
| Subtítulo de zona meta («Zona 1») | LOW | `Mundo/SAEVUSInfoZona.h:22` (dato de mapa) |
| Texto de tutorial con teclas de teclado mientras se juega con mando | MEDIUM | IA-9 |
| Destello rojo a pantalla completa en cada golpe (sin opción de quitarlo) | LOW | `WidgetInterfaz.cpp:513-539` |
| Menú principal sobre el mapa del tutorial cargado (el HUD se ve por debajo) | LOW | `Nucleo/SAEVUSModoJuego.cpp:30`, `ControladorJugador.cpp:50` (orden Z 1) |
| HUD de depuración accesible con F6 en la versión final | LOW | IA-10 |

## 6. Escala ×10 (enemigos, mapas, contenido)

**Coste por fotograma con 10× enemigos** (supuesto: arena con unos 30 y mapa con unos 200):

| Fuente | Dónde | Hoy | ×10 | Comentario |
|---|---|---|---|---|
| Tick por enemigo: `CharacterMovement` + IA + animación (25 huesos) + sonido + partículas | `Enemigos/`, `Animacion/` | dominante | **dominante** (lineal) | La animación ya se salta fuera de pantalla (`ComponenteAnimacionProcedural.cpp:348`). La IA y el movimiento no tienen LOD de tick. Es el primer cuello. |
| HUD: `TActorIterator<ASAEVUSEnemigo>` + 3 `FindComponentByClass` por fotograma | `EstadoInterfaz.cpp:22-98` | ~µs | decenas de µs | Barato pero inútil; usar un registro. |
| Mirada del jugador: barrido de enemigos a 5 Hz | `ComponenteAnimacionProcedural.cpp:1147` | despreciable | bajo | Mismo registro. |
| Música: barrido de enemigos por intervalo | `SubsistemaMusica.cpp:133` | despreciable | bajo | Mismo registro. |
| `FindComponentByClass` en eventos (no por fotograma) | `ComponenteSonido.cpp:253` (cada ataque), `ComponenteCombate.cpp:26` (cada golpe), `ComponenteGuardia.cpp:155-159` | despreciable | bajo | Aceptable. |
| Un `USoundWaveProcedural` nuevo por sonido | `ComponenteSonido.cpp:~249` | bajo | presión de GC | IA-24. |
| Guardado de exploración cada 5 s | `Exploracion.cpp:111` | bajo | medio (mapas grandes) | IA-25. |
| Mapa de pausa: un widget por trozo, recolección del mundo | `Mapa.cpp:117-185` | tirón pequeño | **tirón visible**, roto con World Partition | IA-12. |

**Contenido ×10:**
- **Textos.** Los textos del tutorial y de las zonas viven en actores de mapa. Sin string tables ni glosario, la traducción y la coherencia se vuelven inmanejables (IA-18).
- **Mapas.** Hay una sola ranura de punto de control por mapa y el último gana. Las rutas de los botones del menú son fijas (`ModoJuego.h:66-70`); con más zonas hace falta una tabla de zonas (`UDataAsset` con mapa, nombre, subtítulo y música).
- **Jefes.** La UI solo pinta una barra (IA-22).
- **Pruebas.** Las de equilibrio listan los data assets a mano (`PruebasEquilibrio.cpp:21-38`), así que cada enemigo nuevo exige editar la prueba. Mejor recorrer el Asset Registry por clase.
- **Guardado.** Crece con `Exploracion` por mapa (`TArray<FIntPoint>` sin comprimir) y se reserializa entero en cada guardado.

## 7. Oportunidades (mayor impacto)

1. **Pantallas de muerte y victoria con ceremonia** (IA-7): rótulo serif, sonido grave, «Reintentar», y nombre del jefe con «derrotado» y la recompensa. Es el mayor aumento de inmersión por coste.
2. **Feed de notificaciones con iconos en rombo** (IA-5): hace que explorar recompense, y es barato.
3. **Progreso de vida visible** (IA-6): la barra crece o se segmenta al completar una máscara.
4. **Menú de opciones por categorías con accesibilidad mínima** (IA-3): remapeo, volumen por canales, sacudida y destello, tamaño de texto, alternar las acciones mantenidas.
5. **Guardado robusto** (IA-1/2): versión, copia de respaldo y una sola escritura en vuelo.
6. **Tipografía y paleta del concepto**: fuente serif propia para títulos y una secundaria legible; tonos hueso y oro viejo en vez de azul frío. Basta con tocar `SAEVUSEstilo`.
7. **Registro de enemigos en un subsistema**: sirve al HUD, la música, la mirada, varios jefes y la telemetría, y quita todos los barridos.
8. **Marcado por acción en los textos** (`{Dash}`) con glifos de mando (IA-9).
9. **CI con pruebas Unit/Contenido, humo y recorrido, y crash reporting** (IA-16/17).
10. **Minimapa circular diegético o semidiegético** (concepto), alimentado por el mismo mapa horneado de IA-12. Opcional en el HUD, para no saturar.

## 8. Contraste con el concepto de UI (`077deb6e-image.png`, sección A–C)

| El concepto pide | Estado en el código | Cambio técnico que lo haría posible |
|---|---|---|
| Emblema con el retrato de la máscara del protagonista, puntiagudo | Rombo plano gris con un «ojo» cian cuadrado (`WidgetInterfaz.cpp:205-229`) | `UImage` con textura (el primer asset de UI), o un `NativePaint` con un polígono en estrella. La regla «sin assets» bloquea el retrato. |
| Barra de vida roja con brillo y barra azul debajo | **Sí**: barras finas, rastro de daño, alma que brilla y late (`:231-237`, `:487-540`) | Añadir un degradado o brillo con un pincel de dos colores. |
| Rombos de habilidades bajo el emblema e indicadores con contador («3», «3 / 12») | **Falta** (no hay contador de curas, cargas de dash ni objetos) | Widget de «indicadores» que lea `Alma` (curas = `Alma/CosteCuracion`) y el presupuesto de dash. |
| Panel «Objetivo / Enemigo» arriba a la izquierda con el icono del jefe, «Rey Goblin … Fase 2» | Barra abajo al centro con nombre y «Fase N» (`:269-293`, `:461`) | Es una cuestión de colocación; el dato ya existe. Opción de layout y un icono por jefe en el data asset. |
| Minimapa circular | **Falta** (solo el mapa de pausa) | Mapa horneado (IA-12) más un `NativePaint` circular con máscara. |
| Interacción: icono en rombo con la tecla «E» debajo | Texto «[E]  Sentarse» con la tecla dibujada (`:332-339`, `EstadoInterfaz.cpp:43`) | Icono por tipo de interactuable (`ISAEVUSInteractuable::ObtenerIcono`) y glifo de dispositivo. |
| Menú principal: Continuar, Nueva partida, Ajustes, Créditos, Salir | Tutorial, Mazmorra, Continuar, Opciones, Salir (`WidgetPausa.cpp:242-262`) | IA-20, y una pantalla de créditos. |
| Opciones: Juego, Gráficos, Audio, Controles, Idioma | Calidad, Sensibilidad, Volumen (`:265-271`) | IA-3. |
| Pausa: Reanudar, Opciones, Volver al menú, Salir | Continuar, Volver al punto de control, Opciones, Menú principal, Salir, más el mapa a la derecha | Coincide en lo básico; **sobra o es discutible** «Volver al punto de control», que hace falta como salida de un atasco y conviene renombrar o confirmar. |
| Muerte: «HAS MUERTO», Reintentar, Volver al menú | «Has caído» sin botones (`:357`) | IA-7. |
| Victoria: «VICTORIA», recompensas, Continuar | **Falta** | IA-7. |
| Notificaciones: Objeto obtenido, Nueva entrada en el códice, Punto de control activado, Has desbloqueado una ruta | Solo aviso de tutorial reutilizado; **no hay códice** | IA-5, y un sistema de códice si se quiere (narrativa). |
| Tipografía serif (SAEVUS, RUINAS DE VARDEN, REY GOBLIN, CONFIGURACIÓN) y secundaria sans | Roboto Light espaciada y Roboto Regular (`EstiloInterfaz.cpp:54-72`) | Fuente serif propia (asset `UFont` con licencia) en `SAEVUSEstilo::FuenteTitulo`. |
| Iconografía en rombos (Vida, Ataque, Defensa, Dash, Wall-run, Parry, Interacción, Mapa, Punto de control, Jefe, Objeto, Estado negativo) | **Nada** | Un atlas de iconos y `SAEVUSEstilo::Icono(ETipo)`; o, mientras no haya assets, rombos con glifos dibujados en `NativePaint`. |
| Botones con marco biselado («CONTINUAR», tecla «Dash») | Botón de solo texto con velo azulado (`EstiloInterfaz.cpp:95-111`) | Un `FSlateBrush` de caja con borde doble, o un 9-slice cuando haya textura. |

**Sobra o se aleja del concepto:**
- el texto de arma y munición abajo a la derecha (el concepto no lo muestra, y el arsenal por defecto ya es solo la espada, `PruebaHumo.h:90`);
- el destello rojo a pantalla completa;
- la paleta azul fría de los menús;
- el «Zona 1» como subtítulo.

**Lo que no se puede representar sin assets:** el retrato del emblema, los iconos, la tipografía serif y los marcos ornamentados. La regla del proyecto «nada de assets» es aquí el límite real. **RECOMENDADO:** admitir assets 2D de UI (fuente, atlas de iconos, 9-slices) aunque los modelos 3D sigan pendientes.

## 9. Arquitectura propuesta

**Conservar** (funciona y escala):
- La composición por componentes con el personaje como raíz de composición (`Personaje/SAEVUSPersonaje.cpp`, 575 líneas). No es un God object.
- El patrón «estado calculado aparte del widget» (`FSAEVUSEstadoInterfaz`), con pruebas puras.
- Los subsistemas por responsabilidad: guardado (instancia de juego); tutorial, exploración y música (mundo).
- Los ajustes en structs `UPROPERTY` y los data assets de enemigos, armas y repertorio.
- `TWeakObjectPtr` en referencias no propietarias y delegados retirados en `EndPlay`.
- La prueba de humo y el recorrido en el juego real (un activo muy raro en proyectos de este tamaño).
- La generación de música en un hilo (`SubsistemaMusica`).

**Refactorizar** (incremental, por orden de riesgo y beneficio):
1. **Guardado** (IA-1/2/25): versión, respaldo, cola de una escritura, intervalo de 30 s. Coste S.
2. **Registro de enemigos y jefes** (`USAEVUSSubsistemaEnemigos`: alta y baja en `BeginPlay`/`EndPlay`, evento de muerte). Sustituye cinco barridos y el delegado estático a medio plazo.
3. **Notificaciones** separadas del tutorial (IA-5).
4. **Menús**: filas de opción reutilizables y categorías; el foco y el «atrás» en una clase base `USAEVUSMenuBase`.
5. **HUD de juego**: moverlo a `Interfaz/SAEVUSHUD` y dejar `HUDDepuracion` como extensión solo de desarrollo (IA-10/14).
6. **Animación**: struct de entrada y constructores de pose de jugador y enemigo separados (IA-13). Solo cuando entre el siguiente tipo de enemigo con esqueleto.
7. **Geometría del mapa**: a `Mundo/`, horneada en un data asset; dibujo con `NativePaint` (IA-12/14).

**Eliminar:**
- La duplicación `FSAEVUSAjusteInterfaz::VidaPorMascara`: no como `UPROPERTY` (no se puede borrar), pero sí retirar su uso cuando haya personaje y marcarla `DeprecatedProperty` en el futuro.
- El dibujo de depuración y los atajos en Shipping.
- La rotación cíclica de opciones, sustituida por filas ◂ ▸.
- Las medidas del mapa copiadas a mano en las pruebas de obstáculos (leerlas de etiquetas del mapa).

**Crear:**
- `USAEVUSSubsistemaNotificaciones`, `USAEVUSSubsistemaEnemigos`, `USAEVUSAjustesJuego` (opciones y accesibilidad).
- Un data asset de zonas (`USAEVUSDatosZona`: mapa, nombre, subtítulo, música, icono) para no tener rutas fijas.
- Un glosario y string tables de localización.
- Telemetría local opcional.
- `.gitattributes`.
- Separación de pruebas `Unit` y `Contenido`.
- Pipeline de CI.

Más adelante, el módulo `SAEVUSEditor` (herramientas de mapa y horneado).

**No recomiendo**, por dogma o por coste sin beneficio:
- partir el módulo de juego en muchos módulos ahora;
- introducir GAS o CommonUI de golpe (CommonUI ayudaría con el foco y la entrada; se valora cuando se rehagan los menús, IA-3/8);
- interfaces abstractas para cada componente.


---

# SAEVUS v2: inventario de textos visibles y de nombres de contenido

Este inventario es la base del informe narrativo. Todas las rutas son relativas a `Source/SAEVUS/` y las líneas son de la v2. Los textos se copian literalmente; las mayúsculas de origen se conservan, aunque la UI las convierta con `ToUpper` donde se indica.

Leyenda de la columna «Cómo»:
- **LOC**: `LOCTEXT`/`NSLOCTEXT` (traducible).
- **FS**: `FText::FromString`/`FromName`/`FString` (no traducible).
- **DATO**: `FText` editable en un mapa o data asset; su contenido real es DESCONOCIDO.

## 1. HUD de juego (`Interfaz/`)

| Archivo:línea | Cómo | Namespace / clave | Texto | Contexto |
|---|---|---|---|---|
| `Interfaz/SAEVUSWidgetInterfaz.cpp:357` | LOC | `SAEVUSInterfaz` / `HasCaido` | `Has caído` | Pantalla de muerte. Se muestra en mayúsculas (`ToUpper`, `:141`): «HAS CAÍDO». |
| `Interfaz/SAEVUSWidgetInterfaz.cpp:461` | LOC | `SAEVUS` / `FaseJefe` | `Fase {0}` | Junto al nombre del jefe, desde la fase 2. Mayúsculas. |
| `Interfaz/SAEVUSEstadoInterfaz.cpp:43` | LOC | `SAEVUSInterfaz` / `Interaccion` | `[{0}]  {1}` | Aviso de interacción: `{0}` = nombre de la tecla (`FKey::GetDisplayName`, por defecto `E`), `{1}` = `ObtenerTextoInteraccion()` del objeto. |
| `Interfaz/SAEVUSEstadoInterfaz.cpp:50` | LOC | `SAEVUSInterfaz` / `SinArma` | `Aguijón` | Nombre de arma cuando la ranura no tiene definición y hay más de una ranura. |
| `Interfaz/SAEVUSEstadoInterfaz.cpp:50` | FS | — | `Arma->NombreVisible` (`FName`) | Nombre del arma equipada, solo con 2 o más ranuras. |
| `Interfaz/SAEVUSEstadoInterfaz.cpp:54` | LOC | `SAEVUSInterfaz` / `Recargando` | `Recargando...` | Munición mientras recarga. |
| `Interfaz/SAEVUSEstadoInterfaz.cpp:56` | LOC | `SAEVUSInterfaz` / `MunicionInfinita` | `{0} / ∞` | Munición con reserva infinita. |
| `Interfaz/SAEVUSEstadoInterfaz.cpp:57` | LOC | `SAEVUSInterfaz` / `Municion` | `{0} / {1}` | Munición: cargador / reserva. |
| `Interfaz/SAEVUSWidgetInterfaz.cpp:325-326` | DATO | — | `Zona->Nombre`, `Zona->Subtitulo` | Título de zona al entrar (mayúsculas en el nombre). Lo define `ASAEVUSInfoZona` (§4). |
| `Interfaz/SAEVUSWidgetInterfaz.cpp:440,445` | DATO | — | `Mensaje->Titulo`, `Mensaje->Texto` | Aviso de tutorial y notificaciones (§3 y §4). El texto admite `[Tecla]`, que se dibuja como tecla, y `\n` para varias líneas. |
| `Interfaz/SAEVUSWidgetInterfaz.cpp:459` | DATO | — | `NombreJefe` = `FSAEVUSAjusteEnemigo::NombreVisible` | Nombre del jefe (mayúsculas), color hueso. |
| `Interfaz/SAEVUSWidgetInterfaz.cpp:157` | FS | — | trozos de `TrocearTeclas` | Recompone avisos e interacción como `FString` y los vuelve a convertir con `FromString`. |

## 2. Menús (principal, pausa, opciones) — `Interfaz/SAEVUSWidgetPausa.cpp` (namespace `SAEVUSPausa`)

| Línea | Clave | Texto | Dónde aparece |
|---|---|---|---|
| 230 | `TituloJuego` | `SAEVUS` | Título del menú principal (88 pt, mayúsculas). |
| 230 | `Pausa` | `Pausa` | Título del menú de pausa. |
| 244 | `Tutorial` | `Tutorial` | Botón del menú principal → `L_Tutorial`. |
| 245 | `Mazmorra` | `Mazmorra` | Botón del menú principal → `L_Mazmorra`. |
| 249 | `ContinuarPartida` | `Continuar` | Botón del menú principal; solo si hay un punto de control guardado. |
| 254 | `Continuar` | `Continuar` | Pausa: cerrar el menú. |
| 255 | `Reiniciar` | `Volver al punto de control` | Pausa. |
| 257 | `Opciones` | `Opciones` | Principal y pausa. |
| 260 | `MenuPrincipal` | `Menú principal` | Pausa: carga `L_Tutorial` con el menú. |
| 262 | `Salir` | `Salir` | Principal y pausa (sale del juego sin confirmación). |
| 271 | `Volver` | `Volver` | Opciones. |
| 283 | `Calidad` | `Calidad: {0}` | Opciones. |
| 73 | `Baja` | `Baja` | Valor de calidad. |
| 75 | `Media` | `Media` | Valor de calidad. |
| 77 | `Alta` | `Alta` | Valor de calidad (también se muestra para Épica y Cinemática). |
| 286 | `Sensibilidad` | `Sensibilidad del ratón: {0}` | Opciones (en realidad afecta también al stick). |
| 288 | `Volumen` | `Volumen: {0} %` | Opciones. |

Todos los botones se muestran en mayúsculas (`ToUpper`, `:121`).

## 3. Notificaciones y mensajes del mundo escritos en código

Todas se muestran en la misma ranura que el aviso de tutorial.

| Archivo:línea | Cómo | Namespace / clave | Título | Texto | Duración |
|---|---|---|---|---|---|
| `Personaje/SAEVUSPersonaje.cpp:235` | LOC | `SAEVUSPersonaje` / `Fragmento` | `Fragmento de máscara` | — | 3 s (`:39`) |
| `Personaje/SAEVUSPersonaje.cpp:236` | LOC | `SAEVUSPersonaje` / `MascaraEntera` | (Fragmento de máscara) | `La máscara está entera: una máscara de vida más.` | 3 s |
| `Personaje/SAEVUSPersonaje.cpp:237` | LOC | `SAEVUSPersonaje` / `FragmentosFaltan` | (Fragmento de máscara) | `{0} de {1}.` | 3 s |
| `Mundo/SAEVUSBanco.cpp:112` | LOC | `SAEVUSBanco` / `Descansas` | `Descansas` | — | `SAEVUSBanco::DuracionAviso` |
| `Mundo/SAEVUSBanco.cpp:112` | LOC | `SAEVUSBanco` / `DescansasTexto` | (Descansas) | `Vida recuperada. Si caes, vuelves aquí; "Continuar" también.` | ídem |
| `Mundo/SAEVUSBanco.cpp:83` | LOC | `SAEVUSBanco` / `Sentarse` | — | `Sentarse` | Texto de interacción del banco: «[E]  Sentarse». |

## 4. Textos definidos en mapas o data assets (contenido real DESCONOCIDO: no hay Content)

| Archivo:línea | Campo | Valor por defecto en C++ | Uso |
|---|---|---|---|
| `Tutorial/SAEVUSVolumenTutorial.h:102` | `FText Titulo` | vacío | Aviso del tutorial al entrar en el volumen. Duración por defecto 9 s (`:109`). |
| `Tutorial/SAEVUSVolumenTutorial.h:105` | `FText Texto` (MultiLine) | vacío | Cuerpo del aviso. Por convención lleva teclas entre corchetes: ejemplo de prueba `[W] [A]: moverte. Mantén [Mayús]` (`Pruebas/SAEVUSPruebasInterfaz.cpp:166`). |
| `Tutorial/SAEVUSPuertaArena.h:51` | `FText TituloAlAbrir` | vacío | Mensaje al abrirse la puerta de una arena vaciada. |
| `Tutorial/SAEVUSPuertaArena.h:54` | `FText TextoAlAbrir` | vacío | ídem |
| `Mundo/SAEVUSPalanca.h:63` | `FText TituloAlAccionar` | vacío | Mensaje al accionar una palanca. |
| `Mundo/SAEVUSPalanca.h:66` | `FText TextoAlAccionar` | vacío | ídem |
| `Mundo/SAEVUSInfoZona.h:20` | `FText Nombre` | vacío | Nombre de la zona (título grande al entrar). |
| `Mundo/SAEVUSInfoZona.h:24` | `FText Subtitulo` | vacío | Comentario de ejemplo: `("Zona 1")` (`:22`). |
| `Enemigos/SAEVUSTiposEnemigo.h:185` | `FText NombreVisible` | `Rastrero` (LOC `SAEVUSEnemigos` / `Rastrero`, `Enemigos/SAEVUSTiposEnemigo.cpp:10`) | Nombre del enemigo; en jefes, el de la barra. |
| `Enemigos/SAEVUSTiposEnemigo.h:77` | `FName Nombre` (ataque enemigo) | `Embestida` | No se muestra en la UI; sale en el log de equilibrio. |
| `Armas/SAEVUSTiposArma.h:74` | `FName NombreVisible` | `Arma` | Nombre del arma en el HUD (no traducible). |
| `Combate/SAEVUSTiposCombate.h:78` | `FName Nombre` (ataque del jugador) | `Ataque` | Solo en el HUD de depuración. |

## 5. Nombres de ataques del repertorio por defecto (no visibles al jugador; HUD de depuración y log)

Todos están en `Combate/SAEVUSTiposCombate.cpp`:

| Línea | Nombre |
|---|---|
| 23 | `Tajo1` |
| 27 | `Tajo2` |
| 34 | `Remate` |
| 57 | `TajoAereo` |
| 76 | `TajoAereo2` |
| 80 | `RemateAereo` |
| 93 | `TajoArriba` |
| 116 | `TajoEnCarrera` |
| 138 | `AtaqueCargado` |

Tipos de ataque enemigo (`UENUM`, `Enemigos/SAEVUSTiposEnemigo.h:32-44`): Embestida, Salto, Golpe, Proyectil, Picado.

Gestos enemigos (`:51-62`): Automatico, Tajo, Mazazo, Manotazo, Estocada…

Estados de IA (`:14-30`): Reposo, Perseguir, Telegrafiar, Atacar, Recuperarse, Aturdido, Muerto. Solo se ven en depuración.

## 6. Mapas, zonas, rutas y etiquetas

| Archivo:línea | Valor | Qué es |
|---|---|---|
| `Nucleo/SAEVUSModoJuego.h:66` | `/Game/SAEVUS/Mapas/L_Tutorial` | Mapa del botón «Tutorial» y de «Menú principal». |
| `Nucleo/SAEVUSModoJuego.h:70` | `/Game/SAEVUS/Mapas/L_Mazmorra` | Mapa del botón «Mazmorra» (descrito como «mapa de práctica», `:68`). |
| `Nucleo/SAEVUSModoJuego.cpp:30` | `L_Tutorial` | Mapas que arrancan con el menú principal. |
| `Nucleo/SAEVUSModoJuego.cpp:20` | `portal` | Opción de URL de llegada por paso. |
| `Nucleo/SAEVUSModoJuego.cpp:23` | `continuar` | Opción de URL de «Continuar». |
| `Mundo/SAEVUSVolumenCambioMapa.cpp:21-22` | `%s#%s`, `portal=%s` | Paso entre mapas con etiqueta de llegada. |
| `Mundo/SAEVUSVolumenCambioMapa.h:10` | «paso del tutorial a la Zona 1, entre zonas» | Comentario: existe una «Zona 1» tras el tutorial. |
| `Pruebas/SAEVUSPruebasMapas.cpp:13` | `DesdeMazmorra` | Etiqueta de `PlayerStart` de llegada, de ejemplo en una prueba. |
| `Pruebas/SAEVUSPruebasGuardado.cpp:13` | `L_Tutorial` | Mapa de la prueba de guardado. |
| `Pruebas/SAEVUSPruebasObstaculos.cpp:16` | «Los obstáculos de L_Tutorial…» | Obstáculos del tutorial: bloque de doble salto (220 cm), foso de dash (800), foso de rebote con pinchos (1200), foso de carrera por pared (900), cornisa (450) (`:31-41`). |
| `Nucleo/SAEVUSGuardado.cpp:13` | `SAEVUS` | Nombre de la ranura de guardado. |
| `Pruebas/SAEVUSPruebasGuardado.cpp:12` | `SAEVUS_Prueba` | Ranura de pruebas. |
| `Combate/SAEVUSTiposCombate.cpp:7` | `SAEVUS.BlancoPractica` | Etiqueta de actor «blanco de práctica». |
| `Interfaz/SAEVUSMapa.cpp:22` | `SinMapa` | Etiqueta de actores excluidos del mapa (fondos de fosos). |

## 7. Enemigos, jefes y data assets referenciados

Data assets y mallas (solo referenciados en pruebas; su existencia es DESCONOCIDA):

| Archivo:línea | Ruta |
|---|---|
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:21` | `/Game/SAEVUS/Datos/DA_RepertorioMelee_Base` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:23` | `/Game/SAEVUS/Datos/DA_Arma_Pistola` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:24` | `/Game/SAEVUS/Datos/DA_Arma_Rifle` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:25` | `/Game/SAEVUS/Datos/DA_Arma_Escopeta` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:28` | `/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Rastrero` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:29` | `/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Saltador` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:30` | `/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Escupidor` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:31` | `/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Revoloteador` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:32` | `/Game/SAEVUS/Datos/Enemigos/DA_Enemigo_Acorazado` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:33` | `/Game/SAEVUS/Datos/Enemigos/DA_Jefe_Guardian` (jefe) |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:34` | `/Game/SAEVUS/Datos/Enemigos/DA_Jefe_MadreAlada` (jefe) |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:35` | `/Game/SAEVUS/Datos/Enemigos/DA_Goblin` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:36` | `/Game/SAEVUS/Datos/Enemigos/DA_Goblin_Maza` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:37` | `/Game/SAEVUS/Datos/Enemigos/DA_Goblin_Hondero` |
| `Pruebas/SAEVUSPruebasEquilibrio.cpp:38` | `/Game/SAEVUS/Datos/Enemigos/DA_Jefe_ReyGoblin` (jefe) |
| `Pruebas/SAEVUSPruebasEnemigos.cpp:446,541` | `/Game/SAEVUS/Datos/Enemigos/DA_Goblin` («Goblin con esqueleto (REF-05)», `:444`) |
| `Pruebas/SAEVUSPruebasEnemigos.cpp:497` | `/Game/SAEVUS/Datos/Enemigos/DA_Jefe_ReyGoblin` («Rey Goblin reconstruido desde las vistas de REF-06», `:496`) |
| `Pruebas/SAEVUSPruebasAnimacion.cpp:16`, `Pruebas/SAEVUSPruebasRevision.cpp:422` | `/Game/SAEVUS/Arte/Personajes/Caballero/SK_Caballero` (el protagonista se llama «el caballero» en el código) |

Nombres de enemigos y jefes deducidos de esos assets (el texto visible real es el `NombreVisible` del asset, DESCONOCIDO):
- **Enemigos menores:** Rastrero, Saltador, Escupidor, Revoloteador, Acorazado, Goblin, Goblin (de) Maza, Goblin Hondero.
- **Jefes:** Guardián, Madre Alada, Rey Goblin.

Otras menciones en código o comentarios:
- `Interfaz/SAEVUSWidgetInterfaz.cpp:276`: `(REF-08 A: "Rey Goblin ... Fase 2")`.
- `Animacion/SAEVUSAnimacionEnemigo.cpp:3`: «goblins y Rey Goblin, REF-05/REF-06».
- `Interfaz/SAEVUSEstadoInterfaz.h:42`: «REF-08 A: "Fase 2"».
- `Pruebas/SAEVUSPruebasInterfaz.cpp:103`: nombre de jefe de prueba `Guardián`.
- `Pruebas/SAEVUSPruebasProyectiles.cpp:56`: prueba «ProyectilEscupidor».

Comportamientos que se deducen de las pruebas:
- **Volador:** `SAEVUS.Enemigo.Volador`, `AleteoVolador`.
- **Escudo:** `SAEVUS.Enemigo.Escudo`.
- **Fases de jefe:** `SAEVUS.Enemigo.FasesJefe`.
- **Proyectiles devolvibles con un tajo:** `SAEVUS.Combate.DevolverProyectil`.

## 8. Armas y objetos

| Archivo:línea | Texto | Contexto |
|---|---|---|
| `Interfaz/SAEVUSEstadoInterfaz.cpp:50` | `Aguijón` | Nombre por defecto del arma cuerpo a cuerpo (LOC). |
| `Pruebas/SAEVUSPruebasArmas.cpp:45` | `Pistola` | Arma de prueba. |
| `Pruebas/SAEVUSPruebasArmas.cpp:46` | `Rifle` | Arma de prueba. |
| `Pruebas/SAEVUSPruebasArmas.cpp:51` | `Espada` | Arma de prueba. |
| `Pruebas/SAEVUSPruebasArmas.cpp:157` | `Escopeta` | Arma de prueba. |
| `Pruebas/SAEVUSPruebasInterfaz.cpp:58` | `Pistola` | Arma de prueba en la UI. |
| `Pruebas/SAEVUSPruebasArmas.cpp:334` | `AperturaPrueba` | Ataque de prueba. |
| `Pruebas/SAEVUSPruebasDash.cpp:287,300,303` | «aguijón» | Velocidad del dash «con aguijón», frente a la del arma de fuego. |
| `Depuracion/SAEVUSSubsistemaPruebaHumo.h:90` | «ahora solo la espada» | El arsenal por defecto lleva solo la espada. |

Objetos y elementos del mundo (nombres de clase o concepto, no texto visible):
- `ASAEVUSBanco` (banco: descansar, punto de control, curación completa y munición);
- `ASAEVUSFragmentoMascara` (fragmento de máscara: 2 por máscara, +20 de vida, `Personaje/SAEVUSPersonaje.h:51,55`);
- `ASAEVUSParedRompible` (pared falsa);
- `ASAEVUSPalanca` (palanca con compuertas);
- `ASAEVUSPuertaArena` (puerta de arena);
- `ASAEVUSPeligro` (pinchos);
- `ASAEVUSPlataformaMovil`;
- `ASAEVUSFuegoBrasero` (brasero);
- `ASAEVUSMunecoEntrenamiento` (muñeco/diana);
- `ASAEVUSPuntoControl`;
- `ASAEVUSVolumenCaida`.

Recursos del jugador:
- **Vida en «máscaras»** (`VidaPorMascara = 20`).
- **«Alma»:** máximo 99, 11 por golpe, 33 por cura (`Combate/SAEVUSComponenteAlma.h:22,26,30`).

## 9. Textos de depuración (visibles con F6 o CVars; no son para el jugador, pero se compilan en Shipping)

| Archivo:línea | Texto |
|---|---|
| `Depuracion/SAEVUSHUDDepuracion.cpp:82` | `SAEVUS DEPURACIÓN  (F6 / SAEVUS.Depuracion.HUD)` |
| `:83` | `Paso: %s` |
| `:84` | `Velocidad: %.0f / %.0f cm/s` |
| `:85` | `Velocidad vertical: %.0f cm/s` |
| `:86` | `Modo: %s` |
| `:87` | `Dash: %s, dashes aéreos %d` con `activo` / `listo` / `bloqueado` |
| `:89` | `Dobles saltos %d, saltos de pared %d, escaladas %d, en pared %.2f s` |
| `:91` | `Posición: %s` |
| `:94` | `Ataque: ninguno` |
| `:99` | `Ataque: %s #%d %s %.2f s` |
| `:104` | ` (en buffer)` |
| `:109` | `Arma: ninguna` |
| `:114` | `Arma: %s [%d/%d]` |
| `:117` | ` munición %d/%d reserva %d` |
| `:122` | ` (cambiando a %d)` |
| `:126` | ` (recargando)` |
| `:133-134` | `Vida: %.0f/%.0f%s%s` con ` MUERTO` / ` (invulnerable)` |
| `:138` | `Guardia: %s, bloqueos %d` con `activa` / `pedida` / `no` |
| `:144` | `Alma: %.0f/%.0f (curando %.0f %%)` |
| `:145` | `Alma: %.0f/%.0f` |
| `:61` | `%.0f` (números de daño flotantes) |
| `Mundo/SAEVUSMunecoEntrenamiento.cpp:89` | `Golpes %d \| Último %.0f \| Total %.0f` (texto 3D sobre el muñeco, solo con el HUD de depuración) |
| `Depuracion/SAEVUSVariablesDepuracion.cpp:11` | `Muestra el HUD de depuración de SAEVUS (paso, velocidad, modo de movimiento, combate, armas).` |
| `Depuracion/SAEVUSVariablesDepuracion.cpp:17` | `Dibuja las esferas de golpe activas y las trazas de disparo (amarillo: sin golpe, rojo: golpe).` |
| `Depuracion/SAEVUSVariablesDepuracion.cpp:23` | `Muestra números de daño flotantes sobre quien recibe el golpe (blancos: enemigos y muñecos; rojos: el jugador).` |

## 10. Textos en pruebas que revelan convenciones de contenido

| Archivo:línea | Texto |
|---|---|
| `Pruebas/SAEVUSPruebasInterfaz.cpp:88` | Título `Saltar`, texto `Espacio` (aviso de ejemplo). |
| `Pruebas/SAEVUSPruebasInterfaz.cpp:166` | `[W] [A]: moverte. Mantén [Mayús]` (formato de los avisos del tutorial con teclas). |
| `Pruebas/SAEVUSPruebasInterfaz.cpp:176` | `Pulsa [E` |
| `Pruebas/SAEVUSPruebasTutorial.cpp:45` | `Título` / `Texto` |
| `Pruebas/SAEVUSPruebasEnemigos.cpp:168` | Ataque `Picado` |
| `Pruebas/SAEVUSPruebasEnemigos.cpp:417` | Ataque `Salto` |

## 11. Documentos, herramientas y referencias de arte citados en el código

| Referencia | Dónde se cita |
|---|---|
| `Documentos/ARTE_SAEVUS.md` | `Interfaz/SAEVUSWidgetInterfaz.h:20`, `Mundo/SAEVUSKitInstancias.h:14` |
| `Documentos/movimiento.md` | `Movimiento/SAEVUSAjustesMovimiento.h:106,117,184`, `Movimiento/SAEVUSComponenteMovimiento.h:28` |
| `Herramientas/Ejecutar-Recorrido.ps1` | `Depuracion/SAEVUSSubsistemaRecorrido.h:19` |
| `Herramientas/Arte/blender_caballero.py` | `Animacion/SAEVUSComponenteAnimacionProcedural.h:23,215` |
| `saevus_crear_assets.py`, `construir_mapa_tutorial` | `Mundo/SAEVUSKitInstancias.h:17`, `Pruebas/SAEVUSPruebasObstaculos.cpp:16` |
| `Verificar-Arranque.ps1` | `Pruebas/SAEVUSPruebasEnemigos.cpp:448,502,546` |
| `Generar-Caballero.ps1` | `Pruebas/SAEVUSPruebasAnimacion.cpp:56` |

Secciones de referencia de arte:
- **REF-01/REF-02:** caballero, capa, hoja.
- **REF-04:** poses numeradas 1–15.
- **REF-05:** goblin.
- **REF-06:** Rey Goblin.
- **REF-07:** kit modular de mazmorra.
- **REF-08:** A = HUD, B = tipografía y color hueso `#DFC6AD`, D = dash, E = estela.

## 12. Textos del concepto de UI (`077deb6e-image.png`)

Estos textos son del dueño, **no están en el código**; sirven para contrastar.
- **HUD:** «Objetivo / Enemigo», «Rey Goblin», «Fase 2», «Minimapa», «Interacción» con tecla «E», indicadores «3» y «3 / 12».
- **Menú principal:** Continuar, Nueva partida, Ajustes, Créditos, Salir.
- **Opciones:** Juego, Gráficos, Audio, Controles, Idioma.
- **Pausa:** Reanudar, Opciones, Volver al menú, Salir.
- **Muerte:** «HAS MUERTO», Reintentar, Volver al menú.
- **Victoria:** «VICTORIA», Recompensas, Continuar.
- **Notificaciones:** «Objeto obtenido», «Nueva entrada en el códice», «Punto de control activado», «Has desbloqueado una ruta».
- **Tipografía:** título «SAEVUS»; nombre de zona «RUINAS DE VARDEN»; nombre de jefe «REY GOBLIN»; título de menú «CONFIGURACIÓN»; botón «CONTINUAR»; tutorial «Dash»; texto secundario «La velocidad es una forma de supervivencia.».
- **Iconografía:** Vida, Ataque, Defensa, Dash, Wall-run, Parry, Interacción, Mapa, Punto de control, Jefe, Objeto, Estado negativo.
- **Láminas:** Goblin normal, Goblin élite, Jefe (Fase 1 / Fase 2), Protagonista, Rey Goblin.

## 13. Observaciones para el informe narrativo

1. **Textos que existen hoy en código.** Hay solo unos 30 textos visibles en código; el resto (tutorial, zonas, puertas, palancas, nombres de enemigos y jefes) vive en mapas y assets que no tenemos.
2. **Terminología de Hollow Knight.** «Aguijón», «máscaras», «fragmento de máscara», «alma», el banco y «Descansas», y los valores de alma 99/33/11 son de Hollow Knight (ver IA-19 en `interfaz_arquitectura.md`).
3. **Cuarta pared.** «Has caído» y «Descansas» son sobrios y van con el tono, pero el texto del banco rompe la cuarta pared al mencionar el menú «Continuar».
4. **Nombres del concepto sin rastro en el código.** «Ruinas de Varden», el códice y la frase «La velocidad es una forma de supervivencia» no aparecen; «Zona 1» y «Mazmorra» son nombres de trabajo.
5. **Consistencia de nombres.** El protagonista es «el caballero» en el código; enemigos y jefes mezclan nombres descriptivos (Rastrero, Escupidor) con nombres de especie (Goblin). Conviene fijar un glosario.
