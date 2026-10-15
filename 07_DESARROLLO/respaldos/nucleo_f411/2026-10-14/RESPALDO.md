# Respaldo previo de NUCLEO-F411RE · 14-10-2026

ST-LINK `0672FF564881774867244057`; firmware ST-LINK V2J46M33. MCU STM32F411xC/E, ID `0x431`, revisión A, flash detectada 512 KiB. La unidad USB anuncia NOD_F411RE y el puerto es COM5. Pantalla MCUFRIEND UNO declarada por Armando.

Antes de cualquier grabación se leyeron `0x08000000–0x0807FFFF`, 524288 bytes, dos veces con OpenOCD sin halt/reset y una con CubeProgrammer en Hot Plug. Los tres archivos coinciden:

`8d88e79ec69e18e0d3038e93306b7bb4e4ad0ef1cd04b2f73c3d7b9ce3ac3722`

- `flash_lectura1.bin`, `flash_lectura2.bin`, `flash_cubeprogrammer.bin`: copias del programa previo, no compilaciones propias. Nombre/versión/fuente de ese programa desconocidos.
- `option_bytes_lectura.bin`: lectura de 16 bytes desde `0x1FFFC000`; hash `d292558017cf9ca0a2e40e262a5c1daa4b305ccf084ce06128133d282f905115`. Interpretación de CubeProgrammer en log: RDP nivel 0, sin protección de escritura, watchdog por software. No se escribieron option bytes.
- `MBED.HTM`: acceso virtual del ST-LINK, guardado únicamente aquí y excluido de Git por contener un código de dispositivo. No es el firmware.
- `diagnostico_v0.1.*` y posteriores `banco_v*.*`: binarios de pruebas nuevos, diferenciados del respaldo previo.
- `vistas_v0.3_01.bin/.elf`: primera adaptación de diez escenas, sustituida por la compilación 02.
- `vistas_v0.3_02.bin/.elf`: banco visual final de [ENS-0016](../../../../06_TRABAJO/ensayos/ENS-0016_vistas_frontal_nucleo.md), grabado/verificado. BIN 56724 B, SHA-256 `fbf86237a4888b81c80508a69d8648d6ae25fa0596848108d972be4f159031a1`. B1 navega; captura GRAM por filas confirmadas. Son compilaciones nuevas, no lecturas del programa previo.

Tras la lectura, CubeProgrammer confirma el núcleo en ejecución. Esto no prueba la función de la aplicación anterior ni el estado visual del LCD. No hubo modificación de firmware hasta la fase posterior de diagnóstico registrada en ENS-0015.

Recuperación: usar CubeProgrammer, conexión SWD al número de serie anterior, grabar `flash_lectura1.bin` en `0x08000000`, verificar los 524288 bytes y reiniciar. No utilizar opciones de desbloqueo/protección ni escribir el archivo de option bytes como si fuera un firmware. El procedimiento de recuperación está preparado; no se afirma una restauración que no conste en el ensayo.

Los binarios se excluyen del repositorio público. Son copias locales en el mismo ordenador; no copia de seguridad externa, ni respaldo de SD/memoria externa del shield. [Logs y hashes](../../../../06_TRABAJO/evidencias/nucleo_f411_2026-10-14/) · [ENS-0015](../../../../06_TRABAJO/ensayos/ENS-0015_nucleo_f411_pantalla_banco.md).
