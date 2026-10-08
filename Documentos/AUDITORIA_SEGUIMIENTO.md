# Seguimiento de la auditoría detallada v2

Fuente: `SAEVUS_Auditoria_Detallada.md` (cuatro áreas: gameplay, animación/VFX/cámara, enemigos/audio, interfaz/arquitectura). Aquí solo se lleva el estado de cada hallazgo; el detalle y la justificación están en la auditoría.
Estados: `[x]` hecho y probado · `[~]` hecho en parte · `[ ]` pendiente · `[-]` decidido no hacer (con motivo).

## Decisiones tomadas al aplicarla
- **GAMEPLAY-1 (dash):** el dueño pidió (2026-10-04) dash «sin límite de uno por salto» como en GunZ. Se mantiene encadenar dashes y el dash aéreo ilimitado, pero la invulnerabilidad pasa de 0,15 s a 0,10 s y **solo el primer dash de una racha** la da (`VentanaRachaSinInvulnerabilidad` 0,45 s). El planeo (velocidad vertical 0) queda como está: es parte del ritmo GunZ y es un dato (`VelocidadVerticalDashAereo`, `bSuspenderGravedadEnDash`). Pendiente de confirmar con el dueño.
- **Guardado:** la ranura del editor pasa a `SAEVUS_Editor` (no pisa la partida del ejecutable).

## Gameplay
- [~] GAMEPLAY-1 dash en racha sin invulnerabilidad (ver decisión)
- [x] GAMEPLAY-2 estado del mundo persistente (`EstadoMundo`): puertas, palancas, paredes rotas y jefes vencidos; elegir un mapa en el menú lo empieza de nuevo
- [x] GAMEPLAY-3 `FellOutOfWorld` → muerte y reaparición (`USAEVUSComponenteVida::Matar`)
- [~] GAMEPLAY-4 defensa: tras soltar la guardia hay que esperar 0,35 s para tener ventana de parry (`EsperaTrasSoltar`); falta bloqueo parcial/postura
- [x] GAMEPLAY-5 buffer de cancelación (dash y salto adelantados), test `Combate.BufferCancelacion`
- [~] GAMEPLAY-6 los jefes vuelven a vida máxima, fase 0 y su sitio al morir o reiniciar el jugador; falta puerta de jefe y repoblar al descansar
- [x] GAMEPLAY-7 guardado: un solo guardado en vuelo, copia `_bak`, versión y migración
- [ ] GAMEPLAY-8 tutorial «haciendo»
- [ ] GAMEPLAY-9 parry con respuesta propia
- [ ] GAMEPLAY-10 tajo hacia arriba por inclinación de cámara
- [ ] GAMEPLAY-11 estado del jugador entre mapas
- [ ] GAMEPLAY-12 abanico aéreo bajo
- [x] GAMEPLAY-13 punto de control sin escala ni alabeo
- [ ] GAMEPLAY-14 equilibrio con cancelaciones
- [~] GAMEPLAY-15 pruebas nuevas: buffer de cancelación, dash en racha, fuera del mundo, estado del mundo y copia de seguridad del guardado; faltan Continuar con `?continuar` y doble reaparición
- [ ] GAMEPLAY-16 invulnerabilidad durante la parada de golpe
- [~] GAMEPLAY-17 retroceso 550→380 y el golpe corta la estocada; falta ajustar por tipo de ataque enemigo
- [ ] GAMEPLAY-18 salto variable
- [x] GAMEPLAY-19 cadena de combo y medio (aire/suelo)
- [x] GAMEPLAY-20 sentado: recibir un golpe, levantar la guardia y curar levantan al jugador
- [x] GAMEPLAY-21 `EscalasVelocidadAccion` a 1 y comprobación de índice
- [ ] GAMEPLAY-22 recursos de los muñecos
- [x] GAMEPLAY-23 cola de avisos con tiempo mínimo de lectura (2,5 s); falta registrar los vistos en el guardado
- [ ] GAMEPLAY-24 opciones de control del dash
- [x] GAMEPLAY-25 exploración cada 30 s y al pausar
- [x] GAMEPLAY-26 migración: se descarta `None` de los fragmentos
- [x] GAMEPLAY-27 tiempo de plataforma y brasero envuelto
- [ ] GAMEPLAY-28 golpes contra el escenario con sonido y chispa
- [ ] GAMEPLAY-29 código muerto del pogo
- [ ] GAMEPLAY-30 puerta de arena y streaming
- [x] GAMEPLAY-31 sensibilidad acotada al guardar, ranura del editor aparte (falta sonido propio del fragmento)

