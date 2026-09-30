# Recepción y organización de los archivos

## Resultado

La entrega está conservada en `02_ORIGINALES/entrega_2026-09-28`, manteniendo nombres, estructura de los proyectos y archivos comprimidos. Las tres entradas trasladadas desde la raíz fueron la carpeta `Relatórios de anos anteriores`, el ZIP `Smart Eletric Kart Cross-2223406-2223403.zip` y `Loop paragraph.loop`.

Se registró una huella SHA-256 de los **1.093 archivos recibidos** antes del traslado y se verificó de nuevo después. No se borraron duplicados ni se ejecutaron programas de los proyectos.

## Descompresión

| Paquete | Archivos internos | Extraídos ahora | Ya existentes y verificados |
|---|---:|---:|---:|
| Smart Eletric Kart Cross-2223406-2223403.zip | 1.691 | 1.691 | 0 |
| PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos.zip | 598 | 0 | 598 |
| ProjetoKart2324.zip | 484 | 0 | 484 |

Los tres ZIP se leyeron y sus miembros se comprobaron por contenido. No se encontraron otros ZIP anidados ni archivos RAR, 7z, TAR o GZ. Los documentos Office conservan su estructura interna; no se confunden con paquetes de entrega ZIP.

## Inventario final

- **2.784 archivos**, con un tamaño lógico total de **1.464.292.384 bytes** (aproximadamente 1,46 GB), incluyendo ZIP y sus extracciones.
- **2.361 contenidos binarios únicos** y **423 copias idénticas adicionales** (aproximadamente 16,4 MB). Se conservan porque algunas forman parte de proyectos independientes.
- **42 documentos**: 34 PDF, 3 Word, 3 Excel, 1 PowerPoint y 1 Loop.
- Además, un registro de pruebas `KART_LOG.CSV` con 801 filas de datos y 13 columnas, separado del catálogo documental; ver [datos y proyectos](DATOS_Y_PROYECTOS.md).
- Los PDF suman **1.007 páginas**. Hay **9 informes PDF** que suman **772 páginas**, incluidas dos versiones distintas de 2023/24.
- La presentación contiene **17 diapositivas**; se ha extraído su texto, no se han revisado todos sus objetos.
- Se identificaron **270 archivos de metadatos de macOS**. No deben interpretarse como documentos ni procesarse con OCR.

El tamaño cuenta los ZIP además de sus archivos descomprimidos. Las copias duplicadas se detectan por contenido binario, no por similitud de texto. Dos planos sin texto no son duplicados por ese motivo.

## Qué se ha procesado

1. Inventario y huellas de todos los archivos: completo.
2. Verificación de los tres ZIP y descompresión necesaria: completa.
3. Extracción de texto de los 34 PDF, dos informes Word y la presentación: realizada; no equivale a lectura técnica integral.
4. Word `SetupControladores2324`: cinco capturas, sin texto sustantivo. Recursos gráficos extraídos; primera imagen revisada, otras cuatro pendientes.
5. Excel: nombres de hojas inventariados; fórmulas, mediciones y gráficos pendientes de análisis.
6. Loop: conservado como formato binario; necesita apertura/exportación desde la aplicación de origen.
7. Lectura técnica: parcial, con alcance registrado en cada ficha. Ningún informe se marca como completamente revisado.

## Calidad de la extracción

Hay 49 páginas PDF con menos de 80 caracteres de texto extraído. Incluyen páginas vacías, portadas gráficas y planos; no son 49 fallos de OCR. Nueve PDF de una página son planos o capas de PCB que requieren inspección visual. El informe de 2008/09 tiene texto fragmentado, y la ficha de batería mezcla columnas e idiomas. Se revisaron visualmente cinco páginas seleccionadas de PDF para comprobar fechas y conclusiones, además de una captura del Word de configuración.

No se ha aplicado OCR masivo: se decidirá por página cuando falte texto necesario. Las capturas de parámetros y los diagramas necesitan comprobación visual incluso con OCR.

## Acceso y reutilización

- [Índice de documentos y fichas](INDICE_DOCUMENTOS.md).
- [Plan de lectura por lotes](../06_TRABAJO/PLAN_DE_LECTURA.md).
- [Síntesis inicial del informe 2025/26](../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md).
- [Diferencias y dudas entre versiones](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0001_versiones_documentales.md).
- `inventario_archivos.csv`: rutas, tipos, tamaños, huellas y duplicados de todos los archivos.
- `documentos.csv`: catálogo documental, extracción y cobertura de lectura.
- `duplicados_exactos.csv`: copias adicionales; no es una lista de archivos autorizados para borrar.

Los originales y derivados permanecen locales en esta etapa. No se ha realizado commit ni push de esta recepción. La entrega contiene archivos grandes, entre ellos el ZIP principal y el PowerPoint; la publicación del archivo bruto necesita una estrategia de almacenamiento distinta de subir indiscriminadamente toda la carpeta. Las notas y fichas pueden versionarse por separado. Los enlaces a fuentes locales funcionarán en esta bóveda; en GitHub necesitarán que la fuente esté publicada o un enlace externo estable.

La copia en el mismo disco no sustituye una copia de seguridad independiente, todavía pendiente.
