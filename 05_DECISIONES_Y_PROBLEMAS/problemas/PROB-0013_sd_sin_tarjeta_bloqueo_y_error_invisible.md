---
estado: abierto; correcciones parciales instaladas, ensayo SD y latencia del driver pendientes
actualizado: 2026-10-22
resumen: Sin microSD, cada intento de montaje bloquea ~30 s y el estado ERROR dura ~50 ms, así que la pantalla muestra casi siempre SD:ABRE E:0.
---

# PROB-0013 · SD sin tarjeta: intento bloqueante y error casi invisible

**23-10:** [ENS-0029](../../06_TRABAJO/ensayos/ENS-0029_instalacion_historial_sd.md), dev12-log instalada. Detección PC13 y DMA con búfer/cache dedicados; fallo real dev11 TX_UNDERRUN corregido. Escritura/cierre de sesiones reales sin pérdidas/error SD, máximo acumulado observado 203 ms por llamada. Retirada/reinserción y ausencia física todavía pendientes: no cerrar el problema ni extrapolar esta latencia a todos los fallos de tarjeta.

**22-10:** Armando declara memoria disponible/instalada y pide plan para historial/replay. Confirma microSD en la STM32 de la pantalla; capacidad/formato pendientes, todavía sin acreditar escritura real. [Plan](../../06_TRABAJO/PLAN_HISTORIAL_SD_Y_PANTALLA_2026-10-22.md) prioriza detección/latencias/cola y ensayo con tarjeta antes de considerar fiable el historial. Fuente revisada dev10-ui, última instalación verificada dev9 (ENS-0026/0027). Sin nuevo ensayo físico; el fallo medido abajo corresponde a dev4.

Observado en [ENS-0019](../../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md) con stm32_frontal-v1.3-dev4 en la F746 real, sin microSD (declaración de Armando) y grabación solicitada.

**Hecho medido:** ciclo repetido de ABRIENDO (~30 s) → ERROR código 1 (`FR_DISK_ERR`) durante ~50 ms → ABRIENDO de nuevo. Registros por SWD 13:33–16:39. La pantalla muestra casi todo el tiempo `SD:ABRE E:0`. Armando sí interpretó que «la SD no está», pero el código de error apenas aparece.

**Causa en el código:** en `Kart_SdTick` ([kart_frontend.c](../../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c)), `sd_last_try` se fija antes de `f_mount`. Como el montaje bloquea ~30 s sin tarjeta, la espera de 1 s entre intentos ya está cumplida al volver y se reintenta enseguida. El «reintento espaciado 1 s» del README no se cumple en este caso, y una petición de pausa puede esperar hasta 30 s.

**Impacto observado:** ninguno sobre el control. El tick, el watchdog y las tramas CAN siguieron normales durante más de 3 h, porque la tarea SD es de prioridad baja. Es un problema de diagnóstico visible y de latencia de la grabación.

**Posibles tratamientos, por decidir:** usar la detección de tarjeta de la Discovery, medir la espera desde el final del intento o mantener visible el último error durante el reintento. Cualquier cambio requiere pruebas en PC y repetir el ensayo con y sin tarjeta. No se ha probado todavía la grabación con tarjeta.

## Continuación local · dev5, 15-10-2026

[ENS-0021](../../06_TRABAJO/ensayos/ENS-0021_sd_reintentos_locales.md) adopta la espera desde el final del fallo y su limpieza, conserva el último error durante `ABRE` y atiende pausa tras montaje/apertura/cabecera antes de continuar escribiendo. 120 comprobaciones en PC: 16 fallos antes → 0 después; ARM compilado, sin grabar. Dev4 continúa instalado.

**No cerrado:** `f_mount` sigue siendo bloqueante y la pausa puede esperar a que retorne. `BSP_SD_IsDetected` del driver local devuelve siempre `SD_PRESENT`; la detección física no está implementada ni comprobada. Pendiente decidir ese tratamiento y probar dev5 en F746 con/sin tarjeta, tiempos, pausa, reinserción y CSV real. `E` durante `ABRE` representa el último error, no un nuevo diagnóstico de tarjeta ausente.

16-10: dev6-ui conserva las correcciones dev5 y añade página SD con petición, estado, filas y último error ([ENS-0022](../../06_TRABAJO/ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md)). «Pedir pausa» expresa una solicitud, no una parada inmediata del driver. Probado en PC, sin nueva grabación; el ensayo físico pendiente puede realizarse ya sobre dev6-ui. Problema abierto por latencia/detección.

16-10, [ENS-0023](../../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md): dev7-ui grabada y verificada, incluye estas correcciones y rechazo conservador de CSV parcial/incompatible (error 128, archivo preservado). Regresiones positivas y negativas documentadas. En las capturas reales SD está en pausa, sin escritura con tarjeta; no se cierra latencia ni detección.
