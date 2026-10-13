---
estado: corrección comprobada y compilada; sin grabación ni validación física
fecha: 2026-10-11
configuracion: ESP32 v1.3-dev3 en PC; STM32 v1.3-dev2 de trabajo; ESP32 físico v1.2 según Armando
---

# ENS-0011 · STOP conservado al recibir un lote CAN

## Objetivo y motivo

Continuar la revisión de control y DIF/BLQ/ESC solicitada por Armando. [Contexto confirmado y análisis](../RETOMA_PROGRAMA_2026-10-11.md). La v1.3-dev2 guardaba únicamente la última orden en `Controller::receive`. `receiveCan` puede drenar hasta ocho tramas antes de `Controller::step` (límite temporal de recepción 2000 µs en código, no medido hoy). Por ello, STOP seguido de START antes de `step` borraba la parada tanto con tracción como durante precarga. Hallazgo de software; no se ha observado ni se atribuye a una incidencia física.

## Configuración y procedimiento

- Rama `codex/robustez-v13`; HEAD inicial `4f7f150`, fuente dev2 de control en `8a60a6e`. Se preservan los cambios locales ajenos al arreglo.
- MSVC x64 y fixtures del núcleo sin periféricos; pruebas frontales con respuestas HAL/SD controladas. No modelo dinámico, hardware, escritura de flash ni conexión al kart.
- Antes de editar control, se copian `.cpp`, `.h` y tests a [antes](../evidencias/REV_2026-10-11/antes/). No contienen credenciales. [Manifiesto SHA-256](../evidencias/REV_2026-10-11/hashes.json) identifica fuentes, logs y binarios; [diff](../evidencias/REV_2026-10-11/cambio_control.diff) conserva el cambio sobre HEAD.
- Primero se repiten pruebas existentes. Después se añaden cinco comprobaciones: STOP/START en el mismo lote con tracción, no reutilización del START al volver a N, nuevo OFF→START válido, STOP/START durante precarga y ausencia de reinicio de precarga con START mantenido. Se ejecutan antes y después del arreglo.

## Cambio

`Controller::receive` memoriza cada STOP válido mediante `stop_pending_`. `step` lo consume y aplica salidas de parada antes de considerar órdenes de marcha, incluso si otra trama ha sustituido al mando recibido. Cuando el último mando del lote ya era START, ese flanco no queda habilitado para el ciclo siguiente: se exige recibir de nuevo OFF y después START con las condiciones de arranque vigentes. STOP continuo conserva su comportamiento anterior y sigue prevaleciendo sobre diagnóstico frontal inválido.

ESP32 pasa a `esp32_trasero-v1.3-dev3`; STM32 permanece en dev2. No cambian estructura CAN/JSON, pines, relés, tiempos, calibraciones, geometría, modos, ESC, R20 ni aplicación de salidas. El estado de inhibición usa el motivo existente `start_off` cuando se aplica el STOP conservado; la orden recibida puede ya indicar START, de forma que orden y actuación son deliberadamente distintas durante ese ciclo.

## Resultados originales

| Comprobación | Resultado | Log |
|---|---|---|
| Núcleo anterior, pruebas existentes | 51; 0 fallos | [control.log](../evidencias/REV_2026-10-11/control.log) |
| Nuevas regresiones sobre núcleo anterior | 56; **4 fallos** | [antes del arreglo](../evidencias/REV_2026-10-11/control_regresion_antes.log) |
| Núcleo dev3 corregido | 56; **0 fallos** | [control_dev3.log](../evidencias/REV_2026-10-11/control_dev3.log) |
| Frontal dev2 sin modificar | 54; 0 fallos | [frontend.log](../evidencias/REV_2026-10-11/frontend.log) |
| Panel JavaScript | 16; 0 fallos | [panel_js.log](../evidencias/REV_2026-10-11/panel_js.log) |
| Panel Python, ejecución final | 26 tests; OK | [panel_python_final.log](../evidencias/REV_2026-10-11/panel_python_final.log) |
| ESP32 dev3, PlatformIO entorno usb, solo compilación | SUCCESS; RAM 49 244 B, flash 949 795 B | [build_esp32_dev3_final.log](../evidencias/REV_2026-10-11/build_esp32_dev3_final.log) |

Binario/ELF/bootloader/particiones dev3 conservados en `07_DESARROLLO/respaldos/esp32/v1.3-dev3/`, excluidos de Git y separados de dev2/v1.2/original. Son resultados de compilación, no lecturas de placa; el binario incorpora configuración Wi-Fi privada. Plataforma pioarduino 55.03.311/core 3.3.11 ya fijada; se mantienen tres advertencias de operaciones `volatile` en contadores Hall y dos advertencias del arnés frontal MSVC. No se afirma compilación sin advertencias. Firmware físico intacto.

Intentos negativos del entorno preservados: [primer Python](../evidencias/REV_2026-10-11/panel_python.log), 25 errores ligados a permisos de temporales; [reintento con TEMP/TMP locales](../evidencias/REV_2026-10-11/panel_python_reintento.log), una prueba TCP loopback bloqueada por WinError 10013. La ejecución con permisos del entorno completó los 26 tests; no conecta al kart. [Primera compilación ESP32](../evidencias/REV_2026-10-11/build_esp32_dev3.log) bloqueada por acceso a caché PlatformIO; se repite usando la caché existente, sin borrarla. Estas limitaciones no se contabilizan como fallos del firmware.

## Límite y siguiente paso

Se acredita la regresión de software y su corrección en PC. No se mide latencia real de STOP, contactos/PWM, tráfico CAN, ni eficacia de los modos. Las pruebas anteriores no cubrían agrupación STOP/START; no constituyen una demostración exhaustiva. PROB-0008 permanece abierto a validación física.

Siguiente: pruebas conjuntas frontal/trasero y presentación de estados, después identificación/respaldo STM32+QSPI, banco y captura integrada por Wi-Fi. USB del ESP32 solo fuera del kart. Sin grabación, commit, push ni copia independiente nueva. La confirmación de v1.2 instalada es declaración de Armando, no lectura binaria de esta sesión.
