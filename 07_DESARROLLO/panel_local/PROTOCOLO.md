# Telemetría de lectura propuesta · versión 1

Este formato está implementado en el lector del PC. **Aún no está implementado ni verificado en los micros del kart.** El firmware existente sigue siendo fuente de referencia sin cambios.

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

Los rangos en `server.py` solo validan representación de datos. No autorizan actuaciones ni son límites seguros del kart. Se rechazan valores no finitos, fuera de representación o booleanos donde se exige entero.

La caducidad visual inicial es 1500 ms, 3000 ms para SD y 8000 ms para batería/BMS: criterios provisionales de presentación, independientes del timeout CAN de control. Afinar después de medir frecuencias. Puerto cerrado/error de servicio hace que ninguna muestra se presente como vigente. No inferir fallo físico del micro a partir de pérdida de observabilidad.

Primer emisor: integrar pequeñas muestras ESP32 con versión, sesión y secuencia en una copia v0; cola/buffer limitado y envío no bloqueante a cadencia medida. Registrar carga y pérdidas y asegurar que diagnóstico no retrasa el control. No copiar impresiones JSON extensas al bucle crítico sin análisis.
