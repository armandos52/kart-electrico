# Evidencia ENS-0018 · Pantalla de desarrollo

[Ensayo, alcance y resultados](../../ensayos/ENS-0018_mejoras_pantalla_desarrollo.md). PC Windows, sin conexión/grabación. STM32 dev4 y banco v0.4-dev; ESP32/control/contrato binario sin cambios.

- `frontend_antes.log`: 101 comprobaciones, 13 fallos antes de corregir.
- `frontend_dev4.log`: 101 comprobaciones, 0 fallos después.
- `banco_host.cpp`, `banco_host.cmd`, `banco_host.log`: ejecutan en PC el núcleo y diez escenas; 155 comprobaciones, 0 fallos. Incluyen las 101 anteriores.
- `build_stm32_dev4.log`: compilación/enlace F746 correctos, advertencias conservadas.
- `build_banco_v04.log`: intento fallido por permiso de caché; `build_banco_v04_final.log`: compilación correcta con permisos necesarios, sin upload.
- `test_capture_compatibility.py`, `capture_compatibility.log`: cinco escenarios del capturador con puerto falso. `captura_simulada/` conserva datos/píxeles artificiales; **no imágenes del LCD físico**.
- `antes/`, `despues/`, `cambios.patch`: fuentes de alcance y diferencias. El verificador de build queda copiado aunque no cambia; el nuevo `display_cases.h` solo tiene copia posterior.
- `compilacion/`: binarios, mapa y manifiesto de build; los binarios se conservan localmente y se excluyen de Git.
- `manifest.json`, `conservar_evidencia.py`, `estado_git_inicial.txt`, `commit_base.txt`: huellas, conservación y estado inicial.

Reproducir regresión frontal con su `tests/compilar.cmd` desde la carpeta de tests; ARM con el `tools/build.py` documentado. Ejecutar `banco_host.cmd` desde esta carpeta y `test_capture_compatibility.py` con el Python del entorno kart-esp32; los dos usan las fuentes actuales. El segundo bloquea las llamadas reales a pyserial mediante mocks. Para F411, `platformio run` en su proyecto compila sin grabar. Las copias/hashes fijan la versión analizada si los fuentes cambian después.

Los contadores de pruebas describen software con periféricos simulados. No se acredita LCD/táctil/CAN/SD reales, tiempos físicos, franja blanca ni marcha del kart. Pantallas finales pendientes según DEC-0010.
