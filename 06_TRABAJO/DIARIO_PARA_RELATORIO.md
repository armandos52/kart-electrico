---
estado: vigente
actualizado: 2026-10-09
resumen: Índice y diario de evidencias para el relatório final de ingeniería del kart.
---

# Diario para el relatório final

## ENS-0009 / PROB-0008 / DEC-0006 · 08–09-10-2026

Armando autoriza implementar todas las propuestas de la auditoría y R a 20 km/h, sin pre-simulación dinámica ni conexión frontal disponible. Se conserva la configuración v1.2 como referencia y copia STM32 v0 completa verificada (`2deb9ef`); ESP32 dev1 (`53ebf40`) y después ESP32/STM32 dev2, más panel. Objetivo, motivos, políticas, parámetros históricos, procedimiento, intentos negativos, límites y siguiente ensayo están en [ENS-0009](ensayos/ENS-0009_robustez_firmware_v13.md), [seguimiento A01–A12/R20](IMPLEMENTACION_ROBUSTEZ_V13.md) y [DEC-0006](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0006_robustez_v13_y_limite_marcha_atras.md).

Solo PC: 51 comprobaciones de control, 54 del frontal/SD/textos, 26 tests Python y 16 JavaScript pasados; ambos firmwares compilados, binarios/hashes conservados. Se corrigieron dependencia IWDG ausente, omisión de startup y verificador de símbolo débil; fallos y logs anteriores preservados. 1673 originales STM32 nuevamente verificados. No grabación, simulación dinámica, medida física, conexión al kart ni push. Frontend instalado sin identificar, ESC/calibraciones/BMS y límite R físico pendientes; la limitación de propulsión no implementa frenado eléctrico. Siguiente: respaldo/comparación STM32+QSPI y banco, después ensayo integrado por Wi-Fi; USB ESP32 solo fuera del kart.


Regla permanente en [AGENTS.md](../AGENTS.md): documentar cambios y ensayos con datos, resultados y límites. Este diario facilita redactar después; no sustituye los registros ni transforma hipótesis en resultados.

| Fecha | Trabajo y motivo | Evidencia / resultado | Límites y siguiente paso |
|---|---|---|---|
| 07-10-2026, tarde | Preservar firmware previo a cambios | [Respaldo original](../07_DESARROLLO/respaldos/esp32/2026-10-07_180133/RESPALDO.md): tres lecturas de 4 MB con mismo SHA-256 | No identifica con certeza el fuente exacto; copia externa pendiente |
| 07-10-2026, tarde | Añadir telemetría USB/Wi-Fi de lectura | [ENS-0001](ensayos/ENS-0001_telemetria_esp32_usb_wifi.md): v1 sufrió panic; parche DLC de mcp_can en v1.1; transmisión y reconexión sin pérdidas en banco | Sin mandos CAN reales ni MPU operativo; no valida circulación |
| 07-10-2026, noche | Recuperar funcionamiento tras prueba fallida con v1.1 | [ENS-0002](ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md): usuario declara motores sin funcionar con v1.1; imagen original restaurada y verificada en 4 MB; después confirma que el kart volvió a funcionar | Comparación funcional declarada por el usuario, sin logs en el kart. La causa de la regresión no está identificada |
| 07-10-2026, noche | Regla documental y revisión v1.2/panel | [DEC-0003](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0003_trazabilidad_para_relatorio.md), regla en AGENTS; [ENS-0003](ensayos/ENS-0003_v12_diagnostico_y_panel.md), commit firmware `905b09e`: USB 321 y TCP 129/43/43 sin saltos/reinicios; diagnóstico disponible sin CAN; resumen, buscador, gráficas y animaciones, 13 tests y QA Chrome | Solo ESP32 fuera del kart por USB. Dirección ADC estimada sin calibrar. Prueba de motores/CAN/MPU/alimentación en el kart pendiente; [PROB-0006](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md) sigue abierto |
| 08-10-2026, confirmación | Registrar prueba real con motores tras preparar v1.2 | [ENS-0004](ensayos/ENS-0004_prueba_real_motores_confirmada.md): Armando confirma que la prueba real en el kart con motores funcionó bien; última versión grabada de referencia `905b09e` | Resultado funcional declarado por el usuario. Fecha exacta del ensayo, duración, modos, identificación de versión durante la prueba y registros Wi-Fi no aportados; causa de la regresión v1.1 pendiente |
| 08-10-2026, propuesta de dirección | Registrar grados variables con ruedas quietas en el portátil y preparar centro/topes/perfiles | [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md), [DEC-0004](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0004_proponer_calibracion_antes_de_aplicar.md), [propuesta con fuentes, hashes y QA](PROPUESTA_CALIBRACION_DIRECCION_2026-10-08.md): 11 comprobaciones sintéticas satisfactorias; captura inválida rechazada y fluctuación conservada | Declaración de Armando, sin datos reales; calidad del sensor y asimetría son hipótesis. App/firmware intactos, sin conexión al kart. Aplicación al panel pendiente de aprobación; después ensayo por Wi-Fi y contraste angular físico |
| 08-10-2026, revisión v1.2 | Investigar funciones/modos antes de modificar firmware | [Revisión y prioridades](REVISION_CODIGO_V12_2026-10-08.md), [PROB-0008, A01–A12](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0008_robustez_control_modos_y_pantalla.md): 19 observaciones reproducidas en PC con periféricos simulados; incluye defectos y protecciones | Firmware intacto; no prueba física ni identificación frontal. Falta acceso a pantalla/registro Wi-Fi; implementación solo propuesta |
| 08–09-10-2026, implementación | Corregir A01–A12 y limitar R a 20 km/h por petición de Armando | [ENS-0009](ensayos/ENS-0009_robustez_firmware_v13.md), [DEC-0006](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0006_robustez_v13_y_limite_marcha_atras.md): ESP32/STM32 dev2 compilados, pruebas en PC y hashes | Sin grabar; comparación frontal, calibraciones y ensayo físico por Wi-Fi pendientes |

