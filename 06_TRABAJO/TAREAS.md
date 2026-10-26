---
estado: vigente
actualizado: 2026-10-23
resumen: Tareas próximas y completadas del proyecto del kart.
---

# Tareas

## Prioridad comunicada · 26-10-2026

- [ ] Contrastar condensador masa–REF / masa–VCC y pérdida aproximada de medio rango ADC declarados por Armando: esquema, fotos, componente y medidas antes/después; no dar reparación por realizada.
- [ ] Identificar IMU existente y comparar candidatos conforme a señales, interfaz, alimentación, ruido y latencia; sin modelo elegido ni modos habilitados.
- [ ] Completar lectura microSD/replay real y diagnóstico de errores ADC/reversa.

[Contexto, configuración y criterios](AVANCES_2026-10-26.md).

## Próximas

- [x] Aprender PID e IMU con la web de Steppe School y dejar [guía](../04_CONOCIMIENTO/software_y_telemetria/GUIA_PID_E_IMU.md) y [simulador](../04_CONOCIMIENTO/software_y_telemetria/simulador_pid_imu.html) aplicados al ESC (23-10). Sin cambios de firmware.
- [ ] IMU futura: resolver [PROB-0017](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0017_imu_no_disponible.md); en banco (módulo fuera del kart) medir sesgo/ruido/ejes/`yaw_sign`, valorar rango ±500 °/s, medir `dt` real del lazo; después proponer PI con anti-windup y consigna limitada por μ·g/v, probado en PC antes del kart.

- [x] Implementación del plan e instalación autorizadas por Armando. Frontal **dev12-log** instalado, oscuro fijo/botón global y escritura microSD observada; fallo real SD de dev11 corregido con DMA. [ENS-0029](ensayos/ENS-0029_instalacion_historial_sd.md).
- [ ] Continuidad y cierre/reanudación comprobados en ENS-0029; completar retirada/reinserción y replay con archivos reales (usuario sin lector); extender cobertura de pruebas del nuevo importador/reproductor. ESP32 diagnóstico ampliado compilado, **sin instalar**. Sensibilidad del pedal dev10 incluida en dev12, pendiente de aceptación específica. Estos puntos sustituyen las tareas inferiores que aún describen plan sin aprobar y dev9 instalada.

