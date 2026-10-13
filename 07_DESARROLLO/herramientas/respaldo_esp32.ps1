# Respaldo de solo lectura de la flash completa del ESP32 del kart.
# No borra, no escribe flash y no toca eFuses. Cada conexión usa default-reset, que entra
# directamente en el modo descarga de la ROM: la aplicación no se ejecuta entre lecturas.
param(
    [Parameter(Mandatory)] [string]$Puerto,
    [int]$Baudios = 460800,
    [string]$Destino = "C:\kart proyecto\07_DESARROLLO\respaldos\esp32"
)
$ErrorActionPreference = 'Stop'
$venv = "$env:USERPROFILE\.venvs\kart-esp32\Scripts"
$esptool = "$venv\esptool.exe"
$espefuse = "$venv\espefuse.exe"
$dir = Join-Path $Destino (Get-Date -Format 'yyyy-MM-dd_HHmmss')
New-Item -ItemType Directory -Force $dir | Out-Null
$log = Join-Path $dir 'log.txt'

function Escribe([string]$t) { $t; Add-Content -Path $log -Value $t -Encoding utf8 }

function Paso([string]$nombre, [string[]]$argumentos, [string]$exe = $esptool) {
    Escribe "`n### $nombre`n> $(Split-Path $exe -Leaf) $($argumentos -join ' ')"
    # Salida directa al log mediante cmd: procesarla línea a línea en PowerShell frena a
    # esptool y puede cortar la lectura ("Serial data stream stopped"). Manda el código de salida.
    $env:PYTHONIOENCODING = 'utf-8'
    $linea = ($argumentos | ForEach-Object { if ("$_" -match '\s') { "`"$_`"" } else { "$_" } }) -join ' '
    cmd /c "`"$exe`" $linea >> `"$log`" 2>&1"
    $codigo = $LASTEXITCODE
    if ($codigo -ne 0) { throw "Falló: $nombre (código $codigo); ver $log" }
}

Escribe "Respaldo ESP32 $(Get-Date -Format s) puerto=$Puerto"
Paso 'versión esptool' @('version')
$base = @('--port', $Puerto, '--baud', $Baudios, '--before', 'default-reset', '--after', 'no-reset')
Paso 'identificación chip' ($base + 'chip-id')
Paso 'identificación flash' ($base + 'flash-id')
# Tamaño real detectado por la herramienta, no supuesto.
$m = Select-String -Path $log -Pattern 'Detected flash size:\s*(\d+)\s*MB' | Select-Object -Last 1
if (-not $m) { throw 'No se pudo leer el tamaño de flash detectado; revisar log.txt antes de seguir.' }
$tam = [int64]$m.Matches[0].Groups[1].Value * 1MB
Escribe "Tamaño detectado: $tam bytes"
Paso 'lectura 1' ($base + @('read-flash', '0', $tam, (Join-Path $dir 'flash_lectura1.bin')))
Paso 'lectura 2' ($base + @('read-flash', '0', $tam, (Join-Path $dir 'flash_lectura2.bin')))
# eFuses en modo resumen (solo lectura) para comprobar cifrado y protecciones.
Paso 'eFuses (resumen)' @('--port', $Puerto, '--before', 'default-reset', 'summary') $espefuse

$h1 = Get-FileHash (Join-Path $dir 'flash_lectura1.bin') -Algorithm SHA256
$h2 = Get-FileHash (Join-Path $dir 'flash_lectura2.bin') -Algorithm SHA256
$t1 = (Get-Item $h1.Path).Length; $t2 = (Get-Item $h2.Path).Length
$ok = ($h1.Hash -eq $h2.Hash) -and ($t1 -eq $t2)
@"

### Verificación
lectura1: $t1 bytes SHA-256 $($h1.Hash)
lectura2: $t2 bytes SHA-256 $($h2.Hash)
coinciden: $ok
"@ | ForEach-Object { Escribe $_ }
if (-not $ok) { Write-Warning 'Las lecturas NO coinciden: el respaldo no está verificado.' }
"Carpeta: $dir"
