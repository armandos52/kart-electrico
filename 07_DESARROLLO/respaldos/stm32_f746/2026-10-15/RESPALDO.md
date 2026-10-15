---
estado: vigente
actualizado: 2026-10-15
resumen: Respaldo previo de la F746 frontal (flash interna 1 MiB + QSPI 16 MiB) antes de grabar dev4; identifica el firmware anterior.
---

# Respaldo previo del frontal STM32F746G-DISCO · 15-10-2026

ST-LINK V2-1 `0670FF3332584B3043206038`, firmware V2J47M34, COM6; Windows anuncia la unidad `DIS_F746NG`. MCU `DBGMCU_IDCODE=0x10016449` (STM32F74x/75x, rev. 0x1001), UID `0x00420026 0x54435003 0x20353036`, flash 1024 KiB. QSPI detectada por OpenOCD como Winbond W25Q128FV/JV, ID `0x1840EF`, 16 MiB. Tensión informada por el ST-LINK: 3,21–3,22 V, que es la lectura del programador y no una medida calibrada. Configuración declarada por Armando: USB del ST-LINK, transceptor/bus CAN con ESP32 encendido y pedal/dirección conectados.

Lecturas con OpenOCD xPack 0.12.0-7, con la aplicación en ejecución, **sin halt, reset, borrado ni escritura**. La QSPI ya estaba en modo mapeado por la aplicación instalada (`QUADSPI_CCR=0x0F182DEB`, FMODE=3), así que se leyó `0x90000000` sin reconfigurarla. CubeProgrammer, usado el 14-10, ya no estaba instalado en la ruta registrada, por lo que no hay tercera herramienta independiente.

| Archivo | Rango | SHA-256 | Observación |
|---|---|---|---|
| `flash_interna_lectura1.bin`, `flash_interna_lectura2.bin` | `0x08000000`, 1 MiB | `23779871036d65106712829c8a88d86526604a8e3daa76c6c07e427c2dfd8855` | Dos lecturas idénticas |
| `qspi_lectura1.bin` (4 MHz), `qspi_lectura2.bin` y `qspi_lectura3.bin` (1,8 MHz) | `0x90000000`, 16 MiB | `323636a3f8ec570ae952559c0e6a688c33f7d699b91cf61f1d8676c19a358247` | Tres lecturas idénticas |
| `qspi_lectura2_interrumpida.bin` | 2.289.664 B | `02cc3e3622615ad35349366e6b8605ea3cb4b8cbcca50b86359dbee18e51fbbd` | Intento a 4 MHz cortado por error SWD; es un prefijo exacto de la lectura 1. Resultado negativo conservado |
| `option_bytes_lectura.bin` | `0x1FFF0000`, 32 B | `f737c160b297f470cb35621361e026a80c3fd1a197436b9d38d74903207c5602` | RDP nivel 0 (`0xAA`), sin protección de escritura, IWDG/WWDG por software. Solo leídos |
| `otp_lectura.bin` | `0x1FF0F000`, 1056 B | `cfb8375808408fa803db5b09ab90086eb9fab195ae5b14a2084b1c66f9e567d2` | Zona OTP e identificación. Solo leída |

**Identificación del firmware anterior:** los 243.692 bytes de aplicación de la flash interna y los 6.351.932 bytes de la QSPI coinciden byte a byte (0 diferencias) con `STM32-final/STM32CubeIDE/Debug/STM32F746G_DISCO.hex` de la entrega histórica (`02_ORIGINALES/entrega_2026-09-28/Smart Eletric Kart Cross-2223406-2223403/`). Escribe `kart_log.csv` con el esquema antiguo `Tick,Accel,Steer,Brake,ModeD,ModeR,Vel,...`. **No era ninguna compilación v1.3.**

Fuera de esa imagen hay restos de grabaciones anteriores más grandes: 32.752 y 117.714 bytes distintos de `0xFF` en los sectores internos 5 y 6 (`0x08040000–0x080BFFFF`), y datos en la QSPI hasta `0x90E3BEB8`. Al grabar dev4 solo se borraron los sectores que ocupa su imagen, así que esos restos siguen en la placa.

`relectura_dev4/` conserva la relectura posterior a grabar dev4 (interna `0x08000000–0x0803CC23` y QSPI `0x90000000–0x9060EC3B`), con 0 diferencias frente al HEX dev4. Son copias del binario nuevo, no del firmware anterior.

## Recuperación preparada

Para volver al firmware histórico funcional basta con grabar el HEX original citado (interna + QSPI) con `program <hex> verify reset` y `board/stm32f746g-disco.cfg`, como en `06_TRABAJO/evidencias/frontal_f746_2026-10-15/grabar_dev4.cfg`. Para una restauración exacta, restos incluidos, hay que grabar `flash_interna_lectura1.bin` en `0x08000000` y `qspi_lectura1.bin` en `0x90000000`, y verificar ambos. Nunca escribir los option bytes ni la OTP como si fueran firmware ni usar opciones de protección o desbloqueo. Procedimiento preparado, **no ejecutado**.

Los binarios están excluidos del repositorio público. Son copias locales en el mismo ordenador, no copia de seguridad externa. [ENS-0019](../../../../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md) · [evidencias](../../../../06_TRABAJO/evidencias/frontal_f746_2026-10-15/README.md).
