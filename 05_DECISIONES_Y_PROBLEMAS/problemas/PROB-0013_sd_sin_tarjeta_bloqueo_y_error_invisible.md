---
estado: abierto; observado en placa real, sin corrección
actualizado: 2026-10-15
resumen: Sin microSD, cada intento de montaje bloquea ~30 s y el estado ERROR dura ~50 ms, así que la pantalla muestra casi siempre SD:ABRE E:0.
---

# PROB-0013 · SD sin tarjeta: intento bloqueante y error casi invisible

Observado en [ENS-0019](../../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md) con stm32_frontal-v1.3-dev4 en la F746 real, sin microSD (declaración de Armando) y grabación solicitada.

**Hecho medido:** ciclo repetido de ABRIENDO (~30 s) → ERROR código 1 (`FR_DISK_ERR`) durante ~50 ms → ABRIENDO de nuevo. Registros por SWD 13:33–16:39. La pantalla muestra casi todo el tiempo `SD:ABRE E:0`. Armando sí interpretó que «la SD no está», pero el código de error apenas aparece.

**Causa en el código:** en `Kart_SdTick` ([kart_frontend.c](../../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c)), `sd_last_try` se fija antes de `f_mount`. Como el montaje bloquea ~30 s sin tarjeta, la espera de 1 s entre intentos ya está cumplida al volver y se reintenta enseguida. El «reintento espaciado 1 s» del README no se cumple en este caso, y una petición de pausa puede esperar hasta 30 s.

**Impacto observado:** ninguno sobre el control. El tick, el watchdog y las tramas CAN siguieron normales durante más de 3 h, porque la tarea SD es de prioridad baja. Es un problema de diagnóstico visible y de latencia de la grabación.

**Posibles tratamientos, por decidir:** usar la detección de tarjeta de la Discovery, medir la espera desde el final del intento o mantener visible el último error durante el reintento. Cualquier cambio requiere pruebas en PC y repetir el ensayo con y sin tarjeta. No se ha probado todavía la grabación con tarjeta.
