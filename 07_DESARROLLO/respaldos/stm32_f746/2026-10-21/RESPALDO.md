# Respaldo previo a dev9-ui · F746 · 21-10-2026

[ENS-0026](../../../../06_TRABAJO/ensayos/ENS-0026_diagnostico_bloqueo_marchas.md), [log](../../../../06_TRABAJO/evidencias/marchas_2026-10-21/respaldo.log), [verificación y SHA-256](verificacion.json).

UID `00420026 54435003 20353036`, ST-LINK `0670FF3332584B3043206038`. CPU en ejecución, solo lectura sin reconfigurar QSPI. Dos lecturas completas de flash interna (1.048.576 B cada una), coincidentes. Una lectura QSPI completa de 16.777.216 B, idéntica byte a byte por SHA-256 al respaldo completo del 16-10; no se realizó una segunda QSPI nueva. OTP 1056 B y opciones 32 B también conservados. El marcador heredado QSPI2_FIN del log no acredita otra lectura.

Firmware identificado **stm32_frontal-v1.3-dev8-ui**, coincidencia byte a byte de las regiones usadas del HEX (interna y QSPI). Para recuperar, usar el HEX completo dev8-ui (solo local: `06_TRABAJO/evidencias/bms_dashboard_2026-10-16/binarios/stm32_frontal-v1.3-dev8-ui.hex`) con el procedimiento verificado de ENS-0024; no escribir OTP/opciones ni desbloquear protección. Respaldos anteriores y fuentes intactos. Copia local, no respaldo externo independiente.
