# SAEVUS — reglas para Claude Code

SAEVUS: RPG de acción 3D en tercera persona, Unreal Engine 5.8 C++. El movimiento es el núcleo (movimiento acrobático, cuerpo a cuerpo, armas de fuego, PvE, jefes, más adelante PvP). Inspiración: GunZ (movimiento y cancelaciones) + Hollow Knight (combate, tono, sombreado). Arte estilizado dibujado, tono oscuro postapocalíptico. No fotorrealista.

## Idioma
- **Todo en español**: documentos, código (clases, funciones, variables, comentarios, logs), nombres de assets, mensajes de commit y respuestas al dueño. Los nombres de la API del motor no cambian; el prefijo de clases `SAEVUS` se mantiene.

## Fuente de verdad
- El código y los documentos del repo mandan; la conversación es contexto extra. Si una conversación antigua contradice el repo, confía en el repo e informa la contradicción.
- Estado operativo: `Documentos/ESTADO_PROYECTO.md`. Historial: `Documentos/REGISTRO_SESIONES.md`. Cola de tareas autónoma: `PROGRESS.md` (raíz, con cerrojo de sesión para `loop.ps1`).

## Protocolo de sesión
- **Inicio:** leer este archivo → `PROGRESS.md` (cerrojo y cola) → `Documentos/ESTADO_PROYECTO.md` → última entrada de `Documentos/REGISTRO_SESIONES.md` solo si hace falta → `git status`. Nunca rehacer trabajo terminado solo porque el chat es nuevo.
- **Fin de una tarea/fase:** actualizar `ESTADO_PROYECTO.md`, `PROGRESS.md` (y su latido), añadir una entrada breve en `REGISTRO_SESIONES.md`, revisar `git status`, commit.
- El dueño autorizó trabajo autónomo continuo: al terminar la cola, proponer y ejecutar tareas nuevas.

## Restricciones
- Hardware modesto (máquina de desarrollo: 7 GB RAM, Vega 8 integrada). Nada de Lumen, Nanite, ray tracing, Virtual Shadow Maps, MegaLights ni shaders pesados.
- Sin assets externos (Fab, personajes, armas, mapas, animaciones, recursos de GunZ o Hollow Knight). Todo se genera: mallas por script en Blender, efectos y animación por código, contenido del motor.
- GunZ y Hollow Knight: estudiar cómo se sienten y qué permiten; no copiar su código, fórmulas, datos ni recursos.
- Jugabilidad pensada para poder replicarse más adelante (sin red completa todavía).
- Prototipos y bocetos: pulir animación y efectos, no diseño final. Todo modificable y escalable.

## Código
- Prefijo `SAEVUS` (`ASAEVUSPersonaje`, `USAEVUSComponenteMovimiento`). Carpetas por función bajo `Source/SAEVUS/` (Nucleo, Personaje, Movimiento, Camara, Combate, Armas, Animacion, Efectos, Interfaz, Enemigos, Mundo, Depuracion, Pruebas...).
- Clases pequeñas con una responsabilidad, composición antes que herencia, sin números mágicos: los valores de juego viven en structs de ajuste / Data Assets.
- Validar tras cada bloque: compilar, corregir, tests, assets, smoke test, revisar el log. Commits pequeños.
- Toolchain: VS 2022 Build Tools 17.14 (MSVC 14.44), Windows SDK 10.0.26100. No cambiar a VS 2026 sin hablarlo.

