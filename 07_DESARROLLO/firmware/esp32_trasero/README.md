---
estado: vigente
actualizado: 2026-10-13
resumen: Proyecto PlatformIO del ESP32 trasero — partes, entorno fijado, comandos y telemetría Wi-Fi de solo lectura (v1).
---

# ESP32 trasero — proyecto de trabajo

**Actualización 11-10: fuente y compilación actuales `esp32_trasero-v1.3-dev3`.** Corrige STOP sobrescrito por START en el mismo lote CAN, sin cambiar modos/umbrales/contrato. [ENS-0011](../../../06_TRABAJO/ensayos/ENS-0011_stop_en_lote_can.md): 56 comprobaciones sin fallos; no grabado. La declaración «B1.3» del 13-10 deja las versiones físicas actuales pendientes de identificar; v1.2 fue la última grabación documentada. Las secciones dev2 siguientes describen la base conservada.

**Base histórica del 09-10: `esp32_trasero-v1.3-dev2`, compilada y comprobada en PC; sin grabar en aquella sesión.** [ENS-0009](../../../06_TRABAJO/ensayos/ENS-0009_robustez_firmware_v13.md), [seguimiento](../../../06_TRABAJO/IMPLEMENTACION_ROBUSTEZ_V13.md) y [protocolo](../common/PROTOCOLO_V13.md). Estados/permisos centralizados, STOP/rearme, rampa efectiva, R 20 km/h estimados, CAN/ADC/Hall/MPU supervisados y watchdog. ESC sigue experimental; geometría/PPR/calibraciones reales pendientes. No hay frenado eléctrico ni garantía física en pendiente. La última grabación conocida sigue siendo v1.2.

Módulos actuales: `lib/kart_control` decide permisos y consignas; `src/main.cpp` adapta periféricos/aplica salidas; `include/hardware_kart.h` fija pines/PPR; `telemetria.cpp` publica por cola sin bloquear control. Wi-Fi sigue siendo solo lectura. Búfer de línea 2048 y TX USB 3072; `mpu_ok` representa ahora lectura calibrada reciente. Parámetros por motor/dirección se preparan en Config y hardware; no se transfieren automáticamente desde la calibración visual del PC.

`tests/compilar.cmd`: 56 comprobaciones dirigidas en dev3 (51 en dev2), sin modelo dinámico ni placa. Rama `codex/robustez-v13`; respaldo dev2/ELF/huellas enlazado en ENS-0009. Conservar original/v1.2 antes del ensayo físico. Las secciones v1.2 siguientes son históricas.

**Referencia física anterior: Armando confirma el 08-10-2026 una prueba real satisfactoria en el kart con motores**, [ENS-0004](../../../06_TRABAJO/ensayos/ENS-0004_prueba_real_motores_confirmada.md), tras la grabación de v1.2 de diagnóstico. La versión se vincula por continuidad de la sesión; no se recibieron identificación durante la prueba ni logs. Banco USB/Wi-Fi documentado en [ENS-0003](../../../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md). La imagen original restaurada también recuperó el funcionamiento según Armando ([ENS-0002](../../../06_TRABAJO/ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md)); permanece respaldada. Este fuente reconstruido no equivale de forma demostrada al binario original. La causa de la regresión de marcha de v1.1 permanece en [PROB-0006](../../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md).

## Cambios de v1.2 y límites

Wi-Fi/diagnóstico se inician antes de esperar al CAN: fase, intentos y relés visibles incluso sin MCP2515. El control sigue esperando a que CAN inicie correctamente; RPM/PWM no se publican antes de terminar su configuración. Trama con `control_ready`, `can_init_ok`, `can_init_attempts`, `control_state`, `drive_inhibit`, `start_requested`, `drive_direction` y `reset_reason`. Buffer 1536 y TX USB 2048. La trama completa evaluada ocupa 808 B y el panel reconoce todas sus señales.

