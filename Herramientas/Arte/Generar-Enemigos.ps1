# Genera los cuerpos boceto de los enemigos con Blender en ArteFuente\Enemigos\.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Arte\Generar-Enemigos.ps1
# Después, Herramientas\Verificar-Arranque.ps1 los importa a /Game/SAEVUS/Arte/Enemigos.

param(
    [string]$Blender = "C:\Program Files\Blender Foundation\Blender 4.2\blender.exe"
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$Salida = Join-Path $RaizProyecto "ArteFuente\Enemigos"
New-Item -ItemType Directory -Force $Salida | Out-Null

# Blender escribe avisos inofensivos por stderr; Windows PowerShell 5.1 se detendría con ellos.
$ErrorActionPreference = "Continue"
$salidaBlender = & $Blender --background --factory-startup --python (Join-Path $PSScriptRoot "blender_enemigos.py") -- $Salida 2>&1 | ForEach-Object { "$_" }
$ErrorActionPreference = "Stop"
$salidaBlender | Select-String -Pattern "\[SAEVUS arte\]|Error|Traceback"
if (-not (Test-Path (Join-Path $Salida "SM_Jefe_ReyGoblin.fbx"))) { throw "Blender no generó los cuerpos de los enemigos" }