## Glosario (nombres fijos)
| Concepto | Nombre |
|---|---|
| personaje / modo de juego / controlador | `ASAEVUSPersonaje`, `ASAEVUSModoJuego`, `ASAEVUSControladorJugador` |
| movimiento, ajustes | `USAEVUSComponenteMovimiento`, `USAEVUSAjustesMovimiento`, `FSAEVUSAjusteMovimientoSuelo/Aire`, `FSAEVUSAjusteDash`, `FSAEVUSAjusteSaltoPared`, `FSAEVUSAjusteCarreraPared`, `FSAEVUSAjusteEscaladaPared` |
| paso (gait) | `ESAEVUSPaso`: `Caminar`, `Correr`, `Esprint` |
| modos personalizados | `ESAEVUSModoMovimientoPersonalizado`: `CarreraPared`, `EscaladaPared` |
| cámara | `USAEVUSComponenteCamara`, `USAEVUSAjustesCamara`, `FSAEVUSAjusteCamara` |
| combate | `USAEVUSComponenteCombate`, `FSAEVUSDefinicionAtaque`, `ESAEVUSFaseAtaque` (`Preparacion`, `Activa`, `Recuperacion`), `ESAEVUSAccionCancelacion` (`Dash`, `Salto`, `CambioArma`), `FSAEVUSRepertorioMelee`, `USAEVUSDatosRepertorioMelee` |
| vida, alma | `USAEVUSComponenteVida` (invulnerabilidad tras golpe), `USAEVUSComponenteAlma`, `FSAEVUSAjusteAlma` |
| tajos direccionales | `ESAEVUSDireccionAtaque` (`Frontal`, `Arriba`, `Abajo`), `FSAEVUSParadaGolpe` |
| animación | `USAEVUSComponenteAnimacionProcedural`, `FSAEVUSAjusteAnimacion`, `ESAEVUSHueso` (esqueleto propio del caballero) |
| efectos | `USAEVUSComponenteParticulas`, `USAEVUSComponenteEstelaDash`, `USAEVUSComponenteEfectosCombate` |
| enemigos | `ASAEVUSEnemigo`, `USAEVUSComponenteIAEnemigo`, `ESAEVUSEstadoEnemigo`, `FSAEVUSAjusteEnemigo`, `FSAEVUSAtaqueEnemigo`, `FSAEVUSFaseEnemigo`, `USAEVUSDatosEnemigo` |
| tutorial | `USAEVUSSubsistemaTutorial`, `ASAEVUSVolumenTutorial`, `ASAEVUSPuntoControl`, `ASAEVUSVolumenCaida`, `ASAEVUSPuertaArena`; mapa `L_Tutorial` (por defecto), `L_Pruebas` (prueba de humo) |
| armas | `USAEVUSComponenteArmas`, `USAEVUSDefinicionArma`, `FSAEVUSAjusteArmaFuego`, `ESAEVUSTipoArma` (`CuerpoACuerpo`, `ArmaFuego`) |
| mundo / depuración | `ASAEVUSMunecoEntrenamiento`, `ASAEVUSHUDDepuracion`, `USAEVUSSubsistemaPruebaHumo`, CVars `SAEVUS.Depuracion.*` |
| sonido | `USAEVUSComponenteSonido`, `FSAEVUSAjusteSonidos`, `FSAEVUSSonidoSintetizado`, `SAEVUSSintesis` |
| interfaz | `USAEVUSWidgetInterfaz`, `FSAEVUSEstadoInterfaz`, `FSAEVUSAjusteInterfaz` |
| tests | `SAEVUS.Movimiento.*`, `SAEVUS.Combate.*`, `SAEVUS.Armas.*`, `SAEVUS.Vida.*`, `SAEVUS.Alma.*`, `SAEVUS.Animacion.*`, `SAEVUS.Estilo.*`, `SAEVUS.Enemigo.*`, `SAEVUS.Tutorial.*`, `SAEVUS.Interfaz.*`, `SAEVUS.Audio.*` |
| assets | `/Game/SAEVUS/{Entrada,Datos,Blueprints,Mapas,Arte}`; `IA_`, `IMC_`, `DA_`, `BP_`, `L_`, `SK_`, `SM_`, `M_`, `MI_`, `T_` |
| herramientas | `Herramientas/` (`Verificar-Arranque.ps1`, `Ejecutar-Pruebas.ps1`, `Ejecutar-PruebaHumo.ps1`, `ScriptsEditor/`, `Pipeline/`) |
