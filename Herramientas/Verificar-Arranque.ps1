# Compila SAEVUS, crea los assets, ejecuta los tests de automatización y verifica los assets.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Verificar-Arranque.ps1 [-SinAssets] [-ConJuego]
# -ConJuego añade la prueba de humo (L_Pruebas) y el recorrido del tutorial en el juego real (con render; varios minutos).
# Los logs van a Saved\Logs\Verificacion\. Código de salida 0 solo si pasan todos los pasos.

param(
    [string]$RaizMotor = "C:\Program Files\Epic Games\UE_5.8",
    [switch]$SinAssets,
    [switch]$ConJuego
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent $PSScriptRoot
$Proyecto = Join-Path $RaizProyecto "SAEVUS.uproject"
$EditorCmd = Join-Path $RaizMotor "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$DirLogs = Join-Path $RaizProyecto "Saved\Logs\Verificacion"
New-Item -ItemType Directory -Force $DirLogs | Out-Null
& (Join-Path $PSScriptRoot "Limpiar-Logs.ps1") -DirLogs $DirLogs

# ArgsEditor se pasa literal: Windows PowerShell 5.1 estropea los argumentos nativos entre comillas con espacios.
function Invocar-Editor([string]$Nombre, [string]$ArgsEditor) {
    $log = Join-Path $DirLogs "$Nombre.log"
    $argumentos = "`"$Proyecto`" $ArgsEditor -unattended -nopause -nullrhi -nosplash -NoSound `"-abslog=$log`""
    Start-Process -FilePath $EditorCmd -ArgumentList $argumentos -Wait -NoNewWindow | Out-Null
    if (Select-String -Path $log -Pattern "GetLastError=4551" -Quiet) {
        throw "Smart App Control de Windows bloqueó UnrealEditor-SAEVUS.dll. Desactívalo y vuelve a ejecutar."
    }
    return $log
}

Write-Host "== 1/4 Compilar SAEVUSEditor"
& (Join-Path $RaizMotor "Engine\Build\BatchFiles\Build.bat") SAEVUSEditor Win64 Development "-Project=$Proyecto" -WaitMutex -NoHotReload |
    Tee-Object -FilePath (Join-Path $DirLogs "compilacion.log") | Select-String -Pattern "error|warning C|Result:"
if ($LASTEXITCODE -ne 0) { throw "La compilación falló (ver compilacion.log)" }

if (-not $SinAssets) {
    Write-Host "== 2/4 Crear assets (los tests de animación usan el caballero importado)"
    $logAssets = Invocar-Editor "assets" "-ExecutePythonScript=`"$(Join-Path $PSScriptRoot 'ScriptsEditor\saevus_crear_assets.py')`""
    Select-String -Path $logAssets -Pattern "\[SAEVUS assets\]" -Encoding UTF8 | ForEach-Object { "  " + ($_.Line -replace '^.*?\[SAEVUS', '[SAEVUS') }
    if (Select-String -Path $logAssets -Pattern "\[SAEVUS assets\] FALLO" -Quiet -Encoding UTF8) { throw "Falló el script de assets (ver assets.log)" }
    # Un error de sintaxis de Python no escribe FALLO: se exige el "hecho" final.
    if (-not (Select-String -Path $logAssets -Pattern "\[SAEVUS assets\] hecho" -Quiet -Encoding UTF8)) { throw "El script de assets no terminó (ver assets.log)" }
}

Write-Host "== 3/4 Tests de automatización SAEVUS.*"
& (Join-Path $PSScriptRoot "Ejecutar-Pruebas.ps1") -RaizMotor $RaizMotor
if ($LASTEXITCODE -ne 0) { throw "Fallaron tests (ver pruebas.log)" }

Write-Host "== 4/4 Verificar assets"
$logVerificar = Invocar-Editor "verificar" "-ExecutePythonScript=`"$(Join-Path $PSScriptRoot 'ScriptsEditor\saevus_verificar_assets.py')`""
Select-String -Path $logVerificar -Pattern "\[SAEVUS verificar\] (FALLO|SUPERADA|FALLIDA)" -Encoding UTF8 | ForEach-Object { "  " + ($_.Line -replace '^.*?\[SAEVUS', '[SAEVUS') }
if (-not (Select-String -Path $logVerificar -Pattern "\[SAEVUS verificar\] SUPERADA" -Quiet -Encoding UTF8)) { throw "Falló la verificación de assets (ver verificar.log)" }

if ($ConJuego) {
    Write-Host "== 5/6 Prueba de humo en el juego (L_Pruebas)"
    & (Join-Path $PSScriptRoot "Ejecutar-PruebaHumo.ps1") -RaizMotor $RaizMotor -Escalabilidad 1 | Select-String -Pattern "FALLO|SUPERADA|FALLIDA|fotogramas"
    if ($LASTEXITCODE -ne 0) { throw "Falló la prueba de humo (ver humo.log)" }
    Write-Host "== 6/6 Recorrido del tutorial (L_Tutorial)"
    & (Join-Path $PSScriptRoot "Ejecutar-Recorrido.ps1") -RaizMotor $RaizMotor -Escalabilidad 1 | Select-String -Pattern "FALLO|SUPERADO|FALLIDO"
    if ($LASTEXITCODE -ne 0) { throw "Falló el recorrido del tutorial (ver recorrido.log)" }
}

Write-Host "Todas las comprobaciones pasaron."
