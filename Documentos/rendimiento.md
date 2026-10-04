# Rendimiento de SAEVUS

Estado: línea base medida en la máquina de desarrollo, que también es el objetivo "modesto" de referencia.

## Cómo medir

```
powershell -ExecutionPolicy Bypass -File Herramientas\Ejecutar-PruebaHumo.ps1 [-Escalabilidad 0..3] [-ComandosExtra "sg.ShadowQuality 1"] [-PerfilGPU]
```

`USAEVUSSubsistemaPruebaHumo` (`-SAEVUSPruebaHumo`) recorre sus etapas solo en el mapa de pruebas y escribe una línea de resumen. Los tiempos se muestrean solo en las etapas de acción (se excluyen aparición y asentado):

- **media / peor**: tiempo de fotograma real;
- **juego / render**: `GGameThreadTime` / `GRenderThreadTime`;
- **GPU**: `RHIGetGPUFrameCycles()`.

`-PerfilGPU` captura además un fotograma de `ProfileGPU` a mitad del esprint (tiempos de GPU por pasada en el log).

La primera ejecución tras compilar o con la DDC vacía no vale: la carga de shaders y assets da tirones de 400 ms y sube el tiempo de juego (visto: 27,8 ms en frío frente a 8,9 ms en caliente). Ejecutar dos veces y quedarse con la segunda.

## Línea base (2026-10-03)

Máquina: Ryzen 5 2500U, Vega 8 (driver 27.20.22002.57), 7 GB RAM. 1280 × 720 en ventana, DX12 SM6, mapa de pruebas, TAA, sin Lumen / Nanite / RT / VSM. Caché caliente.

| Escalabilidad | Fotograma (ms) | fps | Juego (ms) | GPU (ms) | Ejecuciones |
|---|---|---|---|---|---|
| 0 Baja | 8,5 | 118 | 7,5 | 6,4 | 1 |
| 1 Media | 14,7 – 19,5 | 51 – 68 | 7,4 – 9,8 | 12,9 – 16,9 | 5 |
| 2 Alta | 20,4 – 22,4 (un atípico de 27,5) | 45 – 49 | 7,1 – 8,4 | 18,5 – 19,6 (24,2) | 3 |
| 3 Épica | 27,2 | 37 | 6,7 | 24,5 | 1 |
| ninguna (ajustes guardados = Épica) | 29,1 | 34 | 7,1 | 25,4 | 1 |

Alta con un grupo bajado a Media (una ejecución cada uno, ruido de ±1 ms): Efectos 19,0 ms, Post-proceso 20,1, Reflejos 20,3, Sombreado 20,4, Sombras 20,6, Iluminación global 21,3. Forzando resolución nativa (`sg.ResolutionQuality 100`): Alta 22,3 ms (igual), Media 20,2 – 23,0 ms (sube desde ~15).

## Lectura

- **Limitado por la GPU.** El hilo de juego es plano, 7–10 ms, en todos los niveles; el de render sigue a la GPU + ~2 ms (esperándola).
- **La resolución es la palanca grande.** Ningún grupo de escalabilidad explica el salto de Alta a Media (cada uno ahorra ≤ 1,5 ms); la ganancia de Media viene de renderizar por debajo de la resolución nativa.
- **Media es el nivel de 60 fps en esta máquina** a 720p, con poco margen (51–68 fps según la ejecución; el portátil varía, probablemente por temperatura o memoria compartida). Alta ronda los 45 fps.
- El hilo de juego ya cuesta unos 8–10 ms con un personaje y un mapa vacío; vigilarlo según se añaden sistemas (animación procedural, enemigos, efectos).
- El mapa de pruebas es geometría trivial: estas cifras son un suelo, no un presupuesto para niveles reales.

## Con el juego completo (2026-10-04)

Media, 720p, prueba de humo (una ejecución cada una, ruido de ±1–2 ms):

| Mapa | Media | Hilo de juego | Notas |
|---|---|---|---|
| `L_Pruebas` | 57–63 fps (15,9–17,5 ms) | 10,8–13,2 ms | Cielo con atmósfera; caballero animado, estela, interfaz UMG, contorno, 3 enemigos lejos |
| `L_Tutorial` | 67–68 fps (14,7–14,9 ms) | 11,3–12,5 ms | Noche sin atmósfera, niebla, dos luces direccionales (una con sombras), 3 enemigos activos |

- El tutorial cumple los 60 fps en Media; el mapa de pruebas queda justo (lo encarece la atmósfera del cielo).
- El **hilo de juego** ha subido de 7–10 ms a 11–13 ms con animación procedural, IA, partículas por CPU e interfaz: es el siguiente cuello si se añaden muchos enemigos. Candidatos: animación procedural solo para mallas visibles y cercanas, IA a menos frecuencia lejos del jugador, interfaz que solo recalcule al cambiar.
- **Intento sobre el hilo de juego (tarea 10f):** la interfaz solo toca textos, barras y visibilidad cuando cambian (cambiarlos invalida el diseño de Slate) y los enemigos en reposo lejos del jugador (más de 2,5 × su radio de detección) se quedan quietos con la IA cada 0,25 s. Resultado en `L_Tutorial`: 11,6–12,3 ms de hilo de juego, **sin cambio medible** (antes 11,3–12,5). El coste está en otra parte: la prueba corre con el binario del editor (`UnrealEditor -game`, Development), que añade trabajo propio del editor, y con el HUD de depuración activo. Siguiente paso para medir de verdad: un empaquetado Development y `stat game` / Unreal Insights.
- `ProfileGPU` (`-PerfilGPU`) **no registra datos con este driver** (Vega 8, Adrenalin 21.20, 2021): el desglose por pasada sale vacío. Para medir pasadas habrá que usar otra herramienta (RenderDoc / Radeon GPU Profiler) o un driver más nuevo.

## Sin decidir

- Escalabilidad y resolución por defecto del juego final, y si ejecutar el benchmark del motor al primer arranque (es parte del menú de ajustes).
- Desglose de GPU por pasada: `-PerfilGPU` no da datos en esta máquina (ver arriba).
