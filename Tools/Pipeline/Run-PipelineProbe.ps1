# Blender -> Unreal round trip check for the asset pipeline (Docs/asset-pipeline.md).
# Exports a character-sized probe mesh from Blender (FBX + GLB), imports both into a scratch
# folder in UE, logs their bounds, and deletes the scratch folder.
# Usage (from the project root):  powershell -ExecutionPolicy Bypass -File Tools\Pipeline\Run-PipelineProbe.ps1

param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [string]$Blender = "C:\Program Files\Blender Foundation\Blender 4.2\blender.exe"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$Project = Join-Path $ProjectRoot "SAEVUS.uproject"
$ProbeDir = Join-Path $ProjectRoot "Saved\PipelineProbe"
$LogDir = Join-Path $ProjectRoot "Saved\Logs\Verify"
New-Item -ItemType Directory -Force $ProbeDir, $LogDir | Out-Null

Write-Host "== Blender export"
# Blender writes harmless notices to stderr; Windows PowerShell 5.1 would stop on them.
$ErrorActionPreference = "Continue"
$blenderOutput = & $Blender --background --factory-startup --python (Join-Path $PSScriptRoot "blender_probe_export.py") -- $ProbeDir 2>&1 | ForEach-Object { "$_" }
$ErrorActionPreference = "Stop"
$blenderOutput | Select-String -Pattern "\[SAEVUS probe\]|Error|Traceback"

Write-Host "== Unreal import"
$log = Join-Path $LogDir "pipeline-probe.log"
$script = Join-Path $PSScriptRoot "ue_probe_import.py"
# One verbatim argument string: Windows PowerShell 5.1 mangles quoted native arguments that contain spaces.
$argLine = "`"$Project`" -ExecutePythonScript=`"$script`" -unattended -nopause -nullrhi -nosplash -NoSound `"-abslog=$log`""
Start-Process -FilePath (Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe") -ArgumentList $argLine -Wait -NoNewWindow | Out-Null
Select-String -Path $log -Pattern "\[SAEVUS probe\]" | ForEach-Object { "  " + ($_.Line -replace '^.*?\[SAEVUS probe\]', '[SAEVUS probe]') }
