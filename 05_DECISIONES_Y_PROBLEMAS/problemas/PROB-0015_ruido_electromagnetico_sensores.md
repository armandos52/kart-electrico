---
estado: abierto; ruido declarado, caracterización instrumental pendiente
actualizado: 2026-10-23
---

# PROB-0015 · Interferencia declarada en dirección y otros sensores

## Actualización · 26-10-2026

[Declaración de Armando y plan de contraste](../../06_TRABAJO/AVANCES_2026-10-26.md): informa de conexión incorrecta del condensador de dirección entre masa y REF, prevista entre masa y VCC, y atribuye pérdida aproximada de medio rango ADC. Sin esquema ni medidas antes/después; reparación y relación con ruido general pendientes. Además busca una IMU para modos dependientes y telemetría; sin selección, instalación ni validación nueva. Este problema continúa abierto.

Armando informa el 17-10-2026 de mucho ruido electromagnético que atribuye a los diferenciales: afecta dirección y otros sensores, con señal de referencia turbia e inconsistente. Propone observar frecuencias con osciloscopio y diseñar filtro adecuado. Las pruebas de motores se realizan en banco con ruedas elevadas según su aclaración.

Se registra como observación/atribución del usuario. No se han recibido trazas, amplitudes, frecuencias, identificación exacta de otros sensores ni del punto llamado «referencia». Fuente, ruta de acoplamiento y causalidad de variaciones previas quedan pendientes. No se considera medida la frecuencia PWM de código ni la nominal del fabricante.

El frontend dev8-ui toma muestras ADC individuales, valida rango y normaliza; no filtra fluctuaciones dentro de rango. PWM→Kelly tiene su propio acondicionamiento analógico histórico, distinto del filtro de entrada de sensores. La calibración visual del PC no corrige automáticamente el control.

[Plan de caracterización y filtrado](../../06_TRABAJO/REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md): comparar condiciones eléctricas controladas, medir señal/alimentación/referencia/masas y Hall, conservar trazas/FFT/ajustes, identificar acoplamiento; dimensionar acondicionamiento y filtro analógico antes de elegir filtro digital. Medir atenuación y latencia, conservar brutos y no filtrar paradas ni esconder exceso de velocidad. Frecuencias/cortes/componentes/coeficientes sin elegir hasta medir.

Cierre: evidencia reproducible antes/después bajo iguales condiciones, variación y retardo aceptables para el control, ausencia de saturación/pérdidas y contraste del supervisor de reversa. [PROB-0007 dirección](PROB-0007_variacion_y_centro_direccion.md), [PROB-0012 pedal](PROB-0012_pedal_reposo_fuera_de_pedal_libre.md), [PROB-0014 reversa](PROB-0014_reversa_y_arquitectura_diferencial.md), [ENS-0025](../../06_TRABAJO/ensayos/ENS-0025_revision_diferencial_y_reversa.md). Sin filtro implementado ni ensayo nuevo de hardware en esta sesión.


## Investigación · 23-10-2026

Armando declara ≈1–casi 2 V con osciloscopio y atribuye el problema a recorrido limitado del potenciómetro/divisor incorrecto. [Revisión ADC, tabla y plan de contraste](../../06_TRABAJO/INVESTIGACION_ADC_DIRECCION_2026-10-23.md): rango estrecho compatible con pérdida de aprovechamiento; no confirma causa del ruido ni divisor sin esquema/medidas. ADC3 CH8 PF10, 12 bits, límites 1042–3313 (942–3413 admisibles) confirmados en dev9 preservada y fuente local dev11-log. 3,3 V nominal, no medido. No aplicar 0–5 V directamente. Pendientes captura/punto de medida, centro/topes, VREF y conciliación con ADC 813 del 21-10. Sin cambios ni ensayo físico; problema abierto.


## Ampliación del usuario · Ruido general y PCB · 23-10-2026

Armando reafirma ruido general del sistema e indica placas sin GND extendido por toda su superficie. Propone filtros pasaaltos/pasabajos y posterior mejora de arquitectura, planos de masa y pistas diseñadas por función eléctrica. [Registro para el informe y criterios de diseño](../../06_TRABAJO/INVESTIGACION_ADC_DIRECCION_2026-10-23.md#ampliación-para-el-informe-ruido-general-filtrado-y-placas--23-10-2026). Declaración sin inspección física/layout nueva; causa y eficacia pendientes. Seleccionar filtro por señal: conservar continua de dirección/pedal, estudiar retornos y desacoplo; un plano GND no garantiza eliminar todo ruido. No se cierra el problema por la hipótesis del potenciómetro ni se implementa filtro/PCB.


**Rectificación posterior de Armando, 23-10:** solo pasabajos para el ruido general; pasaaltos retirado de la propuesta vigente. Corte/orden/componentes pendientes de caracterización por señal; siguen las mejoras de PCB/retornos. No se afirma eficacia medida ni implementación.
