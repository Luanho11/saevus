# Entorno de desarrollo de SAEVUS

## Herramientas

| Herramienta | Versión | Ruta |
|---|---|---|
| Unreal Engine (binario del Epic Launcher) | 5.8.3 (CL 58210709) | `C:\Program Files\Epic Games\UE_5.8` |
| VS 2022 Build Tools | 17.14, MSVC 14.44.35207 | `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools` |
| Windows SDK | 10.0.26100.0 | |
| .NET Framework SDK | 4.8 (+ pack de destino 4.6.2) | `C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8` |
| Git / Git LFS | 2.54.0 / 3.7.1 | |
| Python (sistema) | 3.14.5 | el editor usa su propio Python embebido |
| CMake | 4.4.3 | |
| Blender | 4.2.16 LTS | `C:\Program Files\Blender Foundation\Blender 4.2` |

Mantener VS 2022 / MSVC 14.44: UE 5.8 se validó con MSVC 14.38–14.50 y VS 2026 trae un compilador más nuevo. UnrealBuildTool necesita el .NET Framework SDK (SwarmInterface); sin él la compilación falla con *"Could not find NetFxSDK install dir"*. UBT compila con `/utf-8`, así los literales con tildes funcionan.

## Requisito de Windows: Smart App Control desactivado

Smart App Control bloquea toda DLL sin firma ni reputación en la nube, incluidas las que produce UnrealBuildTool (`GetLastError=4551`, evento CodeIntegrity 3077). Comprobar el estado:

```powershell
(Get-ItemProperty "HKLM:\SYSTEM\CurrentControlSet\Control\CI\Policy").VerifiedAndReputablePolicyState
# 0 = desactivado, 1 = forzado, 2 = evaluación
```

Se desactiva en Seguridad de Windows → Control de aplicaciones y navegador → Smart App Control.

## Compilar, probar, verificar

Desde la raíz del proyecto:

```powershell
# Todo: compilar, tests SAEVUS.*, crear assets, verificar assets. Logs en Saved\Logs\Verificacion\
powershell -ExecutionPolicy Bypass -File Herramientas\Verificar-Arranque.ps1
# Sin recrear assets
powershell -ExecutionPolicy Bypass -File Herramientas\Verificar-Arranque.ps1 -SinAssets
# Solo tests (filtro opcional)
powershell -ExecutionPolicy Bypass -File Herramientas\Ejecutar-Pruebas.ps1 -Filtro SAEVUS.Combate
# Prueba de humo con render (la primera pasada tras compilar es "en frío"; repetir para medir)
powershell -ExecutionPolicy Bypass -File Herramientas\Ejecutar-PruebaHumo.ps1 -Escalabilidad 1
# Ida y vuelta Blender -> UE
powershell -ExecutionPolicy Bypass -File Herramientas\Pipeline\Ejecutar-SondaPipeline.ps1
```

Abrir el editor: doble clic en `SAEVUS.uproject` (compilar antes para que no pida recompilar).

## Máquina con poca memoria (7 GB RAM, Vega 8)

- Cerrar navegadores y otras aplicaciones antes de compilar o abrir el editor. UBT compila de una en una con menos de 1,5 GB libres: una recompilación completa del módulo tarda más de 10 minutos; una incremental, 2–4.
- El primer arranque del editor compila shaders y llena la DDC; es lento una vez.
- Las funciones de render para GPUs potentes están desactivadas en todo el proyecto (ver `arquitectura.md`). No activarlas en mapas ni volúmenes de post-proceso.
- Para trabajar solo en código, usar ejecuciones `-nullrhi` (tests, scripts), que no usan la GPU.

## Repositorio

- Git LFS lleva `*.uasset`, `*.umap` y el arte fuente (`*.blend`, `*.fbx`, `*.png`, `*.tga`, `*.psd`, `*.wav`). Sin `lockable`: no hay servidor LFS con bloqueos, y con él los assets quedaban de solo lectura.
- Ignorados: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, archivos de IDE.
- `user.name` solo está configurado en este repo (`Hostos`).
- Trabajo autónomo: `loop.ps1` (del dueño) lanza `claude -p` cada 30 min con `PROMPT_AUTONOMIA.md`; `PROGRESS.md` tiene un cerrojo con latido para que dos sesiones no trabajen a la vez (`python Herramientas/actualizar_latido.py`).
