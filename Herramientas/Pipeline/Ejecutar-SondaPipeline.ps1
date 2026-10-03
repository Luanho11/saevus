# Comprobación de ida y vuelta Blender -> Unreal del pipeline de assets (Documentos/pipeline-assets.md).
# Exporta desde Blender una malla de prueba del tamaño de un personaje (FBX + GLB), importa ambas en
# una carpeta temporal de UE, escribe sus límites y borra la carpeta temporal.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Pipeline\Ejecutar-SondaPipeline.ps1

param(
    [string]$RaizMotor = "C:\Program Files\Epic Games\UE_5.8",
    [string]$Blender = "C:\Program Files\Blender Foundation\Blender 4.2\blender.exe"
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$Proyecto = Join-Path $RaizProyecto "SAEVUS.uproject"
$DirSonda = Join-Path $RaizProyecto "Saved\SondaPipeline"
$DirLogs = Join-Path $RaizProyecto "Saved\Logs\Verificacion"
New-Item -ItemType Directory -Force $DirSonda, $DirLogs | Out-Null

Write-Host "== Exportar desde Blender"
# Blender escribe avisos inofensivos por stderr; Windows PowerShell 5.1 se detendría con ellos.
$ErrorActionPreference = "Continue"
$salidaBlender = & $Blender --background --factory-startup --python (Join-Path $PSScriptRoot "blender_sonda_exportar.py") -- $DirSonda 2>&1 | ForEach-Object { "$_" }
$ErrorActionPreference = "Stop"
$salidaBlender | Select-String -Pattern "\[SAEVUS sonda\]|Error|Traceback"

Write-Host "== Importar en Unreal"
$log = Join-Path $DirLogs "sonda-pipeline.log"
$script = Join-Path $PSScriptRoot "ue_sonda_importar.py"
# Una sola cadena literal de argumentos: Windows PowerShell 5.1 estropea los argumentos nativos entre comillas con espacios.
$argumentos = "`"$Proyecto`" -ExecutePythonScript=`"$script`" -unattended -nopause -nullrhi -nosplash -NoSound `"-abslog=$log`""
Start-Process -FilePath (Join-Path $RaizMotor "Engine\Binaries\Win64\UnrealEditor-Cmd.exe") -ArgumentList $argumentos -Wait -NoNewWindow | Out-Null
Select-String -Path $log -Pattern "\[SAEVUS sonda\]" -Encoding UTF8 | ForEach-Object { "  " + ($_.Line -replace '^.*?\[SAEVUS sonda\]', '[SAEVUS sonda]') }
