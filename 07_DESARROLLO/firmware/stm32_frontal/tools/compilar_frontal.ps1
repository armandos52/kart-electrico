# Compila el firmware frontal con el entorno del proyecto: ARM GNU portable + Python 3.12 del venv.
# No graba nada. La grabación sigue README_ROBUSTEZ.md y exige respaldo previo de interna + QSPI.
param(
    [string]$Python = 'C:\Users\nandi\.venvs\kart-esp32\Scripts\python.exe'
)
$ErrorActionPreference = 'Stop'
$build = Join-Path $PSScriptRoot 'build.py'
$toolchain = Resolve-Path (Join-Path $PSScriptRoot '..\..\..\herramientas\toolchains\bin')
if (-not (Test-Path $Python)) { throw "Falta Python 3.12 en $Python (usar -Python para indicar otro)." }
if (-not (Test-Path (Join-Path $toolchain 'arm-none-eabi-gcc.exe'))) { throw "Falta la toolchain ARM en $toolchain" }
& $Python $build
if ($LASTEXITCODE -ne 0) { throw "La compilación falló (código $LASTEXITCODE)." }
$salida = Join-Path $PSScriptRoot '..\.build'
Get-ChildItem $salida -Filter 'stm32_frontal-*.hex' | Sort-Object LastWriteTime -Descending | Select-Object -First 1 | ForEach-Object {
    "{0}  {1:N0} bytes  SHA-256 {2}" -f $_.Name, $_.Length, (Get-FileHash $_.FullName -Algorithm SHA256).Hash
}