## Cómo utilizar las evidencias al redactar

### Diagnóstico v1.2, modos y pantalla · 08-10-2026

Armando reconfirma marcha positiva y pide investigar/proponer antes de actuar. [Revisión](REVISION_CODIGO_V12_2026-10-08.md), [PROB-0008, índice A01–A12](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0008_robustez_control_modos_y_pantalla.md), [evidencia y procedimiento](evidencias/AUD_V12_2026-10-08/README.md): fuente ESP32 v1.2 `905b09e` intacto, frontal histórico STM32 2025/26 sin identificación de lo instalado. Revisión de funciones y de selección/transmisión/ejecución de modos; 19 escenarios en MSVC x64 con funciones originales y periféricos simulados. Resultado: 19 observaciones reproducidas, incluidas protecciones y defectos; no equivale a 19 pruebas de seguridad superadas. Rampa descartada, STOP temprano perdido, rearranque tras CAN y asimetría ESC confirmados en software; SD/GUI frontal diagnosticados por lectura. Primer intento 18/19 por contaminación del temporizador static del banco simulado; corregido el aislamiento del ensayo, conservado el negativo. Hashes, código, resultados, advertencias de compilación y límites enlazados. Sin medición física, conexión, cambios de firmware/panel ni commit/push. Propuesta de prioridades guardada; no se adopta como decisión de implementación. Siguiente: pantalla/versión frontal y registro Wi-Fi, después primera corrección acotada. La marcha positiva de ENS-0004 permanece; no se atribuye causa a la regresión v1.1.

### Lados y sentido de ruedas en la vista · 08-10-2026

Armando señala pulsador izquierdo y ruedas que parecen girar en sentidos contrarios en la demo. [ENS-0008](ensayos/ENS-0008_lados_y_sentido_ruedas_visual.md), versión `0.3.2-sentido-ruedas`: corregidos lado del pulsador, asignación I/D, signo/velocidad visual de rodadura, reparto sintético y cámara superior. Ensayo previo reproduce 9 fallas entre 18 comprobaciones; final 19 dirigidas y 32 de regresión satisfactorias. Fuentes antes/después, datos y capturas conservados en el ensayo. El giro delantero ya concordaba y no se invierte la señal; sin cambios de calibración/firmware ni conexión al kart. Pendiente contraste físico de polaridad y geometría.

