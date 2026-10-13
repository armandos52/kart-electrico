# Herramientas de consulta documental

Los programas usan Python y archivos locales. No llaman a un modelo, no generan embeddings y no ejecutan programas recibidos. Sus salidas breves permiten consultar el archivo sin añadir su contenido completo a cada conversación.

## Consulta habitual

Usar el Python del entorno con los paquetes `pypdf` y `python-docx`. En este equipo se ha usado el runtime de Codex. Desde la raíz de la bóveda:

```text
python 07_DESARROLLO/herramientas/buscar_documentos.py --consulta "contator AND vibrações" --documento DOC-0042 --limite 3
python 07_DESARROLLO/herramientas/buscar_documentos.py --documento DOC-0042 --pagina 97
```

Los números con coma o punto deben ir entre comillas dobles dentro de la consulta: `--consulta '"52,5"'`. La búsqueda devuelve documento, página y extracto. Usar términos del original: por ejemplo `contator`, `travão`, `bateria`, `CAN`. Admite consultas FTS5 con AND/OR y no distingue acentos. No es búsqueda semántica: los sinónimos o traducciones requieren consultas adicionales.

## Actualización incremental

```text
python 07_DESARROLLO/herramientas/inventariar.py inventory
python 07_DESARROLLO/herramientas/inventariar.py documents
python 07_DESARROLLO/herramientas/buscar_documentos.py --indexar
```

`inventariar.py` tiene configurada la carpeta de esta entrega. Para otra entrega, ampliar el alcance antes de ejecutarlo. La primera pasada calcula todas las huellas; las siguientes reutilizan la huella si tamaño y fecha de modificación no cambiaron. Los textos se cachean por SHA-256 y versión del extractor. Para una auditoría que no confíe en los metadatos hay que forzar una nueva comprobación de contenido.

Si cambia una fuente, tendrá una huella nueva y debe revisarse la vigencia de sus resúmenes. No marcar como revisadas páginas solo porque existe texto extraído. Los libros Excel se inventarían por hojas y no se recalculan.

`preparar_catalogo.py` es el generador de las fichas de **esta recepción inicial**, con clasificación editorial de DOC-0001 a DOC-0042. No ejecutarlo indiscriminadamente después de nuevas revisiones: sus descripciones y coberturas iniciales podrían reemplazar curación posterior. Las síntesis técnicas viven en `04_CONOCIMIENTO`.

## Recepción y ZIP

Las acciones `baseline`, `verify` y `unzip` se usaron para esta recepción. `baseline` espera las tres entradas en la raíz y no debe repetirse después de trasladarlas. `verify` comprueba sus huellas en la ubicación final. `unzip` preserva archivos, comprueba rutas y hashes, y reutiliza extracciones existentes cuando coinciden. Si hay diferencias, extrae una copia aparte.

El registro de descompresión y la línea base de recepción están en `08_INDICE_GENERADO`. Los originales, ZIP y catálogo deben conservarse aunque se regenere el índice. Los archivos derivados no sustituyen a las fuentes ni a una copia de seguridad.
