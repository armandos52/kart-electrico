---
estado: abierto; motivo de reset observado, causa eléctrica sin medir
actualizado: 2026-10-21
---

# PROB-0016 · ESP32 informa último reinicio por baja tensión

[ENS-0026](../../06_TRABAJO/ensayos/ENS-0026_diagnostico_bloqueo_marchas.md): cabecera y 61 tramas TCP reales identifican `esp32_trasero-v1.2-diagnostico`, sesión `6fc7b05399a0f013`, `reset_reason=brownout_tension`. [Registro original](../../06_TRABAJO/evidencias/marchas_2026-10-21/wifi_raw.log) y [eventos UTC](../../06_TRABAJO/evidencias/marchas_2026-10-21/wifi_eventos.json).

Es el motivo conservado del último arranque: no se observó un nuevo reset durante los 12 s de captura, ni se midieron tensión mínima, duración o punto de caída. No prueba una batería descargada ni causalidad con EMI. Frontal recibe CAN fresco en las observaciones y el bloqueo D/R se reproduce por pedal; no atribuirlo a este reset sin evidencia temporal.

Pendiente: capturar alimentación/referencia/masa del ESP32 durante conexión del contactor y funcionamiento, con osciloscopio y registro Wi-Fi, y correlacionar reinicios/sesiones. Sin deshabilitar detector brownout. Ver [PROB-0015](PROB-0015_ruido_electromagnetico_sensores.md).
