# Evidencias ENS-0023 · 16-10-2026

[Ensayo y límites](../../ensayos/ENS-0023_bugs_e_instalacion_dashboard.md). Cuatro fallos reproducidos y corregidos en `stm32_frontal-v1.3-dev7-ui`.

- `antes/`, `despues/`, `cambios.patch`: fuente previa dev6-ui y resultado conservado. Dependencias completas en el proyecto de firmware; no se duplican aquí.
- `frontend_antes.log`, `dashboard_antes.log`: resultados negativos reproducidos, 140/5 y 144/3.
- `frontend_final.log`, `dashboard_despues.log`: 150/0 y 145/0. `frontend_despues.log` conserva el paso 149/0 previo al último control de pausa/cabecera.
- `banco_host.log`: 204/0, incluidas 150 frontales. `swd_tests.log`: 3/0.
- `renders.jsonl`: geometría/datos sintéticos; no capturas de LCD.
- `build_f746.log`, `build_f411.log`, `binarios/`: compilación y paquetes conservados. F411 no se graba.
- `identificar*.log`, `respaldar.cfg`, `respaldo.log`: acceso e identificación/respaldo físicos. La copia de memorias queda en `../../../07_DESARROLLO/respaldos/stm32_f746/2026-10-16/`.
- `grabar_dev7.cfg`, `releer_dev7.cfg`: programación preparada y lectura independiente. Su existencia no acredita ejecución; consultar logs y ensayo.
- `preparar_instalacion.py`: archivo de fuentes/binarios, parser HEX con checksum, comparación de respaldos y relectura; no programa hardware.
- `manifest.json`: tamaños y SHA-256 de evidencias, generado al cerrar. Binarios/volcados se conservan localmente; no equivale a copia independiente.

Reproducción PC desde raíz: ejecutar `tests/compilar.cmd` en el directorio de pruebas frontal, `compilar_dashboard.cmd <ruta_JSONL>`, `python tests/test_observar_swd.py`, `cmd /c banco_host.cmd` aquí, y `python tools/build.py` para F746. Rutas/herramientas exactas en README del firmware y logs. Ejecutar el archivo de conservación solo con todos los builds correctos; `backup`/`verify` exigen lecturas completas coincidentes.
