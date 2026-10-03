# Runs SAEVUS automation tests headless and prints one line per test.
# Usage (from the project root):  powershell -ExecutionPolicy Bypass -File Tools\Run-Tests.ps1 [-Filter SAEVUS.Movement]
# Exit code 0 only if every test passed. Log: Saved\Logs\Verify\tests.log

param(
    [string]$Filter = "SAEVUS",
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $ProjectRoot "SAEVUS.uproject"
$LogDir = Join-Path $ProjectRoot "Saved\Logs\Verify"
New-Item -ItemType Directory -Force $LogDir | Out-Null
$log = Join-Path $LogDir "tests.log"

# One verbatim argument string: Windows PowerShell 5.1 mangles quoted native arguments that contain spaces.
$argLine = "`"$Project`" -ExecCmds=`"Automation RunTests $Filter;Quit`" -TestExit=`"Automation Test Queue Empty`" -unattended -nopause -nullrhi -nosplash -NoSound `"-abslog=$log`""
Start-Process -FilePath (Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe") -ArgumentList $argLine -Wait -NoNewWindow | Out-Null

$results = Select-String -Path $log -Pattern "Test Completed\. Result=\{(\w+)\}.*Path=\{([^}]+)\}"
$results | ForEach-Object { "{0,-8} {1}" -f $_.Matches[0].Groups[1].Value, $_.Matches[0].Groups[2].Value }
Select-String -Path $log -Pattern "LogAutomationController: (Error: Expected|Error: .*SAEVUS|.*((apex|speed|distance|height change|Climb height).*cm|After the climb|lasted .* steps))" |
    ForEach-Object { "  " + ($_.Line -replace '^.*?LogAutomationController: ', '') }
if (Select-String -Path $log -Pattern "Critical error|Fatal error" -Quiet) { Write-Host "CRASH (see $log)"; exit 2 }
if (-not $results) { Write-Host "No test results (see $log)"; exit 1 }
if ($results | Where-Object { $_.Matches[0].Groups[1].Value -ne "Success" }) { exit 1 }
Write-Host "All $($results.Count) tests passed."
