# Evidencia · revisión de diferencial y reversa · 17-10-2026

[ENS-0025](../../ensayos/ENS-0025_revision_diferencial_y_reversa.md) y [plan](../../REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md). Solo PC, periféricos simulados, sin implementación de correcciones ni hardware. Datos sintéticos no equivalen a mediciones del kart.

- `fuentes.json`: HEAD y SHA-256 de ocho fuentes de alcance y `.ino` v1.2; snapshots en `fuentes/`. `estado_inicial.txt` conserva cambios previos. No credenciales.
- `baseline.log`: 56/0 sobre copia del núcleo dev3.
- `diagnostico_dev3.log`: 16 comprobaciones, 5 fallos intencionalmente visibles, correspondientes a dos comportamientos pendientes (rampa y memoria de corte con pedal libre). Once controles positivos.
- `diagnostico_v12.log`: 3/3 reproducciones de defecto, PWM 0/0 a pedal pleno en R y RPM elevadas. Cero discrepancias **no** significa aprobar el firmware antiguo.
- `build_*.log`: compilaciones MSVC; advertencias v1.2 y arnés conservadas. `resultados_exit.txt`: dev3=1 por requisitos propuestos incumplidos; v12=0 por reproducciones logradas.
- `declaracion_usuario.json`: declaraciones/atribuciones y límites de medición.
- `verificacion_integridad.json`: producción intacta respecto a snapshot; equivalencia de fuente v1.2/arnés. `hashes.json`: scripts, fuentes, logs, ejecutables de PC y dependencias del arnés histórico.

Reproducción desde esta carpeta: `comprobar.cmd` con MSVC instalado usa los snapshots ya conservados. `preparar.py` fue ejecutado una vez y se niega a sobrescribir `fuentes/`. Los ejecutables `.exe` son para Windows; no son firmware ni se deben grabar en placas. El arnés v1.2 incluye el [host histórico](../AUD_V12_2026-10-08/control_host.cpp), sus [stubs](../AUD_V12_2026-10-08/stubs.hpp) y [casos](../AUD_V12_2026-10-08/casos.cpp); se verifican sus hashes y equivalencia del cuerpo con `905b09e`.
