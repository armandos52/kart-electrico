---
estado: vigente
actualizado: 2026-10-09
resumen: Tareas próximas y completadas del proyecto del kart.
---

# Tareas

## Próximas

- [x] Implementar las propuestas A01–A12 y R a 20 km/h en código, sin grabar. ESP32/STM32 `v1.3-dev2`, compilaciones y pruebas dirigidas en [ENS-0009](ensayos/ENS-0009_robustez_firmware_v13.md). No simulación dinámica; [seguimiento completo](IMPLEMENTACION_ROBUSTEZ_V13.md).
- [ ] Identificar/resguardar firmware frontal y QSPI, comparar con la copia histórica, calibrar entradas/PPR/MPU y verificar BMS; ejecutar banco y ensayo real de v1.3 por Wi-Fi. [Procedimiento](ensayos/ENS-0009_robustez_firmware_v13.md). No declarar cerrado PROB-0008 ni garantizado límite físico hasta medirlo.

- [ ] Ejecutar la calibración física de dirección por Wi-Fi: centro alineado, topes, tres ciclos de retorno, tolerancias y referencias angulares. Asistente aprobado e implementado en [ENS-0005](ensayos/ENS-0005_calibracion_direccion_panel.md); [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md) sigue abierto. No hay perfil medido ni causa confirmada; control de motores sin cambios.

- [ ] Caracterizar y registrar por Wi-Fi v1.2 en el kart: identificar versión, condiciones, duración y modos; capturar CAN, START, motivos de parada, GPIO35, relés, RPM/PWM y alimentación. Armando confirma prueba real satisfactoria con motores en [ENS-0004](ensayos/ENS-0004_prueba_real_motores_confirmada.md); quedan pendientes datos instrumentados y causa de la regresión v1.1 en [PROB-0006](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md). USB no accesible con el módulo instalado.
- [ ] Mantener el [diario para el relatório](DIARIO_PARA_RELATORIO.md) en cada cambio/ensayo, según [DEC-0003](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0003_trazabilidad_para_relatorio.md) y regla permanente de AGENTS.md.

- [ ] Ejecutar en otra sesión el [prompt de respaldo ESP32/VS Code/Wi-Fi directo](PROMPT_RESPALDO_ESP32_WIFI_VSCODE_2026-10-07.md): identificación y flash completa verificada antes de modificar código/grabar; después copia de trabajo, entorno compatible y monitorización de lectura. Respaldo y puerto del kart aún pendientes; Wi-Fi pasa a ser prioridad de observación, cable para respaldo/recuperación inicial.
- [ ] Contrastar con Armando el [modelo 3D basado en las tres fotos recibidas](ensayos/ENS-0006_modelo_visual_fotografias.md). Medir entre ejes, vías, ruedas y alturas; confirmar electrónica oculta y capturar señales por Wi-Fi. Proporciones/giro ilustrativos; sin trayectoria ni ángulos independientes medidos.
- [ ] Recargar Codex, comprobar disponibilidad de Chrome DevTools MCP en el chat y probar la interfaz/URL que indique Armando; [configuración y evidencia](CHROME_MCP_2026-10-07.md). La prueba de conexión ya pasó; pruebas de aplicación pendientes.
- [ ] Continuar el [panel local](../07_DESARROLLO/panel_local/README.md): aplicación de lectura/mapa animado creada y probada sin hardware. Identificar/resguardar ESP32, preparar entorno/v0 e instrumentar señales; después STM32/SD/BMS y ajustes necesarios. [Sesión inicial](ARRANQUE_PANEL_PORTATIL_2026-10-07.md) y [nueva prioridad Wi-Fi](PROMPT_RESPALDO_ESP32_WIFI_VSCODE_2026-10-07.md). Usuario dispone de laboratorio/osciloscopio y ESP32; conexión aún sin verificar. iPad aplazado.
- [ ] Evaluar ideas de ajuste a partir de necesidades y ensayos. Armando aclara que «autofrenado» era regeneración para recargar batería, sin saber si el Kelly lo permite: comprobar modelo/configuración/controlador y batería/BMS antes de decidir. Las ideas aún no son funciones comprometidas; priorizar diagnóstico/registro.
- [ ] Medir área visible afectada y comprobar respuesta táctil de la franja blanca superior declarada; [PROB-0004](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md). Usar área legible medida para el diseño frontal.
- [ ] Estudio solicitado el 07-10: comenzar por historia y mapa físico de la [guía](GUIA_DE_ESTUDIO_DEL_KART.md); registrar aprendizaje y respuestas en la [sesión](SESION_ESTUDIO_2026-10-07.md). No marcar como completada lectura integral ni aprendizaje del usuario.
- [ ] Concretar mejora de pantalla: prioridades de Armando, capturas actuales, legibilidad, avisos de datos antiguos y estado de SD; propuesta en [ideas](IDEAS.md).
- [ ] Caracterizar el apagado y energía residual declarados el 07-10: síntoma, tiempo, recuperación, USB y rutas de alimentación; preparar ensayo con el orientador según [PROB-0003](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0003_energia_residual_al_abrir_contactor.md) antes de diseñar la nueva PCB.

