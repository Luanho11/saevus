# Ejecuta la prueba de humo en el juego (USAEVUSSubsistemaPruebaHumo) en una sesión -game real con render.
# Uso (desde la raíz del proyecto):
#   powershell -ExecutionPolicy Bypass -File Herramientas\Ejecutar-PruebaHumo.ps1 [-Escalabilidad 0..3] [-ResX 1280 -ResY 720] [-ComandosExtra "r.ScreenPercentage 75"] [-PerfilGPU] [-Mapa /Game/SAEVUS/Mapas/L_Pruebas]
# Muestra las líneas [SAEVUS humo]; código de salida 0 solo si SUPERADA. Captura: Saved\Screenshots\WindowsEditor\SAEVUS_PruebaHumo.png

param(
    [int]$Escalabilidad = -1,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [string]$ComandosExtra = "",
    [switch]$PerfilGPU,
    # La prueba de humo necesita el mapa de pruebas (suelo llano largo); el juego arranca en el tutorial.
    [string]$Mapa = "/Game/SAEVUS/Mapas/L_Pruebas",
    [string]$RaizMotor = "C:\Program Files\Epic Games\UE_5.8"
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent $PSScriptRoot
$Proyecto = Join-Path $RaizProyecto "SAEVUS.uproject"
$DirLogs = Join-Path $RaizProyecto "Saved\Logs\Verificacion"
New-Item -ItemType Directory -Force $DirLogs | Out-Null
$log = Join-Path $DirLogs "humo.log"

$comandos = @()
if ($Escalabilidad -ge 0) { $comandos += "scalability $Escalabilidad" }
if ($ComandosExtra) { $comandos += $ComandosExtra }
$argComandos = if ($comandos) { "-ExecCmds=`"$($comandos -join ',')`"" } else { "" }

# Una sola cadena literal de argumentos: Windows PowerShell 5.1 estropea los argumentos nativos entre comillas con espacios.
$argPerfil = if ($PerfilGPU) { "-SAEVUSPruebaHumoPerfilGPU" } else { "" }
$argumentos = "`"$Proyecto`" $Mapa -game -windowed -ResX=$ResX -ResY=$ResY -SAEVUSPruebaHumo $argPerfil -nosplash -NoSound $argComandos `"-abslog=$log`""
Start-Process -FilePath (Join-Path $RaizMotor "Engine\Binaries\Win64\UnrealEditor.exe") -ArgumentList $argumentos -Wait | Out-Null

Select-String -Path $log -Pattern "\[SAEVUS humo\]|Critical error" -Encoding UTF8 | ForEach-Object { $_.Line -replace '^.*?\[SAEVUS humo\]', '[SAEVUS humo]' }
if (-not (Select-String -Path $log -Pattern "\[SAEVUS humo\] SUPERADA" -Quiet -Encoding UTF8)) { exit 1 }
