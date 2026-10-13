# Plan de lectura por lotes

Fecha de preparación: 2026-09-28. El inventario y la extracción inicial están completados; la síntesis exhaustiva sigue pendiente.

## Orden de trabajo

| Lote | Material | Resultado que debe quedar guardado | Estado |
|---|---|---|---|
| L0 | Toda la entrega | Inventario, ZIP verificados, catálogo, texto por página y buscador | Completado |
| L1 | Informe 2025/26 DOC-0042; Word DOC-0041 y presentación DOC-0028 como contraste | Arquitectura documentada, componentes, resultados y pendientes con fuentes | Síntesis inicial; falta lectura integral |
| L2 | Batería, BMS, contactor, fuentes y componentes asociados | Fichas de componentes con revisión, unidades, condiciones y correspondencia con el montaje | Clasificación inicial |
| L3 | Controladores, sensores, planos, capturas y hojas de cálculo | Parámetros y esquemas vinculados a la versión del kart y a sus pruebas | Clasificación inicial |
| L4 | Código STM32/ESP32/Arduino, manuales de placas y datos de tarjeta SD | Mapa del software propio, protocolo CAN, entradas/salidas y relación con los informes | Auditoría 29-09 e inventario 30-09 disponibles; firmware real, compilación y lectura integral pendientes |
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

## Consulta de estudio del 07-10-2026

Se preparó la [guía personal](GUIA_DE_ESTUDIO_DEL_KART.md). Cobertura exacta en la [sesión](SESION_ESTUDIO_2026-10-07.md): texto seleccionado de L1 (pantalla, alimentación, contactor y fallos), L3 (manual Kelly) y conclusiones/resúmenes de L5. No se cierra ningún lote; no hubo nueva revisión visual de figuras/tablas. Próximo: respuestas del montaje y continuar las secciones coherentes de L1, sin repetir estos extractos salvo necesidad de contexto.

## Regla de economía de contexto

Batería (13-10): índice, fichas y notas; buscador DOC-0030 PDF 1/DOC-0042 PDF 69. Revisión visual completa de ambas páginas (tabla, §3.4/figura 17). [Tutorial y cobertura](../04_CONOCIMIENTO/bateria_y_bms/TUTORIAL_CELDAS_Y_CARGA_16S.md). Discrepancia 60/70 Ah; ficha de una página revisada pero correspondencia física pendiente. No se cierran L1/L2 ni se revisa el resto/Loop.

START físico momentáneo (13-10): índice/ficha DOC-0042, inventario/notas de software, ENS-0011 y código de trabajo STM32/ESP32; [ENS-0013](ensayos/ENS-0013_start_stop_momentaneo.md). Defecto de interpretación de nivel reproducido y corregido en copia frontal, pruebas sintéticas y compilación. Sin nuevas páginas originales/lectura visual ni cierre L1/L4. «B1.3» y síntoma son declaraciones del usuario; no identificación ni medición física de esta sesión.

Calibración por pasos (11-10): consulta focalizada de índice/ficha DOC-0042, notas PROB-0007/ENS-0005, panel de trabajo y seis registros Wi-Fi del 09-10. [ENS-0012](ensayos/ENS-0012_calibracion_por_pasos.md). Sin nuevos originales ni ampliación de lectura integral L1/L4. Lecturas históricas analizadas, ensayos nuevos solo sintéticos; sin conexión física.

Retoma de control (11-10): [análisis y cobertura](RETOMA_PROGRAMA_2026-10-11.md), [ENS-0011](ensayos/ENS-0011_stop_en_lote_can.md). Revisión focalizada de notas/código de trabajo y corrección STOP en lote CAN; sin nuevas páginas de originales ni mediciones físicas. L1/L4 no se cierran; ESP32 físico v1.2 declarado, frontal instalado aún sin identificar.

Revisión v1.2/modos (08-10): [cobertura exacta y propuesta](REVISION_CODIGO_V12_2026-10-08.md), [PROB-0008](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0008_robustez_control_modos_y_pantalla.md). ESP32 de trabajo y telemetría completos, funciones propias focalizadas STM32, vista frontal, Model/Presenter y enlace del botón generado; 19 casos de software con periféricos simulados. DOC-0042 texto PDF 32,57,96 recuperado con buscador; sin nueva verificación visual de figura/ecuaciones, sin identificación del frontal instalado ni ensayo físico. L1/L4 no se cierran. Esta revisión actualiza la vigencia de notas de código del 30-09.

Corrección de lados/rodadura (08-10): consulta focalizada de código visual, notas e índice/ficha DOC-0044; referencias fotográficas ya revisadas. [ENS-0008](ensayos/ENS-0008_lados_y_sentido_ruedas_visual.md). Sin nuevos originales ni cobertura adicional de L1/L4; pruebas de software, no medidas físicas.

Esquema superior (08-10): cuarta fotografía nueva de [DOC-0044](../03_CATALOGO/fichas/DOC-0044.md) revisada completa; ficha/resumen visual actualizados con cubierta en T y cajas observables. Consulta focalizada del panel, sin nuevos originales históricos. [ENS-0007](ensayos/ENS-0007_esquema_superior_fotografia.md). No amplía L1/L4 ni demuestra medidas o componentes ocultos.

