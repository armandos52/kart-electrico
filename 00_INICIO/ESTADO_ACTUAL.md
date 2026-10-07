---
estado: vigente
actualizado: 2026-10-07
resumen: Lo conocido, lo pendiente y el siguiente paso del proyecto; traspaso entre sesiones.
---

# Estado actual

Última actualización: 2026-10-07.

## ESP32 trasero: respaldo, entorno y telemetría Wi-Fi · 07-10-2026 (tarde)

Rama `firmware/esp32-telemetria-wifi`. Commits locales, sin push.

- **Respaldo:** hecho y verificado. Tres lecturas completas de la flash de 4 MB con el mismo SHA-256: [[07_DESARROLLO/respaldos/esp32/2026-10-07_180133/RESPALDO|RESPALDO]].
  - Placa: ESP32-D0WD-V3 rev 3.1, CP2102 en COM4, sin cifrado ni Secure Boot.
  - **Coredump guardado:** la placa se colgó alguna vez. Puede relacionarse con PROB-0002; para descodificarlo hace falta el ELF.
  - La NVS contiene credenciales Wi-Fi de un firmware anterior. Los `.bin` están fuera de Git.
  - Segunda copia solo en el mismo disco. **Falta otro soporte.**
- **Lo grabado:** compilado con core 3.3.10. Coincide con el fuente entregado en tamaño (±736 B) y en cadenas, sin estar demostrado.
- **Firmware:**
  - [[07_DESARROLLO/firmware/esp32_trasero/README|PlatformIO]] (pioarduino 55.03.311, core 3.3.11, bibliotecas fijadas). Abierto en VS Code.
  - v0 = fuente entregado (etiqueta `esp32-trasero-v0`) y compila.
  - v1 = telemetría de solo lectura (SoftAP WPA2 + TCP 3333 + USB). Compila, **no grabada ni ensayada**.
- **Panel:** acepta `--wifi socket://192.168.4.1:3333`. 10 tests en verde.
- **Siguiente:** grabar v1 por USB con el banco preparado. Ensayar la telemetría por USB y luego por Wi-Fi, incluida pérdida y reconexión, y medir `loop_max_us`. El portátil solo tiene Internet por Wi-Fi (eduroam): para la prueba Wi-Fi hace falta otra vía de Internet o aceptar quedar desconectado.

## Estudio y montaje · 07-10-2026

- Preparado [prompt para respaldo ESP32, VS Code y Wi-Fi directo](../06_TRABAJO/PROMPT_RESPALDO_ESP32_WIFI_VSCODE_2026-10-07.md). Ejecutado la misma tarde (sección anterior).
- Armando elige [3D sencillo basado en fotos reales](../06_TRABAJO/ANALISIS_VISTA_3D_DIFERENCIAL_2026-10-07.md), priorizando matemática, datos y movimiento sobre detalle visual; [DEC-0002](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0002_panel_real_y_calibracion.md). Demo actual sintética; fotos/medidas, firmware físico y emisor pendientes. **3D aplazado**; primero respaldo y monitorización en la siguiente sesión.
- Creada [aplicación del portátil](../07_DESARROLLO/panel_local/README.md): kart seleccionable/animado, gráficas, consola, lector USB sin TX y registro. [Sesión](../06_TRABAJO/ARRANQUE_PANEL_PORTATIL_2026-10-07.md): 7 pruebas del lector y Chrome pasadas. Placas aún desconectadas; COM3 es Intel AMT SOL. Sin captura física ni firmware cambiado; telemetría en micros pendiente. Panel iniciado en `127.0.0.1:8766`, sin serie abierta.
- Armando aplaza iPad; ahora prioriza Wi-Fi directo para monitorización por incomodidad del USB instalado. El cable se mantiene para identificación, respaldo y primera grabación. Declara osciloscopio y laboratorio completo; siguiente paso en otra sesión: identificar/resguardar ESP32 y preparar entorno/copia de trabajo.
- Configurado [Chrome DevTools MCP](../06_TRABAJO/CHROME_MCP_2026-10-07.md): su prueba sintética pasó; recarga/disponibilidad directa pendientes. El panel se comprobó aparte con Playwright/Chrome; sin ensayo físico.
- Armando elige [panel de placas reales y calibración](../06_TRABAJO/PANEL_REAL_CALIBRACION_2026-10-07.md), [DEC-0002](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0002_panel_real_y_calibracion.md): observar y ajustar parámetros preparados sin regrabar por cada ajuste. Aplicación local de lectura creada; conexión física, instrumentación y calibración pendientes. iPad aplazado. La [viabilidad de simulación](../06_TRABAJO/PROPUESTA_SIMULADOR_2026-10-07.md) queda como estudio previo.
- Declara franja blanca horizontal superior de la pantalla táctil: [PROB-0004](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md). Extensión/píxeles, causa y respuesta táctil pendientes; considerar área útil en el diseño frontal.
- Preparada la [guía de estudio del kart](../06_TRABAJO/GUIA_DE_ESTUDIO_DEL_KART.md): historia, funcionamiento por sistemas, preguntas y recorrido personal. Consulta focalizada de texto; lectura integral y figuras pendientes. Cobertura en la [sesión](../06_TRABAJO/SESION_ESTUDIO_2026-10-07.md).
- Armando reconfirma **16 celdas** y tiene el kart disponible. Declaración del montaje actual; conexión 16S activa y parámetros/16 lecturas del BMS pendientes de contrastar. La tabla histórica 15S no se adopta como configuración actual.
- Solicita mejora de pantalla y PCB; declara energía residual al abrir el contactor que afecta al programa y la atribuye a los variadores. [PROB-0003](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0003_energia_residual_al_abrir_contactor.md): causa y ruta no confirmadas; puede relacionarse con PROB-0002. [Ideas](../06_TRABAJO/IDEAS.md) ampliadas.
- Próximo paso de esta sesión: respuestas sobre síntoma/tiempo/USB, prioridad de pantalla y etiquetas/configuración visibles. Después, mapa físico y ensayo de alimentación preparado con el orientador. Notas guardadas localmente, sin publicación ni cambios de firmware/hardware.

