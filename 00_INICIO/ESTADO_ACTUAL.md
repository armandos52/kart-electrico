---
estado: vigente
actualizado: 2026-10-09
resumen: Lo conocido, lo pendiente y el siguiente paso del proyecto; traspaso entre sesiones.
---

# Estado actual

Última actualización: 2026-10-09.

## Robustez v1.3 implementada en código · 09-10-2026

- Armando autorizó todas las correcciones y R a **20 km/h**, trabajando desde el fuente STM32 histórico sin conexión a pantalla. Rama `codex/robustez-v13`; ESP32/STM32 `v1.3-dev2` y panel con nuevos estados. [Alcance](../06_TRABAJO/IMPLEMENTACION_ROBUSTEZ_V13.md), [ENS-0009](../06_TRABAJO/ensayos/ENS-0009_robustez_firmware_v13.md), [DEC-0006](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0006_robustez_v13_y_limite_marcha_atras.md).
- STOP/rearme/rampa, marcha atrás, ADC/CAN/Hall/MPU, watchdog, SD y GUI corregidos; solicitado/recibido/aplicado separados. Comprobaciones en PC y compilaciones documentadas. **Sin grabar ni probar físicamente; v1.2 sigue como última grabación conocida.**
- R corta propulsión según velocidad estimada, sin frenado eléctrico ni garantía física en pendiente. ESC experimental; calibraciones/PPR/BMS pendientes. No se acredita firmware instalado STM32 ni regeneración CubeMX.
- Siguiente: identificar y respaldar frontal+QSPI, comparar versión, banco preparado y después integración por Wi-Fi. USB ESP32 solo fuera del kart. Originales y respaldos anteriores conservados; no push ni copia de seguridad independiente configurada. Las secciones anteriores de v1.2 se mantienen como historia.

## Revisión v1.2 y modos · 08-10-2026

- Armando reconfirma marcha satisfactoria y pide investigar antes de modificar; declara modos/pantalla poco funcionales. [Diagnóstico y propuesta](../06_TRABAJO/REVISION_CODIGO_V12_2026-10-08.md), [PROB-0008](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0008_robustez_control_modos_y_pantalla.md): rampa sin efecto, STOP temprano perdido, recuperación CAN sin rearme, ESC asimétrico y SD sin reanudación, entre otros hallazgos.
- 19 observaciones reproducidas en PC con funciones originales y periféricos simulados; incluye defectos y protecciones, no validación física. ESP32 `905b09e` intacto; frontal revisado es fuente histórico, binario instalado sin identificar. Propuesta: estados/protecciones → DIF/BLQ y calibración → pantalla/SD → ESC → limpieza. Acceso a pantalla y registro Wi-Fi pendientes; sin grabación ni cambio de panel.

## Modelo visual del kart basado en fotos · 08-10-2026

- Panel `0.3.2-sentido-ruedas` en **http://127.0.0.1:8766/**, desconectado. 3D aceptado por Armando y esquema superior redibujado desde la cuarta foto de [DOC-0044](../03_CATALOGO/fichas/DOC-0044.md): bastidor rojo, asiento amarillo, cubierta trasera en T y ruedas diferentes. [Abrir esquema](http://127.0.0.1:8766/#esquema), con botón «Foto superior» y selección de piezas.
- [ENS-0006](../06_TRABAJO/ensayos/ENS-0006_modelo_visual_fotografias.md): 21 tests Python y 32 comprobaciones web satisfactorias con datos sintéticos. Originales/hashes preservados; sin publicación ni cambios de firmware. [DEC-0005](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0005_modelo_visual_basado_en_fotos.md).
- Actualización superior [ENS-0007](../06_TRABAJO/ensayos/ENS-0007_esquema_superior_fotografia.md): 30 comprobaciones del esquema y 32 de regresión 3D satisfactorias, sin hardware. Cuarta foto copiada/verificada, galería ampliada; ubicaciones ocultas pendientes.
- Corrección puntual [ENS-0008](../06_TRABAJO/ensayos/ENS-0008_lados_y_sentido_ruedas_visual.md): pulsador a la izquierda del conductor, lados I/D coherentes, rodadura y reparto de demo corregidos; «Arriba» ahora coincide con el esquema. 19 comprobaciones dirigidas y 32 de regresión satisfactorias. Señal de dirección/calibración y firmware intactos.
- Proporciones aproximadas, componentes ocultos por confirmar; no CAD medido. Próximo: contraste de Armando, cotas y registro Wi-Fi integrado. Catálogo: 44 entradas, incluido el conjunto de cuatro fotos; cobertura histórica sin cambios.

