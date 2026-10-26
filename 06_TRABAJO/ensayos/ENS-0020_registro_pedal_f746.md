# ENS-0020 · Registro dedicado del pedal F746

Estado al 15-10-2026: **preparado; sin muestras del pedal**. Asociado a [PROB-0012](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0012_pedal_reposo_fuera_de_pedal_libre.md).

La sesión anterior dejó [registrar_pedal_swd.py](../../07_DESARROLLO/firmware/stm32_frontal/tools/registrar_pedal_swd.py) y un intento técnico en [prueba_tecnica](../evidencias/pedal_f746_2026-10-15/prueba_tecnica/). Su [openocd.log](../evidencias/pedal_f746_2026-10-15/prueba_tecnica/openocd.log) termina en `Error: open failed` durante `init`. No hay `muestras.csv`, fases ni resumen de una captura. El fallo de acceso no permite diagnosticar el pedal ni distinguir por sí solo cable, placa ausente o interfaz ocupada.

En la retoma, Armando indica: **«Ahora no; continuar con código y pruebas locales»**. No se intenta conectar, grabar ni medir hardware. Las lecturas 149–194 cuentas siguen siendo las cuatro observaciones anteriores de ENS-0019, con reposo supuesto, no una calibración dedicada.

## Procedimiento pendiente

1. Documentar configuración física y estado de tracción inhibida antes de pedir un recorrido del pedal. Confirmar F746, firmware y disponibilidad del operador.
2. Usar el ELF realmente instalado: por ahora [dev4 conservado, solo local](../../07_DESARROLLO/respaldos/stm32/v1.3-dev4/stm32_frontal-v1.3-dev4.elf), no el nuevo dev5 compilado. El registrador exige `--serie` y `--elf` explícitos; observa por SWD sin detener CPU ni escribir.
3. Revisar el registrador y ejecutar el guion corto de conexión en una carpeta nueva; después reposo, apoyo, fondo repetido y recorridos lentos del guion completo. Registrar si el operador siguió cada fase: las etiquetas temporales no lo demuestran.
4. Conservar muestras físicas/normalizadas, validez, coherencia y errores; comparar distribuciones y retornos al reposo antes de proponer cambios en `ACCEL_MIN`, `ACCEL_MAX` o pedal libre.

Frecuencia objetivo del script: 20 Hz, por verificar. La herramienta está preparada, no validada con captura satisfactoria. No se cambian umbrales ni se acredita recorrido, ruido o reposo con este intento fallido. Ensayos integrados del ESP32 por Wi-Fi; USB del ESP32 solo fuera del kart.
