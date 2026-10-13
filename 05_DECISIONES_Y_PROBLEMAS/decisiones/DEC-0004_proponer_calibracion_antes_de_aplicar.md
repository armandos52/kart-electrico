---
estado: vigente
actualizado: 2026-10-08
resumen: Armando aprueba la etapa A de calibración visual en el portátil el 08-10-2026; control de motores sin cambios.
---

# DEC-0004 · Proponer calibración antes de aplicar

Armando solicita el 08-10-2026 registrar la variación de dirección, diseñar una calibración de centro/izquierda/derecha y generar una demostración gráfica. Establece explícitamente: primero proponer cómo hacerlo y después aplicar con su aprobación.

La primera autorización permitió consultar fuentes, documentar la incidencia y crear la demostración aislada. La [propuesta](../../06_TRABAJO/PROPUESTA_CALIBRACION_DIRECCION_2026-10-08.md) recomienda captura y perfiles en el PC como primera etapa.

Motivo: revisar el procedimiento frente a la fluctuación y posible asimetría declaradas en [PROB-0007](../problemas/PROB-0007_variacion_y_centro_direccion.md).

## Aprobación y ejecución · 08-10-2026

Armando responde «si apruebo» a la pregunta sobre implementar el asistente en el panel para capturar y calibrar la visualización, manteniendo el control de motores sin cambios. Queda aprobada la etapa A: lectura, capturas, referencias, perfiles y exportación en PC. No autoriza grabación ni cambios del control STM32/ESP32.

Implementación y pruebas de software en [ENS-0005](../../06_TRABAJO/ensayos/ENS-0005_calibracion_direccion_panel.md). El ensayo físico, las tolerancias apropiadas para el montaje y la causa de la fluctuación siguen pendientes.
