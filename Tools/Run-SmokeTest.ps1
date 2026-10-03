# Runs the in-game smoke test (USAEVUSSmokeTestSubsystem) in a real rendered -game session.
# Usage (from the project root):
#   powershell -ExecutionPolicy Bypass -File Tools\Run-SmokeTest.ps1 [-Scalability 0..3] [-ResX 1280 -ResY 720] [-ExtraCmds "r.ScreenPercentage 75"]
# Prints the [SAEVUS smoke] lines; exit code 0 only on PASSED. Screenshot: Saved\Screenshots\WindowsEditor\SAEVUS_SmokeTest.png

param(
    [int]$Scalability = -1,
    [int]$ResX = 1280,
    [int]$ResY = 720,
    [string]$ExtraCmds = "",
    [switch]$ProfileGPU,
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $ProjectRoot "SAEVUS.uproject"
$LogDir = Join-Path $ProjectRoot "Saved\Logs\Verify"
New-Item -ItemType Directory -Force $LogDir | Out-Null
$log = Join-Path $LogDir "smoke.log"

$cmds = @()
if ($Scalability -ge 0) { $cmds += "scalability $Scalability" }
if ($ExtraCmds) { $cmds += $ExtraCmds }
$execArg = if ($cmds) { "-ExecCmds=`"$($cmds -join ',')`"" } else { "" }

# One verbatim argument string: Windows PowerShell 5.1 mangles quoted native arguments that contain spaces.
$profileArg = if ($ProfileGPU) { "-SAEVUSSmokeProfileGPU" } else { "" }
$argLine = "`"$Project`" -game -windowed -ResX=$ResX -ResY=$ResY -SAEVUSSmokeTest $profileArg -nosplash -NoSound $execArg `"-abslog=$log`""
Start-Process -FilePath (Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe") -ArgumentList $argLine -Wait | Out-Null

Select-String -Path $log -Pattern "\[SAEVUS smoke\]|Critical error" | ForEach-Object { $_.Line -replace '^.*?\[SAEVUS smoke\]', '[SAEVUS smoke]' }
if (-not (Select-String -Path $log -Pattern "\[SAEVUS smoke\] PASSED" -Quiet)) { exit 1 }
