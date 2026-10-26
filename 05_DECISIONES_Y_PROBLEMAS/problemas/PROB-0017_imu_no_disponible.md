---
estado: abierto; indisponibilidad informada por firmware
actualizado: 2026-10-21
---

# PROB-0017 · MPU no disponible en ESP32 v1.2

## Actualización · 26-10-2026

[Declaración de Armando y plan de contraste](../../06_TRABAJO/AVANCES_2026-10-26.md): informa de conexión incorrecta del condensador de dirección entre masa y REF, prevista entre masa y VCC, y atribuye pérdida aproximada de medio rango ADC. Sin esquema ni medidas antes/después; reparación y relación con ruido general pendientes. Además busca una IMU para modos dependientes y telemetría; sin selección, instalación ni validación nueva. Este problema continúa abierto.

[ENS-0026](../../06_TRABAJO/ensayos/ENS-0026_diagnostico_bloqueo_marchas.md): 61 muestras Wi-Fi de la sesión `6fc7b05399a0f013` reportan `mpu_ok=0`, firmware `esp32_trasero-v1.2-diagnostico`. [Telemetría original](../../06_TRABAJO/evidencias/marchas_2026-10-21/wifi_raw.log). No hubo identificación física, inspección I²C o medida de alimentación de la IMU.

En v1.2 esa bandera depende de la detección inicial; no identifica por sí misma cable, sensor ni causa averiada. El modo ESC requiere MPU y su lógica inhibe tracción cuando no está disponible. En la captura real el modo era DIF y el motivo `neutral`; por tanto este hallazgo no explica el bloqueo del selector frontal ni constituye un ensayo físico de ESC.

Pendiente: revisar alimentación/masa/I²C e identidad del módulo, comparar con diagnóstico al arrancar y validar lectura. No sustituir guiñada ausente por cero válido ni afirmar reparación con cambios del pedal.
