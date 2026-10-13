---
estado: vigente
actualizado: 2026-10-13
resumen: Registro de versiones de firmware del kart (DEC-0001) — qué cambia, si se grabó y con qué ensayo.
---

# Versiones de firmware

## Entrega pública · 13-10-2026

Corte `revision-academica-2026-10-13`: fuentes ESP32 y STM32 **v1.3-dev3**, panel **0.4.0-calibracion-pasos**. [PUB-0001](../../06_TRABAJO/PUBLICACION_2026-10-13.md) registra publicación y verificación. Las notas siguientes de «sin commit/push» conservan el contexto de cada ensayo y quedan superadas solo en su estado de publicación. No cambia qué se grabó ni acredita nuevas pruebas físicas. Tras «B1.3» declarada el 13-10, versiones de placas pendientes de identificar.

## Pulsador START · 13-10-2026

STM32 **v1.3-dev3**, memoria de START/STOP para pulsador momentáneo y antirrebote; 83 comprobaciones y compilación ARM ELF/HEX correctas. ESP32 **v1.3-dev3 sin modificar**, 56 comprobaciones satisfactorias. [ENS-0013](../../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md) enlaza copias antes/después, diff, logs, manifiestos SHA-256 y binarios preservados en `07_DESARROLLO/respaldos/stm32/v1.3-dev3/`. Cambios locales sobre HEAD `4f7f150`, sin commit/push ni grabación. Armando declara pruebas reales «B1.3» y pulsador momentáneo, **sin identificar versiones exactas de las placas**; la confirmación física v1.2 del 11-10 queda histórica. Identificación/respaldo frontal+QSPI y ensayo por Wi-Fi pendientes.

## Continuación · 11-10-2026

ESP32 **v1.3-dev3**, compilado y sin grabar: conserva STOP ante STOP→START en el mismo lote CAN y exige un nuevo arranque deliberado. [ENS-0011](../../06_TRABAJO/ensayos/ENS-0011_stop_en_lote_can.md): 56 comprobaciones de control sin fallos, regresión reproducida antes del arreglo; fuentes/binarios y hashes enlazados. Cambios locales sobre HEAD `4f7f150`, sin commit/push. STM32 permanece en **v1.3-dev2**, con sus 54 comprobaciones repetidas. Armando confirma hoy que la versión física sigue en **v1.2** y no tiene acceso al frontal; no se ha leído ninguna placa.

Regla ([DEC-0001](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos.md)):
- Un cambio pequeño por commit.
- Etiqueta Git por versión grabada.
- La versión se muestra al arrancar.

## ESP32 trasero (`esp32_trasero/`)

