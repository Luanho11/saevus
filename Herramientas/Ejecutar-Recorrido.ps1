# Recorre el tutorial en el juego real (USAEVUSSubsistemaRecorrido): avisos, arenas, puertas y jefe, con capturas.
# Uso (desde la raíz del proyecto):
#   powershell -ExecutionPolicy Bypass -File Herramientas\Ejecutar-Recorrido.ps1 [-Escalabilidad 0..3] [-ResX 1280 -ResY 720] [-ComandosExtra "r.ScreenPercentage 75"] [-PerfilGPU] [-Mapa /Game/SAEVUS/Mapas/L_Tutorial]
# Muestra las líneas [SAEVUS recorrido]; código de salida 0 solo si SUPERADO. Capturas: Saved\Screenshots\WindowsEditor\SAEVUS_Recorrido_*.png

param(
    [int]$Escalabilidad = -1,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [string]$ComandosExtra = "",
    [switch]$PerfilGPU,
    [string]$Mapa = "/Game/SAEVUS/Mapas/L_Tutorial",
    [string]$RaizMotor = "C:\Program Files\Epic Games\UE_5.8"
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent $PSScriptRoot
$Proyecto = Join-Path $RaizProyecto "SAEVUS.uproject"
$DirLogs = Join-Path $RaizProyecto "Saved\Logs\Verificacion"
New-Item -ItemType Directory -Force $DirLogs | Out-Null
& (Join-Path $PSScriptRoot "Limpiar-Logs.ps1") -DirLogs $DirLogs
$log = Join-Path $DirLogs "recorrido.log"

$comandos = @()
if ($Escalabilidad -ge 0) { $comandos += "scalability $Escalabilidad" }
if ($ComandosExtra) { $comandos += $ComandosExtra }
$argComandos = if ($comandos) { "-ExecCmds=`"$($comandos -join ',')`"" } else { "" }

# Una sola cadena literal de argumentos: Windows PowerShell 5.1 estropea los argumentos nativos entre comillas con espacios.
$argPerfil = if ($PerfilGPU) { "-SAEVUSPruebaHumoPerfilGPU" } else { "" }
$argumentos = "`"$Proyecto`" $Mapa -game -windowed -ResX=$ResX -ResY=$ResY -SAEVUSRecorrido -SAEVUSSinGuardar $argPerfil -nosplash -NoSound $argComandos `"-abslog=$log`""
Start-Process -FilePath (Join-Path $RaizMotor "Engine\Binaries\Win64\UnrealEditor.exe") -ArgumentList $argumentos -Wait | Out-Null

Select-String -Path $log -Pattern "\[SAEVUS recorrido\]|Critical error" -Encoding UTF8 | ForEach-Object { $_.Line -replace '^.*?\[SAEVUS recorrido\]', '[SAEVUS recorrido]' }
if (-not (Select-String -Path $log -Pattern "\[SAEVUS recorrido\] SUPERADO" -Quiet -Encoding UTF8)) { exit 1 }
