---
estado: cerrado para F01–F03; corregido en dev4 y comprobado en la F746 real
actualizado: 2026-10-15
resumen: Versión visible obsoleta, umbral R no condicionado y prioridad de avisos en frontal dev3; corregidos en dev4 y validados en hardware (ENS-0019).
---

# PROB-0011 · Presentación del frontal antes de instalar

**Cierre 15-10-2026.** dev4 ([ENS-0018](../../06_TRABAJO/ensayos/ENS-0018_mejoras_pantalla_desarrollo.md)) se grabó y verificó en la F746 real ([ENS-0019](../../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md)). En el framebuffer leído por SWD, con la lectura de Armando en el panel:

- **F01:** `STM V1.3-DEV4`.
- **F02:** `R:--` con un ESP32 que no envía 0x124, sin cifra de umbral.
- **F03:** `MODO: PONER N` y `MODO: PARAR Y SOLTAR PEDAL` en la quinta fila, con la causa `ESP SIN ESTADO RECIENTE` y el estado SD visibles.

No se observaron `R:LIM`/`R:INACT`, porque no hubo un ESP32 con 0x124. Los problemas nuevos de banco se registran aparte: [PROB-0012](PROB-0012_pedal_reposo_fuera_de_pedal_libre.md) y [PROB-0013](PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md).

## Revisión original (dev3)

Revisión solicitada por Armando del STM32F746/TouchGFX **v1.3-dev3** de trabajo. [ENS-0017](../../06_TRABAJO/ensayos/ENS-0017_revision_preinstalacion_frontal.md) reúne alcance, configuración, procedimiento, resultados, fuentes/hashes y preparación para instalar después. Solo PC, sin lectura ni grabación de placas.

| Hallazgo | Fuente concreta | Consecuencia y cierre requerido |
|---|---|---|
| F01, P2: se muestra `STM V1.3-DEV2` con cabecera dev3 | [kart_display.c](../../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_display.c), línea 18 | Confunde identificación; usar una única fuente de versión para paquete/CSV/UI y comprobar texto/glyphs/anchura. Antecedente ya registrado en ENS-0015 |
| F02, P2: `R:20KM/H` siempre, incluso ESP32 legado sin estado extendido | Mismo archivo, línea 14 | Sugiere una prestación no confirmada en la pareja instalada; separar objetivo, capacidad y actuación. 0x124 no transporta umbral numérico; no prometer un límite físico |
| F03, P2: aviso de modo reemplaza indiscriminadamente la causa de inhibición | Mismo archivo, líneas 11–12; `kart_frontend.c` conserva aviso hasta 3 s | Caso reproducido: rechazo de modo en D seguido de cargador conectado; pantalla sigue `MODO: PONER N`. Priorizar causas pertinentes o separar mensajes; comprobar además pérdida CAN y fallos |

[Arnés y datos originales](../../06_TRABAJO/evidencias/revision_frontal_2026-10-15/README.md): cuatro comprobaciones nuevas, tres fallos de aceptación y un control positivo. Los 83 tests frontales y 56 traseros existentes pasan; compilación ARM correcta y binarios idénticos a dev3 preservado. Son huecos de cobertura de presentación, no tres fallos de compilación ni prueba de pérdida del corte de tracción.

Sin correcciones aplicadas en esta revisión. No confundir estos casos con la franja blanca física ([PROB-0004](PROB-0004_franja_blanca_pantalla_frontal.md)), el comportamiento de motores/modos declarado ([PROB-0008](PROB-0008_robustez_control_modos_y_pantalla.md)) o el relé observado ([PROB-0009](PROB-0009_start_pulsador_momentaneo.md)); no hay causalidad física acreditada.
