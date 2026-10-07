---
estado: banco y software verificados; integración en kart pendiente
actualizado: 2026-10-07
resumen: ESP32 v1.2 observado por USB/Wi-Fi sin MCP2515 y panel mejorado con diagnóstico, resumen, señales y animaciones verificadas.
---

# ENS-0003 · v1.2 de diagnóstico y panel de telemetría

Fecha: 07-10-2026. Referencia: [ENS-0001](ENS-0001_telemetria_esp32_usb_wifi.md), [recuperación ENS-0002](ENS-0002_recuperacion_esp32_tras_prueba_kart.md), [PROB-0006](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md).

## Objetivo y configuración

Corregir la falta de observabilidad durante el inicio CAN y facilitar la lectura del panel después de que el usuario confirme recuperación de marcha con el binario original. **Solo ESP32 fuera del kart por USB**, sin placa trasera/MCP2515/STM32/MPU/actuadores. La ausencia de CAN es intencional; no hay ensayo de motores.

Firmware `esp32_trasero-v1.2-diagnostico`, commit `905b09ea26a153da4ff281861f6e41fc71838462`, etiqueta `esp32-trasero-v1.2-diagnostico` (validada solo en banco). pioarduino 55.03.311, Arduino-ESP32 3.3.11, bibliotecas de framework leídas como 5.5.5+sha.b774170ff46, mcp_can local 1.5.1 con parche DLC y MPU6050 1.4.5. La instalación recreó el entorno Python de PlatformIO por diferencia de intérprete 3.11/3.12.

## Cambios de firmware y justificación

- Telemetría y Wi-Fi arrancan antes de esperar al MCP2515. El CAN conserva los reintentos sin límite; mientras falla se publican fase, intentos y relés, sin entrar en el control. No se publica RPM/PWM antes de que termine la configuración de salidas y sensores.
- `CAN.setMode(MCP_NORMAL)` y `readMsgBuf` deben devolver éxito; un error de lectura no se trata como orden válida.
- Se publican `control_ready`, `can_init_ok`, `can_init_attempts`, `control_state`, `drive_inhibit`, `start_requested`, `drive_direction` y `reset_reason`.
- Motivos de parada: arranque, ausencia de órdenes, relés pendientes, freno, pedal cero, neutro, cambio de dirección bloqueado por RPM, timeout CAN y ESC sin MPU. La marcha ordenada describe salidas software, no sentido físico medido.
- Si el MPU no respondió al arrancar, se omiten calibración/lecturas inválidas y guiñada en la telemetría. **ESC detiene la orden de motores si el MPU está ausente**; DIF/BLQ conservan sus cálculos. `mpu_ok` sigue siendo una comprobación al arrancar, no supervisión continua del sensor. La trama CAN 0x123 mantiene su formato histórico, con guiñada cero si no hay MPU; ese canal todavía no tiene bandera de validez.
- Búfer de línea ampliado de 640 a 1536 y TX USB a 2048 para incorporar diagnóstico con margen. La tarea de red comprueba su creación. Tras el arranque se reinicia la medida del periodo de control para no mezclar espera CAN con `loop()`.
- Constantes de motores, PWM, geometría, tiempos de relés, formato de mandos y timeout CAN de 100 ms conservados. No se drena toda la cola CAN ni se modifica hardware en este ensayo.

## Revisión de tamaño de trama

[verificar_tramas_telemetria.py](../../07_DESARROLLO/herramientas/verificar_tramas_telemetria.py) evalúa los literales printf del fuente, incluyendo mandos y todos los campos, con extremos de representación. No ejecuta control ni valida CAN físico.

- v1.1: 601 bytes en el caso evaluado; **cabe en 640**. No se confirma una truncación como causa del fallo previo.
- v1.2: 808 bytes en el caso evaluado; cabe con terminador NUL en 1536.
- El lector reconoce todas las señales y acepta la trama completa sintética.

Datos: [limites_tramas.json](datos/ENS-0003/limites_tramas.json) y [trama completa sintética](datos/ENS-0003/trama_completa_sintetica.jsonl). No incluyen mediciones del kart.

## Compilación, grabación y preservación

