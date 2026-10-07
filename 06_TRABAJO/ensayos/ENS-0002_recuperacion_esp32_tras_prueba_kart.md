---
estado: flash verificada; recuperación funcional confirmada por el usuario
actualizado: 2026-10-07
resumen: Incidencia declarada tras probar v1.1 en el kart, diagnóstico USB fuera del vehículo y recuperación de la imagen original.
---

# ENS-0002 · Recuperación del ESP32 tras prueba en el kart

Fecha: 2026-10-07. Relacionado con [ENS-0001](ENS-0001_telemetria_esp32_usb_wifi.md) y [PROB-0002](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender.md).

## Declaración del usuario y condiciones

- El usuario probó la placa en el kart después de la v1.1 y declara que los motores ya no funcionan y que no pudo ver datos en el panel.
- Para esta recuperación confirma **solo el módulo ESP32 conectado por USB, fuera del kart**. La placa trasera y el módulo CAN no están conectados.
- No se dispone de una captura durante el fallo en el kart. Estado del contactor, órdenes STM32, alimentación y causa pendientes.

## Comprobaciones

- Al principio Windows solo detecta Intel AMT en COM3 y un dispositivo USB con fallo al solicitar el descriptor. Tras reconexión aparece CP210x en COM4.
- Lectura sin TX de COM4 durante 8 s: `CAN init failed, retrying...`, 0 tramas JSON. Log: [01_usb_fuera_kart.log](datos/ENS-0002/01_usb_fuera_kart.log).
- **El error CAN es esperable en estas condiciones:** falta el módulo MCP2515. No demuestra que ese mismo bloqueo causara el fallo en el vehículo.
- Identidad por esptool: ESP32-D0WD-V3 rev. 3.1, MAC `70:4b:ca:7c:df:68`, misma placa del respaldo original.
- Respaldo original completo de 4 MB: SHA-256 comprobado `31a14b7b4c80eeca6b1ebe5374180bfeccc6c7d6cb5c17f61d70b5365ffb01a8`.
- Respaldo previo a recuperación en `07_DESARROLLO/respaldos/esp32/2026-10-07_212814/`: dos lecturas completas idénticas, SHA-256 `33275b8680982c0f4ed5a4731c878dd1ff22da03c40debc9697a88e2adad0a62`.

## Panel corregido

- No había servidor del panel en 8765/8766. Se inicia en `http://127.0.0.1:8765/`. Tras terminar la restauración se conecta a COM4, con el módulo fuera del kart confirmado por el usuario, para mostrar los mensajes reales de arranque.
- Wi-Fi directo `socket://192.168.4.1:3333` aparece por defecto, sin necesidad de `--wifi`.
- Intel AMT queda fuera del selector; el diagnóstico inicial muestra ESP32 con contador CAN, cargador y estado MPU.
- Botón, estado y sesión identifican correctamente USB/TCP; textos obsoletos que negaban Wi-Fi corregidos.
- 10 tests Python pasados. QA de Chrome pasada, incluyendo fuente Wi-Fi por defecto, rótulos TCP, ausencia de actuación y cuatro tamaños de pantalla. QA con datos sintéticos, sin abrir la placa.

## Recuperación y límites

Se restaura la **imagen binaria original extraída de esta placa**, no una recompilación de v0. La imagen original no incluye la telemetría Wi-Fi de v1.1. Se conserva el desarrollo v1.1 y dos copias idénticas de la flash previa a recuperación.

- Escritura completa desde `0x0`, conservando modo/frecuencia/tamaño de flash: exit 0, hash verificado. [02_restauracion_original.log](datos/ENS-0002/02_restauracion_original.log).
- Verificación independiente con `verify-flash` de **4 194 304 bytes**: exit 0, `Verification successful (digest matched)`. Reinicio por RTS. [03_verificacion_original.log](datos/ENS-0002/03_verificacion_original.log).
- Consola del panel conectada a COM4, solo lectura. [04_panel_original_usb.json](datos/ENS-0002/04_panel_original_usb.json). Sin módulo CAN sigue habiendo reintentos; no se espera JSON ni gráficos con la imagen original.

**Tras la restauración, Armando confirma que el kart volvió a funcionar** (respuesta del 07-10 en la siguiente sesión). Es una confirmación funcional del usuario; no hay captura de CAN, alimentación o contactor durante la comparación. La causa de la regresión de v1.1 sigue pendiente. El módulo vuelve a estar solo fuera del kart por USB para desarrollar la siguiente versión.

## Siguiente comprobación física

Armando confirmó recuperación con la imagen original. Continúa la investigación de diferencias en [ENS-0003](ENS-0003_v12_diagnostico_y_panel.md) y [PROB-0006](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md); integración estática de v1.2 con CAN/STM32 y diagnóstico de las condiciones de parada pendiente.
