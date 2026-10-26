# Evidencias ENS-0026 · 21-10-2026

[Ensayo y límites](../../ensayos/ENS-0026_diagnostico_bloqueo_marchas.md).

- `observacion_inicial/`: acceso limitado fallido, sin muestras.
- `observacion_host/`: volcados reales, snapshot decodificado y framebuffer PNG; Pillow faltaba en el primer intérprete, conversión posterior sin nueva medición.
- `serie_inicial/`: 160 muestras del registrador anterior. Etiqueta de reposo planificada, no confirmación física. Error final cp1252 solo en impresión del resumen.
- `estado_selector/`, `transicion_selector_freno/`: CSV completos, log original de cada lectura SWD, identidad y estadísticas.
- `reposo_confirmado/`, `reposo_OFF_N/`: cancelados porque la entrada D estaba activa. No completar ni interpretar como fases de 30 s.
- `wifi_raw.log`, `wifi_eventos.json`, `capturar_wifi.py`: TCP solo recepción, firmware identificado v1.2 y red restaurada.
- `leer_gpio.cfg`, `gpio_inicial.log`: lectura de registros GPIO/UID sin modificar.
- `antes/`, `reproducir_bloqueo.cpp`, `reproducir.cmd`, `reproduccion.log`: fuente base y reproducción PC, 11/0. No firmware reparado/grabado en este corte.

Horas de logs Python antiguos locales; registros nuevos usan UTC explícito. Europe/Lisbon el 21-10 = UTC+1. No credenciales almacenadas. Binarios/volcados locales según reglas de publicación existentes.
