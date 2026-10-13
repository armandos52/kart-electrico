---
estado: implementada en copia de trabajo; validación física pendiente
fecha: 2026-10-13
---

# DEC-0008 · START/STOP por pulsación, con memoria y antirrebote

Autorización: Armando solicita mostrar soluciones e implementar la mejor; confirma pulsador físico momentáneo. [PROB-0009](../problemas/PROB-0009_start_pulsador_momentaneo.md), [ENS-0013](../../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md).

| Alternativa | Evaluación |
|---|---|
| Filtrar nivel alto/bajo, sin memoria | Evita transitorios, pero soltar seguiría parando. Insuficiente para el pulsador declarado. |
| **Memoria por pulsación y antirrebote en STM32** | Elegida: una pulsación pide marcha; liberación conserva estado; siguiente pulsación pide parada. No precisa cambiar hardware ni protocolo. |
| Pulsador enclavado o filtro eléctrico | Puede ser pertinente tras medir desgaste/ruido y circuito; no se cambian componentes ni se inventan valores RC. Un pulsador enclavado requiere otra semántica de firmware. |
| Ignorar STOP temporalmente en ESP32 | Descartada: retrasaría/ocultaría una petición de parada y no arreglaría la interpretación de la liberación. |

Política STM32 `stm32_frontal-v1.3-dev3`:

- Al iniciar/reinicializar, petición OFF y pulsación desarmada; exige observar liberación estable **40 ms** antes de aceptar pulsar.
- En OFF, START exige **40 ms** continuos de nivel alto observado; cada cambio de nivel reinicia el tiempo. No se suman pulsos ni se bloquea la tarea con esperas.
- Una pulsación consumida no se repite por mantenerla. Soltar no modifica la petición; únicamente habilita otra pulsación tras **40 ms** estables en bajo.
- En ON y con la siguiente pulsación habilitada, el primer alto observado pide STOP, sin esperar otros 40 ms. Consumir esa pulsación impide que sus rebotes arranquen otra vez. Puede parar ante un pulso de ruido: se prioriza la parada frente a ignorarla.
- Tiempos de desarrollo elegidos en esta sesión, **no duración medida del rebote**. Son sobre muestras de una tarea nominal de 5 ms; latencia real GPIO→CAN→salida queda por medir. Pulsaciones/rebotes que incluyan 40 ms estables de liberación pueden reconocerse como eventos distintos.
- START sigue siendo una solicitud: ESP32 conserva requisitos N, freno, pedal libre, quietud, rearme y STOP prioritario. Si un arranque es rechazado o la precarga aborta, no se reintenta automáticamente: restablecer condiciones y dar OFF→START deliberado.

El formato CAN no cambia: bit 2 de 0x100 sigue siendo la petición mantenida, ahora memorizada por STM32. Snapshot/CSV `Start` y Wi-Fi `start_requested` son petición lógica, **no nivel eléctrico del pulsador ni estado medido del contactor**. No se modifica el programa ESP32, ni tiempos de potencia, ni GPIO, ni ADC/modos/panel.

Referencia externa: [controlador oficial de botones ST](https://github.com/STMicroelectronics/stm32-button-part-drivers/blob/main/README.md), consultado el 13-10-2026, describe antirrebote por software configurable. Sirve como referencia del mecanismo; no se incorpora esa biblioteca ni se toma de ella el umbral de 40 ms. Implementación propia sobre la tarea existente.

Límites: la memoria no mide continuidad del cable/contacto y no sustituye un circuito independiente de parada. Un reset STM32 borra la petición. El cambio necesita programar el **frontal**; cambiar solo ESP32 no aplica esta corrección.
