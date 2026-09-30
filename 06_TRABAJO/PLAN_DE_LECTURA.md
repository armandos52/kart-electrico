# Plan de lectura por lotes

Fecha de preparación: 2026-09-28. El inventario y la extracción inicial están completados; la síntesis exhaustiva sigue pendiente.

## Orden de trabajo

| Lote | Material | Resultado que debe quedar guardado | Estado |
|---|---|---|---|
| L0 | Toda la entrega | Inventario, ZIP verificados, catálogo, texto por página y buscador | Completado |
| L1 | Informe 2025/26 DOC-0042; Word DOC-0041 y presentación DOC-0028 como contraste | Arquitectura documentada, componentes, resultados y pendientes con fuentes | Síntesis inicial; falta lectura integral |
| L2 | Batería, BMS, contactor, fuentes y componentes asociados | Fichas de componentes con revisión, unidades, condiciones y correspondencia con el montaje | Clasificación inicial |
| L3 | Controladores, sensores, planos, capturas y hojas de cálculo | Parámetros y esquemas vinculados a la versión del kart y a sus pruebas | Clasificación inicial |
| L4 | Código STM32/ESP32/Arduino, manuales de placas y datos de tarjeta SD | Mapa del software propio, protocolo CAN, entradas/salidas y relación con los informes | Archivos inventariados; código sin auditar |
| L5 | Informes 24/25, 23/24, 18/19, 15/16, 13/14, 10/11 y 08/09 | Evolución por configuración, cambios y problemas resueltos o heredados | Lectura inicial de portadas y secciones seleccionadas |

Las fichas del [índice](../03_CATALOGO/INDICE_DOCUMENTOS.md) asignan cada documento a su lote principal. Un documento puede servir de fuente a varios sistemas sin copiarlo ni volver a extraerlo. El archivo Loop queda pendiente de apertura/exportación.

## Lote siguiente: L1

Leer DOC-0042 por secciones coherentes de aproximadamente 15–25 páginas, ajustando los cortes para conservar tablas y explicaciones juntas:

1. Contexto, objetivos y unidad frontal.
2. Unidad trasera y diferencial electrónico.
3. CAN, batería y BMS.
4. Alimentación, contactor, carga y cableado.
5. Pruebas, conclusiones y pendientes.

Al cerrar cada sección, guardar una síntesis breve con páginas exactas, configuración, hechos, hipótesis y preguntas. Comparar el Word y la presentación cuando aporten diferencias o información adicional. No releer automáticamente las tres versiones completas.

La primera pasada ya cubre la portada, el resumen, el índice y las conclusiones del PDF. Los apartados técnicos intermedios todavía requieren análisis. La cobertura precisa consta en la ficha DOC-0042.

## Regla de economía de contexto

1. Leer `ESTADO_ACTUAL.md` y la tarea.
2. Buscar el tema en el índice y en el buscador local.
3. Recuperar las páginas pertinentes; ampliar a la sección completa si hace falta contexto.
4. Verificar tablas, figuras, parámetros y conclusiones relevantes en el original.
5. Guardar resultados y registrar cobertura antes de cambiar de lote.

No cargar bibliotecas, objetos de compilación ni archivos de macOS para resumir informes. No volver a extraer un documento cuyo contenido no cambió. No generar una traducción completa de todos los documentos: redactar las notas en español y conservar referencias a los originales en portugués o inglés.

## Criterio para dar un lote por terminado

La extracción automática no basta. Deben estar leídas sus secciones sustantivas, contrastados los elementos visuales relevantes, registrados los límites y enlazadas las conclusiones a las fuentes. No se fija un presupuesto arbitrario que pueda omitir evidencia: el ahorro viene de evitar repeticiones y recuperar solo lo necesario.
