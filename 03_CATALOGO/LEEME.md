# Catálogo

`documentos.csv` contiene solo la cabecera hasta que se incorporen fuentes reales. Se usa CSV UTF-8 con coma como separador y comillas para campos que contengan comas o saltos de línea.

- `id`: identificador estable, por ejemplo DOC-0001.
- `ruta_original`: ruta relativa a la raíz de la bóveda.
- `titulo`, `fecha_documento`, `sistema`, `configuracion`: descripción y alcance; usar «desconocido» cuando corresponda.
- `estado`: pendiente_verificar, vigente, historico o sustituido. La vigencia requiere evidencia.
- `resumen`: orientación breve, sin reemplazar la fuente.
- `referencia_precisa`: páginas, hojas o secciones pertinentes.
- `sha256`: huella para detectar archivos idénticos y cambios, no similitud conceptual.
- `fecha_indexacion`, `version_extractor`: trazabilidad del procesamiento.
- `sustituido_por`, `duplicado_de`: identificadores relacionados, cuando estén comprobados.

Las notas derivadas deben citar el ID y enlazar al original. No abrir el catálogo completo en cada tarea cuando haya crecido: buscar o filtrar por sistema, configuración y estado.
