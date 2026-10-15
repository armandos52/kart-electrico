# Índice de ensayos

Corte de revisión: 15-10-2026. Cada ficha contiene objetivo, configuración, procedimiento, resultados, limitaciones y enlaces a datos. Los fallos también forman parte de la evidencia.

| Ensayo | Tipo de evidencia |
|---|---|
| [ENS-0001 · Telemetría del ESP32 trasero por USB y Wi-Fi directo](ENS-0001_telemetria_esp32_usb_wifi.md) | Banco USB/Wi-Fi; incidencia posterior declarada |
| [ENS-0002 · Recuperación del ESP32 tras prueba en el kart](ENS-0002_recuperacion_esp32_tras_prueba_kart.md) | Recuperación en banco; fallo en kart declarado |
| [ENS-0003 · v1.2 de diagnóstico y panel de telemetría](ENS-0003_v12_diagnostico_y_panel.md) | Banco USB/Wi-Fi; panel en PC |
| [ENS-0004 · Prueba real en el kart con motores](ENS-0004_prueba_real_motores_confirmada.md) | Funcionamiento en kart declarado por Armando |
| [ENS-0005 · Calibración de dirección en el panel](ENS-0005_calibracion_direccion_panel.md) | PC / datos sintéticos |
| [ENS-0006 · Modelo visual del kart a partir de fotografías](ENS-0006_modelo_visual_fotografias.md) | PC / datos sintéticos |
| [ENS-0007 · Esquema superior basado en una nueva fotografía](ENS-0007_esquema_superior_fotografia.md) | PC / datos sintéticos |
| [ENS-0008 · Pulsador izquierdo y sentido de ruedas en la visualización](ENS-0008_lados_y_sentido_ruedas_visual.md) | PC / datos sintéticos |
| [ENS-0009 · Implementación y comprobación de robustez v1.3](ENS-0009_robustez_firmware_v13.md) | PC / datos sintéticos |
| [ENS-0010 · Velocidad visual de las ruedas](ENS-0010_velocidad_visual_ruedas.md) | PC / datos sintéticos |
| [ENS-0011 · STOP conservado al recibir un lote CAN](ENS-0011_stop_en_lote_can.md) | PC / datos sintéticos |
| [ENS-0012 · Calibración por pasos y diagnóstico visible](ENS-0012_calibracion_por_pasos.md) | Registros Wi-Fi anteriores + pruebas nuevas en PC |
| [ENS-0013 · START/STOP con pulsador momentáneo](ENS-0013_start_stop_momentaneo.md) | PC / datos sintéticos |
| [ENS-0014 · Carga completa y comprobación del SOC 16S](ENS-0014_carga_completa_bateria_16s.md) | Planificado para 12–18 de octubre; sin mediciones ni ejecución |
| [ENS-0015 · Nucleo F411 + MCUFRIEND ILI9341](ENS-0015_nucleo_f411_pantalla_banco.md) | CPU/LCD/B1 reales; 83 comprobaciones con periféricos simulados; confirmación visual de Armando |
| [ENS-0016 · Vistas del frontal en Nucleo F411](ENS-0016_vistas_frontal_nucleo.md) | Diez escenas sintéticas en LCD físico, navegación B1, 137 comprobaciones y capturas GRAM para informe |
| [ENS-0017 · Revisión del frontal antes de instalar](ENS-0017_revision_preinstalacion_frontal.md) | PC: 83+56 regresiones correctas, build dev3 reproducido y tres hallazgos de presentación; sin grabar |
| [ENS-0018 · Correcciones de pantalla de desarrollo](ENS-0018_mejoras_pantalla_desarrollo.md) | PC: 101 comprobaciones, 13 fallos → 0; 155 del banco y cinco casos de captura simulada; STM32 dev4/F411 v0.4-dev compilados sin grabar |
| [ENS-0019 · Frontal dev4 en la F746 real](ENS-0019_frontal_dev4_f746_real.md) | **Placa real:** respaldo interna+QSPI, firmware previo identificado (original STM32-final), dev4 grabado/verificado; framebuffer, táctil, START, CAN con ESP32, SD sin tarjeta y franja medida |

[Diario para el relatório](../DIARIO_PARA_RELATORIO.md) · [Versiones de firmware](../../07_DESARROLLO/firmware/VERSIONES.md) · [Tareas pendientes](../TAREAS.md). No equivale a validación física completa ni a certificación del vehículo.
