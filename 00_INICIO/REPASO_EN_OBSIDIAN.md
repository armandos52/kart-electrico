# Repasar el kart en Obsidian

## Empieza por aquí

1. Abre el [índice de los 42 documentos](../03_CATALOGO/INDICE_DOCUMENTOS.md).
2. Pulsa un enlace **DOC-…**: abre su ficha, con una descripción breve, formato, procedencia y alcance de lectura.
3. Dentro de la ficha, pulsa **abrir archivo** para consultar el original.
4. Vuelve a esta página y usa la [lista de repaso personal](../06_TRABAJO/MI_REPASO.md) para apuntar lo que entiendes y tus dudas.

**Ejemplo:** [ficha DOC-0042 · informe 2025/26](../03_CATALOGO/fichas/DOC-0042.md) → original PDF. Su [síntesis inicial en español](../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md) está en una nota separada.

## Dónde están los resúmenes

- [Síntesis inicial de 2025/26](../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md): visión conjunta y pendientes.
- [Auditoría del 29 de septiembre](../06_TRABAJO/AUDITORIA_2026-09-29.md): evolución por curso, código y datos. Los datos sin **[V]** están pendientes de comprobar según la propia nota.
- [Fichas individuales](../03_CATALOGO/INDICE_DOCUMENTOS.md): orientación breve de cada documento. **No son 42 resúmenes exhaustivos.**

El catálogo inicial y algunas notas conservan estados anteriores a la auditoría. Consulta también su alcance; no interpretes «extraído» como «leído por completo» ni una revisión personal como validación técnica.

## Qué tipos de archivos tienes

| Grupo | Formatos presentes | Para qué sirven / dónde entrar |
|---|---|---|
| Informes, manuales y fichas técnicas | PDF; Word `.docx` | [Índice documental](../03_CATALOGO/INDICE_DOCUMENTOS.md). El catálogo contiene 34 PDF y 3 Word. |
| Tablas y mediciones | Excel `.xlsx`; datos `.csv` | 3 Excel catalogados y el registro `KART_LOG.CSV`. [Datos y proyectos](../03_CATALOGO/DATOS_Y_PROYECTOS.md). |
| Presentación | PowerPoint `.pptx` | [DOC-0028](../03_CATALOGO/fichas/DOC-0028.md). |
| Código | `.ino`, `.c`, `.h`, `.cpp`, `.hpp` | Arduino/ESP32 y STM32, con bibliotecas incluidas. [Entradas al código](../03_CATALOGO/DATOS_Y_PROYECTOS.md). |
| Configuración de placas y pantalla | `.ioc`, `.touchgfx` | Configuración STM32 y proyecto de interfaz TouchGFX; se consultan con sus herramientas de desarrollo. |
| Planos y piezas | PDF, `.SLDPRT`, `.SLDDRW` | Piezas y dibujos de SolidWorks; los PDF de planos están en el lote L3 del índice. |
| Electrónica y placas PCB | `.pdsprj` y PDF | Proyecto Proteus y planos de las capas de las placas. [Proyectos](../03_CATALOGO/DATOS_Y_PROYECTOS.md) y lote L3. |
| Imágenes | `.png`, `.jpg` | Capturas, fotografías y recursos de los proyectos; su significado depende de la carpeta de procedencia. |
| Notas y mapas de Obsidian | `.md`, `.canvas` | Notas enlazadas y lienzos de la bóveda. El [mapa del proyecto](MAPA_DEL_PROYECTO.md) es una página de navegación. |
| Archivos de compilación | `.o`, `.elf`, `.hex`, `.map`, entre otros | Productos auxiliares del software. Un `.map` de compilación describe la distribución de memoria; no es un mapa del kart. |
| Paquetes y formato pendiente | `.zip`, `.loop` | ZIP conservados junto a sus extracciones; [Loop](../03_CATALOGO/fichas/DOC-0001.md) pendiente de abrir/exportar. |

Los **42 documentos** son las entradas del catálogo, no todos los archivos: el inventario de recepción registra **2.784 archivos**, incluyendo bibliotecas, compilaciones, ZIP y copias. [Fuente y detalle](../03_CATALOGO/RECEPCION_2026-09-28.md).

## Cómo moverte

- **Ctrl+O**: abre una nota por nombre. Prueba `DOC-0042`, `INDICE_DOCUMENTOS` o `MI_REPASO`.
- **Ctrl+E**: alterna lectura y edición. En lectura, pulsa los enlaces directamente; en edición puedes usar **Ctrl+clic**.
- **Ctrl+Shift+F**: busca palabras en las notas, por ejemplo `batería` o `CAN`. No sustituye la búsqueda del contenido de todos los PDF y proyectos.
- La flecha **atrás** de la parte superior te devuelve a la nota anterior.
- El icono **Vista gráfica** de la barra izquierda muestra conexiones entre notas; los planos técnicos están en el catálogo.

Obsidian permite leer notas y PDF. Word, Excel, PowerPoint, código y CAD pueden necesitar una aplicación externa asociada al formato. Si un enlace no abre un formato, localiza su ruta desde la ficha o desde Datos y proyectos y ábrelo con su aplicación.

## Un repaso que deje rastro

Empieza por DOC-0042 y la presentación DOC-0028, como propone el [plan de lectura](../06_TRABAJO/PLAN_DE_LECTURA.md). Para cada documento anota: **qué contiene**, **a qué año/configuración corresponde**, **qué páginas has revisado** y **qué dudas quedan**. Puedes leer después por tema o por curso.

Las casillas de tu lista son un seguimiento personal. La cobertura técnica del catálogo se actualiza por separado, con páginas y evidencias.
