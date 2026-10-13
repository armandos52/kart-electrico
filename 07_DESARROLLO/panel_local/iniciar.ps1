param(
    [string]$Python = '',
    [int]$Puerto = 8766,
    # Fuente Wi-Fi opcional del ESP32, p. ej. -Wifi socket://192.168.4.1:3333
    [string]$Wifi = ''
)
if (-not $Python) {
    $entornoPanel = Join-Path $PSScriptRoot '.venv\Scripts\python.exe'
    if (Test-Path -LiteralPath $entornoPanel) { $Python = $entornoPanel }
    elseif (Get-Command python -ErrorAction SilentlyContinue) { $Python = 'python' }
    elseif (Get-Command py -ErrorAction SilentlyContinue) { $Python = 'py' }
    else { throw 'Instala Python 3.12+ o indica -Python C:\ruta\python.exe' }
}
$argumentos = @((Join-Path $PSScriptRoot 'server.py'), '--port', $Puerto)
if ($Wifi) { $argumentos += @('--wifi', $Wifi) }
& $Python @argumentos
