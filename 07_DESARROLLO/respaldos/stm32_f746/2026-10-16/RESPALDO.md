# Respaldo F746 previo a dev7-ui · 16-10-2026

[ENS-0023](../../../../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md). Armando declara pantalla conectada, kart encendido y ruedas levantadas; autoriza actualizar. ST-LINK `0670FF3332584B3043206038`, UID `0x00420026 0x54435003 0x20353036`, ID `0x10016449`, RDP 0, flash interna 1024 KiB y QSPI 16 MiB.

Lectura SWD a 1,8 MHz, sin halt/reset ni escritura, con QSPI ya mapeada (`CCR=0x0F182DEB`, antes y después). Inicio 13:21:53, interna terminada 13:22:14, QSPI1 13:24:54, QSPI2 13:27:35, hora local Europe/Lisbon. [Guion](../../../../06_TRABAJO/evidencias/frontal_bugs_2026-10-16/respaldar.cfg) y [log](../../../../06_TRABAJO/evidencias/frontal_bugs_2026-10-16/respaldo.log).

| Archivos | Tamaño por lectura | SHA-256 común |
|---|---:|---|
| `flash_interna_lectura1.bin`, `flash_interna_lectura2.bin` | 1.048.576 B | `91a9a63a33ded0cd08a9a74f7ffe13dd95d03e5349e5284bfafedc20d9620f91` |
| `qspi_lectura1.bin`, `qspi_lectura2.bin` | 16.777.216 B | `2285da63bfd381a58ad3e29601021430e3d407c1b88f38def657d39188eb42a6` |

Comparación completa de cada pareja: idénticas. [Resultado estructurado](verificacion.json). Ambas regiones ocupadas por el HEX conservado **`stm32_frontal-v1.3-dev4` coinciden byte a byte** con la imagen instalada. Se identificó dev4; no se dedujo solo de la cadena de versión. El resto de la flash también queda conservado. OTP (1056 B) y option bytes (32 B) se leen únicamente como evidencia; nunca se programan como firmware.

Recuperación funcional: HEX dev4 preservado en `../../stm32/v1.3-dev4/`, programando interna + QSPI y verificando. Recuperación exacta de estas memorias: interna en `0x08000000`, QSPI en `0x90000000`, con configuración F746G-DISCO y verificación posterior; no tocar protecciones/OTP/option bytes. Recuperación preparada, no ejecutada.

Copias locales en el mismo PC; no son respaldo externo. El respaldo del 15-10 (firmware original de la entrega) se conserva separado. Nueva imagen dev7-ui, relectura y capturas pertenecen a la evidencia de ENS-0023.