- [ ] **Ruido general/PCB (23-10):** caracterizar señales/alimentación/masas; revisar planos GND, retornos, desacoplo y arquitectura/trazado de placas. Estudiar únicamente pasabajos según banda útil, conservando continua de dirección/pedal (corrección expresa de Armando del 23-10); registrar comparación antes/después. [Propuesta para el informe](INVESTIGACION_ADC_DIRECCION_2026-10-23.md#ampliación-para-el-informe-ruido-general-filtrado-y-placas--23-10-2026), PROB-0015 abierto; sin filtro/PCB implementados.

- [x] Investigar ADC de dirección y registrar observación de osciloscopio del 23-10: [tabla/diagnóstico](INVESTIGACION_ADC_DIRECCION_2026-10-23.md).
- [ ] Medir cursor/PF10, alimentación/referencia, resistencias y centro/topes; contrastar ≈1–2 V declarado con ADC 813 del 21-10. Evaluar ajuste mecánico/calibración o sensor adecuado; no aplicar 5 V directamente. Causa del ruido/divisor pendiente; PROB-0007/0015 abiertos.

- [x] Revisar grabación/panel y preparar [plan de oscuro fijo e historial SD](PLAN_HISTORIAL_SD_Y_PANTALLA_2026-10-22.md) el 22-10. Solo revisión/documentación; [DEC-0011](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0011_oscuro_fijo_y_acceso_grabacion.md), [PROB-0018](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0018_historial_sd_y_reproduccion_incompletos.md).
- [ ] **Petición 22-10:** aprobar plan antes de implementar; soporte/ubicación y panel 3D confirmados por Armando, capacidad/formato pendientes. Orden propuesto: registro fiable/sesiones + botón global → importación/historial/replay → cobertura trasera autónoma → ensayo completo. Autograbación propuesta, pendiente de aprobar. No formatear ni sobrescribir la tarjeta.
- [ ] Validar filtro/sensibilidad **dev10-ui**, compilado en PC y sin instalar ([ENS-0027](ensayos/ENS-0027_pedal_media_ponderada.md)); última instalada **dev9-ui**, ESP32 **v1.2-diagnostico** identificado por Wi-Fi en [ENS-0026](ensayos/ENS-0026_diagnostico_bloqueo_marchas.md). Mantener separado del cambio de registrador. Las tareas antiguas que dicen «sin identificar» describen su corte histórico.

- [x] Revisar arquitectura/matemática DIF/BLQ/ESC y reversa; registrar ruido declarado. [ENS-0025](ensayos/ENS-0025_revision_diferencial_y_reversa.md): diagnóstico PC y [plan presentado](REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md), sin modificar firmware.
- [ ] **Prioridad 17-10:** revisar el plan con Armando antes de implementar, según su última indicación. Identificar ESP32/configuración Kelly/PPR, corregir supervisión común R y rampa, verificar modos; [PROB-0014](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0014_reversa_y_arquitectura_diferencial.md). Los 50 km/h son indicación de pantalla en banco elevado en todos los modos, no velocidad de desplazamiento. Esta indicación prevalece sobre autorizaciones históricas para continuar código.
- [ ] Caracterizar EMI de dirección y otros sensores con osciloscopio; [PROB-0015](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0015_ruido_electromagnetico_sensores.md). Conservar señal bruta, alimentación/referencia, espectro y condiciones; dimensionar filtro después de medir y verificar retardo. Fuente/frecuencias/amplitudes pendientes, sin filtro implementado.

- [x] Instalar y validar manualmente navegación/temas de la pantalla: Armando confirma funcionamiento y prefiere oscuro. **Dev8-ui instalada**, SOC/temperatura provisionales visibles en Resumen/BMS; [ENS-0024](ensayos/ENS-0024_bms_visible_diagnostico.md). Respaldo y cuatro bugs corregidos en ENS-0023.
- [ ] Medir carga/latencia GUI y contrastar SOC/temperatura/protocolo con el BMS. La aceptación manual no sustituye estas medidas. Oscuro fijo solicitado el 22-10; retirar claro junto con el acceso global SD según el plan pendiente.

- [ ] Completar aceptación de pedal y medir centro/topes/retorno de dirección ([PROB-0012](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0012_pedal_reposo_fuera_de_pedal_libre.md), ENS-0026/0027). Medición y corrección dev9 ya realizadas; dev10 y rango de dirección pendientes de banco.
- [ ] Probar SD con/sin microSD sobre versión identificada y resolver detección/latencia del driver ([PROB-0013](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md)). Dev9 última instalada, dev10 local; reintentos/pausa de ENS-0021 y rechazo de CSV parcial de ENS-0023 conservados. Memoria declarada disponible el 22-10; escritura real todavía sin acreditar.

- [ ] **Esta semana (12–18 de octubre):** preparar y realizar [ENS-0014 · carga completa 16S](ensayos/ENS-0014_carga_completa_bateria_16s.md), solicitado por Armando el 13-10. Identificar equipos y criterio de terminación; conservar 16 tensiones, corriente final, temperatura y balanceo para verificar final de carga y referencia del SOC. Día/hora pendientes; no marcar 100 % alcanzado hasta disponer de evidencia.

- [ ] Confirmar identidad/carga 16S: etiquetas celda/cargador, 16 tensiones precisas en reposo, temperatura y BMS/protecciones/balanceo; resolver 60/70 Ah de [PROB-0010](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0010_identificacion_y_carga_bateria_16s.md). [Tutorial 13-10](../04_CONOCIMIENTO/bateria_y_bms/TUTORIAL_CELDAS_Y_CARGA_16S.md) completado como orientación, sin carga ni ajustes físicos.

- [ ] Recoger y vincular el feedback de profesores sobre la entrega del 13-10: aportación, protocolo experimental y prioridades. [Dossier](../00_INICIO/REVISION_PARA_PROFESORES.md), [PUB-0001](PUBLICACION_2026-10-13.md).

- [ ] Validar START momentáneo en el kart con relé. El frontal ya está respaldado, identificado y con dev4 grabado; en ENS-0019 START conmutó una vez por pulsación en el frontal. Falta identificar el ESP32 y ejecutar [ENS-0013](ensayos/ENS-0013_start_stop_momentaneo.md) por Wi-Fi. Confirmar pulsar/soltar/segunda pulsación y medir si hay rebotes; [PROB-0009](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0009_start_pulsador_momentaneo.md) abierto a prueba física. ESP32 dev3 de trabajo intacto; no se ha grabado esta corrección.

- [ ] Continuar control DIF/BLQ/ESC según prioridad de Armando del 11-10: pruebas conjuntas frontal→CAN→control→estado→pantalla y vista previa de estados sintéticos. [Retoma](RETOMA_PROGRAMA_2026-10-11.md). ENS-0013 cubre START frontal→control, no toda la cadena/modos/pantalla. Desarrollo ESP32 dev3/STM32 dev4; versiones físicas actuales pendientes de identificar tras declaración «B1.3» del 13-10.

- [x] Clasificar los QA antiguos del panel como antecedentes en [qa/README](../07_DESARROLLO/panel_local/qa/README.md); `qa/check-guided.cjs` cubre ya el asistente actual (ENS-0012). Legados: `qa/check-calibration.cjs` (flujo manual anterior; el ID `#cal-raw` vuelve a existir, pero no lo hace compatible) y `qa/check-panel.cjs` (puerto 8765, espera lista de placas). Hallados en [ENS-0010](ensayos/ENS-0010_velocidad_visual_ruedas.md); anteriores a ese cambio.
- [x] Implementar las propuestas A01–A12 y R a 20 km/h en código, sin grabar. ESP32/STM32 `v1.3-dev2`, compilaciones y pruebas dirigidas en [ENS-0009](ensayos/ENS-0009_robustez_firmware_v13.md). No simulación dinámica; [seguimiento completo](IMPLEMENTACION_ROBUSTEZ_V13.md).
- [ ] Identificar/resguardar firmware frontal y QSPI, comparar con la copia histórica, calibrar entradas/PPR/MPU y verificar BMS; ejecutar banco y ensayo real de v1.3 por Wi-Fi. [Procedimiento](ensayos/ENS-0009_robustez_firmware_v13.md). No declarar cerrado PROB-0008 ni garantizado límite físico hasta medirlo.

- [ ] Repetir en el kart la calibración con panel 0.4.0 por Wi-Fi, siguiendo colocar → Medir → mantener; exportar si falla. [ENS-0012](ensayos/ENS-0012_calibracion_por_pasos.md) analiza seis intentos anteriores del 09-10 (ninguno aceptado, dos a 0); origen físico pendiente. Contrastar: centro alineado, topes, tres ciclos de retorno, tolerancias y referencias angulares. Asistente aprobado e implementado en [ENS-0005](ensayos/ENS-0005_calibracion_direccion_panel.md); [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md) sigue abierto. No hay perfil medido ni causa confirmada; control de motores sin cambios.

- [ ] Caracterizar y registrar por Wi-Fi v1.2 en el kart: identificar versión, condiciones, duración y modos; capturar CAN, START, motivos de parada, GPIO35, relés, RPM/PWM y alimentación. Armando confirma prueba real satisfactoria con motores en [ENS-0004](ensayos/ENS-0004_prueba_real_motores_confirmada.md); quedan pendientes datos instrumentados y causa de la regresión v1.1 en [PROB-0006](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md). USB no accesible con el módulo instalado.
- [ ] Mantener el [diario para el relatório](DIARIO_PARA_RELATORIO.md) en cada cambio/ensayo, según [DEC-0003](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0003_trazabilidad_para_relatorio.md) y regla permanente de AGENTS.md.

- [x] Ejecutar preparación ESP32/VS Code/Wi-Fi del 07-10: respaldo inicial, v0, entorno, telemetría y grabación v1.2 realizados; [ENS-0003](ensayos/ENS-0003_v12_diagnostico_y_panel.md). Rótulo pendiente corregido el 11-10; copia independiente y captura integrada siguen abiertas en sus tareas.
- [ ] Contrastar con Armando el [modelo 3D basado en las tres fotos recibidas](ensayos/ENS-0006_modelo_visual_fotografias.md). Medir entre ejes, vías, ruedas y alturas; confirmar electrónica oculta y capturar señales por Wi-Fi. Proporciones/giro ilustrativos; sin trayectoria ni ángulos independientes medidos.
- [ ] Recargar Codex, comprobar disponibilidad de Chrome DevTools MCP en el chat y probar la interfaz/URL que indique Armando; [configuración y evidencia](CHROME_MCP_2026-10-07.md). La prueba de conexión ya pasó; pruebas de aplicación pendientes.
- [ ] Continuar el [panel local](../07_DESARROLLO/panel_local/README.md): contrastar animación 0.3.4, actualizar QA antiguos y registrar/calibrar señales reales por Wi-Fi. Respaldo ESP32, entorno/v0 y telemetría ya realizados; no repetirlos como tareas iniciales. iPad aplazado.
- [ ] Evaluar ideas de ajuste a partir de necesidades y ensayos. Armando aclara que «autofrenado» era regeneración para recargar batería, sin saber si el Kelly lo permite: comprobar modelo/configuración/controlador y batería/BMS antes de decidir. Las ideas aún no son funciones comprometidas; priorizar diagnóstico/registro.
- [ ] Diseñar las pantallas finales sin contenido en las filas 0–47 (daño físico medido en 0–43 el 15-10); táctil por zonas sin medir. [PROB-0004](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md).
- [ ] Estudio solicitado el 07-10: comenzar por historia y mapa físico de la [guía](GUIA_DE_ESTUDIO_DEL_KART.md); registrar aprendizaje y respuestas en la [sesión](SESION_ESTUDIO_2026-10-07.md). No marcar como completada lectura integral ni aprendizaje del usuario.
- [x] Describir el programa frontal y concretar propuestas visuales de desarrollo solicitadas el 15-10: [análisis, pendientes y dos vistas navegables](PROPUESTA_PANTALLA_FRONTAL_2026-10-15.md). Diseño aceptado posteriormente por Armando e implementado en ENS-0022; maqueta no equivale a validación física.
- [x] Corregir cuatro fallos reproducidos e instalar dashboard **dev7-ui** en la F746: respaldo doble, verificación y arranque real. [ENS-0023](ensayos/ENS-0023_bugs_e_instalacion_dashboard.md). Validación manual y pruebas integradas pendientes arriba.
- [x] Implementar la ampliación de pantalla de desarrollo aprobada, con temas claro/oscuro y siete páginas. [ENS-0022](ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md): TouchGFX dev6-ui, causa/aviso separados, condiciones D/R, edades/validez Hall, pruebas y build locales terminados. Instalación, consumo/latencias y LCD/táctil pendientes en tarea superior. Pantallas finales siguen aplazadas.
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
- [x] Crear v0 y [VERSIONES](../07_DESARROLLO/firmware/VERSIONES.md): ESP32 el 07-10 y STM32 en `2deb9ef`, antes de ENS-0009. Estado antiguo corregido el 11-10; no es una nueva copia física.
- [ ] Incorporar la investigación de Armando sobre giroscopios y sensores de dirección, y definir los requisitos de sensores antes de la nueva PCB; ver [ideas](IDEAS.md).
- [ ] Validar físicamente las correcciones derivadas de la [revisión v1.2](REVISION_CODIGO_V12_2026-10-08.md): implementación autorizada y realizada en ENS-0009, continuación STOP en ENS-0011. Siguen pendientes identidad frontal, pantalla real, caracterización temporal/eléctrica y pruebas por modo; no falta nueva autorización para continuar código dentro de ese alcance.
- [ ] Averiguar qué versión de firmware generó KART_LOG.CSV y a qué ensayo corresponde. Se sabe que fue una versión del STM32 anterior al 10-07-2026; el archivo es del 04-07-2026. Dato del 15-10: su cabecera usa `;` y el firmware que había en la F746 (STM32-final) escribe `Tick,Accel,…` con `,` en `kart_log.csv`, así que no fue esa compilación ([ENS-0019](ensayos/ENS-0019_frontal_dev4_f746_real.md)).
- [ ] Comprobar si el `.elf` de CubeIDE del 10-07-2026 corresponde al `main.c` entregado. **Grabado en placa: sí**; el 15-10 su contenido cargable coincidía byte a byte con la interna y la QSPI de la F746, compilado con CubeIDE 2.1.0/GCC 14.3.1 ([RESPALDO](../07_DESARROLLO/respaldos/stm32_f746/2026-10-15/RESPALDO.md)). La correspondencia con `main.c` sigue pendiente.
- [ ] Contrastar en el kart las hipótesis de la auditoría (contactor/ESP32, desequilibrio de celdas, SOC a 0 %) antes de proponer soluciones.
- [ ] Acordar canal de acceso a originales históricos completos cuando los profesores necesiten contrastarlos. Notas/catálogo y desarrollo incluidos en [PUB-0001](PUBLICACION_2026-10-13.md); originales completos locales, con referencias explícitas.

## Completadas

- [x] Pruebas reales del frontal dev4 en la F746 (15-10), [ENS-0019](ensayos/ENS-0019_frontal_dev4_f746_real.md):
  - Respaldo: interna ×2 y QSPI ×3 idénticos; firmware previo identificado como original STM32-final.
  - Grabación: dev4 grabado, Verified OK, relectura sin diferencias.
  - Funcionamiento: 3 h 10 min sin reinicios; F01–F03 en el framebuffer y el panel; táctil y modos; START una conmutación por pulsación; CAN con ESP32 sin 0x124.
  - Hallazgos: franja medida (filas 0–43); PROB-0012 y PROB-0013 abiertos.
- [x] Corregir F01–F03 en pantalla de desarrollo (15-10): [ENS-0018](ensayos/ENS-0018_mejoras_pantalla_desarrollo.md), frontal dev4; 101 comprobaciones con 13 fallos antes → 0 después, 155 del banco en PC, cinco casos de captura con transporte falso. F746 y F411 v0.4-dev compilados/preservados sin grabar. Interfaz final aplazada expresamente en DEC-0010; validación física pendiente.

- [x] Analizar preparación del frontal y nuevas mejoras (15-10): [ENS-0017](ensayos/ENS-0017_revision_preinstalacion_frontal.md), 83+56 comprobaciones existentes correctas y compilación ARM reproducida idéntica a dev3 preservado. Tres casos de presentación pendientes reproducidos; diagnóstico y secuencia de instalación documentados. Solo PC, sin modificar ni grabar firmware.

- [x] Registrar aceptación visual/manual de v0.3 (14-10): Armando confirma legibilidad y recorrido de las diez escenas con vuelta 10→01 sin saltos múltiples. [ENS-0016](ensayos/ENS-0016_vistas_frontal_nucleo.md). Declaración del usuario; no nuevo log serie ni cambio de firmware.

- [x] Adaptar la vista frontal a 320×240, diez escenas DIF/BLQ/ESC/reversa/fallos y navegación por B1; banco v0.3 grabado/verificado y 137 comprobaciones satisfactorias. Capturas leídas del LCD, PNG/CRC/logs y figuras para el informe; reinicio manual durante un intento preservado. [ENS-0016](ensayos/ENS-0016_vistas_frontal_nucleo.md). Periféricos del kart simulados.

- [x] Identificar/preparar Nucleo F411 con pantalla de banco (14-10): respaldo triple, instalación de herramientas STM32, LCD ILI9341 identificado, banco v0.2 grabado/verificado, 83 pruebas satisfactorias ejecutadas en MCU y B1 real registrado. Armando confirma visual/START ON–OFF. [ENS-0015](ensayos/ENS-0015_nucleo_f411_pantalla_banco.md), [DEC-0009](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0009_banco_nucleo_f411.md). No ensayo del kart ni CAN/ADC/SD/relés físicos.

- [x] Preparar entrega para profesores del 13-10: portada, dossier, índices, estado consolidado, navegación, guía de reproducción y pruebas. Publicación/verificación remota registrada en [PUB-0001](PUBLICACION_2026-10-13.md).

- [x] Corregir interpretación de START momentáneo (13-10): memoria y antirrebote en STM32 dev3; 83 comprobaciones frontal/integración y 56 del ESP32, compilación ARM correcta. [ENS-0013](ensayos/ENS-0013_start_stop_momentaneo.md), [DEC-0008](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0008_start_momentaneo_con_memoria.md). Solo PC, no resolución física acreditada.

- [x] Rediseñar calibración en bloques y separar colocación/medición (11-10): diagnóstico visible, reintento del paso y tema suave. [ENS-0012](ensayos/ENS-0012_calibracion_por_pasos.md), PC: 35 Python, 33 idiomas, 23 asistente y 32 regresión visual. Sin prueba física nueva.

- [x] Retomar control el 11-10 y corregir STOP perdido en lote CAN: ESP32 dev3 compilado; regresión 4 fallos → 0, 56 comprobaciones de control satisfactorias. [ENS-0011](ensayos/ENS-0011_stop_en_lote_can.md). Armando confirma v1.2 física sin cambios; fuente STM32 dev2, sin acceso a placa. Solo pruebas de software.

- [x] Hacer visible y realista la velocidad de las ruedas del panel (09-10): iteración 1 (escala 3 %) rechazada por lenta; iteración 2 con giro real 1:1, desenfoque de movimiento, demo ≈ 50 km/h y selector Real/Lento. [ENS-0010](ensayos/ENS-0010_velocidad_visual_ruedas.md): contrato 48/48, Chrome 53/53 y regresión 19/32/30 satisfactorios; sintético. Pendiente que Armando lo valore en su pantalla y, con RPM reales, ajustar umbrales; contrastar Ø de rueda y PPR.
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

- [x] Inventariar el código por sistema y versión, con su estructura y las mejoras candidatas (2026-09-30); ver [inventario](../04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md).

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
