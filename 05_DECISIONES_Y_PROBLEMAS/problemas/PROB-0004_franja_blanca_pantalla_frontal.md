---
estado: declarado_sin_diagnostico
actualizado: 2026-10-07
resumen: El usuario declara una franja blanca horizontal superior en la pantalla táctil; área afectada y causa pendientes.
---

# PROB-0004 · Franja blanca en la pantalla frontal

Fecha: 07-10-2026. Fuente: declaración de Armando en la conversación sobre panel gráfico. Configuración: pantalla frontal del kart disponible; referencia documental STM32F746G-DISCO 2025/26, correspondencia exacta pendiente.

## Observación declarada

Armando indica una franja horizontal blanca en la parte superior de la pantalla táctil que limita dónde puede situar contenido. No conoce cuántos píxeles están afectados. Atribuye el síntoma a una zona dañada, sin diagnóstico independiente en esta sesión.

## Pendientes y tratamiento

- Registrar extensión en píxeles, persistencia y momento de aparición (arranque y distintas pantallas).
- Separar fallo de imagen y respuesta del táctil; la declaración no demuestra que el táctil esté dañado.
- Distinguir panel, conexión y generación de imagen antes de establecer causa o reparación. No se propone desmontaje ni sustitución con esta evidencia.
- Diseñar el contenido frontal esencial en el área comprobada como legible, una vez medida. No asignar un margen arbitrario como especificación.

No se vincula causalmente a CAN, SD, control de motores o problemas de alimentación. El panel externo de [DEC-0002](../decisiones/DEC-0002_panel_real_y_calibracion.md) puede diseñarse independientemente de esta limitación visual.

Evidencia técnica complementaria pendiente: comprobación física y registro del área afectada. No hay nuevos originales incorporados ni revisión visual de la pantalla en esta sesión.
