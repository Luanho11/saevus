# Genera el protagonista con Blender (malla + esqueleto propio) en ArteFuente\Caballero\.
# El cuerpo sale de las vistas de REF-01 (Documentos/REFERENCIAS_ARTE.md), con su pintura en una textura:
#   1) reconstruir_desde_vistas.py (Python del sistema): volumen y color desde las vistas → puntos;
#   2) blender_malla_desde_puntos.py: puntos → malla densa con color por vértice;
#   3) blender_hornear_color.py: malla de juego reducida con UV y la pintura horneada en T_Caballero.png;
#   4) blender_caballero.py: esqueleto, pesos, pose de reposo de la animación, espada y FBX.
# Con -SinReconstruccion se genera el cuerpo de piezas (v3) sin textura.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Arte\Generar-Caballero.ps1
# Después, Herramientas\Verificar-Arranque.ps1 lo importa a /Game/SAEVUS/Arte/Personajes/Caballero.

param(
    [string]$Blender = "C:\Program Files\Blender Foundation\Blender 4.2\blender.exe",
    [string]$Python = "python",
    [int]$Triangulos = 30000,
    [int]$ResolucionTextura = 2048,
    [switch]$SinReconstruccion
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$Salida = Join-Path $RaizProyecto "ArteFuente\Caballero"
# Intermedios pesados (puntos, malla densa) fuera del repositorio.
$Intermedios = Join-Path $RaizProyecto "Saved\ArteIntermedio\Caballero"
New-Item -ItemType Directory -Force $Salida, $Intermedios | Out-Null

function Ejecutar([string]$Descripcion, [scriptblock]$Orden) {
    # Blender escribe avisos inofensivos por stderr; Windows PowerShell 5.1 se detendría con ellos.
    $ErrorActionPreference = "Continue"
    $texto = & $Orden 2>&1 | ForEach-Object { "$_" }
    $codigo = $LASTEXITCODE
    $ErrorActionPreference = "Stop"
    $texto | Select-String -Pattern "\[SAEVUS arte\]|\[reconstruir\]|Error|Traceback"
    if ($codigo -ne 0) { throw "$Descripcion terminó con código $codigo" }
}

$argumentosCaballero = @($Salida)
if (-not $SinReconstruccion) {
    Push-Location $RaizProyecto
    try {
        $puntos = Join-Path $Intermedios "protagonista.npz"
        $densa = Join-Path $Intermedios "densa.blend"
        $modelo = Join-Path $Intermedios "modelo.blend"
        $textura = Join-Path $Salida "T_Caballero.png"
        Ejecutar "La reconstrucción" { & $Python (Join-Path $PSScriptRoot "reconstruir_desde_vistas.py") (Join-Path $PSScriptRoot "vistas\protagonista.json") $puntos }
        Ejecutar "La malla densa" { & $Blender --background --factory-startup --python (Join-Path $PSScriptRoot "blender_malla_desde_puntos.py") -- $puntos $densa 3000000 }
        Ejecutar "El horneado" { & $Blender --background $densa --python (Join-Path $PSScriptRoot "blender_hornear_color.py") -- $modelo $textura $Triangulos $ResolucionTextura }
        if (-not (Test-Path $textura)) { throw "El horneado no generó $textura" }
        $argumentosCaballero += $modelo
    }
    finally { Pop-Location }
}
Ejecutar "El caballero" { & $Blender --background --factory-startup --python (Join-Path $PSScriptRoot "blender_caballero.py") -- @argumentosCaballero }
if (-not (Test-Path (Join-Path $Salida "SK_Caballero.fbx"))) { throw "Blender no generó SK_Caballero.fbx" }
