---
estado: vigente; ejecutado en placa real; dev4 grabado y verificado
actualizado: 2026-10-15
resumen: Respaldo F746+QSPI, identificación del firmware previo, grabación de dev4 y pruebas reales de pantalla, táctil, CAN, SD y franja.
---

# ENS-0019 · Frontal dev4 en la F746 real

## Objetivo y alcance

Armando conecta la placa frontal y pide «realizar las pruebas reales del último código de la pantalla frontal». El objetivo es validar en hardware **stm32_frontal-v1.3-dev4** ([ENS-0018](ENS-0018_mejoras_pantalla_desarrollo.md)), y antes respaldar e identificar lo que había instalado, como exigía el estado del proyecto. Pantallas de desarrollo según [DEC-0010](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md); no se rediseña nada.

## Configuración real

| Elemento | Dato |
|---|---|
| PC y repositorio | Windows 11, rama `main`, HEAD `43b0c29` con cambios locales previos sin commit. Fuentes dev4 idénticas al manifiesto de ENS-0018 (9/9 hashes) |
| Placa | STM32F746G-DISCO por SWD: ID `0x10016449`, UID `0x00420026 0x54435003 0x20353036`, 1 MiB, QSPI Winbond W25Q128FV/JV 16 MiB. ST-LINK V2-1 `0670FF3332584B3043206038`, COM6 |
| Conexiones (declaración de Armando) | USB del ST-LINK; transceptor/bus CAN con **ESP32 encendido**; pedal y dirección en las ADC; **sin microSD** |
| Binario grabado | `stm32_frontal-v1.3-dev4.hex`, SHA-256 `b758edd7d09f8a651b5111b7854037b5869d3f5960e4e48fc6b8636c65f10316` (ELF `5878e195…069a`) |
| Herramientas | OpenOCD xPack 0.12.0-7 local, ARM GNU 14.3.1 (`nm`/`gdb` para símbolos), MSVC para la regresión, Python 3 + Pillow. Nueva herramienta [observar_swd.py](../../07_DESARROLLO/firmware/stm32_frontal/tools/observar_swd.py) |

## Procedimiento y resultados

| Hora | Paso | Resultado |
|---|---|---|
| 12:5x | Identificación sin halt/reset | RDP nivel 0, sin WRP, QSPI en modo mapeado por la aplicación (`CCR FMODE=3`) |
| 12:57–13:27 | Respaldo de solo lectura en ejecución | Interna 1 MiB ×2 idénticas; QSPI 16 MiB ×3 idénticas. Un intento a 4 MHz falló («Fail reading CTRL/STAT») y se repitió a 1,8 MHz. [RESPALDO.md](../../07_DESARROLLO/respaldos/stm32_f746/2026-10-15/RESPALDO.md) |
| — | Identificación del firmware previo | Interna y QSPI coinciden **0 diferencias** con `STM32-final/STM32CubeIDE/Debug/STM32F746G_DISCO.hex` de la entrega histórica. No era v1.3. Hay restos de imágenes antiguas fuera de ese rango |
| 12:59 | Regresión en PC antes de grabar | **101 comprobaciones, 0 fallos**; [log](../evidencias/frontal_f746_2026-10-15/frontend_host_dev4.log). `cmd` no ejecutó el `.exe` por ruta relativa y se lanzó directamente |
| 13:27 | «Antes» con firmware original | La pantalla «KART CROSS 2025/2026» pinta las **filas 0–47 en blanco puro** en el framebuffer. CAN: TEC 81→112 en 1 s, LEC=stuff, FIFO vacío. `uwTick` indicaba arranque hacia 13:19:46; causa de ese reinicio no registrada |
| 13:28–13:29 | `program … verify` con comprobación de chip, UID y RDP | Interna + QSPI grabadas, **Verified OK** en 77 s; `reset run` a las 13:29:41 |
| 13:31 | Relectura independiente | 248.860 B internos y 6.351.932 B QSPI con **0 diferencias** frente al HEX |
| 13:29–16:39 | Observación por SWD sin detener la CPU | `initialized=1` y `watchdog_ready=1`. **11.405 s (3 h 10 min) sin reinicios**, `uwTick` a 1 kHz. Errores CAN TX/RX y ADC a 0 |

**Pantalla y F01–F03 en hardware.** El framebuffer leído del LTDC (RGB565 480×272, doble buffer en `0xC0000000`) muestra el texto previsto, y Armando lee en el panel los avisos citados:

