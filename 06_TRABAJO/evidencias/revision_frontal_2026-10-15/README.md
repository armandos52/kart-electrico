# Evidencia de ENS-0017 · 15-10-2026

[Dictamen, procedimiento y preparación de instalación](../../ensayos/ENS-0017_revision_preinstalacion_frontal.md). Ejecución en PC Windows, sin hardware, red del kart ni modificaciones al firmware de trabajo.

- `frontend_tests.log`: 83 comprobaciones existentes, 0 fallos.
- `backend_tests.log`: 56 comprobaciones existentes, 0 fallos.
- `build_stm32.log`: compilación ARM, 151 unidades, ELF/HEX correctos, con advertencias; no upload.
- `audit_display.cpp`, `compilar_audit.cmd`, `audit_display.log`: cuatro comprobaciones adicionales; tres fallos F01/F02/F03 reproducidos y un control positivo. El comando devuelve 1 mientras los hallazgos sigan presentes.
- `fuentes/`: 36 archivos conservados con sus rutas, sin credenciales; selección revisada y dependencias directas del arnés. No son todos los archivos del proyecto ni acreditan lectura integral de cada dependencia.
- `compilacion/`: ELF/HEX, mapa y manifiesto de build del 15-10. ELF/HEX idénticos por SHA-256 a los dev3 preservados del 13-10; no nueva versión ni imagen leída del kart. Binarios locales sujetos a exclusiones del repositorio.
- `manifest.json`: hashes de fuentes y resultados, commit completo y comparación con binarios preservados.
- `estado_git_inicial.txt`, `commit_base.txt`: punto de partida con trabajo previo conservado.
- `conservar_evidencia.py`: reproducción de la conservación, limitada a archivos explícitos de esta revisión.

Para reproducir desde la raíz, ejecutar los dos `tests/compilar.cmd` en sus respectivas carpetas, `tools/build.py` con el Python documentado en el README frontal y el `compilar_audit.cmd` de esta carpeta. Este último usa las **fuentes de trabajo actuales**: tras corregir firmware su resultado podrá cambiar; las copias/hash de esta sesión permiten reconstruir el estado analizado. No ejecutar los binarios de firmware para instalar sin seguir la preparación descrita en el ensayo.

Los resultados negativos quedan preservados deliberadamente. No hay prueba visual F746, SD/CAN físico ni medición de tiempo de parada. No es respaldo independiente.