## Dirección: asistente implementado y aprobado · 08-10-2026

- Armando observa unos grados de variación en la **app del portátil con las ruedas quietas**; no dispone de esa indicación en la pantalla del kart. Sospecha calidad del sensor, recorridos distintos y centro incorrecto: causas/asimetría sin medir. [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md).
- Armando aprueba la etapa A, [DEC-0004](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0004_proponer_calibracion_antes_de_aplicar.md). Implementado «Calibrar dirección» en el [panel local](../07_DESARROLLO/panel_local/README.md), versión `0.2.0-direccion`: centro, extremos, tres ciclos de retorno, estadísticas, perfiles y exportación de muestras. Solo calibración visual del PC; firmware/control intactos.
- [ENS-0005](../06_TRABAJO/ensayos/ENS-0005_calibracion_direccion_panel.md): 21 pruebas Python y 16 comprobaciones Chrome satisfactorias con datos sintéticos. **Panel disponible en http://127.0.0.1:8766/, desconectado; sin perfil físico activo.** Próximo: capturas reales por Wi-Fi, tolerancias y mediciones angulares. `steer_raw` es CAN normalizado; no ADC original. Sin referencias físicas se muestra porcentaje, no grados calibrados.

## Prueba real con motores satisfactoria · confirmación 08-10-2026

- **Armando confirma que la prueba real en el kart con los motores funcionó bien.** Registrado en [ENS-0004](../06_TRABAJO/ensayos/ENS-0004_prueba_real_motores_confirmada.md) y en el [diario para el relatório](../06_TRABAJO/DIARIO_PARA_RELATORIO.md) como comprobación funcional declarada por el usuario.
- Versión de referencia: última imagen grabada `esp32_trasero-v1.2-diagnostico`, commit `905b09e`; no se comunicó otra grabación ni se recibió identificación del firmware durante esta prueba. El USB no es accesible instalado; la observación integrada se prepara por Wi-Fi.
- Funcionamiento básico de motores confirmado; no se adjuntaron logs, duración, modos ni condiciones detalladas. La caracterización de CAN/MPU/alimentación/control y la recepción del panel en el kart siguen pendientes de datos. [PROB-0006](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md) conserva abierta la causa de la regresión v1.1, con resultado funcional favorable registrado para la continuación v1.2.

Las secciones siguientes conservan el estado al terminar cada fase anterior; ENS-0004 actualiza el resultado funcional en el kart.

## v1.2 de diagnóstico, panel y regla del relatório · 07-10-2026 (noche)

