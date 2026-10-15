---
estado: vigente
actualizado: 2026-10-15
resumen: Evidencias de ENS-0019: respaldo/identificación F746, grabación dev4, capturas de framebuffer, registros CAN/táctil/SD y franja.
---

# Evidencias ENS-0019 · Frontal dev4 en la F746 real

Ensayo: [ENS-0019](../../ensayos/ENS-0019_frontal_dev4_f746_real.md). Respaldo y recuperación: [RESPALDO.md](../../../07_DESARROLLO/respaldos/stm32_f746/2026-10-15/RESPALDO.md). Integridad: `manifest.json`, generado por `conservar.py` con SHA-256 de todos los archivos, incluidos los `.raw` y los binarios de respaldo, que no se versionan.

| Grupo | Archivos | Contenido |
|---|---|---|
| Identificación | `identificar.cfg`, `identificacion_openocd.log`, `identificacion_tras_fallo.log` | ID, UID, option bytes, estado QSPI y vector de reset, antes y después del fallo SWD |
| Respaldo | `respaldar.cfg`, `respaldo_openocd_intento1.log`, `respaldar_qspi.cfg`, `respaldo_qspi_repeticion.log`, `hashes_respaldo.txt` | Lecturas sin halt/reset. El intento 1 muestra el error SWD de la segunda lectura QSPI a 4 MHz |
| Regresión | `frontend_host_dev4.log` | 101 comprobaciones, 0 fallos, antes de grabar |
| Grabación | `grabar_dev4.cfg`, `grabacion_dev4.log` | Comprobación de chip/UID/RDP, `program … verify`, Verified OK |
| Relectura | `releer_dev4.cfg`, `relectura_dev4.log`, `relectura/comparacion.txt`, `relectura/hashes.txt` | 0 diferencias frente al HEX. Binarios movidos a `respaldos/stm32_f746/2026-10-15/relectura_dev4/` |
| Observaciones SWD | `antes_original/`, `dev4_arranque/`, `dev4_tras_toques/`, `dev4_final/` | `pantalla.png`, `observacion.json` (LTDC, `ui`, `received`, avisos, `uwTick`), símbolos crudos, log de OpenOCD. Las CAN se leen con `estado_can.cfg` |
| Registros de estado | `seguir_tactil.cfg`/`tactil/`, `seguir_tactil2.cfg`/`tactil2/`, `muestrear_ft5336.cfg`/`tactil/ft5336.log`, `franja_fina_y_tactil.cfg`/`franja_fina/` | Cambios de START, modo, avisos, SD y contactos FT5336, con captura del framebuffer en cada cambio. `tactil/seguimiento_fallo_sin_init.log`: primer intento fallido por falta de `init`. Los registradores 2 y 3 se detuvieron a mano para liberar el ST-LINK (anotado al final de cada log) |
| Franja | `patron_franja.cfg`, `franja/`, `restaurar_franja.cfg` | Patrón en RAM del framebuffer, captura de comprobación y restauración a blanco |

Montaje para el informe: `tactil2/montaje_avisos_modos.png`, con avisos PONER N y PARAR Y SOLTAR PEDAL y modos ESC y BLQ seleccionados por táctil. Comparación antes/después: `antes_original/pantalla.png` frente a `dev4_arranque/pantalla.png` y `dev4_tras_toques/pantalla.png`.

**Límites:**

- Las imágenes son el contenido del framebuffer, no fotos del panel; lo visible en el panel lo declara Armando.
- En los registradores, los tiempos `t=` son del PC y el `tick` es del MCU.
- Los registros `tactil/` (13:33–13:43) no contienen las pulsaciones de Armando, que llegaron horas después. Son válidos como periodo sin contactos espontáneos.

Para reproducir la observación:

```powershell
python 07_DESARROLLO/firmware/stm32_frontal/tools/observar_swd.py --serie <ST-LINK> --salida <carpeta> --elf 07_DESARROLLO/respaldos/stm32/v1.3-dev4/stm32_frontal-v1.3-dev4.elf
```
