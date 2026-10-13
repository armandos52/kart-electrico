---
estado: revisión focalizada y continuación de control
actualizado: 2026-10-11
resumen: Punto de partida físico, estado del software y continuación DIF/BLQ/ESC sin acceso al frontal.
---

# Retoma del programa · 11-10-2026

## Punto de partida confirmado

Armando declara hoy que no ha tocado físicamente el kart, el ESP32 sigue en v1.2 y todavía no ha accedido a la tarjeta/pantalla STM32. Quiere continuar programando la copia frontal y revisar sus pantallas/funciones con el material disponible. Elige como prioridad control del kart y DIF/BLQ/ESC. La dificultad para leer el frontal no tiene causa diagnosticada ni código de error aportado; no se atribuye a protección, cableado o firmware sin evidencia.

| Parte | Estado comprobado en esta sesión | Límite |
|---|---|---|
| ESP32 instalado | v1.2 según declaración de Armando; referencia `905b09e` | Sin lectura de placa ni nueva captura Wi-Fi |
| ESP32 en desarrollo | Base v1.3-dev2; corrección STOP v1.3-dev3 en [ENS-0011](ensayos/ENS-0011_stop_en_lote_can.md) | No grabado ni ensayado físicamente |
| STM32 de trabajo | v1.3-dev2; ADC/CAN/control, SD y textos; 54 comprobaciones repetidas, sin fallos | Copia histórica; firmware instalado y QSPI sin identificar/resguardar |
| Panel portátil | `0.3.4-velocidad-real`, compatible con telemetría v1.2/v1.3; 26 tests Python y 16 JS repetidos, satisfactorios | No se comprueba disponibilidad de una instancia HTTP ni hay captura del kart en esta sesión |
| Git | Rama `codex/robustez-v13`, base HEAD `4f7f150`; firmware dev2 en `8a60a6e` | Numerosos cambios locales anteriores del panel/documentación preservados; dev3 sin commit/push |

La implementación A01–A12 figura en [seguimiento v1.3](IMPLEMENTACION_ROBUSTEZ_V13.md). El último trabajo visual fue la animación de ruedas de [ENS-0010](ensayos/ENS-0010_velocidad_visual_ruedas.md), pendiente de valoración perceptiva por Armando. Que las pruebas anteriores pasaran no demuestra ausencia de defectos: esta revisión encuentra un caso STOP no cubierto.

## Qué hacen ahora los modos

Fuente: `esp32_trasero/lib/kart_control/kart_control.cpp`, funciones `steeringDegrees` y `Controller::step`; parámetros en `kart_control.h`; decisiones vigentes en [DEC-0006](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0006_robustez_v13_y_limite_marcha_atras.md).

| Modo | Comportamiento implementado | Pendiente real |
|---|---|---|
| DIF | Reparto abierto de consigna izquierda/derecha según dirección y geometría de referencia | Centro/topes/ángulos, geometría, respuesta de cada motor |
| BLQ | Igual consigna con las ganancias actuales 1/1; el volante no introduce reparto | No garantiza igual RPM, par medido ni bloqueo mecánico; comprobar simetría real |
| ESC | Reparto DIF más corrección limitada por error entre guiñada esperada y medida; cálculo firmado en D/R | Eje/signo/escala MPU, ganancias y estabilidad física. Continúa experimental |

Protecciones comunes ya implementadas: rampa de subida, corte inmediato de consigna por parada/fallo, validación CAN y rearme deliberado; cambio de modo en N, parado y pedal libre; selección de D/R con freno, pedal libre y quietud. El límite R usa la rueda estimada más rápida y los parámetros históricos: reduce consigna entre 18–20 km/h, corta a 20 y permite recuperación al llegar a 19 km/h o menos. No frena eléctricamente ni acredita un máximo físico en pendiente.

Los umbrales y parámetros proceden del código de desarrollo y de decisiones anteriores; no son mediciones nuevas. No se han cambiado PPR, geometría, ganancia ESC, rampa ni límite R en esta sesión.

## Pantalla que ya se puede desarrollar sin placa

`stm32_frontal/Core/Src/kart_display.c` y la View propia ya preparan velocidad/marcha, selector DIF/ESC/BLQ, Play/Pausa SD, modo solicitado/recibido/aplicado, motivo de inhibición, CAN/MPU/freno y estado de SD/BMS. Se revisó ese recorrido y se repitieron las pruebas de textos. No se ejecutó TouchGFX Simulator ni se comprobó visualmente el LCD: el formato de las cadenas no acredita legibilidad, táctil o ausencia de solapamientos.

Para la siguiente iteración de control, la prioridad propuesta es probar en PC el recorrido conjunto **frontal → CAN → control → estado → pantalla** para los tres modos y recuperar fallos. Son pruebas del contrato de software, sin modelo dinámico del vehículo. Después, revisar una vista previa de estados normales y bloqueados con datos explícitamente sintéticos. No es necesario esperar acceso a la placa para ese trabajo; identificar/resguardar frontal+QSPI sí sigue pendiente antes de grabarlo.

No hace falta volver a decidir R a 20 km/h, BLQ por consigna ni la calibración del PC separada del control: ya están fijados. Los valores físicos y las ganancias ESC se decidirán con evidencia de banco/kart. Un cambio futuro del significado de BLQ o la incorporación de control cerrado de RPM constituirían requisitos nuevos.

## Cobertura y evidencias

Lectura focalizada de estado, tareas, índice/ficha DOC-0042, inventario, plan de lectura, ENS-0009/0010, contrato, núcleo y adaptación ESP32, control/textos/pruebas STM32 y QA del panel. No se abrieron nuevos originales ni páginas PDF; L1/L4 siguen sin cierre integral. Se conservan los logs y las fuentes previas en [REV_2026-10-11](evidencias/REV_2026-10-11/), enlazados desde [ENS-0011](ensayos/ENS-0011_stop_en_lote_can.md).

Los QA antiguos `check-calibration.cjs` y `check-panel.cjs` siguen pendientes de actualización; no se han presentado como pasados hoy. Las tareas de crear v0/respaldo inicial y obtener autorización de v1.3 eran restos desactualizados: se corrigen sus rótulos con enlaces a resultados existentes, sin atribuir nuevas acciones físicas.