## Animación, VFX y cámara
- [x] ANIMVFX-1 muelles: Euler implícito, frecuencia ≤ 20 Hz, guardas de NaN en muelle y pose; test `Animacion.MuelleEstable`
- [x] ANIMVFX-2 el golpe del enemigo llega en `TiempoImpacto` (0,15 s), igual que el proyectil
- [~] ANIMVFX-3 longitud del ciclo proporcional a la zancada (sin patinar); falta IK/trazas al suelo
- [~] ANIMVFX-4 arco y estela a 1 partícula por paso (falta la cinta como única firma y el arco por gesto)
- [~] ANIMVFX-5 chispas en el punto de la hoja más cercano y en proporción al daño; falta un emisor por familia y sangre/parry
- [ ] ANIMVFX-6 esqueleto por roles
- [ ] ANIMVFX-7 gestos de enemigo con claves
- [ ] ANIMVFX-8 claves de gestos en datos
- [ ] ANIMVFX-9 curva continua en la fase activa
- [~] ANIMVFX-10 la reacción al golpe recibido escala con el daño; falta tambaleo/derribo
- [ ] ANIMVFX-11 giro axial
- [~] ANIMVFX-12 parpadeo a 4 Hz; falta pasarlo a material y opción de accesibilidad
- [~] ANIMVFX-13 el FOV no bombea con los dashes y se puede fijar (`bCampoVisionDinamico`); falta exponerlo en Opciones
- [ ] ANIMVFX-14 cámara de combate direccional
- [ ] ANIMVFX-15 pose de carga, bloqueo con impacto y parry
- [ ] ANIMVFX-16 capa de varias tiras
- [ ] ANIMVFX-17 LOD de animación por distancia
- [~] ANIMVFX-18 muerte: el cuerpo con esqueleto solo se encoge en el último 30 % y dura 1,5 s
- [~] ANIMVFX-19 prueba de tirones largos con ataque aéreo (`Animacion.TironNoRompe`) y de muelles; faltan dt variable en el resto, pies y presupuesto de partículas
- [ ] ANIMVFX-20 motas de ambiente: reciclar las que salen de la caja
- [ ] ANIMVFX-21 eco del dash creado en `BeginPlay`
- [~] ANIMVFX-22 arrastre exponencial en partículas; falta el rango sucio
- [x] ANIMVFX-23 cadera retorcida tras rotaciones forzadas
- [ ] ANIMVFX-24 repetición del reposo y estados emocionales
- [x] ANIMVFX-25 límites de la malla poseable
- [ ] ANIMVFX-26 ocultar el arma sin escalar a 0
- [ ] ANIMVFX-27 sacudida tras la colisión del brazo de cámara

