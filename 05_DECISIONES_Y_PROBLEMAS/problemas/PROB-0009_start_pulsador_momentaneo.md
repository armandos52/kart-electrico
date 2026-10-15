---
estado: corregido en código; frontal identificado y START frontal comprobado en placa; relé pendiente
fecha: 2026-10-13
actualizado: 2026-10-15
resumen: Relé que entra y sale al pulsar START; el frontal instalado era el original STM32-final y dev4 ya conmuta una vez por pulsación en placa.
---

# PROB-0009 · Relé entra y sale al pulsar START

Armando declara durante pruebas reales en «B1.3» que Start activa y desactiva el relé casi instantáneamente; sospecha doble pulso/contacto falso. Confirma que falla el **pulsador físico momentáneo, que vuelve al soltarlo**. No aporta identificación de ESP32/STM32, captura CAN/Wi-Fi ni medición del contacto. «B1.3» se conserva como denominación declarada, sin equipararla a un binario. La referencia física v1.2 del 11-10 queda como antecedente, no como confirmación actual.

**Hecho del código de trabajo:** STM32 dev2 transmitía directamente GPIO alto como START y bajo como STOP, sin memoria ni antirrebote (`Core/Src/kart_frontend.c`, `Kart_ControlTick`, copia previa en ENS-0013). Para el pulsador descrito, soltar genera STOP y cancela la precarga. Reproducido con funciones reales frontal→trama CAN→control ESP32 dev3; también se reproducen ciclos por rebotes sintéticos. **No confirma que ese código sea el instalado ni demuestra rebote eléctrico real.**

[DEC-0008](../decisiones/DEC-0008_start_momentaneo_con_memoria.md) adopta memoria y antirrebote en origen, STM32 dev3. [ENS-0013](../../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md): 19 fallos antes → 0 después sobre 83 comprobaciones; 56 del trasero conservadas; compilación ARM correcta. Fuentes, logs, trazas sintéticas, HEX/ELF y hashes enlazados.

**Actualización 15-10 · [ENS-0019](../../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md):**

- **Frontal:** la F746 conectada ejecutaba el firmware **original de la entrega** (`STM32-final`, coincidencia byte a byte en interna y QSPI), no dev2. La hipótesis basada en dev2 no describe lo instalado; habría que revisar el código original o repetir la prueba con dev4.
- **dev4 grabado y verificado:** tres pulsaciones de START dieron 0→1→0→1 en el frontal, una conmutación por pulsación con muestreo de ~50 ms.
- **ESP32:** nunca emitió 0x124. La hipótesis es un ESP32 v1.2 o anterior, no v1.3; está pendiente de identificar por Wi-Fi.
- **Sin comprobar:** el relé no se observó y no hay medida de rebote.

Pendiente: identificar el firmware del ESP32 (frontal ya respaldado e identificado); probar pulso/soltar/STOP por Wi-Fi con condiciones registradas. Si la orden permanece estable y el relé cae, contrastar reset, CAN, alimentación y contactos: [PROB-0002](PROB-0002_esp32_no_activa_contactor_al_encender.md), [PROB-0003](PROB-0003_energia_residual_al_abrir_contactor.md). Ruido/contacto gastado siguen siendo hipótesis. Sin declaración de resolución física.
