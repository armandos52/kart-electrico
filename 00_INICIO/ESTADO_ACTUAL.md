---
estado: vigente
actualizado: 2026-09-30
resumen: Lo conocido, lo pendiente y el siguiente paso del proyecto; traspaso entre sesiones.
---

# Estado actual

Última actualización: 2026-09-30.

## Conocido por el usuario

- Proyecto de kart eléctrico con historia desde 2008 hasta la actualidad.
- Se ha recibido documentación histórica, fichas técnicas, planos, hojas de cálculo y proyectos de software.
- El usuario ha aprobado instalar y utilizar Obsidian.

## Situación documental

- Generada [propuesta Word V2.0](../07_DESARROLLO/propuesta_2026-27/Proposta_Smart_Electric_Kart_V2_2026-27.docx) en portugués, con objetivos, metodología, cronograma, entregables y referencias. Cuatro páginas revisadas visualmente mediante exportación de Word y renderizado PDF; pendiente de revisión académica. Conserva DOC-0043 como fuente sin modificar.

- Incorporada la propuesta personal de inicio 2026/27: [DOC-0043](../03_CATALOGO/fichas/DOC-0043.md), cronograma del 30-09-2026 copiado desde Descargas y verificado por SHA-256. Son 43 documentos catalogados: 42 históricos y esta propuesta. [Análisis](../06_TRABAJO/INICIO_INVESTIGACION_2026-27.md): concretar mejora del diferencial, adelantar ensayos para autonomía en febrero y documentar desde octubre; sin aprobación académica acreditada ni revisión visual del Word.

- Estructura inicial y plantillas preparadas.
- Obsidian 1.13.7 instalado; existen ajustes locales de la bóveda en `.obsidian`.
- Guía de [repaso en Obsidian](REPASO_EN_OBSIDIAN.md) y [lista personal de 42 documentos](../06_TRABAJO/MI_REPASO.md) preparadas el 30-09; no amplían la cobertura técnica de lectura.
- Entrega conservada en `02_ORIGINALES/entrega_2026-09-28`: 2.784 archivos tras completar la descompresión. Los 1.093 archivos inicialmente recibidos se verificaron por SHA-256 tras trasladarlos.
- Tres ZIP verificados: 1.691 archivos extraídos y 1.082 archivos ya descomprimidos reutilizados. No se borraron duplicados.
- Catálogo: 42 documentos, incluidos 34 PDF con 1.007 páginas. Texto extraído y buscador local disponibles; los Excel solo tienen inventario de hojas y el Loop sigue pendiente.
- Configuración documental de referencia inicial: informe de julio de 2026 (DOC-0042), con STM32 + ESP32 y CAN. Montaje físico y parámetros todavía por verificar.
- Síntesis inicial de resumen y conclusiones guardada; lectura integral de los informes pendiente. Word de configuración y planos requieren lectura visual.
- Auditoría del 2026-09-29: se revisaron el código de 2023–2026, los Excel, las capturas Kelly, la presentación y el texto de los 9 informes. Hay tabla de evolución por curso, protocolo CAN, calibraciones y tres hipótesis de diagnóstico; ver [auditoría](../06_TRABAJO/AUDITORIA_2026-09-29.md). No sustituye la lectura integral por lotes. Los datos sin marca [V] están pendientes de comprobar.
- Inventario del código (30-09): [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]]. Hay tres programas propios. La última versión del STM32 es del 10-07-2026 (con su `.elf` compilado el mismo día) y la del ESP32, del 21-07-2026. Anteriores: STM32 2024/25 y Arduino 2023/24. No hay código de 2008–2019. Se identifican 23 mejoras candidatas; nada se ha compilado.
- KART_LOG.CSV (04-07-2026) lo escribió una versión anterior del STM32, no el ESP32: el firmware entregado usa las mismas columnas, pero con comas y cada 200 ms.
- Copia de seguridad: pendiente de definir y configurar.
- Repositorio público: [armandos52/kart-electrico](https://github.com/armandos52/kart-electrico), con Issues y Discussions habilitados.
- Publicación mediante commits y push; no hay sincronización automática con Obsidian. Los ajustes locales y los índices generados quedan fuera de Git.
- Esta recepción y sus notas nuevas están guardadas localmente, sin commit ni push. Los originales de la entrega se excluyen temporalmente de Git por su volumen y pendiente de preparar su publicación.

## Próximo paso

Analizar el código en el orden del [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]] (sección 9): primero el protocolo CAN y la vigilancia de fallos entre STM32 y ESP32.

En paralelo, continuar con L1 del [plan de lectura](../06_TRABAJO/PLAN_DE_LECTURA.md): analizar el informe DOC-0042 por secciones y vincular sus resultados a las fuentes. Ya se revisaron portada, resumen, índice y conclusiones; no repetir esa lectura sin necesidad.

Entradas principales: [índice documental](../03_CATALOGO/INDICE_DOCUMENTOS.md), [síntesis 2025/26](../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md), [balance de recepción](../03_CATALOGO/RECEPCION_2026-09-28.md).

## Preguntas pendientes

- Aclarar versiones de 2023/24, fecha contradictoria del informe 24/25 y distintos relatos sobre el inicio; ver [PROB-0001](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0001_versiones_documentales.md).
- La propuesta fija diferencial electrónico inteligente y estimación de autonomía hasta febrero de 2027; falta concretar la aportación propia, condiciones y criterios de aceptación con los orientadores.
- ¿Qué documentación describe el montaje realmente existente?

Ver [tareas](../06_TRABAJO/TAREAS.md).
