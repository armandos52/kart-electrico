---
estado: diagnosticado; daño físico del panel confirmado y extensión medida
actualizado: 2026-10-15
resumen: Las filas 0–43 del LCD no muestran imagen (inicio visible en 44–47); la interfaz original pintaba de blanco las filas 0–47.
---

# PROB-0004 · Franja blanca en la pantalla frontal

## Diagnóstico 15-10-2026 · ENS-0019

En [ENS-0019](../../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md) se capturó por SWD el framebuffer de la F746 (480×272 RGB565) y se pintaron patrones de color en la franja.

- **Interfaz original** (`STM32-final`): las filas **0–47 son blanco puro** en el framebuffer. La propia interfaz no dibuja nada en esa zona. Con dev4 ocurre igual.
- **Panel:** con un patrón escrito solo en la RAM del framebuffer, Armando ve solo unas líneas de la banda 36–47 y, en la prueba fina, **solo el cian (filas 44–47)**. Rojo (0–35), amarillo (36–39) y magenta (40–43) no se ven.
- **Conclusión:** daño físico del LCD en las **filas 0–43**; la imagen empieza a verse entre las filas 44 y 47. Lo confirma Armando, que da por sabido el daño. Diseñar el contenido esencial a partir de la fila 48, que es además la frontera que ya usaba la interfaz original.
- **Táctil:** el FT5336 informó contactos durante las pulsaciones pedidas, una de ellas en la franja, pero sin coordenadas. No queda medida la respuesta táctil por zonas.

## Declaración inicial · 07-10-2026

Fecha: 07-10-2026. Fuente: declaración de Armando en la conversación sobre panel gráfico. Configuración: pantalla frontal del kart disponible; referencia documental STM32F746G-DISCO 2025/26, correspondencia exacta pendiente.

## Observación declarada

Armando indica una franja horizontal blanca en la parte superior de la pantalla táctil que limita dónde puede situar contenido. No conoce cuántos píxeles están afectados. Atribuye el síntoma a una zona dañada, sin diagnóstico independiente en esta sesión.

## Pendientes y tratamiento

- ~~Registrar extensión en píxeles~~: medida el 15-10 (filas 0–43 sin imagen). Persistencia con otras pantallas no observada aparte.
- Separar fallo de imagen y respuesta del táctil; la declaración no demuestra que el táctil esté dañado.
- Distinguir panel, conexión y generación de imagen antes de establecer causa o reparación. No se propone desmontaje ni sustitución con esta evidencia.
- Diseñar el contenido frontal esencial en el área comprobada como legible, una vez medida. No asignar un margen arbitrario como especificación.

No se vincula causalmente a CAN, SD, control de motores o problemas de alimentación. El panel externo de [DEC-0002](../decisiones/DEC-0002_panel_real_y_calibracion.md) puede diseñarse independientemente de esta limitación visual.

Pendiente: respuesta táctil con coordenadas por zonas. El área afectada ya está registrada ([evidencias](../../06_TRABAJO/evidencias/frontal_f746_2026-10-15/README.md)).