- Armando confirma que **la imagen original restaurada recuperó la marcha del kart**. Comparación declarada por el usuario; causa de la regresión v1.1 abierta en [PROB-0006](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md).
- **Grabado ahora: `esp32_trasero-v1.2-diagnostico`**, commit `905b09e`, etiqueta `esp32-trasero-v1.2-diagnostico`. Solo módulo ESP32 fuera del kart por USB. La imagen original estable confirmada sigue respaldada.
- [ENS-0003](../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md): red y diagnóstico disponibles incluso sin CAN; USB 321 tramas y TCP 129/43/43, sin saltos, JSON inválidos ni reinicios. Relés ordenados apagados; no se publican RPM/PWM antes de inicializar control. Integración y motores de v1.2 **pendientes**.
- Panel actualizado: **http://127.0.0.1:8766/**, actualmente desconectado tras la captura por COM4. Resumen de señales, diagnóstico, todas las señales/búsqueda, gráficas, rodadura y dirección estimada sin calibrar. 13 tests y QA Chrome pasados. La instancia anterior en 8765 queda desconectada; utilizar 8766. Desconectar desde el panel antes de grabar o trasladar el módulo.
- Armando aclara que **el USB no es accesible con el ESP32 montado en el kart**. El próximo ensayo integrado se registra por Wi-Fi directo: Windows en `kart-esp32`, panel local 8766, fuente «Wi-Fi directo ESP32» (`socket://192.168.4.1:3333`). USB reservado a grabación/recuperación y banco con el módulo fuera. Esta aclaración no confirma que ya se haya montado ni probado v1.2 en el kart.
- Cambios de control acotados: validación de éxito CAN y parada de ESC si el MPU no respondió al arrancar; constantes, tiempos de relés y timeout 100 ms conservados. Revisar límites de ENS-0003 antes de integración.
- Regla permanente en [AGENTS.md](../AGENTS.md), [DEC-0003](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0003_trazabilidad_para_relatorio.md) y [diario para el relatório](../06_TRABAJO/DIARIO_PARA_RELATORIO.md): documentar motivos, configuración, versiones, métodos, datos, resultados positivos/negativos, hipótesis y límites. Binario/ELF v1.2 preservados fuera del repo público; sin push.

## Incidencia en el kart y recuperación · 07-10-2026 (noche)

- Armando declara que, al probar v1.1 en el kart, los motores no funcionaron y no pudo ver el panel. Causa sin confirmar; no hay captura en el vehículo.
- Ahora **solo el módulo ESP32 está fuera del kart, por USB**. El USB tuvo error de descriptor; volvió a aparecer como CP210x COM4. Repite `CAN init failed`, esperable al no tener conectado el MCP2515.
- [ENS-0002](../06_TRABAJO/ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md): **grabada ahora la imagen original**, restauración de 4 MB verificada por `verify-flash`. Flash v1.1 previa preservada en dos lecturas idénticas (`2026-10-07_212814`). Motores pendientes de comprobación física; causa sin confirmar.
- Panel corregido e iniciado en `http://127.0.0.1:8765/`: Wi-Fi disponible por defecto, Intel AMT excluido, diagnóstico ESP32 inicial y rótulos USB/TCP correctos. 10 tests y QA Chrome pasados. **COM4 abierto solo en lectura por el panel**, mostrando reintentos CAN sin el MCP2515. Desconectar en el panel antes de grabar o trasladar la placa.
- **La imagen original no incluye la telemetría Wi-Fi de v1.1.** Prioridad: recuperar y comprobar funcionamiento anterior antes de volver a grabar telemetría.

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
  - v1 = telemetría de solo lectura (SoftAP WPA2 + TCP 3333 + USB). Se grabó en ENS-0001 y sufrió panic; corregido el desbordamiento en v1.1.
- **Panel:** acepta `--wifi socket://192.168.4.1:3333`. 10 tests en verde.
- **Grabado durante el ensayo: v1.1** (etiqueta `esp32-trasero-v1.1`; restaurada después la imagen original en ENS-0002). [[06_TRABAJO/ensayos/ENS-0001_telemetria_esp32_usb_wifi|ENS-0001]] en banco:
  - USB y Wi-Fi sin saltos; pérdida y reconexión correctas.
  - `loop()` ≤ 3,7 ms con cliente TCP.
- **La v1 se colgaba** por un fallo previo de mcp_can con DLC > 8: [[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0005_mcp_can_desborda_con_dlc_mayor_8|PROB-0005]], corregido en v1.1.
- **El MCP2515 tarda entre 19 y 77 s o más en responder al arrancar,** lo que reproduce la hipótesis 3 de PROB-0002.
- **Entradas al aire:** RPM fantasma y GPIO35 a 1 con el kart quieto.
- **Siguiente:**
  - Osciloscopio en la alimentación, MISO, CS y cristal del MCP2515.
  - Ensayo con el STM32 transmitiendo 0x100.
  - Revisar los Hall y GPIO35.
  - Segunda copia del respaldo en otro soporte.
- **Wi-Fi sin perder Internet:** el iPhone se detecta por USB, pero falta el driver de red (app «Dispositivos Apple»). Perfil Windows `kart-esp32` creado en modo manual.

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