## Conocido por el usuario

- El 01-10 confirma 16 celdas funcionales y BMS de los archivos finales; compras en Portugal, preferencia Leiria. [Compra](../06_TRABAJO/COMPRA_CARGADOR_Y_NEUMATICOS.md): tabla histórica del BMS revisada visualmente y configurada para 15 celdas; comprobar configuración actual 16S antes de conectar nuevo cargador.

- Proyecto de kart eléctrico con historia desde 2008 hasta la actualidad.
- Se ha recibido documentación histórica, fichas técnicas, planos, hojas de cálculo y proyectos de software.
- El usuario ha aprobado instalar y utilizar Obsidian.

## Situación documental

- Cronograma resumido corregido a A4 horizontal el 01-10-2026 por indicación del usuario; una página revisada visualmente. Se ajustaron ancho de tabla, espacios y filas vacías para acomodar el formulario; contenido resumido conservado.

- El 01-10-2026 se preparó el [cronograma resumido en la plantilla original](../07_DESARROLLO/propuesta_2026-27/Cronograma_Smart_Electric_Kart_proposta_resumida.docx), a petición del usuario: solo tareas, meses, dos objetivos y fecha. Una página verificada visualmente; original conservado. Esta es la versión breve solicitada para el formulario; aprobación académica pendiente.

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
- Inventario del código (30-09): [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]]. Hay tres programas propios. La última versión del STM32 es del 10-07-2026 (con su `.elf` compilado el mismo día) y la del ESP32, del 21-07-2026. Anteriores: STM32 2024/25 y Arduino 2023/24. No hay código de 2008–2019. Se identifican 24 mejoras candidatas; nada se ha compilado.
- Decisión [[05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos|DEC-0001]] (30-09):
  - el firmware se desarrolla desde una copia `v0` de 2025/26, con Git y un número de versión en cada grabación;
  - cada ensayo se documenta con la [[09_PLANTILLAS/ENSAYO|plantilla de ensayo]], diciendo qué salió bien y qué salió mal.
- KART_LOG.CSV (04-07-2026) lo escribió una versión anterior del STM32, no el ESP32: el firmware entregado usa las mismas columnas, pero con comas y cada 200 ms.
- Copia de seguridad: pendiente de definir y configurar.
- Repositorio público: [armandos52/kart-electrico](https://github.com/armandos52/kart-electrico), con Issues y Discussions habilitados.
- Publicación mediante commits y push; no hay sincronización automática con Obsidian. Los ajustes locales y los índices generados quedan fuera de Git.
- Esta recepción y sus notas nuevas están guardadas localmente, sin commit ni push. Los originales de la entrega se excluyen temporalmente de Git por su volumen y pendiente de preparar su publicación.

## Próximo paso

Traspaso del 30-09 para la siguiente sesión de código:

1. Incorporar la investigación de Armando sobre giroscopios y sensores de dirección. Contrastarla con la idea de sensores de dinámica de [[06_TRABAJO/IDEAS|ideas]].
2. Crear la copia de trabajo `v0` del firmware y `VERSIONES.md` según [[05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos|DEC-0001]].
3. Analizar el código en el orden del [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]] (sección 9). Empezar por el protocolo CAN, la vigilancia de fallos entre STM32 y ESP32 y la plausibilidad del acelerador (mejora 24).
4. Hardware: registrar el fallo de arranque/apagado y preparar diagnóstico según [PROB-0002](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender.md) y [PROB-0003](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0003_energia_residual_al_abrir_contactor.md). La nueva PCB trasera y el lazo de seguridad del contactor son ideas, todavía no decisiones.
5. Pendiente de Armando: decidir si se hace push al repositorio público. Contiene su cronograma y la propuesta Word.

En paralelo, continuar con L1 del [plan de lectura](../06_TRABAJO/PLAN_DE_LECTURA.md): analizar el informe DOC-0042 por secciones y vincular sus resultados a las fuentes. Ya se revisaron portada, resumen, índice y conclusiones; no repetir esa lectura sin necesidad.

Entradas principales: [índice documental](../03_CATALOGO/INDICE_DOCUMENTOS.md), [síntesis 2025/26](../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md), [balance de recepción](../03_CATALOGO/RECEPCION_2026-09-28.md).

## Preguntas pendientes

- Compra prioritaria declarada el 01-10: [cargador y neumáticos](../06_TRABAJO/COMPRA_CARGADOR_Y_NEUMATICOS.md). 16 celdas confirmadas por el usuario el 01-10 y reconfirmadas el 07-10; falta verificar conexión activa y configuración BMS antes de comprar «60 V / 10 A». Candidatos web registrados, compatibilidad y precios pendientes.

- Aclarar versiones de 2023/24, fecha contradictoria del informe 24/25 y distintos relatos sobre el inicio; ver [PROB-0001](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0001_versiones_documentales.md).
- La propuesta fija diferencial electrónico inteligente y estimación de autonomía hasta febrero de 2027; falta concretar la aportación propia, condiciones y criterios de aceptación con los orientadores.
- ¿Qué documentación describe el montaje realmente existente?
- Fallo declarado el 30-09: al encender, a veces el ESP32 no activa el contactor y solo se recupera sacándolo de la placa. Hipótesis, pruebas e ideas para una nueva PCB en [[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender|PROB-0002]].

Ver [tareas](../06_TRABAJO/TAREAS.md).
