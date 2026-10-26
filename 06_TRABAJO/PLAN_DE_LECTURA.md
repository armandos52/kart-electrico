# Plan de lectura por lotes

26-10-2026: preparación de publicación y síntesis de avances; consulta focalizada de estado, problemas, diario y ENS-0029. Sin nuevas páginas de originales ni cierre de lotes/formatos visuales/Loop.

23-10-2026: [investigación ADC dirección](INVESTIGACION_ADC_DIRECCION_2026-10-23.md). Consulta focalizada de índice, fichas DOC-0014/0042, notas y fuentes dev9/local dev11-log. Reconsulta textual DOC-0042 PDF 28 mediante buscador; sin inspección visual nueva ni lectura de DOC-0014. ST DS10916 tabla 63 como contraste externo. No cierra lotes ni formatos visuales/Loop.

17-10-2026, L1/L3/L4 focalizados: [ENS-0025](ensayos/ENS-0025_revision_diferencial_y_reversa.md), índice/fichas DOC-0042/0031, notas software/dirección/control y fuentes propias ESP32 v1.2/dev3, frontend dev8-ui, contrato y arneses. Buscador: texto DOC-0042 PDF 43,45,55–57 y DOC-0031 PDF 4–5; sin nueva inspección visual de figuras/ecuaciones. Cálculos del plan derivados del código; valores históricos no adoptados como calibración actual. Kelly/Analog Devices como fuentes primarias externas. Sin lectura integral de bibliotecas/originales, capturas Kelly/formatos visuales/Loop ni cierre de lotes. Solo diagnóstico PC, plan antes de implementar.

16-10-2026, L4/L2 focalizado: [ENS-0024](ensayos/ENS-0024_bms_visible_diagnostico.md), catálogo L2, ficha DOC-0035, inventario software/BMS, código y CAN real. Buscador devuelve referencias DOC-0042 PDF 65/68; no se leen tablas visuales ni se declara protocolo confirmado. No cierre de lote, lectura integral, formatos visuales o Loop.

Fecha de preparación: 2026-09-28. El inventario y la extracción inicial están completados; la síntesis exhaustiva sigue pendiente.

16-10-2026, L4: [ENS-0023](ensayos/ENS-0023_bugs_e_instalacion_dashboard.md), revisión focalizada de CAN/Hall/SD, renderer y adaptador táctil, regresiones/stubs y scripts OpenOCD/SWD. Cuatro defectos reproducidos y corregidos, paquetes compilados; usuario habilita instalación física después. Sin nuevas páginas originales ni lectura integral de middleware/documentos. Formatos visuales/Loop y cierre de lote pendientes.

15-10-2026, propuesta visual del frontal: consulta focalizada del estado, índice/ficha DOC-0042, inventario, DEC-0010, ENS-0018/0019/0021 y PROB-0012/0013; código propio frontend/protocolo/formateador/View/Model/widget, callbacks generados y arranque de tareas; framebuffer `dev4_final/pantalla.png`. [Descripción y propuesta](PROPUESTA_PANTALLA_FRONTAL_2026-10-15.md) con maqueta sintética en navegador. Sin nuevas páginas originales, cambios de firmware, mediciones físicas ni cierre L1/L4. Formatos visuales históricos y Loop pendientes.

15–16-10-2026, continuación L4: [ENS-0022](ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md), propuesta aceptada y dashboard implementado. Lectura focalizada de frontend/View/Model, APIs locales Font/LCD/Widget/Screen, herramientas SWD y pruebas compartidas; render geométrico de código y glifos en PC. Sin nuevas páginas originales, conexión física ni cierre de lote; formatos visuales/Loop pendientes.

15-10-2026, retoma L4: ENS-0019/PROB-0012/0013, relectura y snapshot final, intento técnico de pedal, frontend SD/FatFs simulado, driver de detección, build y consumidor F411 consultados de forma focalizada. [ENS-0020](ensayos/ENS-0020_registro_pedal_f746.md) registra captura pendiente; [ENS-0021](ensayos/ENS-0021_sd_reintentos_locales.md) corrige SD parcialmente con pruebas sintéticas y compilación dev5. Sin nuevas páginas originales, hardware ni cierre de L1/L4; formatos visuales y Loop pendientes.

15-10-2026, continuación posterior L4: [ENS-0018](ensayos/ENS-0018_mejoras_pantalla_desarrollo.md) implementa F01–F03 tras autorización; consulta de ficha DOC-0042/notas existentes, formateador/cabecera/tests y dependencias directas banco/capturador/build. Nuevas pruebas exclusivamente en PC y compilaciones, sin revisión visual física ni páginas originales. No cierra L1/L4 ni formatos visuales/Loop. Nueve archivos de alcance antes/después, no lectura integral de bibliotecas.

15-10-2026, continuación L4: [ENS-0017](ensayos/ENS-0017_revision_preinstalacion_frontal.md), revisión de preparación F746/TouchGFX dev3. Estado/mapa, catálogo/ficha DOC-0042, inventario/notas/problemas/ensayos; frontend/formateador, View/Model/Presenter, widget/base/fuente gráfica, enganches de tareas, configuración, tests y build; consulta focalizada de inicio SD/driver/linker. Se conservan 36 archivos de alcance; compilar 151 unidades no acredita leerlas todas. Sin nuevas páginas originales, revisión visual del F746 ni cierre L1/L4; formatos visuales/Loop pendientes.

14-10-2026, continuación L4: [ENS-0016](ensayos/ENS-0016_vistas_frontal_nucleo.md) consulta selectivamente código de trabajo `Screen1View`, base generada, `KartStatus`, frontend/formateador, stubs y biblioteca LCD para adaptar vistas/capturar GRAM. Sin nuevas páginas originales ni lectura integral adicional; fuentes consultadas conservadas en la evidencia del ensayo.

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

Historial microSD (22-10): estado/mapa, índice/fichas DOC-0034/0042, inventario, PROB-0013 y ENS-0026/0027; revisión focalizada del registrador/UI, FatFs y panel/QA. Buscador: **DOC-0034 PDF 21 §6.11 y PDF 29 §7.7/tabla 10**, solo texto; sin inspección visual ni hardware. [Plan](PLAN_HISTORIAL_SD_Y_PANTALLA_2026-10-22.md), fuentes/hashes y límites enlazados. No cierre L4, no lectura integral ni resolución de formatos visuales/Loop. Implementación pendiente de aprobación.

Banco F411 (14-10): estado/mapa, índice/ficha DOC-0034, inventario, `.ioc`/linker y funciones/tests de trabajo. Buscador: **texto de DOC-0034 PDF 7**, sin nueva inspección visual. Consulta externa focalizada ST/MCUFRIEND/PlatformIO para placa, herramientas y bus LCD. [ENS-0015](ensayos/ENS-0015_nucleo_f411_pantalla_banco.md): identificación y respaldo físicos, pantalla/botón y pruebas lógicas en MCU con periféricos simulados. No cierra L1/L4, no lee nuevos originales completos ni resuelve formatos visuales/Loop.

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
