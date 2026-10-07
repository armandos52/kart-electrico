# Telemetría de lectura propuesta · versión 1

Formato implementado en el lector del PC y en el ESP32 de trabajo. v1.1 pasó USB/Wi-Fi en banco sin CAN real; v1.2 pasó diagnóstico USB/Wi-Fi con el módulo solo y CAN ausente ([ENS-0003](../../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md)). El binario original restaurado recuperó marcha según el usuario; v1.2 todavía no está validada con motores en el kart. STM32/BMS no emiten aquí este protocolo todavía.

Una línea UTF-8 JSON completa por muestra, terminada con LF. Máximo 8192 bytes por línea. La lectura conserva fragmentos entre timeouts y descarta líneas excesivas hasta el siguiente LF. El texto que no empieza por `{` se registra como mensaje de consola.

Ejemplo sintético de una muestra, no medición del kart:

```json
{"v":1,"type":"telemetry","node":"esp32","session":"arranque-ejemplo-1","seq":1,"tick_ms":250,"fw":"ejemplo-no-grabado","signals":{"rpm_rl":0,"rpm_rr":0,"pwm_left_raw":255,"pwm_right_raw":255,"relay1":0,"relay2":0}}
```

- `node`: `esp32`, `stm32` o `bms`. Indica el origen declarado; una versión posterior debe concretar adquisición/reenvío BMS y timestamp del gateway, sin atribuir un reloj inexistente al BMS.
- `session`: identificador distinto en cada arranque; 1–64 caracteres. En un nuevo arranque se invalidan las muestras anteriores del nodo.
- `seq`: contador uint32 creciente. Se admite rollover; duplicados/orden anterior se rechazan; saltos se cuentan. No es garantía de integridad ni autenticación.
- `tick_ms`: tick uint32 de adquisición declarado, no hora del PC. Su resolución no demuestra precisión de detección. Firmware, adquisición y reloj físico pendientes de contraste.
- `fw`: versión opcional, hasta 100 caracteres. Hasta recibirla no hay versión verificada. Antes de comparar ensayos debe fijarse y leerse la versión compatible.
- `reset_reason`: texto opcional de hasta 64 caracteres, motivo ESP32 leído al arrancar. Se conserva por nodo/sesión y se muestra en el panel; no sustituye el registro de alimentación ni un coredump.
- `signals`: solo las señales presentes. Una trama parcial no refresca las demás. Valores desconocidos no se inventan; el lector cuenta nombres no soportados. Una trama sin señales conocidas se rechaza.

| Señal | Unidad / significado |
|---|---|
| `accelerator_pct` | % de mando procesado; no ADC crudo |
| `steer_deg`, `yaw_dps` | grados de dirección y °/s de guiñada |
| `rpm_fl`, `rpm_fr`, `rpm_rl`, `rpm_rr` | rpm delanteras I/D y traseras I/D; convención del signo por fijar |
| `pwm_left_raw`, `pwm_right_raw` | entero 0–255 escrito por software; fuente 2025/26 invertida: 255 = orden de parada |
| `relay1`, `relay2` | entero 0/1 ordenado, no contacto físico medido |
| `brake`, `charger_connected`, `sd_ready` | entero 0/1 según origen; polaridad/captura por contrastar |
| `gear`, `mode` | N/D/R y DIF/ESC/BLQ |
| `soc_pct`, `battery_v`, `battery_a`, `battery_temp_c` | %, V, A y °C reportados; protocolo BMS y signo de corriente por verificar |
| `can_rx_age_ms` | milisegundos desde la última orden recibida según el micro |
| `accelerator_raw`, `steer_raw` | cuentas recibidas en CAN 0x100 por el ESP32 (sin convertir) |
| `relay_seq`, `emergency_stop` | estado 0–3 de la secuencia de relés del ESP32; 1 = parada por timeout CAN de 100 ms |
| `can_rx_count` | órdenes 0x100 válidas recibidas desde el arranque |
| `loop_max_us`, `telemetry_us_max` | periodo máximo de `loop()` y coste de publicar la telemetría dentro de él (µs) |
| `telemetry_dropped`, `wifi_clients` | tramas descartadas (cola llena o cliente lento) y clientes TCP conectados |
| `wifi_ap`, `mpu_ok` | 0/1: AP creado y respuesta del MPU al arrancar; no vigilancia continua del sensor |
| `control_ready`, `can_init_ok`, `can_init_attempts` | control configurado, inicio CAN válido e intentos acumulados |
| `control_state` | `can_wait`, `mpu_init`, `running`: fase; el control no está activo durante la espera CAN |
| `drive_inhibit` | `can_init`, `initializing`, `no_command`, `relays`, `brake`, `pedal_zero`, `neutral`, `direction_change`, `can_timeout`, `mpu_unavailable`, `none`: razón software de la última orden de parada |
| `start_requested` | 0/1, START recibido en bit 2 del byte 7 de CAN 0x100 |
| `drive_direction` | N/D/R según salidas ordenadas al Kelly derecho, no sentido físico medido |

**Transportes:**
- **USB serie (115200):** el de siempre.
- **TCP:** el ESP32 trasero v1 actúa como punto de acceso y escucha en `socket://192.168.4.1:3333`. El panel la añade con `--wifi socket://IP:PUERTO`, solo con IP privada, y la lee como un puerto más, sin transmitir.

Emisor implementado en `07_DESARROLLO/firmware/esp32_trasero`. v1.2 usa un máximo de 1536 bytes por línea; trama completa evaluada de 808 bytes con extremos de representación. El periodo objetivo es 200 ms; en espera CAN se observaron 200/306 ms por los reintentos. No publica RPM, PWM, marcha ordenada ni periodo de control antes de terminar la inicialización, y omite guiñada si el MPU no respondió. Las mediciones del bucle con CAN real siguen pendientes.

Los rangos en `server.py` solo validan representación de datos. No autorizan actuaciones ni son límites seguros del kart. Se rechazan valores no finitos, fuera de representación o booleanos donde se exige entero.

La caducidad visual inicial es 1500 ms, 3000 ms para SD y 8000 ms para batería/BMS: criterios provisionales de presentación, independientes del timeout CAN de control. Afinar después de medir frecuencias. Puerto cerrado/error de servicio hace que ninguna muestra se presente como vigente. No inferir fallo físico del micro a partir de pérdida de observabilidad.

Primer emisor: integrar pequeñas muestras ESP32 con versión, sesión y secuencia en una copia v0; cola/buffer limitado y envío no bloqueante a cadencia medida. Registrar carga y pérdidas y asegurar que diagnóstico no retrasa el control. No copiar impresiones JSON extensas al bucle crítico sin análisis.
