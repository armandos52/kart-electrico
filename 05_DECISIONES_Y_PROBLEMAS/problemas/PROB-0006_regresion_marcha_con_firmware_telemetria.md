---
estado: abierto
actualizado: 2026-10-07
resumen: El usuario declara fallo de marcha con v1.1 y recuperación con la imagen original. Causa pendiente; v1.2 añade diagnóstico observable.
---

# PROB-0006 · Fallo de marcha al pasar a v1.1

## Hechos y condiciones

- Armando declara que el kart no movía los motores después de instalar v1.1 y no pudo ver telemetría en el panel.
- En [ENS-0002](../../06_TRABAJO/ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md) se restauran exactamente los 4 MB del respaldo original; verificación independiente correcta.
- Después Armando confirma que el kart **volvió a funcionar**. Es su comprobación funcional, sin captura comparativa de CAN, contactor, PWM o alimentación.
- El CAN tardaba decenas de segundos en inicializar en el banco anterior (ENS-0001). La prueba del ESP32 solo, sin MCP2515, no demuestra que ocurriera lo mismo durante el fallo en el vehículo.

## Hipótesis por contrastar

1. Diferencias entre el binario original y la reconstrucción: el fuente entregado es compatible en tamaño y cadenas, pero su equivalencia exacta no está demostrada. Core original 3.3.10 frente a 3.3.11 y bibliotecas elegidas por API.
2. Inicio/recepción CAN fallidos o mandos inesperados. La v1.1 iniciaba Wi-Fi después del CAN y ocultaba el bloqueo si se esperaba observarlo por red.
3. Interbloqueos existentes: GPIO35 de cargador, relés, START, freno, marcha y RPM que bloquean el cambio de dirección. Se observaron RPM y cargador inesperados en banco, sin confirmar esa situación en el kart.
4. Diferencias de alimentación con Wi-Fi activo. No hay mediciones para confirmarlo. En v1.1 la red se activaba después de CAN, por lo que no explica por sí sola un bloqueo inicial previo a Wi-Fi.

## Intervención actual

[ENS-0003](../../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md), v1.2: red y diagnóstico antes del inicio CAN; estado de inicialización, intentos, recepción CAN, START, motivo de parada, marcha ordenada y reset. No se modifican constantes de PWM, geometría, protocolo 0x100/0x123 ni timeout CAN de 100 ms. Se valida el resultado de lecturas CAN y se impide usar ESC cuando el MPU no respondió al arrancar.

La v1.2 pasa USB/Wi-Fi con el módulo solo y CAN ausente; **esto no demuestra que haya corregido la regresión de marcha**. El problema permanece abierto.

## Ensayo necesario

Con montaje preparado y tracción inhabilitada, alimentar la placa trasera en su configuración real y registrar v1.2: `control_state`, `can_init_ok`, `can_init_attempts`, `can_rx_count`, `can_rx_age_ms`, `start_requested`, `drive_inhibit`, cargador, relés, RPM y PWM. Correlacionar órdenes software con alimentación y señales físicas. Conservar la imagen original verificada como referencia de recuperación. No eliminar interbloqueos para conseguir que el kart se mueva.