- [ ] Compra, actualización 01-10: usuario confirma 16 celdas funcionales y preferencia Leiria. Verificar en SmartBMS configuración actual 16S (tabla histórica es 15S); contrastar candidato 58,4 V/10 A y adaptar conector conservando bloqueo de marcha. Ver [investigación](COMPRA_CARGADOR_Y_NEUMATICOS.md).

- [ ] Prioridad declarada 01-10: resolver la [compra de cargador y neumáticos](COMPRA_CARGADOR_Y_NEUMATICOS.md). Usuario confirma 16 celdas el 01-10 y 07-10; verificar conexión 16S activa, perfil de carga, BMS y conector con bloqueo; después seleccionar cargador 10 A. Confirmar cantidades y aptitud de neumáticos 130/60-13 y 4.10/3.50-6.

- [ ] Concretar la [propuesta 2026/27](INICIO_INVESTIGACION_2026-27.md): aportación del diferencial, protocolo de autonomía, ensayos previos a febrero, recursos, responsables y fechas; revisar con los orientadores.

- [ ] Definir y comprobar una copia de seguridad independiente.
- [ ] Completar L1: lectura integral del informe 2025/26 por secciones; ver [plan](PLAN_DE_LECTURA.md).
- [ ] Verificar que el montaje físico coincide con la configuración descrita en DOC-0042.
- [ ] Resolver las dudas documentales de [PROB-0001](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0001_versiones_documentales.md).
- [ ] Revisar planos y las cuatro capturas restantes de DOC-0015; analizar valores y fórmulas de los Excel cuando corresponda a L3.
- [ ] Abrir o exportar el archivo Loop desde su aplicación de origen para incorporarlo a la lectura.
- [ ] Leer las tablas en imagen de DOC-0042: 12–18 (CAN), 19–21 (BMS), 22–30 (cableado) y 31–32 (ensayos); ver [auditoría](AUDITORIA_2026-09-29.md).
- [ ] Si el ESP32 no activa el contactor, registrar secuencia y estados sin retirar módulos energizados. Preparar diagnóstico de alimentación/CAN según [PROB-0002](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender.md) y [PROB-0003](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0003_energia_residual_al_abrir_contactor.md); revisar PCB, GPIO12, IO35 y módulo CAN.
- [ ] Crear la copia de trabajo `v0` del firmware y `VERSIONES.md` según [[05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos|DEC-0001]].
- [ ] Incorporar la investigación de Armando sobre giroscopios y sensores de dirección, y definir los requisitos de sensores antes de la nueva PCB; ver [ideas](IDEAS.md).
- [ ] Continuar la [revisión v1.2](REVISION_CODIGO_V12_2026-10-08.md): diagnóstico estático y 19 casos aislados realizados; faltan identificación frontal, pantalla real, caracterización temporal/eléctrica y pruebas por modo. Implementar por etapas solo después de concretar la intervención con Armando.
- [ ] Averiguar qué versión de firmware generó KART_LOG.CSV y a qué ensayo corresponde. Se sabe que fue una versión del STM32 anterior al 10-07-2026; el archivo es del 04-07-2026.
- [ ] Comprobar si el `.elf` de CubeIDE del 10-07-2026 corresponde al `main.c` entregado y si es lo que está grabado en la placa.
- [ ] Contrastar en el kart las hipótesis de la auditoría (contactor/ESP32, desequilibrio de celdas, SOC a 0 %) antes de proponer soluciones.
- [ ] Preparar la publicación de la recepción: notas y catálogo por Git, almacenamiento apropiado para originales grandes y enlaces accesibles a los profesores.

## Completadas

- [x] Investigar v1.2 y recorrido pantalla-CAN-control (08-10), sin modificar firmware: [revisión](REVISION_CODIGO_V12_2026-10-08.md), matriz de funciones/prioridades y 19 observaciones de software reproducidas con periféricos simulados. Correcciones y validación física pendientes; fuente frontal no identificado en placa.

- [x] Corregir pulsador/lados I-D, rodadura, reparto de demo y orientación de cámara superior (08-10). [ENS-0008](ensayos/ENS-0008_lados_y_sentido_ruedas_visual.md): 19 comprobaciones dirigidas y 32 de regresión satisfactorias; sin modificar señal/calibración de dirección ni firmware. Contraste físico pendiente.

- [x] Adaptar el esquema superior a la cuarta fotografía de Armando (08-10): bastidor/asiento/cubierta en T, ruedas, cajas visibles, foto de referencia y selección. [ENS-0007](ensayos/ENS-0007_esquema_superior_fotografia.md): 30 comprobaciones del esquema y 32 de regresión 3D satisfactorias; medidas y ubicación oculta pendientes.

- [x] Incorporar tres fotos de WhatsApp con copia y SHA-256 ([DOC-0044](../03_CATALOGO/fichas/DOC-0044.md)); implementar modelo 3D local giratorio, cámaras, selección y galería (08-10). [ENS-0006](ensayos/ENS-0006_modelo_visual_fotografias.md): 21 pruebas Python y 32 comprobaciones web; sin ensayo físico ni firmware cambiado.

