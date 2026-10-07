---
estado: vigente
actualizado: 2026-10-07
resumen: Respaldo físico verificado (3 lecturas, mismo SHA-256) de la flash completa de 4 MB del ESP32 trasero, antes de cualquier cambio.
---

# Respaldo de la flash del ESP32 trasero — 2026-10-07 18:01

**Verificado por lectura:** tres lecturas completas e independientes dan el mismo tamaño y SHA-256. No se ha ensayado ninguna restauración.

| Lectura | Carpeta | Bytes | SHA-256 |
|---|---|---|---|
| 1 (17:59) | `../2026-10-07_175906/` (intento cuya 2.ª lectura falló al conectar) | 4 194 304 | `31a14b7b4c80eeca6b1ebe5374180bfeccc6c7d6cb5c17f61d70b5365ffb01a8` |
| 2 (18:01) | esta, `flash_lectura1.bin` | 4 194 304 | ídem |
| 3 (18:03) | esta, `flash_lectura2.bin` | 4 194 304 | ídem |

Entre lecturas cada conexión usa `--before default-reset`: el chip entra directamente en el modo descarga de la ROM y la aplicación no se ejecuta, por lo que no pudo escribir en la flash.

## Placa y chip (leídos por esptool, no supuestos)

- Puente USB: Silicon Labs CP2102 (VID 10C4, PID EA60), COM4 en este portátil con el driver CP210x.
- Chip: **ESP32-D0WD-V3, revisión v3.1**, doble núcleo 240 MHz, cristal 40 MHz, MAC `70:4b:ca:7c:df:68`.
- Flash: **4 MB** (fabricante 0x46, dispositivo 0x4016), tensión 3,3 V fijada por el pin de arranque (GPIO12 bajo al arrancar).
- La ficha [[03_CATALOGO/fichas/DOC-0032|DOC-0032]] (Keyestudio KS0413) es compatible pero no queda confirmada como la placa montada.

## Seguridad (eFuses, solo lectura)

`FLASH_CRYPT_CNT = 0` (sin cifrado de flash), `ABS_DONE_0/1 = False` (sin Secure Boot), `RD_DIS = 0`, `WR_DIS = 0`, `UART_DOWNLOAD_DIS = False`, `JTAG_DISABLE = False`. No se ha modificado ninguna eFuse. El resumen completo está en `log.txt`.

## Contenido (analizado con `07_DESARROLLO/herramientas/analiza_flash_esp32.py`)

| Partición | Tipo | Dirección | Tamaño | Contenido |
|---|---|---|---|---|
| nvs | data/nvs | 0x9000 | 20 KiB | Calibración PHY y **configuración Wi-Fi de estación (`sta.ssid`, `sta.pswd`)**, probablemente de un firmware anterior |
| otadata | data/ota | 0xE000 | 8 KiB | seq 1 → arranca `app0` |
| app0 | app/ota_0 | 0x10000 | 1280 KiB | Aplicación grabada: imagen de **345 808 bytes** (6 segmentos, SHA-256 interno correcto). Le siguen ~585 KB no borrados de un firmware anterior, sin cadenas de aplicación |
| app1 | app/ota_1 | 0x150000 | 1280 KiB | Vacía |
| spiffs | data/spiffs | 0x290000 | 1408 KiB | Vacía |
| coredump | data/coredump | 0x3F0000 | 64 KiB | **Contiene un volcado de fallo ELF de 10 848 bytes** |

Es la tabla «default» de Arduino-ESP32 para 4 MB.

Aplicación `app0`:

- Descriptor: proyecto `arduino-lib-builder`, ESP-IDF **v5.5.4**, compilado «Jun 2 2026 11:21:21» (fecha de las bibliotecas precompiladas, no del sketch).
- Las rutas embebidas indican **Arduino IDE en Windows con el core `esp32` 3.3.10**.
- Las 11 cadenas literales del fuente entregado (`ESP32-final.ino`) aparecen en la imagen: «CAN OK!», «A calibrar giroscopio...», «KART ELÉTRICO - DIF | ESC | BLQ», mensajes de relés y cargador.

- El fuente entregado compilado con PlatformIO (core 3.3.11, mcp_can 1.5.1, MPU6050 1.4.5) da una imagen de 346 544 bytes, 736 bytes más.
- De las cadenas de la imagen grabada, solo difieren rutas e internos del IDF.

Esto es coherente con que el fuente entregado sea el grabado, pero **no lo demuestra**: constantes o lógica podrían diferir sin cambiar los textos. Una comparación más fuerte sería compilar con el core exacto 3.3.10 (arduino-cli) y comparar la imagen por secciones.

## Qué no cubre

- No recupera el fuente editable ni el ELF.
- No respalda las eFuses (se registró su resumen), la RAM ni el estado de los periféricos.
- El coredump no se puede descodificar sin el ELF de esa compilación exacta.
  - Un intento posterior: recompilar con core 3.3.10 y comparar.
  - Puede guardar relación con [[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender|PROB-0002]].

## Copias

1. Esta carpeta: `flash_lectura1.bin`, `flash_lectura2.bin`, `SHA256SUMS.txt`, `log.txt`.
   - Los `.bin` **no se versionan** (`.gitignore`): el repositorio es público y la NVS contiene credenciales Wi-Fi.
2. `%USERPROFILE%\respaldos-kart\esp32\2026-10-07_180133\` (mismo hash comprobado).
   - **Mismo disco: no es una copia independiente.**

**Pendiente:** copia en otro soporte (SSD externo u otro destino que decida Armando).

## Restaurar a ESTA placa (no ejecutado, no ensayado)

Solo si hiciera falta volver al estado del 07-10-2026. El banco tiene que estar igual que en la lectura: tracción sin energía y ruedas inmovilizadas.

```powershell
$e = "$env:USERPROFILE\.venvs\kart-esp32\Scripts\esptool.exe"
$b = "C:\kart proyecto\07_DESARROLLO\respaldos\esp32\2026-10-07_180133\flash_lectura1.bin"
# 1. comprobar el hash del archivo antes de grabar
(Get-FileHash $b -Algorithm SHA256).Hash   # debe ser 31A14B7B...FFB01A8
# 2. grabar la imagen completa desde 0x0, sin cambiar modo/frecuencia/tamaño de flash
& $e --port COM4 --baud 460800 write-flash --flash-mode keep --flash-freq keep --flash-size keep 0x0 $b
# 3. comprobar contra la flash
& $e --port COM4 --baud 460800 verify-flash 0x0 $b
```

La imagen incluye NVS (calibración PHY propia de este chip) y el coredump. En otra placa funcionaría la aplicación, pero se copiarían datos ajenos al chip.

## Reproducir

`07_DESARROLLO/herramientas/respaldo_esp32.ps1 -Puerto COM4` usa esptool 5.4.0 en el entorno `%USERPROFILE%\.venvs\kart-esp32` (Python 3.12). Hace dos lecturas completas, el resumen de eFuses y la comparación SHA-256. No borra, no escribe flash y no toca eFuses.
