# PUB-0002 · Publicación del trabajo acumulado · 26-10-2026

Armando solicita publicar programas, fotos, pruebas y avances de estas semanas. [Síntesis y prioridades](AVANCES_2026-10-26.md). Base local `main`, `f9f66a7`, con dos commits locales y cambios acumulados. Destino comprobado: `armandos52/kart-electrico`, público, permisos ADMIN. Se conserva el trabajo previo y el historial.

Se incorporan fuentes de firmware/panel, ensayos ENS-0020–0027 y ENS-0029, registros, capturas, respaldos documentales y hashes; los binarios/volcados privados y originales ya excluidos permanecen locales. No se recibieron fotos nuevas del condensador. La configuración local `.claude` queda excluida. Se actualizan portada, dossier, estado, tareas y diario.

Hallazgo del condensador y pérdida aproximada de medio rango ADC: declaración del usuario, no medida nueva; esquema, identidad de REF y contraste antes/después pendientes. IMU en búsqueda, no seleccionada. F746 dev12/ESP32 v1.2 según las últimas evidencias; sin instalación ni prueba física en esta sesión.

## Verificaciones y publicación

Verificación local completada. [Evidencias](evidencias/publicacion_2026-10-26/): control 56/0, frontal 170/0, dashboard 163/0, decodificador SWD 3/0 y panel Python 35/0; JavaScript robustez 16/0, velocidad PASS e idiomas 33/33. Enlaces públicos: cero errores.

Primer intento del panel: errores de acceso a temporales del entorno restringido; repetición en host satisfactoria. Idiomas: primer intento bloqueado por alias Python; con runtime explícito se detectaron 30 claves IMU y tres mensajes de historial sin traducción (30/33). Corregidos en `textos.js`; repetición 33/33. No se modifica lógica de control ni firmware. Se corrigieron cuatro enlaces Obsidian y referencias a binarios locales. Se conservan logs negativos y advertencias de compilación.

Rama publicada: `codex/publicacion-avances-2026-10-26`, contenido `76ba5c3` y auditoría `904797e`. [PR #3](https://github.com/armandos52/kart-electrico/pull/3). [CI Linux satisfactoria en 904797e](https://github.com/armandos52/kart-electrico/actions/runs/40393405699): panel, JavaScript, control/frontal y navegación. Auditoría acotada: 1.384 blobs/entradas, 37.262.270 bytes, sin hallazgos ni archivos prohibidos; [informe](evidencias/publicacion_2026-10-26/auditoria_commit.json). No sustituye auditoría exhaustiva.

El conector GitHub devolvió 403 al crear la PR; se creó con GitHub CLI autenticado. Las referencias finales y el estado de integración se consultan en PR #3. No se fuerza ni reescribe el historial. Pruebas PC no equivalen a aceptación del kart ni del replay con archivos reales. Se conserva el diagnóstico negativo de reversa de ENS-0025 y los pendientes de ENS-0029.

Incidencia de entorno: primer fetch no pudo escribir `.git/FETCH_HEAD` y la consulta GitHub no tuvo red en el entorno restringido. Repetidos con permisos del host, ambos correctos. Sin pérdida de archivos.
