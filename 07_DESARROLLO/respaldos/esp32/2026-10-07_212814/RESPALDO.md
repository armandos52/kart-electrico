# Flash antes de recuperar la imagen original · 07-10-2026

Placa ESP32-D0WD-V3 rev. 3.1, MAC `70:4b:ca:7c:df:68`, CP210x COM4. Solo el módulo ESP32 fuera del kart, conectado por USB según el usuario.

Dos lecturas completas de 4 194 304 bytes, con aplicación detenida en modo descarga, coinciden:

`SHA-256 33275b8680982c0f4ed5a4731c878dd1ff22da03c40debc9697a88e2adad0a62`

Archivos: `flash_lectura1.bin`, `flash_lectura2.bin`, `log.txt` (identidad, lecturas y eFuses de solo lectura). Los binarios quedan fuera de Git.

Estado registrado antes de recuperar: última grabación documentada v1.1; USB con `CAN init failed` y sin JSON, esperable sin módulo CAN. No se atribuye el fallo de marcha en el kart a esta captura.

Conservado para poder volver a la flash previa a la recuperación. No se ha ensayado su restauración. Esta copia está en el mismo disco.

Ver [ENS-0002](../../../../06_TRABAJO/ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md).
