# Empaqueta SAEVUS para Windows (Development) con los dos mapas, en Saved\Empaquetado\.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Empaquetar.ps1 [-Configuracion Development|Shipping]
# Compila el juego (no el editor), cocina el contenido, lo empaqueta en .pak y lo archiva.
# La primera vez tarda (compilación de shaders del cocinado); el log va a Saved\Logs\Verificacion\empaquetado.log.

param(
    [string]$RaizMotor = "C:\Program Files\Epic Games\UE_5.8",
    [ValidateSet("Development", "Shipping")]
    [string]$Configuracion = "Development"
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
$Mapas = "/Game/SAEVUS/Mapas/L_Tutorial+/Game/SAEVUS/Mapas/L_Zona1+/Game/SAEVUS/Mapas/L_Pruebas"

& (Join-Path $RaizMotor "Engine\Build\BatchFiles\RunUAT.bat") BuildCookRun "-project=$Proyecto" -noP4 -platform=Win64 `
    "-clientconfig=$Configuracion" -build -cook -stage -pak -archive "-archivedirectory=$Salida" "-map=$Mapas" -utf8output -unattended |
    Tee-Object -FilePath $log | Select-String -Pattern "Error|BUILD SUCCESSFUL|BUILD FAILED|AutomationTool exiting"
if ($LASTEXITCODE -ne 0) { throw "El empaquetado falló (ver empaquetado.log)" }
Write-Host "Empaquetado en $Salida"
