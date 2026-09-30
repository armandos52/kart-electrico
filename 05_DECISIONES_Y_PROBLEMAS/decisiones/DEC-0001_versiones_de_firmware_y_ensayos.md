---
estado: vigente
actualizado: 2026-09-30
resumen: El firmware se desarrolla a partir de una copia de la versión 2025/26 con Git, con número de versión en cada grabación y registro de ensayos que dice qué salió bien y qué salió mal.
---

# DEC-0001 · Control de versiones del firmware y registro de ensayos

Fecha: 2026-09-30.
Estado: aceptada por Armando el 30-09-2026.
Configuración: firmware del STM32 frontal y del ESP32 trasero de 2025/26, y sus versiones siguientes.

## Problema y alternativas

**Problema:**
- `KART_LOG.CSV` salió de una versión del STM32 que no se entregó. No se puede saber con qué código se obtuvieron esos datos.
- En 2025/26 el STM32 empezó un proyecto nuevo desde una plantilla. Las carpetas se llaman «-final», pero no hay historial de cambios.
- La propuesta 2026/27 se compromete a entregar código y configuración identificados, con datos de referencia y de comparación.

**Alternativas:**
1. Modificar los originales. Descartada: se pierde la referencia de lo entregado.
2. Empezar de cero. Descartada: se pierde lo que funciona (DIF y BLQ se probaron) y lo aprendido.
3. Copia de trabajo a partir de 2025/26, con historial. **Es la elegida.**

## Decisión y motivo

1. **Los originales no se tocan.** Los archivos de `02_ORIGINALES` no se modifican.
2. **Copia de trabajo** en `07_DESARROLLO/firmware/`, con una carpeta por controlador (`stm32_frontal/`, `esp32_trasero/`).
   - El primer commit es idéntico a lo entregado, comprobado por huella SHA-256: es la versión `v0`, «tal como se entregó».
3. **Cambios pequeños**, cada uno por un solo motivo y en su propio commit. Cada commit cita la mejora del [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]] o el problema (PROB) que resuelve.
4. **Sin copias `-final` ni `-v2`** y sin carpetas por versión: Git guarda las anteriores.
5. **Cada firmware que se grabe en el kart lleva un número de versión** (etiqueta de Git). El programa:
   - lo muestra al arrancar, por el puerto serie y en la pantalla;
   - lo escribe en la cabecera del registro de la SD;
   - si es posible, lo envía por CAN.
6. **Registro de versiones** en `07_DESARROLLO/firmware/VERSIONES.md`. Cada versión anota:
   - fecha y cambios;
   - si se grabó en el kart;
   - ensayos hechos y su resultado.
7. **Cada ensayo** se documenta con la [[09_PLANTILLAS/ENSAYO|plantilla de ensayo]] y se clasifica como «salió bien», «salió mal» o «no concluyente».
   - Las fichas van en `06_TRABAJO/ensayos/`.
   - Los datos van en `06_TRABAJO/ensayos/datos/`. Se enlazan desde la ficha, sin copiarlos dentro.

**Motivo:**
- Poder atribuir cada dato a un código concreto.
- Comparar la versión de referencia y la modificada en condiciones equivalentes.
- Dejar constancia de lo que funciona y de lo que no, que también sirve para el informe.

## Evidencias

- [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|Inventario del código]], sección 5: `KART_LOG.CSV` es anterior al código final.
- [[06_TRABAJO/INICIO_INVESTIGACION_2026-27|Propuesta 2026/27]]: referencia y comparación del diferencial, y medición de la autonomía.
- `AGENTS.md`: conservar los originales.

## Consecuencias y condiciones de revisión

- El primer paso de la fase de código es crear la copia `v0` y `VERSIONES.md`.
- Las librerías del ESP32 no se entregaron. Hay que fijar sus versiones y anotarlas en la copia de trabajo (inventario, mejora 22).
- Si los datos o los vídeos de los ensayos son demasiado grandes para Git, se guardan fuera con enlace, igual que los originales.
- Revisar esta decisión si los orientadores piden otro formato de entrega.

## Sustituye o sustituida por

Ninguna.
