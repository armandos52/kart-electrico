---
estado: satisfactorio según declaración del usuario
actualizado: 2026-10-08
resumen: Armando confirma que la prueba real en el kart con motores funcionó bien tras la preparación de v1.2.
---

# ENS-0004 · Prueba real en el kart con motores

Fecha de confirmación: **08-10-2026**. Fecha y hora exactas de ejecución: no indicadas.
Resultado: **salió bien, según comprobación de Armando**.

## Objetivo y referencia de firmware

Registrar el resultado funcional de la prueba en el kart después de preparar la telemetría y el panel de v1.2, como continuación de [ENS-0003](ENS-0003_v12_diagnostico_y_panel.md).

Última imagen grabada y verificada antes de esta confirmación: `esp32_trasero-v1.2-diagnostico`, commit `905b09ea26a153da4ff281861f6e41fc71838462`, etiqueta `esp32-trasero-v1.2-diagnostico`. Se vincula la prueba a esta versión por la continuidad de la sesión; no se recibió una nueva lectura de identificación del firmware desde el kart ni se comunicó otra grabación. Firmware STM32 y configuración detallada del montaje: no informados en esta confirmación.

## Condiciones y procedimiento conocidos

- Prueba real en el kart, con los motores funcionando, realizada y evaluada por Armando.
- El usuario había indicado que no puede conectar USB con el ESP32 instalado; el canal previsto para observar el montaje es Wi-Fi directo mediante el panel local.
- No se detallaron duración, modo DIF/BLQ/ESC, marchas ensayadas, posición de las ruedas, conductor/carga, tensión de batería ni secuencia de prueba.

## Evidencia y resultado

Declaración recibida en la conversación el 08-10-2026:

> registra que funciono la pruea real con moteres quedo bueno

**Los motores funcionaron y el usuario considera satisfactoria la prueba real.** Es una comprobación funcional declarada por quien realizó el ensayo. No se recibió en este mensaje un JSONL, captura del panel ni mediciones de CAN, PWM, RPM, alimentación o tiempos. La confirmación de motores no aporta por sí sola un resultado separado sobre pérdidas de tramas Wi-Fi o funcionamiento de todas las animaciones.

## Qué salió bien y qué queda por caracterizar

La prueba real comunica un resultado positivo de motores, frente a la falta de marcha declarada con v1.1 en [ENS-0002](ENS-0002_recuperacion_esp32_tras_prueba_kart.md). La causa del fallo anterior sigue sin identificar en [PROB-0006](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md); no se atribuye la recuperación a un cambio concreto sin datos comparativos.

El funcionamiento básico con motores queda confirmado por el usuario. Para caracterizarlo en el relatório, conservar en un próximo ensayo el registro Wi-Fi, las condiciones del montaje y los modos probados. Las mediciones de control/CAN/MPU/alimentación y la repetibilidad siguen pendientes; los datos de banco de ENS-0003 mantienen su configuración original.