- **F01:** `STM V1.3-DEV4` en la quinta fila.
- **F02:** sin estado extendido, `R:--` y causa `ESP SIN ESTADO RECIENTE`. No aparece ningún umbral numérico.
- **F03:** `MODO: PONER N` y `MODO: PARAR Y SOLTAR PEDAL` (40 columnas exactas) en la quinta fila, mientras la causa de la segunda fila y `SD:ABRE E:0` siguen visibles.

[Montaje de capturas](../evidencias/frontal_f746_2026-10-15/tactil2/montaje_avisos_modos.png). Una captura tomada justo al cambiar el estado aún mostraba el fotograma anterior (el registro va unos cientos de ms por delante del repintado).

**Táctil y modos.** El FT5336 informa contactos durante las pulsaciones pedidas a Armando (16:38: un tramo continuo de ~4,9 s y otro de ~6 s con cortes de 100–300 ms). No se leyeron coordenadas, así que no se asigna cada tramo a su zona. El modo solicitado cambió por táctil DIF→ESC→BLQ→DIF; la mayoría de pulsaciones se rechazaron con aviso 2 por el pedal ([PROB-0012](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0012_pedal_reposo_fuera_de_pedal_libre.md)). Sin contactos espontáneos en unos 4 min de muestreo sin tocar. Queda una anomalía: `sd_requested` pasó a 1 entre 13:29:58 y 13:33:29 sin que Armando declare haber tocado; causa no determinada.

**START.** Armando pulsa START tres veces y el frontal registra 0→1→0→1, una conmutación por pulsación con muestreo de ~50 ms. Es el comportamiento de [DEC-0008](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0008_start_momentaneo_con_memoria.md) en el lado frontal. No se observó el relé ni se midieron rebotes.

**CAN y pareja instalada.** A las 13:30 el bus daba ACK a las tramas del frontal (`ESR=0`, `TXOK0=1`) sin feedback recibido. A las 16:29 llegaban 0x123 fresco (edad 47–94 ms) y BMS 0x355/0x356. **0x124 no se vio en ninguna lectura.** Como el ESP32 v1.3 lo emite cada 100 ms, la hipótesis es que el ESP32 instalado ejecuta un firmware sin 0x124 (v1.2 o anterior), pendiente de confirmar. REC/APL quedan `---`, como prevé [PROTOCOLO_V13](../../07_DESARROLLO/firmware/common/PROTOCOLO_V13.md). BMS sigue oculto.

**SD sin tarjeta.** Ciclo ABRIENDO (~30 s bloqueado en `f_mount`) → ERROR `FR_DISK_ERR` (~50 ms) → reintento. El error casi no se ve en pantalla ([PROB-0013](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md)). El control no se ve afectado. La grabación con tarjeta sigue pendiente.

**Franja ([PROB-0004](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md)).** Patrón escrito solo en la RAM del framebuffer, filas 0–47 de ambos buffers, y restaurado a blanco al terminar. Con bandas de 12 filas Armando ve solo unas líneas del azul (36–47). Con rojo 0–35, amarillo 36–39, magenta 40–43 y cian 44–47 ve **solo el cian**. Conclusión: las filas 0–43 no muestran imagen y el inicio visible está entre las filas 44 y 47. Armando confirma el daño físico y pide no seguir con esa prueba.

## Limitaciones

- Las capturas reflejan el framebuffer que alimenta el LCD, no una fotografía del panel.
- Lo visible en el panel procede de la declaración de Armando.
- Placa en banco con el ESP32 en el bus. No hay marcha, motores ni medida eléctrica; no se observó el relé.
- No hay prueba de grabación SD, coordenadas táctiles ni medida de latencia o jitter.
- No queda identificado el firmware del ESP32.
- Los datos BMS no se interpretan (`BMS_FORMAT_CONFIRMED=0`).
- El respaldo es local, en el mismo PC.

## Conservación y siguiente paso

[Evidencias, scripts y manifiesto SHA-256](../evidencias/frontal_f746_2026-10-15/README.md). Volcados `.raw` locales; PNG equivalentes versionados.

**Estado tras el ensayo:** la F746 queda con **dev4 grabado y verificado**, el ESP32 con firmware sin 0x124 y START activo por la última pulsación de Armando. Siguiente:

1. Calibrar el reposo del pedal (PROB-0012) antes de esperar selección D/R.
2. Identificar el ESP32 por Wi-Fi.
3. Probar la SD con tarjeta.
4. Decidir el tratamiento de PROB-0013.
5. Reservar la franja inutilizable en el diseño final.

Recuperación del original preparada en RESPALDO.md, no ejecutada.
