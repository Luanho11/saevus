# Seguimiento de la auditoría detallada v2

Fuente: `SAEVUS_Auditoria_Detallada.md` (cuatro áreas: gameplay, animación/VFX/cámara, enemigos/audio, interfaz/arquitectura). Aquí solo se lleva el estado de cada hallazgo; el detalle y la justificación están en la auditoría.
Estados: `[x]` hecho y probado · `[~]` hecho en parte · `[ ]` pendiente · `[-]` decidido no hacer (con motivo).

## Decisiones tomadas al aplicarla
- **GAMEPLAY-1 (dash):** el dueño pidió (2026-10-04) dash «sin límite de uno por salto» como en GunZ. Se mantiene encadenar dashes y el dash aéreo ilimitado, pero la invulnerabilidad pasa de 0,15 s a 0,10 s y **solo el primer dash de una racha** la da (`VentanaRachaSinInvulnerabilidad` 0,45 s). El planeo (velocidad vertical 0) queda como está: es parte del ritmo GunZ y es un dato (`VelocidadVerticalDashAereo`, `bSuspenderGravedadEnDash`). Pendiente de confirmar con el dueño.
- **Guardado:** la ranura del editor pasa a `SAEVUS_Editor` (no pisa la partida del ejecutable).

## Gameplay
- [~] GAMEPLAY-1 dash en racha sin invulnerabilidad (ver decisión)
- [ ] GAMEPLAY-2 estado del mundo persistente (puertas, palancas, paredes, jefes)
- [x] GAMEPLAY-3 `FellOutOfWorld` → muerte y reaparición (`USAEVUSComponenteVida::Matar`)
- [ ] GAMEPLAY-4 defensa con coste (parry machacable, guardia gratuita)
- [x] GAMEPLAY-5 buffer de cancelación (dash y salto adelantados), test `Combate.BufferCancelacion`
- [ ] GAMEPLAY-6 morir castiga y el jefe se reinicia
- [x] GAMEPLAY-7 guardado: un solo guardado en vuelo, copia `_bak`, versión y migración
- [ ] GAMEPLAY-8 tutorial «haciendo»
- [ ] GAMEPLAY-9 parry con respuesta propia
- [ ] GAMEPLAY-10 tajo hacia arriba por inclinación de cámara
- [ ] GAMEPLAY-11 estado del jugador entre mapas
- [ ] GAMEPLAY-12 abanico aéreo bajo
- [x] GAMEPLAY-13 punto de control sin escala ni alabeo
- [ ] GAMEPLAY-14 equilibrio con cancelaciones
- [ ] GAMEPLAY-15 pruebas de E2, E3, E5, E12 y del guardado
- [ ] GAMEPLAY-16 invulnerabilidad durante la parada de golpe
- [ ] GAMEPLAY-17 retroceso con fosos
- [ ] GAMEPLAY-18 salto variable
- [ ] GAMEPLAY-19 cadena de combo y medio (aire/suelo)
- [ ] GAMEPLAY-20 sentado: guardia y golpes levantan
- [x] GAMEPLAY-21 `EscalasVelocidadAccion` a 1 y comprobación de índice
- [ ] GAMEPLAY-22 recursos de los muñecos
- [ ] GAMEPLAY-23 cola de avisos
- [ ] GAMEPLAY-24 opciones de control del dash
- [x] GAMEPLAY-25 exploración cada 30 s (falta guardar al pausar)
- [x] GAMEPLAY-26 migración: se descarta `None` de los fragmentos
- [x] GAMEPLAY-27 tiempo de plataforma y brasero envuelto
- [ ] GAMEPLAY-28 golpes contra el escenario con sonido y chispa
- [ ] GAMEPLAY-29 código muerto del pogo
- [ ] GAMEPLAY-30 puerta de arena y streaming
- [x] GAMEPLAY-31 sensibilidad acotada al guardar, ranura del editor aparte (falta sonido propio del fragmento)

