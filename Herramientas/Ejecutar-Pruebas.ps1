# Ejecuta los tests de automatización de SAEVUS sin interfaz y escribe una línea por test.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Ejecutar-Pruebas.ps1 [-Filtro SAEVUS.Movimiento]
# Código de salida 0 solo si pasan todos. Log: Saved\Logs\Verificacion\pruebas.log

param(
    [string]$Filtro = "SAEVUS",
    [string]$RaizMotor = "C:\Program Files\Epic Games\UE_5.8"
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent $PSScriptRoot
$Proyecto = Join-Path $RaizProyecto "SAEVUS.uproject"
$DirLogs = Join-Path $RaizProyecto "Saved\Logs\Verificacion"
New-Item -ItemType Directory -Force $DirLogs | Out-Null
& (Join-Path $PSScriptRoot "Limpiar-Logs.ps1") -DirLogs $DirLogs
$log = Join-Path $DirLogs "pruebas.log"

# Una sola cadena literal de argumentos: Windows PowerShell 5.1 estropea los argumentos nativos entre comillas con espacios.
$argumentos = "`"$Proyecto`" -ExecCmds=`"Automation RunTests $Filtro;Quit`" -TestExit=`"Automation Test Queue Empty`" -unattended -nopause -nullrhi -nosplash -NoSound `"-abslog=$log`""
Start-Process -FilePath (Join-Path $RaizMotor "Engine\Binaries\Win64\UnrealEditor-Cmd.exe") -ArgumentList $argumentos -Wait -NoNewWindow | Out-Null

$resultados = Select-String -Path $log -Pattern "Test Completed\. Result=\{(\w+)\}.*Path=\{([^}]+)\}" -Encoding UTF8
$resultados | ForEach-Object { "{0,-8} {1}" -f $_.Matches[0].Groups[1].Value, $_.Matches[0].Groups[2].Value }
# Líneas informativas de los tests (medidas) y errores de comprobación.
Select-String -Path $log -Pattern "LogAutomationController: (Error: Expected|Error: .*SAEVUS|.*(cm,|pasos, esperados|disparos en))" -Encoding UTF8 |
    ForEach-Object { "  " + ($_.Line -replace '^.*?LogAutomationController: ', '') }
if (Select-String -Path $log -Pattern "Critical error|Fatal error" -Quiet) { Write-Host "CUELGUE (ver $log)"; exit 2 }
if (-not $resultados) { Write-Host "Sin resultados de tests (ver $log)"; exit 1 }
if ($resultados | Where-Object { $_.Matches[0].Groups[1].Value -ne "Success" }) { exit 1 }
Write-Host "Pasaron los $($resultados.Count) tests."
