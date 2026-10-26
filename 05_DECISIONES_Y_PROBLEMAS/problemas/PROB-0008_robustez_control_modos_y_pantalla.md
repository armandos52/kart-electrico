---
estado: abierto; correcciones implementadas, contraste físico pendiente
actualizado: 2026-10-17
resumen: Modos/pantalla poco funcionales declarados por Armando; auditoría v1.2 y frontal con defectos de estados, rampa, ESC y SD.
---

# PROB-0008 · Robustez del control, modos y pantalla

## Continuación 17-10 · reversa y arquitectura

[PROB-0014](PROB-0014_reversa_y_arquitectura_diferencial.md) y [ENS-0025](../../06_TRABAJO/ensayos/ENS-0025_revision_diferencial_y_reversa.md) registran ≈50 km/h indicados en R en todos los modos, banco elevado según Armando. V1.2 sin límite reproducida en PC; dev3 sí corta pero presenta dos hallazgos de rampa/histéresis pendientes. ESP32 instalado sin identificar. [Plan](../../06_TRABAJO/REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md) antes de implementar por última indicación del usuario; no cambios de firmware en esta sesión. Ruido declarado separado en [PROB-0015](PROB-0015_ruido_electromagnetico_sensores.md). Las actualizaciones siguientes conservan su fecha y alcance histórico.

## Actualización 11-10 · STOP en lote CAN

Armando confirma ESP32 físico v1.2 sin cambios, frontal todavía inaccesible y prioridad control/modos. [ENS-0011](../../06_TRABAJO/ensayos/ENS-0011_stop_en_lote_can.md) amplía A01: en dev2 un STOP seguido de START antes de `step` se perdía. Cuatro fallos reproducidos; dev3 conserva el STOP y descarta aquel START como habilitación de arranque. 56 comprobaciones sin fallos y compilación ESP32 correcta; solo PC. No atribuir este hallazgo a un fallo físico observado. STM32 dev2 y límites físicos de ESC/R/calibración permanecen; PROB-0008 sigue abierto.

Armando reconfirma prueba positiva de motores con v1.2 y solicita investigar qué código sirve antes de modificarlo. Declara que los modos o la pantalla prácticamente no funcionan; acceso/observación detallada pendientes. El buen resultado de motores de [ENS-0004](../../06_TRABAJO/ensayos/ENS-0004_prueba_real_motores_confirmada.md) se conserva.

[Revisión completa](../../06_TRABAJO/REVISION_CODIGO_V12_2026-10-08.md): ESP32 v1.2 `905b09e`, sin diferencias en fuentes de control/telemetría respecto a la referencia, y fuente STM32 2025/26, cuya correspondencia con lo grabado no se ha verificado.

## Índice de hallazgos

| ID | Hallazgo | Evidencia y estado |
|---|---|---|
| A01 | STOP temprano ignorado y flanco perdido | Reproducido en host; E:466–474 |
| A02 | Recuperación de CAN permite acelerar sin rearme | Reproducido en host; E:394–403 |
| A03 | ADC inválido/antiguo y contenido CAN sin validación suficiente | STM32 estático; cuatro casos CAN reproducidos |
| A04 | Escritura de PWM anterior a permiso final; apagado por cargador no anula PWM inmediatamente | Reproducido en host; sin pulso físico medido |
| A05 | Rampa calculada sin efecto | Reproducido en host; E:536–564 |
| A06 | ESC asimétrico y sensor solo validado al inicio | Espejo reproducido y lectura estática; sensor físico pendiente |
| A07 | RPM con divisor temporal fijo y velocidad sin frescura/validez | Error temporal reproducido; riesgo de permisos pendiente de banco |
| A08 | Supervisión, manejo de errores y diagnóstico CAN incompletos | Estático; AutoBusOff STM32 sí está habilitado |
| A09 | SD Play/Pausa sin reapertura y acceso desde varias tareas | Estático; frontal instalado sin identificar |
| A10 | GUI con varias rutas de refresco, bloqueo silencioso de modo y sin acuse | Estático; causa del síntoma real pendiente |
| A11 | Cola de telemetría nula puede alcanzar publicación directa en arranque | Estático; fallo de memoria no inyectado |
| A12 | Constantes/restos sin uso y código fuera de USER CODE | Estático; limpieza propuesta después de fijar comportamiento |

Datos, hashes, condiciones y limitaciones de las 19 comprobaciones en [evidencia](../../06_TRABAJO/evidencias/AUD_V12_2026-10-08/README.md). No es ensayo de seguridad superado ni prueba de conducción. No se atribuye a estos defectos la regresión v1.1 de [PROB-0006](PROB-0006_regresion_marcha_con_firmware_telemetria.md), la franja de [PROB-0004](PROB-0004_franja_blanca_pantalla_frontal.md) ni la variación de [PROB-0007](PROB-0007_variacion_y_centro_direccion.md).

## Actualización 09-10 · implementación autorizada

Armando autorizó posteriormente todas las propuestas y R a 20 km/h, sin esperar acceso al frontal. A01–A12 implementados en `v1.3-dev2`, con compilación y comprobaciones de software; [ENS-0009](../../06_TRABAJO/ensayos/ENS-0009_robustez_firmware_v13.md), [seguimiento](../../06_TRABAJO/IMPLEMENTACION_ROBUSTEZ_V13.md) y [DEC-0006](../decisiones/DEC-0006_robustez_v13_y_limite_marcha_atras.md). Los rangos E/T de la tabla corresponden a copias v1.2 conservadas en la auditoría, no al fuente nuevo.

Siguiente: identificar y respaldar frontal+QSPI; calibración y ensayo físico comparativo. La causa real del síntoma de pantalla sigue abierta. No se grabó firmware, no se hizo simulación dinámica y no se promete velocidad física máxima ni estabilidad validada. USB del ESP32 solo fuera del kart; prueba integrada registrada por Wi-Fi.