Modelo visual (08-10): revisión completa de las tres fotografías nuevas catalogadas como [DOC-0044](../03_CATALOGO/fichas/DOC-0044.md). Consulta focalizada de índice, ficha DOC-0042, notas previas y código del panel. Sin nuevas páginas de originales históricos; no se cierra L1/L4 ni se verifica montaje eléctrico. [ENS-0006](ensayos/ENS-0006_modelo_visual_fotografias.md) registra cobertura, fuentes y límites.

Implementación aprobada de dirección (08-10): lectura y modificación del código del panel, pruebas automatizadas y revisión visual de la interfaz. Se reutilizan las referencias de la propuesta; no se abren nuevas páginas originales. [ENS-0005](ensayos/ENS-0005_calibracion_direccion_panel.md). No amplía la lectura integral ni acredita medidas del kart.

Propuesta de calibración de dirección (08-10): consulta focalizada de notas, índice/ficha DOC-0042 y código del panel; STM32 original 407–454, ESP32 de trabajo 123–127, telemetría 191–199 y periodo. Buscador: texto DOC-0042 PDF 28/53, sin revisión visual de tabla/figura ni nuevas especificaciones adoptadas como vigentes. [Cobertura y resultados](PROPUESTA_CALIBRACION_DIRECCION_2026-10-08.md). Demo sintética verificada; no se cierra L1/L4 ni se acredita ensayo físico.

Preparación del traspaso ESP32/Wi-Fi/VS Code (07-10): notas/inventario, ficha DOC-0032, fuente E:1–60 y búsquedas de Serial/LEDC/Wi-Fi, documentación oficial de herramientas. [Prompt y referencias](PROMPT_RESPALDO_ESP32_WIFI_VSCODE_2026-10-07.md). Sin programación, conexión, lectura binaria ni nuevas páginas PDF; no se cierra L4.

Consulta de fotos/3D y diferencial (07-10): E:110–229, 350–434; M:250–279, 389–434; búsqueda focalizada del panel y consulta de notas/protocolo. [Contraste de señales y límites](ANALISIS_VISTA_3D_DIFERENCIAL_2026-10-07.md). Sin programación, nuevas páginas PDF, revisión visual de originales o ensayo físico; no se cierra L4.

Arranque del panel del portátil (07-10): implementación propia basada en las notas existentes y pruebas de software, sin nuevas páginas de originales ni revisión física. [Cobertura y resultados](ARRANQUE_PANEL_PORTATIL_2026-10-07.md). No se cierra L4 ni se acredita captura USB del kart.

Ampliación para panel real/calibración (07-10-2026): consulta de notas/fichas, búsquedas en fuente ESP32 y fragmento 426–496; STM32 `main.c` 348–383. Sin nuevas páginas PDF o revisión visual; cobertura y pendientes en la [propuesta](PANEL_REAL_CALIBRACION_2026-10-07.md). No se cierra L4.

Consulta de viabilidad del simulador (07-10-2026): reutilizada síntesis/inventario y recuperado solo el texto ya conocido de DOC-0042 PDF 5 con el buscador; sin nueva revisión visual. Fragmentos propios ESP32/STM32 y límites de cobertura en la [propuesta](PROPUESTA_SIMULADOR_2026-10-07.md). No se cierra L1 ni L4.

1. Leer `ESTADO_ACTUAL.md` y la tarea.
2. Buscar el tema en el índice y en el buscador local.
3. Recuperar las páginas pertinentes; ampliar a la sección completa si hace falta contexto.
4. Verificar tablas, figuras, parámetros y conclusiones relevantes en el original.
5. Guardar resultados y registrar cobertura antes de cambiar de lote.

No cargar bibliotecas, objetos de compilación ni archivos de macOS para resumir informes. No volver a extraer un documento cuyo contenido no cambió. No generar una traducción completa de todos los documentos: redactar las notas en español y conservar referencias a los originales en portugués o inglés.

## Criterio para dar un lote por terminado

La extracción automática no basta. Deben estar leídas sus secciones sustantivas, contrastados los elementos visuales relevantes, registrados los límites y enlazadas las conclusiones a las fuentes. No se fija un presupuesto arbitrario que pueda omitir evidencia: el ahorro viene de evitar repeticiones y recuperar solo lo necesario.


## Cobertura focalizada de software · 08–09-10-2026

ENS-0009 continúa A01–A12 desde la auditoría v1.2, leyendo y sustituyendo control/adaptador ESP32, main/IRQ/ADC/CAN/SD/Model/Presenter/View STM32; revisión de configuración FreeRTOS, API LCD/Widget, HAL IWDG v1.3.3 y makefiles/startup necesarios para compilar. Copia de 1673 archivos verificada; eso no significa lectura integral de dependencias. Se compilan proyectos y se prueban funciones concretas con entradas controladas, sin modelo dinámico, hardware ni lectura completa de los 42 documentos. No se amplía la cobertura de PDF/formatos visuales/Loop. BMS real, CubeMX y binario STM32 siguen pendientes.

## Preparación de publicación · 13-10-2026

Consulta de estado, mapa, catálogo, fichas DOC-0042/0043/0044, notas y ensayos existentes. Revisión visual de cuatro fotos ya catalogadas para seleccionar tres vistas públicas. Navegación, código de arranque/presentación y QA focalizados; sin nuevas páginas originales, lectura integral ni cierre de lote. [PUB-0001](PUBLICACION_2026-10-13.md).
