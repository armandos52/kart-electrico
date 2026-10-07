param(
    [string]$Python = 'C:\Users\nandi\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe',
    [int]$Puerto = 8765,
    # Fuente Wi-Fi opcional del ESP32, p. ej. -Wifi socket://192.168.4.1:3333
    [string]$Wifi = ''
)
if (-not (Test-Path -LiteralPath $Python)) {
    throw 'Indica un ejecutable Python válido: .\iniciar.ps1 -Python C:\ruta\python.exe'
}
$argumentos = @((Join-Path $PSScriptRoot 'server.py'), '--port', $Puerto)
if ($Wifi) { $argumentos += @('--wifi', $Wifi) }
& $Python @argumentos