- [x] Implementar etapa A aprobada por Armando: asistente de dirección, captura de tramas, perfiles en PC y exportación; 21 tests Python y 16 comprobaciones de interfaz satisfactorias (08-10). [ENS-0005](ensayos/ENS-0005_calibracion_direccion_panel.md). Panel 8766 desconectado, sin ensayo físico ni firmware modificado.

- [x] Registrar variación de dirección comunicada por Armando y preparar propuesta + demostración aislada (08-10-2026). [Detalle y evidencia](PROPUESTA_CALIBRACION_DIRECCION_2026-10-08.md): 11 comprobaciones de software satisfactorias, capturas revisadas; datos sintéticos, sin conexión ni modificación del panel/firmware. Implementación pendiente de aprobación.

- [x] Registrar la confirmación de Armando del 08-10-2026: prueba real en el kart con motores satisfactoria tras preparar v1.2, [ENS-0004](ensayos/ENS-0004_prueba_real_motores_confirmada.md). Resultado declarado por el usuario; no equivale a caracterización instrumentada de todos los modos.

- [x] Crear y probar aplicación del portátil: mapa animado, selección, lector USB sin TX, consola, registro y trazas (2026-10-07). [Resultados](ARRANQUE_PANEL_PORTATIL_2026-10-07.md); 7 pruebas del lector y Chrome pasadas, sin conexión física ni firmware cambiado. COM3 identificado como Intel AMT SOL.
- [x] Configurar Chrome DevTools MCP y verificar conexión, navegador, clic, JavaScript, consola y red con una página local sintética (2026-10-07); [registro](CHROME_MCP_2026-10-07.md). Ajuste general guardado con copia previa; incorporación al catálogo del chat pendiente de recarga.
- [x] Registrar elección del panel real, preparar arquitectura/calibración y anotar franja blanca de pantalla (2026-10-07). [Propuesta](PANEL_REAL_CALIBRACION_2026-10-07.md), [DEC-0002](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0002_panel_real_y_calibracion.md), [PROB-0004](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md); sin mediciones ni firmware modificado.
- [x] Estudiar viabilidad y guardar mini plan de simulador local con alternativas y estimaciones (2026-10-07). [Propuesta](PROPUESTA_SIMULADOR_2026-10-07.md); sin implementación ni validación física.
- [x] Guardar guía inicial de estudio, confirmación de 16 celdas, ideas de pantalla/PCB y problema de energía residual (2026-10-07). Consulta de texto focalizada, sin diagnóstico confirmado ni lectura integral; [cobertura](SESION_ESTUDIO_2026-10-07.md).

- [x] Adaptar la propuesta al [formulario original del cronograma](../07_DESARROLLO/propuesta_2026-27/Cronograma_Smart_Electric_Kart_proposta_resumida.docx), resumida en una página y revisada visualmente (2026-10-01).

- [x] Inventariar el código por sistema y versión, con su estructura y las mejoras candidatas (2026-09-30); ver [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]].

- [x] Crear la [propuesta Word V2.0](../07_DESARROLLO/propuesta_2026-27/Proposta_Smart_Electric_Kart_V2_2026-27.docx) a partir del análisis del cronograma y comprobar visualmente sus cuatro páginas (2026-09-30). Selección de función, criterios y fechas pendientes de acordar con orientadores.

- [x] Incorporar el cronograma personal desde Descargas, verificar copia por SHA-256, catalogar DOC-0043 y analizar su encaje con la base documental (2026-09-30); ver [análisis](INICIO_INVESTIGACION_2026-27.md). Word sin modificar; revisión visual pendiente.

- [x] Abrir Obsidian y preparar una [guía de navegación y formatos](../00_INICIO/REPASO_EN_OBSIDIAN.md) con [lista personal de repaso](MI_REPASO.md) de los 42 documentos (2026-09-30).
- [x] Auditar el contenido extraíble: código, Excel, capturas, presentación y evolución por curso (2026-09-29); ver [auditoría](AUDITORIA_2026-09-29.md).

- [x] Inventariar la entrega completa, conservar originales, verificar tres ZIP y extraer lo que faltaba (2026-09-28).
- [x] Crear catálogo de 42 documentos, fichas por lote y buscador local por página (2026-09-28).
- [x] Revisar una muestra y guardar síntesis inicial del informe 2025/26 y dudas de versiones (2026-09-28).
- [x] Publicar la base documental en [GitHub](https://github.com/armandos52/kart-electrico), habilitar Issues y Discussions y añadir una plantilla de revisión académica (2026-09-28).
- [x] Preparar estructura, mapa del proyecto y plantillas iniciales (2026-09-28).
- [x] Instalar Obsidian 1.13.7 y verificar su registro en Windows (2026-09-28).

Para tareas técnicas extensas, crear una ficha con la [plantilla](../09_PLANTILLAS/TAREA.md) y enlazarla aquí.
