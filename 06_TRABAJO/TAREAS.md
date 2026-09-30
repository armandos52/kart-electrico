---
estado: vigente
actualizado: 2026-09-30
resumen: Tareas próximas y completadas del proyecto del kart.
---

# Tareas

## Próximas

- [ ] Concretar la [propuesta 2026/27](INICIO_INVESTIGACION_2026-27.md): aportación del diferencial, protocolo de autonomía, ensayos previos a febrero, recursos, responsables y fechas; revisar con los orientadores.

- [ ] Definir y comprobar una copia de seguridad independiente.
- [ ] Completar L1: lectura integral del informe 2025/26 por secciones; ver [plan](PLAN_DE_LECTURA.md).
- [ ] Verificar que el montaje físico coincide con la configuración descrita en DOC-0042.
- [ ] Resolver las dudas documentales de [PROB-0001](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0001_versiones_documentales.md).
- [ ] Revisar planos y las cuatro capturas restantes de DOC-0015; analizar valores y fórmulas de los Excel cuando corresponda a L3.
- [ ] Abrir o exportar el archivo Loop desde su aplicación de origen para incorporarlo a la lectura.
- [ ] Leer las tablas en imagen de DOC-0042: 12–18 (CAN), 19–21 (BMS), 22–30 (cableado) y 31–32 (ensayos); ver [auditoría](AUDITORIA_2026-09-29.md).
- [ ] La próxima vez que el ESP32 no active el contactor al encender, hacer las pruebas 1–3 de [[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender|PROB-0002]] antes de sacarlo de la placa. Revisar después la PCB trasera: alimentación, GPIO12, IO35 y módulo CAN.
- [ ] Crear la copia de trabajo `v0` del firmware y `VERSIONES.md` según [[05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos|DEC-0001]].
- [ ] Incorporar la investigación de Armando sobre giroscopios y sensores de dirección, y definir los requisitos de sensores antes de la nueva PCB; ver [ideas](IDEAS.md).
- [ ] Analizar el código siguiendo el orden del [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]] (sección 9): CAN y vigilancia de fallos, temporización, diferencial, registro en SD y estructura.
- [ ] Averiguar qué versión de firmware generó KART_LOG.CSV y a qué ensayo corresponde. Se sabe que fue una versión del STM32 anterior al 10-07-2026; el archivo es del 04-07-2026.
- [ ] Comprobar si el `.elf` de CubeIDE del 10-07-2026 corresponde al `main.c` entregado y si es lo que está grabado en la placa.
- [ ] Contrastar en el kart las hipótesis de la auditoría (contactor/ESP32, desequilibrio de celdas, SOC a 0 %) antes de proponer soluciones.
- [ ] Preparar la publicación de la recepción: notas y catálogo por Git, almacenamiento apropiado para originales grandes y enlaces accesibles a los profesores.

## Completadas

- [x] Inventariar el código por sistema y versión, con su estructura y las mejoras candidatas (2026-09-30); ver [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]].

- [x] Crear la [propuesta Word V2.0](../07_DESARROLLO/propuesta_2026-27/Proposta_Smart_Electric_Kart_V2_2026-27.docx) a partir del análisis del cronograma y comprobar visualmente sus cuatro páginas (2026-09-30). Selección de función, criterios y fechas pendientes de acordar con orientadores.

- [x] Incorporar el cronograma personal desde Descargas, verificar copia por SHA-256, catalogar DOC-0043 y analizar su encaje con la base documental (2026-09-30); ver [análisis](INICIO_INVESTIGACION_2026-27.md). Word sin modificar; revisión visual pendiente.

- [x] Abrir Obsidian y preparar una [guía de navegación y formatos](../00_INICIO/REPASO_EN_OBSIDIAN.md) con [lista personal de repaso](MI_REPASO.md) de los 42 documentos (2026-09-30).
- [x] Auditar el contenido extraíble: código, Excel, capturas, presentación y evolución por curso (2026-09-29); ver [auditoría](AUDITORIA_2026-09-29.md).

- [x] Inventariar la entrega completa, conservar originales, verificar tres ZIP y extraer lo que faltaba (2026-09-28).
- [x] Crear catálogo de 42 documentos, fichas por lote y buscador local por página (2026-09-28).
- [x] Revisar una muestra y guardar síntesis inicial del informe 2025/26 y dudas de versiones (2026-09-28).
- [x] Publicar la base documental en [GitHub](https://github.com/armandos52/kart-electrico), habilitar Issues y Discussions y añadir una plantilla de revisión académica (2026-09-28).
- [x] Preparar estructura, mapa del proyecto y plantillas iniciales (2026-09-28).
- [x] Instalar Obsidian 1.13.7 y verificar su registro en Windows (2026-09-28).

Para tareas técnicas extensas, crear una ficha con la [plantilla](../09_PLANTILLAS/TAREA.md) y enlazarla aquí.
