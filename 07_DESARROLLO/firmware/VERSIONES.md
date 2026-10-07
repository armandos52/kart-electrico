---
estado: vigente
actualizado: 2026-10-07
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
| v1-telemetria | 2026-10-07 | Telemetría de solo lectura: SoftAP WPA2 + TCP 3333 + JSON por USB. Ver `esp32_trasero/README.md` | Pendiente | No | Pendiente |

**Imagen extraída frente a v0:**
- La «imagen extraída» es el binario leído de la placa.
- v0 es el fuente de la entrega.
- Restaurar la placa al estado inicial se hace con la imagen extraída, no recompilando v0.
