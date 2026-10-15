# Contrato CAN v1.3 · dev2/dev3

Actualización 13-10: frontal dev3 adapta el pulsador momentáneo a una **petición memorizada**: START tras alto estable 40 ms, liberación estable 40 ms para otra pulsación y STOP al primer alto de esta. Soltar no envía STOP. Bit 2 de 0x100 y todos los formatos permanecen; `start_requested`/CSV `Start` no son lectura del pin. El ESP32 sigue dev3, con STOP prioritario. [ENS-0013](../../../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md); versiones instaladas por identificar.

Actualización 11-10: ESP32 dev3 conserva el contrato dev2 y publica `fw=esp32_trasero-v1.3-dev3`; frontal permanece dev2. Cada STOP válido recibido queda pendiente hasta ser aplicado aunque otro mando START llegue en el mismo lote. Ese START anterior a aplicar la parada no habilita nuevo arranque: requiere nuevo OFF→START y las condiciones existentes. Durante la aplicación del STOP retenido, `start_requested` puede ser 1 y `drive_inhibit` ser `start_off`; representan orden más reciente y causa aplicada respectivamente. [ENS-0011](../../../06_TRABAJO/ensayos/ENS-0011_stop_en_lote_can.md).

Fuente común: `kart_protocol.h`. CAN estándar 500 kbit/s, datos de 8 bytes; ESP32 MCP2515 con cristal histórico de 8 MHz. Parámetros físicos aún por contrastar. IDs ampliados 0x101/0x124 deben comprobarse libres en el bus real.

| ID | Emisor / periodo nominal | Contenido |
|---|---|---|
| 0x100 | STM32 / 5 ms | ADC normalizados: pedal BE16 bytes 0–1, dirección BE16 2–3, freno 0/1 byte 4, D byte 5, R byte 6, modo bits 0–1 y START bit 2 del byte 7. Resto reservado cero |
| 0x101 | STM32 / 5 ms | Versión 1 byte 0; flags byte 1: ADC pedal válido bit 0, dirección válida bit 1, D/R coherentes bit 2; ADC físicos BE16 bytes 2–3/4–5; secuencia BE16 bytes 6–7. 65535 = conversión fallida |
| 0x123 | ESP32 / 200 ms | Velocidad media estimada km/h entero BE16, RPM I BE16, RPM D BE16, yaw firmado décimas °/s BE16. Formato heredado; validez en 0x124 |
| 0x124 | ESP32 / 100 ms | Versión 1; motivo; modo seleccionado internamente; modo aplicado; flags; estado relés; dirección ordenada; modo recibido, un byte cada uno |

Modos: 0 DIF, 1 ESC, 2 BLQ, 255 ninguno aplicado. Marcha: 0 N, 1 D, 2 R. Relés: 0 apagados, 1 precarga, 2 solapamiento, 3 listo. Flags 0x124: bit 0 salidas inicializadas/calibración inicial terminada, 1 rearme pendiente, 2 velocidad válida, 3 gyro válido, 4 quietud estimada durante 500 ms, 5 limitación R actuando, 6 extensión frontal observada. Bit 0 no significa contactor cerrado ni contactos medidos.

Motivos 0–20 en orden: permitido; sin mando; timeout; mando inválido; START apagado; relés; cargador; rearme; freno; pedal libre; neutro; cambio sentido; cambio modo; velocidad inválida; Hall; MPU; frontal inválido; límite R; salida; CAN; configuración. El modo aplicado solo se publica cuando hay esfuerzo ordenado mayor que cero. PWM/dirección/relés son órdenes de software, no medición de actuación física.

ESP32 acepta el frontal antiguo sin 0x101 para facilitar el contraste, con `frontend_health_seen=0`: no puede verificar ADC físicos ni el ciclo de adquisición. Tras ver 0x101 exige flags válidos y secuencia renovada dentro de 100 ms; no vuelve al perfil antiguo por timeout. STOP válido mantiene prioridad sobre fallo ADC. 0x100 caduca a 100 ms; valores >4095, DLC incorrecto, D+R, flags/bit reservado inválidos se rechazan.

STM32 acepta 0x123 antiguo mostrando recibido/aplicado desconocidos. Tras ver 0x124, requiere que permanezca reciente (300 ms) para permitir selección de marcha; 0x123 caduca a 500 ms. Un cero entero km/h no demuestra quietud: se usa estado explícito o ambas RPM cero en perfil antiguo. CAN sin frescura fuerza N/freno/pedal cero. D→R en movimiento fuerza N, sin conservar D.

BMS 0x355/0x356: conserva ocho bytes y sus edades. No se acredita protocolo/escala del BMS instalado; `BMS_FORMAT_CONFIRMED=0` impide publicar SOC/temperatura como mediciones. No inferir tensión/corriente ni autonomía; verificar primero el mapa real.

Wi-Fi mantiene JSON v1 y `fw=esp32_trasero-v1.3-dev2`. Nuevos campos: `mode_applied`, `rearm_required`, `reverse_limited`, `speed_valid`, `speed_kph_max_wheel`, `reverse_limit_kph`, `frontend_health_seen`, `frontend_health_ok`, `watchdog_ok`, `can_rx_invalid`, `can_tx_errors`, `can_error_flags`, `mpu_errors`, `accelerator_adc_physical`, `steer_adc_physical`. `mode` sigue siendo recibido; `mpu_ok` pasa a lectura calibrada reciente. Panel distingue esta semántica de v1.2. `speed_kph_max_wheel` gobierna R con la rueda más rápida; 0x123 muestra la media.

El esquema no convierte un Hall desconectado en detectable a vehículo parado; tampoco mide sentido. No implementa frenado eléctrico, CRC de aplicación, contador de 0x100 ni autentificación CAN. Se conserva el CRC físico del bus. Errores TX se cuentan; límite interno de espera MCP2515 existente: 2500 µs por espera, sin acreditar tiempo físico total.
