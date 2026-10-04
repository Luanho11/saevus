# Empaqueta SAEVUS para Windows (Development) con los dos mapas, en Saved\Empaquetado\.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Empaquetar.ps1 [-Configuracion Development|Shipping]
# Compila el juego (no el editor), cocina el contenido, lo empaqueta en .pak y lo archiva.
# La primera vez tarda (compilación de shaders del cocinado); el log va a Saved\Logs\Verificacion\empaquetado.log.
#
# Memoria (máquina de 7 GB): el primer intento lo cortó el sistema al compilar con varias acciones a la vez.
# Por defecto compila de una en una (-AccionesParalelas 1) y deja libres casi todos los hilos al compilar
# shaders (-HilosShaderLibres). Cierra el editor y el resto de programas antes. Más lento, pero cabe.
# Después: Herramientas\Probar-Ejecutable.ps1 arranca el .exe con la prueba de humo.

param(
    [string]$RaizMotor = "C:\Program Files\Epic Games\UE_5.8",
    [ValidateSet("Development", "Shipping")]
    [string]$Configuracion = "Development",
    # Acciones de compilación a la vez (UnrealBuildTool). 1 = la que menos memoria pide.
    [int]$AccionesParalelas = 1,
    # Hilos que el cocinado deja sin compiladores de shaders (cuantos más, menos procesos y menos memoria).
    [int]$HilosShaderLibres = 6
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent $PSScriptRoot
$Proyecto = Join-Path $RaizProyecto "SAEVUS.uproject"
$Salida = Join-Path $RaizProyecto "Saved\Empaquetado"
$DirLogs = Join-Path $RaizProyecto "Saved\Logs\Verificacion"
New-Item -ItemType Directory -Force $DirLogs | Out-Null
& (Join-Path $PSScriptRoot "Limpiar-Logs.ps1") -DirLogs $DirLogs
$log = Join-Path $DirLogs "empaquetado.log"

# El tutorial es el mapa por defecto; el de pruebas va para la prueba de humo empaquetada.
$Mapas = "/Game/SAEVUS/Mapas/L_Tutorial+/Game/SAEVUS/Mapas/L_Mazmorra+/Game/SAEVUS/Mapas/L_Pruebas"

$argUbt = "-ubtargs=-MaxParallelActions=$AccionesParalelas"
$argCocinado = "-AdditionalCookerOptions=-ini:Engine:[DevOptions.Shaders]:NumUnusedShaderCompilingThreads=$HilosShaderLibres"
& (Join-Path $RaizMotor "Engine\Build\BatchFiles\RunUAT.bat") BuildCookRun "-project=$Proyecto" -noP4 -platform=Win64 `
    "-clientconfig=$Configuracion" -build -cook -stage -pak -archive "-archivedirectory=$Salida" "-map=$Mapas" -utf8output -unattended `
    $argUbt $argCocinado |
    Tee-Object -FilePath $log | Select-String -Pattern "Error|BUILD SUCCESSFUL|BUILD FAILED|AutomationTool exiting"
if ($LASTEXITCODE -ne 0) { throw "El empaquetado falló (ver empaquetado.log)" }
Write-Host "Empaquetado en $Salida"
