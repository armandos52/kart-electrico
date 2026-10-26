---
estado: abierto; diagnóstico y propuesta, sin implementación nueva
actualizado: 2026-10-17
---

# PROB-0014 · Reversa indica 50 km/h y revisión DIF/BLQ/ESC

Armando declara el 17-10-2026 que, en banco con ruedas siempre elevadas, la pantalla indica aproximadamente 50 km/h en R en los tres modos y las ruedas suenan muy rápidas. No es velocidad sobre el suelo ni se dispone de medición independiente/log de esa prueba. Solicita revisar arquitectura/matemática, mostrar plan y después implementar.

Configuración conocida: F746 dev8-ui instalada en ENS-0024; ESP32 físico pendiente de identificar. La última grabación trasera documentada es v1.2; dev3 está compilada, sin instalación acreditada. La falta de 0x124 en capturas anteriores es un indicio, no identificación concluyente.

[ENS-0025](../../06_TRABAJO/ensayos/ENS-0025_revision_diferencial_y_reversa.md): v1.2 tiene constante de límite sin uso y no reduce mando por velocidad en R; se reproduce PWM pleno en tres modos con RPM sintéticas elevadas. Dev3 corta esos casos, pero dos hallazgos nuevos: rampa distorsiona proporción DIF y memoria del corte no observa exceso con pedal libre. No se atribuye a estos dos hallazgos la causa del 50 físico. BLQ iguala mando, no velocidades; DIF no tiene lazo de RPM; ESC sigue experimental.

[Diagnóstico, matemática, plan y criterios](../../06_TRABAJO/REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md). Cierre exige identificar fuente/binario/configuración, verificar PPR/diámetro/RPM, implementar lo aprobado y demostrar actuación del supervisor común en los tres modos por Wi-Fi, cuantificando latencia/pico/recuperación. Ruido se sigue en [PROB-0015](PROB-0015_ruido_electromagnetico_sensores.md); antecedente [PROB-0008](PROB-0008_robustez_control_modos_y_pantalla.md). No firmware modificado o grabado en esta revisión.