| Versión | Fecha | Cambios | Compila | Grabada en el kart | Ensayo |
|---|---|---|---|---|---|
| v0 | 2026-10-07 | Fuente entregado sin cambios (SHA-256 `fab150ea…b0a32e`, igual a `02_ORIGINALES/.../ESP32-final.ino`) + `platformio.ini` | Sí: pioarduino 55.03.311, core 3.3.11, mcp_can 1.5.1, MPU6050 1.4.5. Imagen 346 544 B | No: lo que hay en la placa es la imagen del [respaldo 2026-10-07](../respaldos/esp32/2026-10-07_180133/RESPALDO.md) (core 3.3.10, 345 808 B), compatible con v0 pero no demostrado idéntico | — |
| v1-telemetria | 2026-10-07 | Telemetría de solo lectura: SoftAP WPA2 + TCP 3333 + JSON por USB; 6 enganches `// v1` en el `.ino`, control sin cambios. Ver `esp32_trasero/README.md` | Sí: imagen 993 680 B, SHA-256 `8c69929a…6fb95f9d` (con `secretos.h` local) | Sí, 07-10 (banco), etiqueta `esp32-trasero-v1` | [ENS-0001](../../06_TRABAJO/ensayos/ENS-0001_telemetria_esp32_usb_wifi.md): **mal**. Cuelgue en `readMsgBuf` ([PROB-0005](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0005_mcp_can_desborda_con_dlc_mayor_8.md), fallo previo de mcp_can) y registros de error del core inundando el puerto serie |
| v1.1-telemetria | 2026-10-07 | mcp_can 1.5.1 en `lib/` con DLC limitado a 8 (PROB-0005); `CORE_DEBUG_LEVEL=0` como la imagen original; AP con `softAP` antes de `softAPConfig` y diagnóstico `wifi_ap`; `mpu_ok` (sin guiñada si el MPU no responde) | Sí: imagen 955 952 B | Sí, 07-10 (banco), etiqueta `esp32-trasero-v1.1`; sustituida por la imagen original en ENS-0002 | ENS-0001: USB y Wi-Fi sin saltos en banco, sin CAN real. Después el usuario declara fallo de marcha y falta de visualización en el kart; causa pendiente (ENS-0002) |
| Imagen original restaurada | 2026-10-07 | Restauración exacta del respaldo completo previo a los cambios, SHA-256 `31a14b7b…ffb01a8`; sin telemetría v1.1. No es una recompilación de v0 | No se recompila | 4 MB escritos y verificación independiente `verify-flash` correcta; sustituida después por v1.2 de diagnóstico | [ENS-0002](../../06_TRABAJO/ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md): usuario confirma recuperación de funcionamiento del kart; sin mediciones comparativas |
| v1.2-diagnostico | 2026-10-07 | Wi-Fi/diagnóstico antes de CAN, estados de inicio/parada/START/marcha/reset; validación de lecturas; MPU ausente sin lecturas/calibración inválidas y ESC detenido; búfer 1536/TX 2048 | Sí: imagen 956 912 B, SHA-256 `d2a1e207…87021794`; core 3.3.11 | Última imagen grabada/verificada, con módulo fuera del kart, commit `905b09e`, etiqueta `esp32-trasero-v1.2-diagnostico`; referencia de la prueba posterior, sin otra grabación comunicada | [ENS-0003](../../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md): banco USB/Wi-Fi sin CAN, 0 saltos/reinicios y panel verificado. [ENS-0004](../../06_TRABAJO/ensayos/ENS-0004_prueba_real_motores_confirmada.md): el 08-10 Armando confirma prueba real con motores satisfactoria; sin identificación de versión durante la prueba, logs ni condiciones detalladas. Caracterización instrumentada pendiente |

**Imagen extraída frente a v0:**
- La «imagen extraída» es el binario leído de la placa.
- v0 es el fuente de la entrega.
- Restaurar la placa al estado inicial se hace con la imagen extraída, no recompilando v0.


## Desarrollo v1.3 · 08–09-10-2026 (sin grabación)

| Versión | Alcance | Verificación / conservación |
|---|---|---|
| STM32 v0 de trabajo | Copia íntegra de 2025/26; 1673 archivos verificados | Commit `2deb9ef`, ORIGEN_V0.json; no identifica instalado |
| ESP32 v1.3-dev1 | Núcleo modular, estados, rampa, R20, sensores/watchdog | Commit `53ebf40`; 46 comprobaciones y compilación; backup dev1 preservado |
| ESP32 v1.3-dev2 | Revisión final STOP/ADC, D-R/ESC/BLQ, contrato, PWM y telemetría | 51 comprobaciones; compila, RAM 49244 B/flash 949743 B; binario+ELF local |
| STM32 v1.3-dev2 | ADC/CAN, GUI coherente, SD, Hall EXTI, IWDG, configuración/build | 54 comprobaciones; ELF/HEX compilados con startup y QSPI; hardware pendiente |
| Panel: compatibilidad v1.3 | 15 señales nuevas, motivos/modo aplicado, validez y calibración | 26 tests Python/16 JS; cambios locales previos conservados, SHA-256 de archivos afectados |

[ENS-0009](../../06_TRABAJO/ensayos/ENS-0009_robustez_firmware_v13.md), [hashes](../../06_TRABAJO/ensayos/datos/ENS-0009/hashes_dev2.json) y [protocolo](common/PROTOCOLO_V13.md). Las compilaciones no son firmware leído de placas. R limita propulsión estimada; ESC experimental, física/calibraciones/BMS pendientes. No push ni copia independiente configurada. V1.2 continúa como última grabación conocida en el kart.
