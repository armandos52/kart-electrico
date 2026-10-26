# Evidencia ENS-0027 · solo PC · 21-10-2026

[Ensayo, diseño y límites](../../ensayos/ENS-0027_pedal_media_ponderada.md). Pantalla desconectada por declaración del usuario; **dev10-ui preparada, no instalada**. Última física dev9-ui, evidencia preservada en [ENS-0026](../marchas_2026-10-21/README.md).

- `fuentes/`: copia del código de alcance y dependencias del arnés; `cambios.patch` frente al dev9 conservado. Originales intactos.
- `frontend_1.log`, `frontend_2.log`, `frontend_final.log`: 185/3 → 185/1 → 185/0. Negativos explicados en ensayo.
- `dashboard.log`, `renders.jsonl`, `renders/`: 163/0 y 124 vistas de PC; no capturas físicas.
- `binarios/`, `build_f746.log`, `identidad.json`: ELF/HEX dev10, mapa, hashes de build y SHA-256 finales; sin programación.
- `datos_originales_resumen.json`: origen, SHA-256, número, media, desviación y extremos de cada CSV histórico. `reposo_replay.csv` conserva sus ADC y tiempos. Los originales completos están en ENS-0026.
- `replay.cpp`, `replay.cmd`, `replay.log`: función real ejecutada sobre 2.061 ADC históricos; otros periféricos simulados. 0 incumplimientos. Para repetir, ejecutar `replay.cmd` desde esta carpeta con MSVC 2022 Build Tools.
- `curvas.csv`: comparación calculada de mando estable de dev8/dev9/dev10. `escalon_sintetico.csv`: salida del código real con entrada sintética y reloj ideal de 5 ms. No son medidas físicas.
- `preparar.py`: empaquetado, extracción de datos históricos y manifiesto; sin acceso al hardware. `manifest.json`: hashes de evidencia, sin ejecutables/objetos temporales.

La prueba de replay no reproduce el espectro real a 200 Hz: los CSV anteriores capturaban aproximadamente a 20 Hz. No se da por resuelta la fuente del ruido, la respuesta mecánica ni la calibración de topes. Sin commit/push ni nueva copia externa.