### Esquema superior a partir de la cuarta foto · 08-10-2026

Armando manifiesta satisfacción con el 3D y solicita adaptar el esquema a una nueva foto de Descargas. [ENS-0007](ensayos/ENS-0007_esquema_superior_fotografia.md), ampliación de [DEC-0005](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0005_modelo_visual_basado_en_fotos.md): versión visual `0.3.1-esquema-superior`, foto original y SHA-256 conservados en DOC-0044; SVG con cubierta en T, asiento, bastidor, suspensión y neumáticos observados. Se conservan selección, señales/animación y 3D; componentes ocultos sin ubicación física asignada. 30 comprobaciones del esquema y 32 del 3D satisfactorias; capturas/hashes en el ensayo. Sin conexión al kart, control ni firmware modificados. Próximo: contraste de Armando, cotas y registro Wi-Fi. No son medidas físicas.

### Modelo visual a partir de fotografías · 08-10-2026

[ENS-0006](ensayos/ENS-0006_modelo_visual_fotografias.md), [DOC-0044](../03_CATALOGO/fichas/DOC-0044.md) y [DEC-0005](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0005_modelo_visual_basado_en_fotos.md): Armando solicita adaptar la app al kart real usando tres fotos de WhatsApp. Copias/hashes preservados; panel `0.3.0-modelo-fotos` con bastidor rojo/arco alto, asiento amarillo, ruedas/suspensión, cinco cámaras, órbita/zoom, selección y galería. 21 pruebas Python y 32 comprobaciones Chrome satisfactorias; evidencia sintética, fallos del procedimiento y capturas en ENS-0006. Proporciones estimadas y electrónica oculta por confirmar. Sin conexión al kart, actuación ni cambios de firmware. Siguiente: revisión visual de Armando, cotas y telemetría integrada por Wi-Fi. Cambios locales sin commit/push; fotos sin publicar.

### Implementación aprobada de dirección · 08-10-2026

[ENS-0005](ensayos/ENS-0005_calibracion_direccion_panel.md), [DEC-0004](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0004_proponer_calibracion_antes_de_aplicar.md) y [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md): tras «si apruebo», panel `0.2.0-direccion` con capturas, retorno al centro, perfiles y ZIP de evidencia. 21 pruebas Python y 16 comprobaciones Chrome pasadas; intentos negativos y correcciones descritos en el ensayo. Evidencia sintética, sin conexión al kart; firmware/control intactos. Captura física y causa de la fluctuación pendientes. Esta entrada actualiza el estado de aprobación de la fila anterior, que describe la propuesta inicial.

### Restricción de acceso durante el montaje · 07-10-2026

Armando indica que no puede conectar USB cuando el ESP32 está instalado en el kart. Por ello, el ensayo integrado pendiente de v1.2 utilizará Wi-Fi directo y el registro JSONL del panel local. El USB se utiliza con el módulo fuera para grabar, respaldar, recuperar y comprobar en banco. Se incorpora esta condición a AGENTS y al procedimiento del panel. Es una declaración sobre el montaje, no un nuevo ensayo ni una confirmación de funcionamiento de los motores con v1.2.

### Criterios de redacción

- Explicar por qué se hizo cada cambio, qué se esperaba y qué ocurrió, incluyendo los intentos fallidos.
- Referenciar versiones en [VERSIONES](../07_DESARROLLO/firmware/VERSIONES.md), ensayos en `ensayos/`, problemas y decisiones en `05_DECISIONES_Y_PROBLEMAS/`.
- Conservar unidades, configuración y condiciones. Separar tiempos del firmware y del PC; capturas sintéticas sirven para evaluar interfaz, no dinámica del kart.
- Registrar las declaraciones del usuario como tales. Completar mediciones en el kart antes de escribir conclusiones sobre control, alimentación y seguridad.
- Preparar figuras y tablas desde datos conservados; ocultar credenciales y no publicar los binarios con NVS.
