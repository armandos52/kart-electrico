# Evidencias ENS-0024 · BMS visible · 16-10-2026

[Objetivo, configuración, resultados y límites](../../ensayos/ENS-0024_bms_visible_diagnostico.md).

- `antes/`, `despues/`, `cambios.patch`: base dev7-ui y nueva dev8-ui; pruebas BMS añadidas en `despues/tests/bms_dashboard_cases.h`.
- `frontend.log`: 150/0; `dashboard.log`: 163/0. `renders.jsonl` y `renders/`: 124 escenas de PC; caso 6 usa bytes del ensayo real con reloj/escenario sintéticos.
- `build_f746.log`, `binarios/`, `identidad.json`: paquete compilado y SHA-256; QSPI idéntica a dev7-ui. No nuevo build F411.
- `grabar_dev8.cfg`, `grabacion.log`, `releer_dev8.cfg`, `relectura/`: programación y lectura independiente; consultar resultados concretos en ensayo.
- `conservar.py`: conservación/preparación/comparación, sin acceso al hardware; reutiliza parser HEX del ensayo anterior.
- `manifest.json`: hashes de evidencias. Volcados y binarios locales excluidos de publicación, sin borrar copias históricas.

Las etiquetas con asterisco significan interpretación provisional del CAN, no validación del BMS ni estimación del SOC a partir de tensión. La revisión documental no confirmó modelo/protocolo. Se conservan flags de validación y bytes originales.
