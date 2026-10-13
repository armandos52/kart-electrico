---
estado: corregido en código; identificación y validación física pendientes
fecha: 2026-10-13
---

# PROB-0009 · Relé entra y sale al pulsar START

Armando declara durante pruebas reales en «B1.3» que Start activa y desactiva el relé casi instantáneamente; sospecha doble pulso/contacto falso. Confirma que falla el **pulsador físico momentáneo, que vuelve al soltarlo**. No aporta identificación de ESP32/STM32, captura CAN/Wi-Fi ni medición del contacto. «B1.3» se conserva como denominación declarada, sin equipararla a un binario. La referencia física v1.2 del 11-10 queda como antecedente, no como confirmación actual.

**Hecho del código de trabajo:** STM32 dev2 transmitía directamente GPIO alto como START y bajo como STOP, sin memoria ni antirrebote (`Core/Src/kart_frontend.c`, `Kart_ControlTick`, copia previa en ENS-0013). Para el pulsador descrito, soltar genera STOP y cancela la precarga. Reproducido con funciones reales frontal→trama CAN→control ESP32 dev3; también se reproducen ciclos por rebotes sintéticos. **No confirma que ese código sea el instalado ni demuestra rebote eléctrico real.**

[DEC-0008](../decisiones/DEC-0008_start_momentaneo_con_memoria.md) adopta memoria y antirrebote en origen, STM32 dev3. [ENS-0013](../../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md): 19 fallos antes → 0 después sobre 83 comprobaciones; 56 del trasero conservadas; compilación ARM correcta. Fuentes, logs, trazas sintéticas, HEX/ELF y hashes enlazados.

Pendiente: identificar ambos firmwares y respaldar frontal/QSPI antes de elegir grabación; probar pulso/soltar/STOP por Wi-Fi con condiciones registradas. Si la orden permanece estable y el relé cae, contrastar reset, CAN, alimentación y contactos: [PROB-0002](PROB-0002_esp32_no_activa_contactor_al_encender.md), [PROB-0003](PROB-0003_energia_residual_al_abrir_contactor.md). Ruido/contacto gastado siguen siendo hipótesis. Sin declaración de resolución física.
