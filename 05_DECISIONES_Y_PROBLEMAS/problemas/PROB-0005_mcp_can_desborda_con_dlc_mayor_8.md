---
estado: vigente
actualizado: 2026-10-07
resumen: mcp_can 1.5.1 no limita el DLC recibido a 8; un DLC 9–15 o una lectura SPI corrupta sobrescribe el puntero SPI y cuelga el ESP32. Corregido en v1.1.
---

# PROB-0005 · El ESP32 se cuelga al leer un mensaje CAN con DLC > 8

Fecha: 2026-10-07.
Estado: resuelto en firmware v1.1 (verificado que no se cuelga en banco; sin tráfico CAN real todavía).
Configuración: ESP32 trasero, biblioteca `coryjfowler/mcp_can` 1.5.1. La imagen original contiene los textos de esta familia de bibliotecas; no se conoce su versión exacta.

## Síntoma y condiciones de la prueba

En [[06_TRABAJO/ensayos/ENS-0001_telemetria_esp32_usb_wifi|ENS-0001]], la v1 publicó una trama y, en la vuelta siguiente de `loop()`, se colgó: `Guru Meditation Error: Core 1 panic'ed (LoadProhibited)` → `Rebooting...`. Antes, el MCP2515 había tardado 19 s en responder al iniciarse.

## Evidencias

Backtrace descodificado con el ELF de la v1:
1. `spiTransferByte`
2. `SPIClass::transfer`
3. `MCP_CAN::mcp2515_readRegisterS`
4. `mcp2515_read_canMsg` (`mcp_can.cpp:781`)
5. `readMsgBuf`
6. `loop()` (`ESP32-final.ino:357`, lectura de 0x100)

`EXCVADDR 0xfb7ba404` es un puntero basura. Log en `06_TRABAJO/ensayos/datos/ENS-0001/01_v1_usb_arranque_panic.log`.

## Causa

**Confirmada por lectura del código y coherente con el backtrace.**

En `mcp2515_read_canMsg`, `m_nDlc = registro & 0x0F` admite valores de 0 a 15. Con ese valor se leen `m_nDlc` bytes en `m_nDta[8]`. En `mcp_can.h` le siguen:
- `m_nRtr` y `m_nfilhit`;
- **`SPIClass *mcpSPI`**.

Con un DLC de 12 a 15 se sobrescribe `mcpSPI`, y la siguiente transferencia SPI salta a una dirección basura. Además, `readMsgBuf()` copia `m_nDlc` bytes al `buf[8]` de `loop()`, lo que desborda también la pila.

Un DLC de 9 a 15 es legal en CAN 2.0 (significa 8 bytes), pero aquí lo más probable es una lectura SPI corrupta del MCP2515: ese día tardaba entre 19 y 77 s en iniciarse ([[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender|PROB-0002]]).

Puede explicar el **coredump** que había guardado en la flash original ([[07_DESARROLLO/respaldos/esp32/2026-10-07_180133/RESPALDO|respaldo]]). Es una hipótesis: falta el ELF de esa compilación para descodificarlo.

## Intentos y resultados

v1.1:
- `mcp_can` 1.5.1 copiada en `07_DESARROLLO/firmware/esp32_trasero/lib/mcp_can/`.
- Una línea añadida: `if (m_nDlc > MAX_CHAR_IN_MESSAGE) m_nDlc = MAX_CHAR_IN_MESSAGE;`

Resultado: tres arranques y unos 4 minutos sin cuelgues.

## Solución y prueba de verificación

- Parche en v1.1. Con él, una trama 0x100 con DLC de 9 a 15 se trata como de 8 bytes; antes desbordaba.
- **Pendiente:** verificar con tráfico CAN real y, si es posible, inyectar tramas con DLC > 8 desde un nodo de prueba.

## Pendientes

- Comprobar si la versión original grabada tenía el mismo fallo, por ejemplo restaurando el respaldo en banco, y anotarlo aquí.
- Valorar en el futuro la migración al controlador TWAI integrado (idea 4 de PROB-0002).
