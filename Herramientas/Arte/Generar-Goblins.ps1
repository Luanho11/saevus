# Genera los goblins y el Rey Goblin (mallas esqueléticas, REF-05 y REF-06) con Blender en ArteFuente\Goblins\.
# El cuerpo del Rey sale de las vistas de REF-06 (vistas\rey_goblin.json), con su pintura en T_ReyGoblin.png, por la
# misma cadena que el protagonista (Generar-Caballero.ps1; Documentos/pipeline-assets.md). Los goblins de REF-05 solo
# tienen vistas de tres cuartos posadas: siguen siendo de piezas. Con -SinReconstruccion el Rey también es de piezas.
# Uso (desde la raíz del proyecto):  powershell -ExecutionPolicy Bypass -File Herramientas\Arte\Generar-Goblins.ps1

param(
    [string]$Blender = "C:\Program Files\Blender Foundation\Blender 4.2\blender.exe",
    [string]$Python = "python",
    [int]$TriangulosRey = 30000,
    [int]$ResolucionTextura = 2048,
    [switch]$SinReconstruccion
)

$ErrorActionPreference = "Stop"
$RaizProyecto = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$Salida = Join-Path $RaizProyecto "ArteFuente\Goblins"
# Intermedios pesados (puntos, malla densa) fuera del repositorio.
$Intermedios = Join-Path $RaizProyecto "Saved\ArteIntermedio\ReyGoblin"
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

$argumentosGoblins = @($Salida)
if (-not $SinReconstruccion) {
    Push-Location $RaizProyecto
    try {
        $puntos = Join-Path $Intermedios "rey.npz"
        $densa = Join-Path $Intermedios "densa.blend"
        $modelo = Join-Path $Intermedios "modelo.blend"
        $textura = Join-Path $Salida "T_ReyGoblin.png"
        Ejecutar "La reconstrucción del Rey" { & $Python (Join-Path $PSScriptRoot "reconstruir_desde_vistas.py") (Join-Path $PSScriptRoot "vistas\rey_goblin.json") $puntos }
        Ejecutar "La malla densa del Rey" { & $Blender --background --factory-startup --python (Join-Path $PSScriptRoot "blender_malla_desde_puntos.py") -- $puntos $densa 3000000 }
        Ejecutar "El horneado del Rey" { & $Blender --background $densa --python (Join-Path $PSScriptRoot "blender_hornear_color.py") -- $modelo $textura $TriangulosRey $ResolucionTextura }
        if (-not (Test-Path $textura)) { throw "El horneado no generó $textura" }
        $argumentosGoblins += $modelo
    }
    finally { Pop-Location }
}
Ejecutar "Los goblins" { & $Blender --background --factory-startup --python (Join-Path $PSScriptRoot "blender_goblins.py") -- @argumentosGoblins }
if (-not (Test-Path (Join-Path $Salida "SK_Goblin.fbx"))) { throw "Blender no generó SK_Goblin.fbx" }
