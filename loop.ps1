# Configuración
$MaxHours = 10
$StartTime = Get-Date
$EndTime = $StartTime.AddHours($MaxHours)
$WaitMinutes = 30 # Tiempo de espera entre reintentos si cae la cuota o falla

Write-Host "=== Iniciando ciclo de trabajo continuo para Claude Code en Windows ===" -ForegroundColor Green

while ((Get-Date) -lt $EndTime) {
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$Timestamp] Iniciando nueva iteración de Claude Code..." -ForegroundColor Cyan
    
    # Invocación directa pasando el flag de permisos necesarios para que trabaje sin pedir confirmación manual
    # Nota: Se pasa una instrucción directa en lugar de todo el texto raw para evitar fallos de escape de comillas en PowerShell.
    claude -p "Lee PROMPT_AUTONOMIA.md en la raíz del proyecto y síguelo al pie de la letra sin detenerte." --dangerously-skip-permissions
    
    $ExitCode = $LASTEXITCODE
    $Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$Timestamp] Claude finalizó con código de salida: $ExitCode" -ForegroundColor Yellow
    
    # Espera antes de la siguiente iteración
    Write-Host "Esperando $WaitMinutes minutos antes de verificar cuota y continuar..." -ForegroundColor Gray
    Start-Sleep -Seconds ($WaitMinutes * 60)
}

Write-Host "=== Tiempo total de ejecución ($MaxHours h+) alcanzado ===" -ForegroundColor Green