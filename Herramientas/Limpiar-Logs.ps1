# Rota los logs de verificación: el motor renombra cada log anterior a "<nombre>-backup-<fecha>.log" y se acumulan.
# Conserva las $Conservar copias más recientes de cada tipo (assets, pruebas, humo...) y borra el resto.
# Lo llaman los scripts de Herramientas al empezar; también se puede ejecutar a mano:
#   powershell -ExecutionPolicy Bypass -File Herramientas\Limpiar-Logs.ps1 [-Conservar 5]

param(
    [int]$Conservar = 5,
    [string]$DirLogs = (Join-Path (Split-Path -Parent $PSScriptRoot) "Saved\Logs\Verificacion")
)

if (-not (Test-Path $DirLogs)) { return }
Get-ChildItem -Path $DirLogs -Filter "*-backup-*.log" |
    Group-Object { $_.Name -replace '-backup-.*$', '' } |
    ForEach-Object {
        $_.Group | Sort-Object LastWriteTime -Descending | Select-Object -Skip $Conservar |
            Remove-Item -Force -ErrorAction SilentlyContinue
    }
