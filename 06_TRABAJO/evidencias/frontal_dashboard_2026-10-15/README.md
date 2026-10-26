# Evidencia ENS-0022 · dashboard frontal · 15–16-10-2026

[Ensayo, método, resultados y límites](../../ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md). Trabajo local, sin conectar ni grabar hardware. Instalado conocido: F746 dev4; fuente/paquete nuevo: `stm32_frontal-v1.3-dev6-ui`.

- `antes/`: base dev5 de los archivos de inicio; no constituye un snapshot completo de todas las dependencias.
- `descartado_otro_chat/`: cambios concurrentes apartados por instrucción explícita del usuario; no son el resultado de este ensayo.
- `despues/`, `cambios.patch`: fuentes propias finales, tests, herramientas y contraste con la base disponible. `manifest.json` identifica sus hashes.
- `build_primero.log`, `build_segundo.log`: fallos de adaptación a API TouchGFX; `build_tercero.log` y `build_final.log`: compilaciones correctas, con advertencias de dependencias.
- `frontend_tests.log`: 130 comprobaciones, 1 fallo de truncamiento del aviso antiguo. `frontend_final.log`: 130/0 tras corregirlo.
- `dashboard_inicial.log`, `dashboard_final.log`: 139/0, función C real con datos sintéticos; repetición final tras compactar el texto compartido con F411. `renders.jsonl`, `renders/`, [galería](GALERIA.md): primitivas y renders de PC, **no framebuffer físico**.
- `swd_tests.log`, `layout_arm.txt`: compatibilidad del decodificador y disposición de la estructura en el ELF ARM. No lectura de placa.
- `banco_host.cpp/.cmd`, `banco_host.log`: 184/0 en PC, incluidas las 130 frontales.
- `build_f411.log`: primer intento bloqueado por caché local; `build_f411_final.log`: repetición con acceso al entorno de compilación.
- `binarios/`: ELF/HEX F746, mapa y hashes del build; ELF/BIN del banco F411 compilado correctamente (63.828 bytes flash, 2.304 bytes RAM estática). Son productos de compilación, no respaldos de una nueva lectura física.
- `conservar.py`, `manifest.json`: copia final y SHA-256. No sustituyen una copia externa del proyecto.
- `comprobacion_entrega.json`: enlaces locales existentes y hashes de fuentes/binarios coincidentes con el build conservado; TTF coincidente con las métricas generadas.

## Reproducir

Desde `07_DESARROLLO/firmware/stm32_frontal/tests`: `compilar.cmd`, `compilar_dashboard.cmd <ruta-renders.jsonl>` y `python test_observar_swd.py`. El segundo emite escenas del renderer usado por el firmware. MSVC 2022 configurado por los `.cmd`.

Desde la raíz: `python 07_DESARROLLO/firmware/stm32_frontal/tools/build.py`. Para imágenes, Python con Pillow: `tools/renderizar_dashboard.py <renders.jsonl> <salida>`. El runtime usado es `C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe`. `tools/generar_fuente_dashboard.py` regenera los glifos usando el TTF y licencia conservados; no hace falta ejecutarlo para compilar el firmware.

Para banco compartido, `banco_host.cmd` desde esta carpeta. Para F411, `python -m platformio run` desde su proyecto. Ninguno de estos comandos graba una placa. Si avanzan las fuentes, usar los archivos de `despues/` y las dependencias de la entrega identificadas por `ORIGEN_V0.json`.
