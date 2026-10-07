---
estado: vigente
actualizado: 2026-10-07
resumen: Primera grabación del ESP32 trasero (v1 y v1.1) y ensayo en banco de la telemetría de solo lectura por USB y Wi-Fi directo.
---

# ENS-0001 · Telemetría del ESP32 trasero por USB y Wi-Fi directo

Fecha: 2026-10-07.
Resultado: v1 salió mal (cuelgue) · **v1.1 salió bien** (telemetría USB y Wi-Fi, pérdida y reconexión).
Firmware: ESP32 `esp32_trasero-v1-telemetria` y `esp32_trasero-v1.1-telemetria` ([[07_DESARROLLO/firmware/VERSIONES|VERSIONES]]). STM32 sin cambios.
Configuración del kart: ESP32 en su placa trasera con el MCP2515, tal como estaba (Armando: «no he movido nada»), y alimentado por USB.

## Objetivo y criterio de éxito

- Que lleguen tramas JSON v1 válidas por USB y por TCP (192.168.4.1:3333) sin saltos de secuencia.
- Que la pérdida del cliente o del Wi-Fi no reinicie ni frene el ESP32.
- Que `loop_max_us` quede muy por debajo del timeout CAN de 100 ms.

## Condiciones

- Banco y tracción inhabilitada según Armando.
- Sin órdenes CAN 0x100 (`can_rx_count` = 0): el STM32 no transmite.
- MPU6050 sin respuesta («MPU-6050 FALHOU!»).
- Portátil a menos de 1 m (señal del 88 %).

## Procedimiento

1. Grabar por USB (`pio run -e usb -t upload`, COM4) y verificar el hash de esptool.
2. Leer el USB sin transmitir: `07_DESARROLLO/herramientas/leer_usb_esp32.py`.
3. Ensayo Wi-Fi automatizado con `07_DESARROLLO/herramientas/prueba_wifi_esp32.py`:
   1. Perfil Windows `kart-esp32` (manual) y TCP 30 s.
   2. Cierre y reconexión TCP durante 10 s.
   3. Corte del Wi-Fi del portátil 10 s.
   4. Reconexión y TCP 10 s.
   5. Vuelta a eduroam con comprobación de Internet.
   El USB se lee en paralelo durante todo el ensayo.

## Datos

En `06_TRABAJO/ensayos/datos/ENS-0001/`:

| Archivo | Contenido |
|---|---|
| `01_v1_usb_arranque_panic.log` | v1: errores I2C del core, *Guru Meditation* y reinicio |
| `02_v11_usb_can_sin_respuesta_40s.log` | v1.1: 40 s sin que el MCP2515 responda |
| `03_v11_usb_60s.log` | v1.1: arranque a los ~77 s y 183 tramas |
| `04_v11b_usb_100s.log` | v1.1 con diagnóstico de AP: 326 tramas |
| `05_v11b_ensayo_wifi.json` | Ensayo Wi-Fi: eventos, tramas USB y TCP con tiempos |

## Resultado

**v1:**
- Cuelgue *LoadProhibited* en el núcleo 1 dentro de `CAN.readMsgBuf()` → `SPIClass::transfer`, seguido de reinicio (`SW_CPU_RESET`).
- Causa: desbordamiento en mcp_can con DLC > 8 ([[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0005_mcp_can_desborda_con_dlc_mayor_8|PROB-0005]]).
- Además, los registros de error del core (no presentes en la imagen original) inundaban el puerto serie.

**v1.1 (DLC limitado, `CORE_DEBUG_LEVEL=0`):**

| Medida | Valor |
|---|---|
| Inicio del CAN | Tras **19 s, ~77 s y ~35 s** de «CAN init failed» en tres arranques; en uno no arrancó en 40 s |
| Tramas USB | Cada 200 ms exactos y 0 saltos (183, 326 y 382 tramas) |
| AP | `softAP=1`, 192.168.4.1, canal 6, WPA2-Personal CCMP, BSSID `70:4b:ca:7c:df:69` |
| TCP fase A (30 s) | 151 tramas, 0 saltos |
| TCP reconexión | 50 tramas, 0 saltos |
| TCP tras cortar el Wi-Fi 10 s | 51 tramas, 0 saltos |
| Reinicios durante el ensayo | Ninguno: una sola sesión `95d4ae6d…` |
| Tramas descartadas | 0 |
| `loop()` sin cliente | Mediana 1,15 ms, máx. 1,2 ms |
| `loop()` con cliente TCP | Mediana **2,7 ms**, máx. **3,7 ms** |
| Coste de publicar en `loop()` | ≤ 0,38 ms |
| Retardo TCP frente a USB | El TCP llega ~120 ms **antes**: el USB a 115200 es más lento |
| Vuelta a eduroam | Internet en 2 s |

**Observado, no explicado:**
- `rpm_rl` ≈ 150–170 y `rpm_rr` ≈ 280–400 con PWM en parada.
- `charger_connected` pasa de 0 a 1 sin cargador.

Apuntan a entradas Hall (GPIO34, GPIO4) y GPIO35 al aire o con ruido en el banco. Falta confirmar si las ruedas giraban.

## Qué salió bien y qué salió mal

**Bien:**
- La telemetría no frena el control: +1,5 ms de periodo con cliente frente a los 100 ms del timeout.
- La pérdida del observador no afecta al ESP32.
- Reconexión inmediata.

**Mal:**
- La v1 se colgaba por un fallo previo de la biblioteca CAN.
- La primera v1.1 no creaba el AP, por el orden `softAPConfig` → `softAP`; corregido.
- El MCP2515 tarda decenas de segundos en responder o no responde.

## Qué se aprende y siguiente paso

- El bloqueo en `CAN init failed` reproduce la hipótesis 3 de [[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender|PROB-0002]]. Siguiente: medir con osciloscopio y multímetro la alimentación del módulo MCP2515/TJA1050, MISO (GPIO19), CS (GPIO5) y el cristal mientras se reintenta.
- Repetir con el STM32 transmitiendo 0x100 para ver mandos reales y `can_rx_count`.
- Comprobar las entradas Hall y GPIO35 (resistencias de *pull*) con las ruedas quietas.
- Para seguir con asistencia durante la prueba Wi-Fi: instalar la app «Dispositivos Apple» (driver de red USB del iPhone) o usar un segundo adaptador.