Lecturas CAN con resultado de error no se interpretan como mandos. Si el MPU no respondió al arrancar, se omiten sus lecturas/calibración y guiñada de telemetría; ESC ordena parada, sin sustituirlo automáticamente por otro modo. `mpu_ok` no detecta desconexiones posteriores. Se conservan PWM, geometría, secuencia de relés y timeout CAN 100 ms. La trama CAN 0x123 conserva su formato y todavía carece de bandera de validez del MPU.

USB/Wi-Fi con CAN ausente: 321 muestras USB y 129/43/43 TCP, sin saltos, JSON inválidos ni reinicios. Cadencia observada durante espera: 200/306 ms. **Funcionamiento de motores confirmado por el usuario en ENS-0004; mediciones de CAN, periodo del control, MPU y alimentación pendientes.** Fuente/ELF/binario, resultados y recuperación documentados en ENS-0003. Usar el panel actualizado de esta sesión en `http://127.0.0.1:8766/`.

Copia de trabajo según [DEC-0001](../../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos.md).
- Historial: [VERSIONES](../VERSIONES.md).
- Respaldo previo de la placa: [2026-10-07](../../respaldos/esp32/2026-10-07_180133/RESPALDO.md).

## Partes

| Archivo | Qué es |
|---|---|
| `src/main.cpp`, `lib/kart_control` | Adaptación hardware y núcleo v1.3; `.ino` anterior preservado en Git/auditoría v1.2 |
| `src/telemetria.cpp`, `include/telemetria.h` | v1: telemetría de solo lectura (cola, tarea de red, JSON) |
| `include/secretos.example.h` | Plantilla del SSID/contraseña Wi-Fi. La real es `include/secretos.h` y no se versiona |
| `platformio.ini` | Plataforma, core, bibliotecas y opciones de flash fijadas; entornos `usb` y `wifi` |
| `.vscode/tasks.json` | Tareas de VS Code: compilar, grabar, monitor USB y monitor Wi-Fi |

## Entorno fijado

- **Plataforma:** pioarduino `55.03.311` (core Arduino-ESP32 **3.3.11**, ESP-IDF 5.5), placa `esp32dev`, flash DIO 80 MHz 4 MB, particiones `default.csv`. Coincide con la imagen grabada.
- **Bibliotecas:**
  - `coryjfowler/mcp_can` 1.5.1, **copiada en `lib/mcp_can`** con el DLC limitado a 8 ([PROB-0005](../../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0005_mcp_can_desborda_con_dlc_mayor_8.md)).
  - `electroniccats/MPU6050@1.4.5`.
  - La entrega no incluía bibliotecas; se eligieron por la API usada (`MCP_ANY`, `MCP_8MHZ`, `getRotation`).
- **`CORE_DEBUG_LEVEL=0`**, igual que la imagen original: sin registros de error del core por el puerto serie.
- **Herramientas:** PlatformIO 6.2.0 y esptool 5.4.0 en `%USERPROFILE%\.venvs\kart-esp32` (Python 3.12).
- **Diferencia con lo grabado:** lo grabado se hizo con core 3.3.10 desde Arduino IDE. pioarduino no tiene versión 3.3.10.

## Comandos (PowerShell, en esta carpeta)

```powershell
$pio = "$env:USERPROFILE\.venvs\kart-esp32\Scripts\pio.exe"
& $pio run -e usb                 # compilar
& $pio run -e usb -t upload       # grabar por USB en COM4 (solo con el banco preparado)
& $pio device monitor -e usb      # consola USB 115200
& $pio device monitor -e wifi     # consola TCP socket://192.168.4.1:3333
```

Panel existente por Wi-Fi: `..\..\panel_local\iniciar.ps1 -Python <python> -Wifi socket://192.168.4.1:3333`. Después se elige la fuente «Wi-Fi directo ESP32» en el selector de puertos.

## Telemetría v1 (solo lectura)

