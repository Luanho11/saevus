# Arranca el juego empaquetado (Herramientas\Empaquetar.ps1) con la prueba de humo en el mapa de pruebas y
# comprueba que pasa: el ejecutable arranca, carga el mapa, el personaje se mueve, ataca, muere y reaparece.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Probar-Ejecutable.ps1 [-Escalabilidad 1]
# Para jugar sin la prueba: Saved\Empaquetado\Windows\SAEVUS.exe (arranca en el tutorial con el menú principal).

param(
    [int]$Escalabilidad = 1,
    [int]$ResX = 1280,
    [int]$ResY = 720
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent $PSScriptRoot
$Salida = Join-Path $RaizProyecto "Saved\Empaquetado"
$Exe = Get-ChildItem -Path $Salida -Recurse -Filter "SAEVUS.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $Exe) { throw "No hay ejecutable en $Salida; empaqueta antes con Herramientas\Empaquetar.ps1" }
$DirLogs = Join-Path $RaizProyecto "Saved\Logs\Verificacion"
New-Item -ItemType Directory -Force $DirLogs | Out-Null
$log = Join-Path $DirLogs "ejecutable.log"

$argumentos = "/Game/SAEVUS/Mapas/L_Pruebas -windowed -ResX=$ResX -ResY=$ResY -SAEVUSPruebaHumo -SAEVUSSinGuardar -nosplash -NoSound -ExecCmds=`"scalability $Escalabilidad`" `"-abslog=$log`""
Start-Process -FilePath $Exe.FullName -ArgumentList $argumentos -Wait | Out-Null

Select-String -Path $log -Pattern "\[SAEVUS humo\]|Critical error" -Encoding UTF8 | ForEach-Object { $_.Line -replace '^.*?\[SAEVUS humo\]', '[SAEVUS humo]' }
if (-not (Select-String -Path $log -Pattern "\[SAEVUS humo\] SUPERADA" -Quiet -Encoding UTF8)) { Write-Host "El ejecutable NO pasó la prueba de humo"; exit 1 }
Write-Host "El ejecutable pasó la prueba de humo: $($Exe.FullName)"
