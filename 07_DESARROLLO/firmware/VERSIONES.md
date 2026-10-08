---
estado: vigente
actualizado: 2026-10-08
resumen: Registro de versiones de firmware del kart (DEC-0001) — qué cambia, si se grabó y con qué ensayo.
---

# Versiones de firmware

Regla ([[05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos|DEC-0001]]):
- Un cambio pequeño por commit.
- Etiqueta Git por versión grabada.
- La versión se muestra al arrancar.

## ESP32 trasero (`esp32_trasero/`)

| Versión | Fecha | Cambios | Compila | Grabada en el kart | Ensayo |
|---|---|---|---|---|---|
| v0 | 2026-10-07 | Fuente entregado sin cambios (SHA-256 `fab150ea…b0a32e`, igual a `02_ORIGINALES/.../ESP32-final.ino`) + `platformio.ini` | Sí: pioarduino 55.03.311, core 3.3.11, mcp_can 1.5.1, MPU6050 1.4.5. Imagen 346 544 B | No: lo que hay en la placa es la imagen del [[07_DESARROLLO/respaldos/esp32/2026-10-07_180133/RESPALDO\|respaldo 2026-10-07]] (core 3.3.10, 345 808 B), compatible con v0 pero no demostrado idéntico | — |
| v1-telemetria | 2026-10-07 | Telemetría de solo lectura: SoftAP WPA2 + TCP 3333 + JSON por USB; 6 enganches `// v1` en el `.ino`, control sin cambios. Ver `esp32_trasero/README.md` | Sí: imagen 993 680 B, SHA-256 `8c69929a…6fb95f9d` (con `secretos.h` local) | Sí, 07-10 (banco), etiqueta `esp32-trasero-v1` | [[06_TRABAJO/ensayos/ENS-0001_telemetria_esp32_usb_wifi\|ENS-0001]]: **mal**. Cuelgue en `readMsgBuf` ([[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0005_mcp_can_desborda_con_dlc_mayor_8\|PROB-0005]], fallo previo de mcp_can) y registros de error del core inundando el puerto serie |
| v1.1-telemetria | 2026-10-07 | mcp_can 1.5.1 en `lib/` con DLC limitado a 8 (PROB-0005); `CORE_DEBUG_LEVEL=0` como la imagen original; AP con `softAP` antes de `softAPConfig` y diagnóstico `wifi_ap`; `mpu_ok` (sin guiñada si el MPU no responde) | Sí: imagen 955 952 B | Sí, 07-10 (banco), etiqueta `esp32-trasero-v1.1`; sustituida por la imagen original en ENS-0002 | ENS-0001: USB y Wi-Fi sin saltos en banco, sin CAN real. Después el usuario declara fallo de marcha y falta de visualización en el kart; causa pendiente (ENS-0002) |
| Imagen original restaurada | 2026-10-07 | Restauración exacta del respaldo completo previo a los cambios, SHA-256 `31a14b7b…ffb01a8`; sin telemetría v1.1. No es una recompilación de v0 | No se recompila | 4 MB escritos y verificación independiente `verify-flash` correcta; sustituida después por v1.2 de diagnóstico | [ENS-0002](../../06_TRABAJO/ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md): usuario confirma recuperación de funcionamiento del kart; sin mediciones comparativas |
| v1.2-diagnostico | 2026-10-07 | Wi-Fi/diagnóstico antes de CAN, estados de inicio/parada/START/marcha/reset; validación de lecturas; MPU ausente sin lecturas/calibración inválidas y ESC detenido; búfer 1536/TX 2048 | Sí: imagen 956 912 B, SHA-256 `d2a1e207…87021794`; core 3.3.11 | Última imagen grabada/verificada, con módulo fuera del kart, commit `905b09e`, etiqueta `esp32-trasero-v1.2-diagnostico`; referencia de la prueba posterior, sin otra grabación comunicada | [ENS-0003](../../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md): banco USB/Wi-Fi sin CAN, 0 saltos/reinicios y panel verificado. [ENS-0004](../../06_TRABAJO/ensayos/ENS-0004_prueba_real_motores_confirmada.md): el 08-10 Armando confirma prueba real con motores satisfactoria; sin identificación de versión durante la prueba, logs ni condiciones detalladas. Caracterización instrumentada pendiente |

**Imagen extraída frente a v0:**
- La «imagen extraída» es el binario leído de la placa.
- v0 es el fuente de la entrega.
- Restaurar la placa al estado inicial se hace con la imagen extraída, no recompilando v0.
