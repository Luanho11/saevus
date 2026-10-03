# Builds SAEVUS, runs the automation tests, creates the bootstrap assets and verifies them.
# Usage (from the project root):  powershell -ExecutionPolicy Bypass -File Tools\Verify-Bootstrap.ps1 [-SkipAssets]
# Logs go to Saved\Logs\Verify\. Exit code 0 only if every step passed.

param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [switch]$SkipAssets
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $ProjectRoot "SAEVUS.uproject"
$EditorCmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$LogDir = Join-Path $ProjectRoot "Saved\Logs\Verify"
New-Item -ItemType Directory -Force $LogDir | Out-Null

# EditorArgs is passed verbatim: Windows PowerShell 5.1 mangles quoted native arguments that contain spaces.
function Invoke-Editor([string]$Name, [string]$EditorArgs) {
    $log = Join-Path $LogDir "$Name.log"
    $argLine = "`"$Project`" $EditorArgs -unattended -nopause -nullrhi -nosplash -NoSound `"-abslog=$log`""
    Start-Process -FilePath $EditorCmd -ArgumentList $argLine -Wait -NoNewWindow | Out-Null
    if (Select-String -Path $log -Pattern "GetLastError=4551" -Quiet) {
        throw "Windows Smart App Control blocked UnrealEditor-SAEVUS.dll. Turn it off, then re-run."
    }
    return $log
}

Write-Host "== 1/4 Build SAEVUSEditor"
& (Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat") SAEVUSEditor Win64 Development "-Project=$Project" -WaitMutex -NoHotReload |
    Tee-Object -FilePath (Join-Path $LogDir "build.log") | Select-String -Pattern "error|warning C|Result:"
if ($LASTEXITCODE -ne 0) { throw "Build failed (see build.log)" }

Write-Host "== 2/4 Automation tests SAEVUS.*"
$testLog = Invoke-Editor "tests" '-ExecCmds="Automation RunTests SAEVUS;Quit" -TestExit="Automation Test Queue Empty"'
$results = Select-String -Path $testLog -Pattern "Test Completed\. Result=\{(\w+)\}.*Path=\{([^}]+)\}"
$results | ForEach-Object { "  {0,-8} {1}" -f $_.Matches[0].Groups[1].Value, $_.Matches[0].Groups[2].Value }
Select-String -Path $testLog -Pattern "Jump apex|Air speed" | ForEach-Object { "  " + ($_.Line -replace '^.*?(Jump apex|Air speed)', '$1') }
if (-not $results) { throw "No test results found (see tests.log)" }
if ($results | Where-Object { $_.Matches[0].Groups[1].Value -ne "Success" }) { throw "Some tests failed (see tests.log)" }

if (-not $SkipAssets) {
    Write-Host "== 3/4 Create bootstrap assets"
    $assetLog = Invoke-Editor "assets" "-ExecutePythonScript=`"$(Join-Path $PSScriptRoot 'EditorScripts\saevus_bootstrap_assets.py')`""
    Select-String -Path $assetLog -Pattern "\[SAEVUS bootstrap\]" | ForEach-Object { "  " + ($_.Line -replace '^.*?\[SAEVUS', '[SAEVUS') }
    if (Select-String -Path $assetLog -Pattern "\[SAEVUS bootstrap\] FAILED" -Quiet) { throw "Asset script failed (see assets.log)" }
}

Write-Host "== 4/4 Verify assets"
$verifyLog = Invoke-Editor "verify" "-ExecutePythonScript=`"$(Join-Path $PSScriptRoot 'EditorScripts\saevus_verify_assets.py')`""
Select-String -Path $verifyLog -Pattern "\[SAEVUS verify\]" | ForEach-Object { "  " + ($_.Line -replace '^.*?\[SAEVUS', '[SAEVUS') }
if (-not (Select-String -Path $verifyLog -Pattern "\[SAEVUS verify\] PASSED" -Quiet)) { throw "Asset verification failed (see verify.log)" }

Write-Host "All bootstrap checks passed."
