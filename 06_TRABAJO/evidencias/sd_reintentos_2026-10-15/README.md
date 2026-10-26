# Evidencia ENS-0021 · SD, retoma local del 15-10-2026

[Ficha del ensayo](../../ensayos/ENS-0021_sd_reintentos_locales.md). Datos de pruebas **sintéticos**, sin hardware conectado.

- `base_commit.txt`, `estado_inicial.txt`: punto de partida y trabajo anterior preservado.
- `antes/`, `despues/`, `cambios.patch`: fuentes para repetir/comparar; la prueba nueva solo aparece en `despues/`.
- `logs/antes.txt`, `logs/antes_casos_fijos.txt`: regresión reproducida sobre dev4, 120 comprobaciones/16 fallos en ambos; ajuste del callback descrito en el ensayo.
- `logs/despues.txt`: dev5, 120/0; `logs/banco_host.txt`: 174/0, incluidas esas 120.
- `banco_host.cpp`, `banco_host.cmd`: arnés reutilizado de ENS-0018 que compila contra las fuentes actuales; los snapshots fijan las fuentes de este ensayo.
- `logs/build_f746.txt`, `logs/build_f411.txt`: compilaciones, tamaños y advertencias. No ejecutan upload.
- Los logs `*_inicial.txt` conservan la primera ejecución positiva; los finales repiten comprobaciones tras explicitar una conversión de enum a `uint8_t` que generaba una advertencia MSVC. El último build F746 reutiliza caché; el inicial conserva las advertencias de dependencias.
- `binarios/`: ELF/HEX F746 dev5, mapa y hashes del build; BIN/ELF F411 de la revisión de fuente 0.4-dev con núcleo dev5, sin grabar.
- `continuidad.json`: contraste SHA-256 focalizado de binarios dev4 y registros previos frente al manifiesto de ENS-0019; no es una nueva lectura de la placa.
- `manifest.json`: SHA-256 de cada archivo de esta evidencia salvo el propio manifiesto. `conservar.py` genera copias y manifiesto; la carpeta no es una copia externa.

Reproducción: `tests/compilar.cmd` desde los tests frontales; `tools/build.py` para F746. `banco_host.cmd` desde esta carpeta; `platformio run` en el proyecto F411. Usar los fuentes conservados si la rama ha avanzado. Entornos y comandos exactos constan en los logs y README del firmware.

El problema de espera dentro del driver continúa pendiente; no se declara validación de tarjeta, montaje eléctrico, escritura real ni respuesta inmediata a pausa. Firmware instalado conocido: F746 dev4 y F411 v0.3.
