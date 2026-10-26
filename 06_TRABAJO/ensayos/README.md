# Índice de ensayos

Corte de revisión: 21-10-2026. Cada ficha contiene objetivo, configuración, procedimiento, resultados, limitaciones y enlaces a datos. Los fallos también forman parte de la evidencia.

| Ensayo | Tipo de evidencia |
|---|---|
| [ENS-0029 · Instalación de historial microSD](ENS-0029_instalacion_historial_sd.md) | F746 real: respaldo dev9, fallo SD dev11, dev12 verificada, sesiones/cierre sin pérdidas SD; 53 errores ADC posteriores, replay real pendiente sin lector |
| [ENS-0026 · Diagnóstico del bloqueo de marcha y modo](ENS-0026_diagnostico_bloqueo_marchas.md) | F746/ESP32 reales por SWD y Wi-Fi; reposo, fondo y retorno medidos; bloqueo reproducido; corrección de zona cero dev9-ui y validación en curso |
| [ENS-0025 · Revisión de diferencial y reversa](ENS-0025_revision_diferencial_y_reversa.md) | Solo diagnóstico PC: dev3 56/0 previas y 16/5 nuevas; v1.2 3/3 defectos reproducidos. Plan antes de implementar; 50 indicados en banco y EMI declarados, sin nueva medida física |
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
| [ENS-0020 · Registro dedicado del pedal F746](ENS-0020_registro_pedal_f746.md) | Preparado; intento técnico previo `open failed`, sin muestras. Calibración pendiente |
| [ENS-0021 · Reintentos y pausa de SD](ENS-0021_sd_reintentos_locales.md) | PC: 120 comprobaciones, 16 fallos → 0; 174 del banco compartido. Dev5 compilado sin grabar; latencia del driver y pruebas físicas pendientes |
| [ENS-0022 · Dashboard frontal claro y oscuro](ENS-0022_dashboard_frontal_claro_oscuro.md) | PC: 130/0 frontal, 139/0 dashboard, 3/0 SWD; 184/0 banco compartido incluyen frontal. Siete páginas, 120 renders sintéticos y paquete F746 dev6-ui compilado, sin grabar |
| [ENS-0023 · Bugs e instalación del dashboard](ENS-0023_bugs_e_instalacion_dashboard.md) | Cuatro fallos corregidos; 150/0 frontal, 145/0 dashboard, 204/0 compartidas, 3/0 SWD. Dev7-ui grabada, respaldo doble/relectura sin diferencias, framebuffer real y aceptación manual; preferencia oscuro |
| [ENS-0024 · BMS visible en diagnóstico](ENS-0024_bms_visible_diagnostico.md) | Dev8-ui: SOC/temperatura provisionales, límites y caducidad; 163/0 dashboard y 150/0 frontal, build correcto. Dev8-ui instalada y captura BMS real con valores provisionales |

[Diario para el relatório](../DIARIO_PARA_RELATORIO.md) · [Versiones de firmware](../../07_DESARROLLO/firmware/VERSIONES.md) · [Tareas pendientes](../TAREAS.md). No equivale a validación física completa ni a certificación del vehículo.
