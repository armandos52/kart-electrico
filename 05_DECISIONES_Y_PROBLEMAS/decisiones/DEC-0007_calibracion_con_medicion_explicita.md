# DEC-0007 · Colocar y medir como acciones separadas

Fecha: 11-10-2026. Estado: implementado en PC; validación física pendiente.

Armando pide corregir la calibración que no pudo completar y hacerla visual, suave y paso a paso. Se decide que cada posición se mide tras un clic explícito, con cuenta atrás, bloques de progreso y motivo de rechazo visible. Un intento rechazado conserva las posiciones previas y admite repetición; una sesión distinta requiere nuevo ensayo. No se eliminan las comprobaciones de señal ni se relajan sus límites para obtener un resultado aceptado.

Motivo: seis registros previos sin centro aceptado; dos llegan a 0 durante la adquisición inicial. La guía confundía colocación y captura y podía esperar indefinidamente estabilidad. El valor es CAN normalizado: no se atribuye la causa al sensor sin datos físicos.

Alcance: panel local `0.4.0-calibracion-pasos`, perfiles de visualización del PC. Sin modificación del firmware ni validación de ángulos físicos. Mantener datos originales y registrar lecturas antes/durante cada medición por separado. [ENS-0012, resultados y límites](../../06_TRABAJO/ensayos/ENS-0012_calibracion_por_pasos.md), [PROB-0007](../problemas/PROB-0007_variacion_y_centro_direccion.md).
