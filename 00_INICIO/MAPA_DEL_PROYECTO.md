---
estado: vigente
actualizado: 2026-10-23
resumen: Mapa de la bóveda del kart: por dónde empezar y dónde guardar cada cosa.
---

# Kart eléctrico · Inicio

Esta base reúne el conocimiento y el trabajo del kart. La entrega histórica está catalogada: 42 documentos y 1.007 páginas PDF; con la propuesta personal y el conjunto de cuatro fotos del 08-10 son 44 entradas. La lectura técnica completa se hará por lotes.

## Para empezar

- [Frontal dev4 en la F746 real](../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md): respaldo e identificación del firmware previo, dev4 grabado y probado en placa (pantalla, táctil, START, CAN, SD, franja). Correcciones de software en [ENS-0018](../06_TRABAJO/ensayos/ENS-0018_mejoras_pantalla_desarrollo.md). Pantallas de usuario final pendientes para el cierre según [DEC-0010](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md).

- [Banco Nucleo F411 + pantalla MCUFRIEND](../07_DESARROLLO/firmware/nucleo_f411_banco/README.md): preparación/respaldo en ENS-0015; diez vistas, navegación B1 y capturas del LCD en [ENS-0016](../06_TRABAJO/ensayos/ENS-0016_vistas_frontal_nucleo.md). No sustituye al frontal F746 del kart.

- [Dossier para profesores](REVISION_PARA_PROFESORES.md): avances al 13-10, evidencias, límites y preguntas de revisión.
- [Índice de ensayos](../06_TRABAJO/ensayos/README.md) y [guía de desarrollo](../07_DESARROLLO/README.md): datos y reproducción en PC.

- [Panel del portátil · Wi-Fi/USB y modelo 3D](../07_DESARROLLO/panel_local/README.md): lectura, kart basado en fotos, calibración visual por pasos ([ENS-0012](../06_TRABAJO/ensayos/ENS-0012_calibracion_por_pasos.md)), consola y gráficas. [Modelo y pruebas](../06_TRABAJO/ensayos/ENS-0006_modelo_visual_fotografias.md); [velocidad visual de ruedas](../06_TRABAJO/ensayos/ENS-0010_velocidad_visual_ruedas.md); cotas y validación física pendientes. USB solo con módulo fuera del kart; iPad aplazado.
- [Aprender el kart: historia, funcionamiento y mejoras](../06_TRABAJO/GUIA_DE_ESTUDIO_DEL_KART.md): guía personal del 07-10, con explicaciones, fuentes, ejercicios y preguntas del montaje.
- [Guía de PID e IMU](../04_CONOCIMIENTO/software_y_telemetria/GUIA_PID_E_IMU.md) (vigente, 23-10): PID, anti-windup, sintonía, filtros de orientación y lectura del ESC v1.3 como control P; con [simulador del kart en vista superior](../04_CONOCIMIENTO/software_y_telemetria/simulador_pid_imu.html) (modelo didáctico, valores supuestos).

- [Inicio de tu investigación 2026/27](../06_TRABAJO/INICIO_INVESTIGACION_2026-27.md): cronograma personal incorporado, análisis y organización recomendada.

- [Repasar en Obsidian](REPASO_EN_OBSIDIAN.md): cómo abrir fichas, resúmenes y originales, reconocer formatos y registrar tu repaso.
- [Índice de los 42 documentos](../03_CATALOGO/INDICE_DOCUMENTOS.md): acceso a originales, fichas y cobertura de lectura.
- [Datos, código y diseños](../03_CATALOGO/DATOS_Y_PROYECTOS.md): entradas a los proyectos completos y al registro de pruebas.
- [Inventario del código](../04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md) (vigente, 30-09): sistemas y últimas versiones (STM32 y ESP32 2025/26), estructura, evolución desde 2023/24 y 24 mejoras candidatas.
- [Resultado de la recepción](../03_CATALOGO/RECEPCION_2026-09-28.md): inventario, descompresión y pendientes de extracción.
- [Plan de lectura por lotes](../06_TRABAJO/PLAN_DE_LECTURA.md): orden y criterio de finalización.
- [Síntesis inicial de 2025/26](../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md): arquitectura y pendientes según el último informe identificado.
- [Estado actual](ESTADO_ACTUAL.md): lo conocido, lo pendiente y el siguiente paso.
- [Tareas](../06_TRABAJO/TAREAS.md): trabajo concreto y criterios de finalización.
- [Ideas](../06_TRABAJO/IDEAS.md): propuestas que aún no son decisiones.
- [Cronología](CRONOLOGIA.md): evolución documentada del proyecto.
- [Guía de uso](GUIA_DE_USO.md): cómo incorporar y consultar información.
- [Revisión con profesores](../CONTRIBUTING.md): cómo dejar observaciones y proponer mejoras en GitHub.

## Dónde guardar cada cosa

| Carpeta | Contenido |
|---|---|
| `01_ENTRADA` | Material recibido pendiente de inventario. |
| `02_ORIGINALES` | Copias conservadas de las fuentes, con sus carpetas y dependencias. |
| `03_CATALOGO` | Índice de documentos y reglas de registro. |
| `04_CONOCIMIENTO` | Resúmenes por sistema, enlazados a las fuentes. |
| `05_DECISIONES_Y_PROBLEMAS` | Decisiones razonadas e incidencias con evidencias. |
| `06_TRABAJO` | Tareas, ideas y notas breves de sesiones. |
| `07_DESARROLLO` | Diseños, cálculos y código en elaboración. |
| `08_INDICE_GENERADO` | Texto extraído, OCR y otros derivados regenerables. |
| `09_PLANTILLAS` | Modelos de fichas reutilizables. |

Los proyectos originales conservan su estructura interna. El catálogo organiza el acceso por tema y lote sin separar dependencias. La configuración descrita en julio de 2026 está identificada documentalmente; su correspondencia con el montaje físico sigue pendiente.