- **Red:** el ESP32 crea la red WPA2 `kart-esp32`, con la contraseña de `include/secretos.h`. Es una red directa con el portátil, sin router ni nube, en 192.168.4.1, canal 6, como máximo 2 clientes y potencia 8,5 dBm. Al conectarse a ella, **el portátil pierde Internet por Wi-Fi** salvo que tenga otra conexión (cable o segundo adaptador).
- **Formato:** TCP 3333, con una línea JSON por muestra según [PROTOCOLO](../../panel_local/PROTOCOLO.md) v1, cada 200 ms. La misma línea sale por USB si cabe en el búfer.
- **Cabecera:** al conectar, el cliente recibe una línea de texto con firmware, sesión y **motivo del último reset**.
- **Sin entrada:** lo recibido por TCP se descarta. No hay comandos, calibración ni OTA.
- **Sin escrituras en flash:** `WiFi.persistent(false)` evita que el punto de acceso escriba en la NVS.

**Cómo no frena el control:**
- `loop()` solo copia variables y hace `xQueueSend` sin espera (cola de 4).
- La red corre en una tarea propia del núcleo 0.
- Un cliente lento se desconecta, y las tramas perdidas se cuentan en `telemetry_dropped`.
- Para medirlo, cada trama lleva `loop_max_us`, el periodo máximo de `loop()` desde la anterior, y `telemetry_us_max`, el coste de publicar dentro de `loop()`. Comparar con el timeout CAN de 100 ms.

### Señales publicadas (solo variables existentes)

| Señal | Origen en el fuente |
|---|---|
| `accelerator_raw`, `steer_raw`, `brake`, `gear`, `mode` | Trama CAN 0x100 recibida. Se publican solo tras la primera orden. `gear` y `mode` siguen la misma prioridad que `processarComando()`. `brake` = `travao == 1` |
| `accelerator_pct` | Misma fórmula que `processarComando()`: `map(raw, 0, 4095, 0, 100)` |
| `can_rx_age_ms`, `can_rx_count`, `emergency_stop` | Edad de la última orden, número de órdenes válidas y bandera de timeout de 100 ms |
| `pwm_left_raw`, `pwm_right_raw` | `motor_esq_speed` / `motor_dir_speed` (255 = parada) |
| `rpm_rl`, `rpm_rr` | `rpm_esq` / `rpm_dir` (Hall, ventana de 500 ms, sin signo) |
| `yaw_dps` | `yaw_rate` del MPU6050 |
| `relay1`, `relay2`, `relay_seq` | Estado **ordenado** de GPIO16/17 (no se mide el contacto) y estado de la secuencia 0–3 |
| `charger_connected` | Nivel de GPIO35, tal como lo interpreta `loop()` |

**No se publica:**
- `steer_deg`: la conversión ±20° del fuente no está calibrada.
- Velocidad: el fuente usa un diámetro de 0,48 frente a un radio de 0,243.
- Tensión, corriente y datos del BMS/STM32: el ESP32 no los recibe; haría falta que el STM32 o el BMS los reenviaran por CAN a un ID nuevo.

## Límites conocidos

- La red no demuestra el estado del kart: **perder la telemetría no significa que el kart se haya parado.**
- Wi-Fi añade consumo, con picos de cientos de mA. Si la alimentación del ESP32 es justa, puede haber resets por *brownout*; la cabecera TCP lo indicaría como `reset=brownout_tension`.
- **v1.1 probada en banco** ([ENS-0001](../../../06_TRABAJO/ensayos/ENS-0001_telemetria_esp32_usb_wifi.md)): USB y Wi-Fi sin saltos, pérdida y reconexión correctas.
  - `loop()` sin cliente: mediana 1,15 ms.
  - Con cliente TCP: mediana 2,7 ms y máximo 3,7 ms.
  - Todavía **sin tráfico CAN real ni MPU**: los mandos no se han visto llegar.
- Señales `wifi_ap` (AP creado) y `mpu_ok` (MPU respondió al arrancar). Sin MPU no se publica `yaw_dps`.
