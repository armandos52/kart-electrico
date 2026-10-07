param(
    [string]$Python = 'C:\Users\nandi\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe',
    [int]$Puerto = 8765
)
if (-not (Test-Path -LiteralPath $Python)) {
    throw 'Indica un ejecutable Python válido: .\iniciar.ps1 -Python C:\ruta\python.exe'
}
& $Python (Join-Path $PSScriptRoot 'server.py') --port $Puerto