- Compilación correcta: RAM estática 49 036 B (15,0 %); flash de aplicación 945 607 B (72,1 %); imagen 956 912 B.
- La compilación completa mostró advertencias de atributos IRAM de los ISR Hall, sin errores. No se altera su lógica. La grabación posterior pasó y verificó el hash de los datos escritos.
- [01_compilacion_grabacion.log](datos/ENS-0003/01_compilacion_grabacion.log).
- SHA-256 `firmware.bin`: `d2a1e207468e89fb3d23dee8b15c95e2eb69673d23cc29fd34d802d287021794`.
- SHA-256 `firmware.elf`: `006f391fe43ae6085e841eac51fcca45994813663a3218df6470f70e426fb154`.
- Binario, ELF y diff preservados fuera del repositorio público en `C:\Users\nandi\respaldos-kart\esp32\v1.2_2026-10-07\`. [Hashes](datos/ENS-0003/hashes_compilacion.json). El respaldo original y el respaldo completo de v1.1 permanecen conservados; esta copia sigue en el mismo disco.

## Ensayo físico USB/Wi-Fi con CAN ausente

Procedimiento: lectura USB sin TX; perfil Windows `kart-esp32` existente y manual; TCP durante 30 s, cierre/reconexión, corte del Wi-Fi del portátil durante 10 s, reconexión y vuelta a eduroam. La lectura USB continúa durante la prueba de red. La prueba no envía comandos al ESP32.

| Comprobación | Resultado |
|---|---|
| USB durante el ensayo de red | 321 tramas, 0 saltos, 0 JSON inválidos |
| TCP 30 s | 129 tramas, 0 saltos |
| Reconexión TCP 10 s | 43 tramas, 0 saltos |
| TCP tras corte Wi-Fi 10 s | 43 tramas, 0 saltos |
| Reinicios | Una única sesión `331fb84e5ad58968`; sin cambio de sesión en la captura |
| Estado | Todas las muestras `can_wait`, `control_ready=0`; relés ordenados a 0 |
| Medidas de motores | No se publican RPM ni PWM con el control sin inicializar |
| Tramas descartadas reportadas | 0; JSON TCP inválidos: 0 |
| Publicación de diagnóstico | Máximo reportado 256 µs; no es medida del bucle de control |
| Intervalos de muestras | 200 y 306 ms durante reintentos CAN; no se promete cadencia exacta de 200 ms en arranque |
| Vuelta a eduroam | Internet comprobado al terminar, duración total aproximada 76 s |

Datos originales: [04_wifi_can_ausente.json](datos/ENS-0003/04_wifi_can_ausente.json); [eventos](datos/ENS-0003/04_wifi_eventos.log); [resumen verificable](datos/ENS-0003/05_resumen_wifi.json); [análisis](datos/ENS-0003/05_analisis_wifi.log).

La primera lectura de 8 s conservó 34 tramas válidas sin saltos, pero el resumidor antiguo falló al intentar medir `loop_max_us`, omitido intencionalmente durante arranque. Además trataba un fragmento al cerrar la captura como JSON inválido. Se corrigió el resumidor para distinguir fragmentos de cierre y campos ausentes; segunda lectura de 3 s correcta, 12 tramas sin saltos. [Captura inicial](datos/ENS-0003/02_usb_can_ausente.log) y [captura posterior](datos/ENS-0003/03_usb_can_ausente.log). El fallo era de la herramienta de resumen, no un panic del firmware.

## Panel y verificación de interfaz

Panel actualizado en **http://127.0.0.1:8766/**, conectado a COM4 solo en lectura al terminar. La instancia anterior de 8765 queda desconectada; usar 8766 en esta sesión.

Código del panel en commit `4aa71cd64f111ffe37391c9278bb8887d850675c`. Commits y etiqueta locales, sin push. El cambio local del usuario en `secretos.example.h` no se incluye en ellos.

- Resumen de ocho señales: RPM I/D, pedal, dirección, marcha, PWM I/D y guiñada. Fase/fallo destacado, versión, motivo de reinicio y sesión visibles.
- Tabla de todas las señales, origen, antigüedad, filtro de búsqueda y opción de ocultar campos nunca recibidos. Gráfica disponible para todas las señales numéricas.
- Rodadura ilustrativa por RPM con bandas dentro de cada neumático; sentido ilustrado según marcha ordenada, porque los Hall traseros no miden sentido. Las ruedas delanteras no se animan si no hay RPM delanteras.
- Dirección suavizada. Si solo llega ADC `steer_raw`, se usa la conversión del fuente `(raw-2047)/47.9`, limitada a ±20°, **rotulada estimación sin calibrar**. No equivale a ángulo físico de cada rueda ni a una geometría Ackermann validada.
- Sin muestras recientes se retiran los valores vigentes y se detiene la rodadura. Si la orden CAN reportada caducó (>100 ms), sus mandos se muestran antiguos y no se usan para representar una dirección vigente. Pausa de animación y preferencia de movimiento reducido respetadas.
- Demo sintética separada de lectura y registro reales. Horas de consola mostradas en Europe/Lisbon; registros originales conservados en UTC.
- 13 tests Python pasados y QA Chrome pasada en 1280/1024/760/375 px: conexión, demo, estimación de dirección, caducidad de mando CAN, pausa, ausencia de animación delantera sin RPM, búsqueda, protección de POST y ausencia de actuación.

Capturas de QA son sintéticas: [panel-animado.png](datos/ENS-0003/panel-animado.png), [panel-desconectado.png](datos/ENS-0003/panel-desconectado.png), [resultados](datos/ENS-0003/resultados.json).

Captura con la placa real y CAN ausente: [panel_real_can_ausente.png](datos/ENS-0003/panel_real_can_ausente.png), [resumen](datos/ENS-0003/panel_real_resumen.json). En esa captura el panel acumulaba 2120 tramas válidas, sin JSON inválidos ni saltos desde su conexión. El panel acepta v1.2, muestra `can_wait` y conserva RPM/PWM como ausentes. No se usaron datos simulados para esa captura.

## Qué salió bien, límites y siguiente ensayo

La falta de CAN ya no impide observar el estado por Wi-Fi. El panel facilita distinguir arranque, mandos, salidas ordenadas y mediciones ausentes. La imagen original recuperó funcionamiento según Armando, pero **la causa de la regresión v1.1 permanece abierta**.

No hay CAN real, MPU operativo, alimentación del kart ni actuadores en esta prueba. No se ha medido el periodo de control de v1.2 ni probado su respuesta de motores, relés o ESC. El siguiente ensayo es integración estática con placa trasera y STM32, tracción inhabilitada, adquisición de diagnóstico y comparación con señales físicas. La etiqueta de banco no convierte la versión en firmware validado para circulación.
