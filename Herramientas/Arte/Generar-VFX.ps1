# Genera las formas de los efectos visuales (REF-08) con Blender en ArteFuente\VFX\.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Arte\Generar-VFX.ps1

param(
    [string]$Blender = "C:\Program Files\Blender Foundation\Blender 4.2\blender.exe"
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$Salida = Join-Path $RaizProyecto "ArteFuente\VFX"
New-Item -ItemType Directory -Force $Salida | Out-Null

# Blender escribe avisos inofensivos por stderr; Windows PowerShell 5.1 se detendría con ellos.
$ErrorActionPreference = "Continue"
$salidaBlender = & $Blender --background --factory-startup --python (Join-Path $PSScriptRoot "blender_vfx.py") -- $Salida 2>&1 | ForEach-Object { "$_" }
$ErrorActionPreference = "Stop"
$salidaBlender | Select-String -Pattern "\[SAEVUS arte\]|Error|Traceback"
if (-not (Test-Path (Join-Path $Salida "SM_VFX_Chispa.fbx"))) { throw "Blender no generó SM_VFX_Chispa.fbx" }