## Animación, VFX y cámara
- [x] ANIMVFX-1 muelles: Euler implícito, frecuencia ≤ 20 Hz, guardas de NaN en muelle y pose; test `Animacion.MuelleEstable`
- [x] ANIMVFX-2 el golpe del enemigo llega en `TiempoImpacto` (0,15 s), igual que el proyectil
- [ ] ANIMVFX-3 contacto con el suelo (zancada por velocidad, IK)
- [~] ANIMVFX-4 arco y estela a 1 partícula por paso (falta la cinta como única firma y el arco por gesto)
- [ ] ANIMVFX-5 emisores y mallas por familia, punto de contacto del impacto
- [ ] ANIMVFX-6 esqueleto por roles
- [ ] ANIMVFX-7 gestos de enemigo con claves
- [ ] ANIMVFX-8 claves de gestos en datos
- [ ] ANIMVFX-9 curva continua en la fase activa
- [ ] ANIMVFX-10 reacciones según daño y dirección
- [ ] ANIMVFX-11 giro axial
- [ ] ANIMVFX-12 parpadeo de invulnerabilidad por material y opción
- [ ] ANIMVFX-13 opciones de comodidad de cámara (sacudida, FOV, roll)
- [ ] ANIMVFX-14 cámara de combate direccional
- [ ] ANIMVFX-15 pose de carga, bloqueo con impacto y parry
- [ ] ANIMVFX-16 capa de varias tiras
- [ ] ANIMVFX-17 LOD de animación por distancia
- [~] ANIMVFX-18 muerte: el cuerpo con esqueleto solo se encoge en el último 30 % y dura 1,5 s
- [ ] ANIMVFX-19 pruebas con dt variable (el de muelles ya está)
- [ ] ANIMVFX-20 a 27 menores

## Enemigos y audio
- [ ] EA-1 percepción con línea de visión y altura, patrulla relativa, navegación
- [~] EA-2 aggro por daño (6 s sin perder al objetivo); falta reiniciar al jefe al morir el jugador
- [ ] EA-3 jefes como experiencia (arena, intro, transición de fase, victoria)
- [x] EA-4 `TiempoImpacto` (ver ANIMVFX-2)
- [x] EA-5 atenuación 3D compartida (`SAEVUSSintesis::ObtenerAtenuacion`) en enemigos y mundo
- [ ] EA-6 sonidos por tipo de enemigo
- [ ] EA-7 sonidos sustituibles por assets, categorías y concurrencia
- [ ] EA-8 director de combate con fichas
- [ ] EA-9 música por secciones, temas por jefe, stingers
- [ ] EA-10 música de pelea solo con pelea visible
- [ ] EA-11 armonías de las capas
- [ ] EA-12 escalón del bucle de exploración
- [ ] EA-13 caché de la música entre mundos
- [ ] EA-14 desenlazar el `Underflow`
- [ ] EA-15 PCM compartido
- [ ] EA-16 elección de ataque con pesos
- [ ] EA-17 giro limitado al avisar
- [ ] EA-18 escudo y pogo
- [ ] EA-19 transición de fase
- [ ] EA-20 ataques de área
- [ ] EA-21 proyectiles
- [~] EA-22 muerte con cadáver (ver ANIMVFX-18)
- [ ] EA-23 legibilidad jugador/goblin
- [x] EA-24 el roce de un enemigo que no ataca no da contraataque
- [ ] EA-25 validación de datos de enemigos
- [ ] EA-26 coste por enemigo fuera de pantalla
- [x] EA-27 `#include "Async/Future.h"`
- [ ] EA-28 pruebas del área
- [ ] EA-29 variaciones de sonido

## Interfaz y arquitectura
- [x] IA-1 partida ilegible: se prueba la copia `_bak` antes de rendirse
- [x] IA-2 guardados serializados
- [ ] IA-3 ajustes completos (remapeo, categorías, accesibilidad)
- [ ] IA-4 calidad Épica rotulada como Alta
- [ ] IA-5 notificaciones separadas del tutorial
- [ ] IA-6 barra de vida que crece con las máscaras
- [ ] IA-7 muerte con elección y victoria
- [ ] IA-8 foco del mando en el menú
- [ ] IA-9 teclas por acción y glifos de mando
- [ ] IA-10 depuración fuera de Shipping
- [ ] IA-11 a 28 resto
