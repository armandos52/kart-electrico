# Verificaciones del panel

Las pruebas usan datos sintéticos y servicios locales aislados. No conectan placas.

## Vigentes

- `python -m unittest discover -s ../tests -v`, desde esta carpeta: lector, HTTP, estados y calibración.
- `node check-robustez.mjs`, `node check-velocidad.mjs`, `node check-idiomas.mjs`: contratos sin navegador. `PYTHON` permite seleccionar intérprete para idiomas.
- `check-guided.cjs`: flujo actual de calibración, servidor `calibration_fixture.py` en 8877 y panel desconectado en 8766; requiere carpeta de salida como primer argumento. Utiliza rutas de Playwright/Chrome del banco de desarrollo y no se ejecuta en CI.
- `check-modelo.cjs`, `check-esquema.cjs`, `check-sentido.cjs`, `check-velocidad-browser.cjs`: pruebas visuales del banco de desarrollo; pueden requerir las cuatro fotos locales, incluida la frontal no publicada. No constituyen un procedimiento portable de aceptación.
- `check-publicacion.cjs`: comprobación del panel recién clonado, sin frontal privada ni conexión al kart. Usa Playwright instalado por separado; admite `KART_PANEL_URL`, `KART_CHROME` y carpeta de salida. No descarga dependencias ni abre puertos serie.

## Antecedentes

`check-calibration.cjs`, `smoke-calibration.cjs` y `check-panel.cjs` corresponden a interfaces anteriores. Se conservan para interpretar la evidencia histórica; para el asistente actual usar `check-guided.cjs` y los tests Python. Las capturas y resultados originales no se regeneran encima de sus archivos.