## Enemigos y audio
- [~] EA-1 no detecta a distinta altura ni tras una pared, patrulla sobre su eje y da la vuelta contra paredes, vuelve a su puesto; falta navegación
- [~] EA-2 aggro por daño (6 s sin perder al objetivo); falta reiniciar al jefe al morir el jugador
- [~] EA-3 arena de jefe (`ASAEVUSArenaJefe`: cierra la entrada y despierta al jefe, se rearma si el jugador muere), rugido de fase invulnerable y rótulo «DERROTADO»; falta música propia por jefe, stinger y postura
- [x] EA-4 `TiempoImpacto` (ver ANIMVFX-2)
- [x] EA-5 atenuación 3D compartida (`SAEVUSSintesis::ObtenerAtenuacion`) en enemigos y mundo
- [~] EA-6 tono según el tamaño, variación por reproducción, «te he visto» y «herido»; faltan los sonidos por tipo en los datos y el escudo roto
- [ ] EA-7 sonidos sustituibles por assets, categorías y concurrencia
- [x] EA-8 director de combate con 2 fichas (`USAEVUSDirectorCombate`); el resto rodea esperando turno
- [ ] EA-9 música por secciones, temas por jefe, stingers
- [x] EA-10 música de pelea solo con pelea visible
- [x] EA-11 armonías de las capas
- [x] EA-12 escalón del bucle de exploración
- [x] EA-13 caché de la música entre mundos
- [x] EA-14 desenlazar el `Underflow`
- [ ] EA-15 PCM compartido
- [x] EA-16 elección al azar sin repetir y castigo al que se cura (opcional por datos: `bAtaquesAleatorios`, activo en el Rey Goblin)
- [x] EA-17 giro limitado al avisar
- [ ] EA-18 escudo y pogo
- [~] EA-19 transición de fase (`DuracionTransicion`); falta `ObtenerProgresoRecuperacion` para la animación
- [~] EA-20 onda al aterrizar del salto (`RadioOnda`, Rey Goblin 450 cm); falta el telegrafiado de área en el suelo
- [~] EA-21 el proyectil que nace sobre el jugador le da; faltan sonido al desviar y presencia
- [~] EA-22 muerte con cadáver (ver ANIMVFX-18)
- [ ] EA-23 legibilidad jugador/goblin
- [x] EA-24 el roce de un enemigo que no ataca no da contraataque
- [x] EA-25 `SAEVUSEnemigo::Validar` + `IsDataValid` del Data Asset
- [ ] EA-26 coste por enemigo fuera de pantalla
- [x] EA-27 `#include "Async/Future.h"`
- [ ] EA-28 pruebas del área
- [~] EA-29 variación de tono de ±5 % en los sonidos de los enemigos; falta el resto y las variantes de síntesis

## Interfaz y arquitectura
- [x] IA-1 partida ilegible: se prueba la copia `_bak` antes de rendirse
- [x] IA-2 guardados serializados
- [~] IA-3 tres opciones de comodidad guardadas (sacudida, campo de visión, parpadeo); faltan remapeo, categorías, volumen por canales, texto
- [x] IA-4 calidad Épica rotulada como Alta
- [ ] IA-5 notificaciones separadas del tutorial
- [x] IA-6 barra de vida que crece con las máscaras
- [~] IA-7 rótulo «DERROTADO» al vencer a un jefe; falta la pantalla de muerte con elección
- [x] IA-8 foco del mando en el menú
- [ ] IA-9 teclas por acción y glifos de mando
- [x] IA-10 depuración fuera de Shipping
- [ ] IA-11 barridos por fotograma en el HUD
- [ ] IA-12 mapa de pausa con widgets por rectángulo
- [ ] IA-13 componente de animación como nudo central
- [ ] IA-14 ciclos de dependencia entre carpetas
- [ ] IA-15 herramientas de QA frágiles
- [ ] IA-16 huecos de cobertura de pruebas
- [ ] IA-17 CI, crash reporting y telemetría
- [ ] IA-18 localización
- [ ] IA-19 identidad: terminología calcada de Hollow Knight
- [ ] IA-20 «Nueva partida» y orden del menú
- [ ] IA-21 zona segura y escala de texto
- [ ] IA-22 barra de jefe: rastro falso al reaparecer
- [ ] IA-23 stick de cámara dependiente del framerate
- [ ] IA-24 caché de síntesis sin límite
- [x] IA-25 exploración cada 30 s y al pausar
- [ ] IA-26 acoplamiento al primer jugador
- [ ] IA-27 opacidad del velo de muerte
- [ ] IA-28 finales de línea mezclados
