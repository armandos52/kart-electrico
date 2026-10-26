---
estado: abierto; observado en placa real, sin corrección
actualizado: 2026-10-16
resumen: Con dev4 en la F746 real, el pedal en reposo queda por encima del umbral de pedal libre y bloquea cambios de modo y selección D/R.
---

# PROB-0012 · Pedal en reposo fuera de «pedal libre»

Observado en [ENS-0019](../../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md). Configuración: F746 con stm32_frontal-v1.3-dev4 y pedal conectado, según declaración de Armando. Lecturas por SWD del snapshot `ui`. Se supone que el pedal no estaba pisado, porque Armando solo tocaba la pantalla; queda pendiente confirmarlo en una medición dedicada.

**Hecho medido:** la ADC física del acelerador en reposo osciló entre **149 y 194 cuentas** en cuatro observaciones (13:29–16:39). La versión normalizada 0–4095 sobre la referencia histórica 124–1861 ([kart_frontend.c](../../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c), `ACCEL_MIN/ACCEL_MAX`) dio 58–165. El código considera el pedal libre solo con normalizado ≤ 40, es decir, física ≤ ~141.

**Consecuencia observada:** la mayoría de pulsaciones de DIF terminaron en `MODO: PARAR Y SOLTAR PEDAL`. Solo dos cambios de modo se aceptaron, en instantes en que la lectura bajó del umbral. Por código, la selección D/R exige además `pedal_free`, de modo que con esta calibración el kart no entraría en marcha de forma fiable. El efecto es conservador: bloquea, no acelera.

**Pendiente:** medir el reposo y el recorrido completo del pedal con registro y repetición, en reposo y pisado, como pide el README del frontal («deben calibrarse físicamente»). Después decidir el nuevo `ACCEL_MIN`, o el margen de pedal libre, con evidencia y pruebas de regresión. No ajustar cifras sin esa medición. Relacionado con la calibración de dirección de [PROB-0007](PROB-0007_variacion_y_centro_direccion.md).

Retoma del 15-10: [ENS-0020](../../06_TRABAJO/ensayos/ENS-0020_registro_pedal_f746.md) deja trazado el registrador preparado y el intento técnico anterior (`open failed`, sin muestras). Armando pide continuar solo con código/pruebas locales. Dev5 cambia SD, conserva estos umbrales y no sustituye la medición pendiente.

16-10: [ENS-0022](../../06_TRABAJO/ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md) añade ADC/umbral y condiciones de D/R a las páginas Entradas/Marcha de dev6-ui. Ensayo sintético ADC 170→108 normalizado reproduce el bloqueo; no se cambia calibración ni se toman medidas nuevas. Armando declara kart suspendido y pruebas con motores posibles; instalación/registro físico pendientes. Problema abierto.

16-10, [ENS-0023](../../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md): dev7-ui instalada. Tres lecturas reales 139/139/152 cuentas físicas y 35/35/66 normalizadas, libre 1/1/0. Sin instrucción/confirmación de posición del pie durante estas capturas, no son serie de reposo ni calibración. Se observa D y START ON posteriormente; no acredita giro ni elimina el problema de variación. Umbrales sin cambiar.
