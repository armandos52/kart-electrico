# Catálogo

`documentos.csv` contiene 43 entradas: 42 de la recepción inicial y DOC-0043, propuesta personal recibida el 30-09-2026. Se usa CSV UTF-8 con BOM, coma como separador y comillas para campos que contengan comas o saltos de línea. La navegación habitual está en [INDICE_DOCUMENTOS.md](INDICE_DOCUMENTOS.md); no es necesario cargar el CSV completo en cada sesión.

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

Se añadieron los campos `formato`, `paginas`, `paginas_poco_texto`, `estado_extraccion`, `estado_lectura`, `ruta_texto`, `copias`, `rutas_alternativas`, `hojas`, `lote`, `tipo` y `cobertura_lectura`. Mantener separadas la vigencia técnica, la extracción automática y la lectura humana/modelo. `extraido_no_revisado` describe que la extracción no certifica el contenido; `estado_lectura` y `cobertura_lectura` registran la revisión posterior.

`inventario_archivos.csv` incluye todos los archivos, también código, dependencias, binarios y metadatos. `duplicados_exactos.csv` registra coincidencias de SHA-256; no autoriza su borrado.
