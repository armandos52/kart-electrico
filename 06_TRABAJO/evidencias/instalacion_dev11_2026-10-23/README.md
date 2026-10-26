# Instalación y diagnóstico real · 23-10-2026

Ver [ENS-0029](../../ensayos/ENS-0029_instalacion_historial_sd.md) para procedimiento, autorización, configuración y límites.

- `antes_host`: observación dev9 real; `antes`: intento limitado fallido.
- `respaldo.log` / `respaldar.cfg`: lectura de respaldo, conservada en 07_DESARROLLO/respaldos/stm32_f746/2026-10-23.
- `binarios`, `fuentes`, `grabacion.log`, `arranque`, `hsd1.bin`, `sd_diag.log`, `relectura`: dev11, instalación y fallo real TX_UNDERRUN. El log de diagnóstico no imprimió mdw, pero el volcado hsd1 sí contiene el estado.
- `dev12/binarios`, `dev12/fuentes`: binario final y diferencias fuente respecto a `fuentes` de dev11. Comentario TX/RX rectificado después de compilar, sin cambio ejecutable. El resto del fuente coincide con el corte dev11 salvo cabecera de versión/adaptador/linker.
- `dev12/grabacion.log`, `dev12/relectura/verificacion.json`: instalación y comparación independiente completa de regiones usadas, cero diferencias.
- `dev12/continuidad`: 1.198 snapshots reales, 59,996 s; grabación y cierre de sesión 2.
- `dev12/reanudacion`: adquisición cancelada al detectar D, no ensayo completo de 60 s.
- `dev12/estado_sd` y `dev12/final`: sesión 3 cerrada, 989 filas, cero pérdidas/error SD, 53 errores ADC acumulados; framebuffer en Entradas.
- Logs `tests*` y `panel_tests*`: pruebas PC, no mediciones físicas. Fallos de permisos/directorio conservados y repetidos correctamente.

`manifest.json` identifica archivos por SHA-256; `verificar_final.py` verifica las relecturas y resume la serie. No contiene credenciales ni originales de la microSD: el usuario no dispone de lector ahora. No se formateó ni borró la tarjeta. ESP32 no grabado en esta sesión.
