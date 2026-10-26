# ENS-0016 · Vistas del frontal adaptadas a Nucleo F411

Fecha: 14-10-2026. Continuación de [ENS-0015](ENS-0015_nucleo_f411_pantalla_banco.md). Banco aislado, con CPU/pantalla/B1 físicos y entradas del kart sintéticas. No ensayo de marcha del vehículo.

## Objetivo y configuración

Armando solicita adaptar las vistas ya desarrolladas del frontal, recorrerlas con el botón azul y conservar capturas para el relatório. La aplicación F746 de trabajo contiene una vista principal `Screen1View`, con variantes de estado; no diez pantallas TouchGFX independientes. Se preparan diez escenas de esa información para el LCD más pequeño.

NUCLEO-F411RE, STM32F411RET6, ST-LINK `0672FF564881774867244057`, COM5 a 115200 bit/s. LCD MCUFRIEND UNO, ID leído `0x9341`/ILI9341, 320×240 horizontal, rotación 1. Conexión declarada por Armando: pantalla montada y un único USB al portátil. B1 azul: PC13 activo bajo; sin táctil ni actuadores. CubeProgrammer observa alimentación de 3,24–3,25 V durante conexión; no medición independiente de estabilidad de la fuente.

Herramientas y dependencias fijadas conservadas de ENS-0015: PlatformIO 6.2.0, ststm32 20.0.0, STM32duino 3.0.0, ARM GCC 12.3.1, MCUFRIEND_kbv 3.1.0-Beta, Adafruit GFX 1.12.6 y BusIO 1.17.4; grabación por CubeProgrammer CLI 2.23.0. Sin nuevas instalaciones ni cambios de drivers.

## Cambio y relación con el frontal

- [Screen1View.cpp](../../07_DESARROLLO/firmware/stm32_frontal/TouchGFX/gui/src/screen1_screen/Screen1View.cpp): se conserva el criterio de validez de velocidad, marcha N/D/R, modo solicitado, freno y estado de grabación. Se adapta la composición de 480×272 a 320×240 con Adafruit GFX, sin portar TouchGFX ni copiar sus imágenes/fuentes.
- [KartStatus.cpp](../../07_DESARROLLO/firmware/stm32_frontal/TouchGFX/gui/src/common/KartStatus.cpp) y [kart_display.c](../../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_display.c): se reutiliza `Kart_FormatDisplay` para modos solicitado/recibido/aplicado, motivo, CAN/MPU/freno, BMS y SD. El banco sustituye únicamente su última línea por `F411 V0.3 / CORE DEV3 / SD FILAS:...`; el literal DEV2 obsoleto del frontal queda pendiente en su proyecto.
- [bench_logic.cpp](../../07_DESARROLLO/firmware/nucleo_f411_banco/src/bench_logic.cpp): inyección de tramas sintéticas al decodificador real del frontal y uso de `Kart_ControlTick`, `Kart_RequestNextMode`, `Kart_ToggleRecording` y `Kart_SdTick` con HAL/FATFS simulados. D/R se solicita parado con freno y pedal libre antes de introducir movimiento sintético. La pérdida de CAN se produce avanzando 501 ms el reloj simulado; el error SD procede del retorno de fallo del montaje simulado.
- [main.cpp](../../07_DESARROLLO/firmware/nucleo_f411_banco/src/main.cpp): dibuja, lee B1 y captura GRAM. B1 avanza al soltar tras estabilidad de 40 ms en pulsación y liberación; mantenerlo no repite, y al arrancar se exige soltar. Después de la escena 10 vuelve a la 1. START mostrado pertenece al caso sintético; B1 ahora navega.

Los fuentes de producción F746 y ESP32 permanecen sin cambios. Se ejecutan también las 83 comprobaciones portables previas de frontal/integración trasera. Las escenas visuales inyectan estados de ejemplo del trasero: no demuestran que el controlador ESP32 produzca esos valores en una dinámica física.

## Escenas

| Nº | Escena | Condición sintética / resultado esperado |
|---|---|---|
| 01 | Reposo | N, 0 km/h, START OFF, freno, APL:--- |
| 02 | Precarga | N, START ON, preparando relés virtuales, APL:--- |
| 03 | Marcha DIF | D, 24 km/h, DIF solicitado/recibido/aplicado, una fila SD simulada |
| 04 | Marcha BLQ | D, 18 km/h, BLQ solicitado/recibido/aplicado |
| 05 | Marcha ESC | D, 22 km/h, ESC solicitado/recibido/aplicado |
| 06 | Límite reversa | R, 20 km/h, motivo `LIMITE R 20 KM/H` |
| 07 | Frenado | D, 0 km/h, freno activo y motivo FRENO |
| 08 | Pérdida CAN | Feedback y estado caducados, velocidad `--`, N, REC/APL `---` |
| 09 | Error SD | D, 24 km/h, montaje FATFS simulado falla: `SD:ERROR E:1` |
| 10 | Cambio rechazado | Pedir otro modo con D: aviso `MODO: PONER N`, DIF conservado |

BMS permanece `--` en todas las vistas. Las velocidades, RPM, guiñada, tiempos de control y fila SD son datos sintéticos, no medidas. Las escenas son instantáneas reproducibles, no una simulación dinámica continua. El rótulo de simulación es permanente.

## Procedimiento y evidencia

1. Consultar estado/catálogo/notas y código de trabajo de Screen1View, base generada, KartStatus, frontend, formato de pantalla y pruebas. No ampliar la lectura integral de originales históricos.
2. Comprobar por HOTPLUG el mismo ST-LINK/MCU. Mantener el respaldo triple original y banco v0.2 de ENS-0015.
3. Compilar el banco v0.3, conservar fuentes/BIN/ELF/mapa, grabar en `0x08000000` con selección explícita de ST-LINK, verificar y reiniciar. No cambiar option bytes ni programar placas del kart.
4. Registrar arranque: identificación LCD, tres colores escritos/leídos y pruebas. El registro de la segunda compilación incluye navegación B1 física.
5. Seleccionar cada escena por UART y solicitar `C`. Leer GRAM del ILI9341 por filas de 320 píxeles RGB565 (640 B de RAM), transmitir al PC y validar tamaño/CRC32. En la versión final cada fila requiere confirmación `A`, con timeout de 10 s; la escena queda fija y B1 no actúa durante la transferencia. El puerto se cierra al finalizar.
6. Conservar bytes RX, RGB565 de cada escena, PNG nativo y ampliación 3× sin interpolación, metadatos, galería y hashes. Estas capturas proceden de memoria gráfica física; no son renders fabricados en PC ni fotografías del cristal.

[Evidencia completa](../evidencias/nucleo_vistas_2026-10-14/) · [galería con pies de figura](../evidencias/nucleo_vistas_2026-10-14/GALERIA.md) · [resultado estructurado](../evidencias/nucleo_vistas_2026-10-14/resultado.json).

## Resultados e incidencias

- Compilación final: flash de programa 56.264 B; RAM estática 2.232 B. BIN almacenado 56.724 B. No se presenta ese valor estático como consumo máximo de pila/heap.
- **137 comprobaciones sin fallos ejecutadas en F411:** 83 previas y 54 nuevas de escenas, validez e interacción B1 con señales/tiempo simulados. RGB escrito/leído 3/3. [Arranque final](../evidencias/nucleo_vistas_2026-10-14/arranque_v03_02.log).
- Navegación B1 real registrada al menos 01→02→03→04→05→06 en el arranque final; no se atribuyen pulsaciones posteriores sin registro ni se mide rebote eléctrico/latencia. La escucha adicional de 120 s al terminar no recibió nuevas pulsaciones. Después, Armando confirma para v0.3: **«Sí, se leen bien y el recorrido funciona»**, al preguntar por las diez escenas, vuelta 10→01 y ausencia de saltos múltiples por pulsación. [Respuesta conservada](../evidencias/nucleo_vistas_2026-10-14/confirmacion_visual_usuario.json). Se cierra la aceptación visual/manual declarada; no se añade un registro serie de ese recorrido ni una medición temporal.
- Primera captura individual completa, CRC correcto. Un lote posterior quedó incompleto: 124.513 B recogidos en lugar de 153.600 y texto de nuevo arranque dentro del flujo. No produjo PNG aceptado. Armando confirma que pulsó RESET o desconectó USB durante esos minutos; su respuesta no distingue cuál de las dos acciones. La hipótesis inicial de pérdida espontánea de bytes USB queda sin acreditar; el reinicio está registrado. Se añade confirmación por fila como mejora de transporte, no como reparación demostrada de un fallo espontáneo del MCU.
- Fallo inicial de compilación por permiso sobre `platforms.lock`, resuelto al ejecutar con acceso al caché existente; no defecto del firmware. Un intento de parche con operaciones duplicadas sobre `main.cpp` fue rechazado sin cambios y reaplicado correctamente. Un sondeo de proceso ya terminado no devolvió sesión; se verificó la finalización mediante el log guardado.
- **Diez capturas finales completas, 153.600 B por escena y CRC32 correcto en las diez**, sin nuevo arranque/aborto dentro de la transferencia. La escena DIF capturada en las dos compilaciones es idéntica byte a byte. Galería revisada visualmente: diez títulos/valores/avisos visibles sin recortes; comprobación ampliada de CAN caducado con velocidad `--`. [Verificación](../evidencias/nucleo_vistas_2026-10-14/verificacion_capturas.json), [manifiesto](../evidencias/nucleo_vistas_2026-10-14/capturas_finales/manifest.json). Se generaron también PNG de 960×720 por repetición de píxeles, sin añadir detalle.

## Identidad y conservación

Base Git `43b0c299af47e30aabcf9b5acd7677b86754a00b`, trabajo local sin nuevo commit/etiqueta/push. Dos compilaciones de banco v0.3 conservadas; identidad exacta por binario y SHA-256:

| Binario | SHA-256 | Estado |
|---|---|---|
| `vistas_v0.3_01.bin` | `037625f5775685604eaf384db2bf4bce34e5e6dee4246d0669caa9068e9d1255` | Primer intento, captura sin confirmación por fila; sustituido |
| `vistas_v0.3_02.bin` | `fbf86237a4888b81c80508a69d8648d6ae25fa0596848108d972be4f159031a1` | Final, grabado y verificado; captura ROWACK |

[BIN/ELF locales](../../07_DESARROLLO/respaldos/nucleo_f411/2026-10-14/) · [fuentes compilación 01](../evidencias/nucleo_vistas_2026-10-14/compilacion_01/) · [fuentes compilación 02](../evidencias/nucleo_vistas_2026-10-14/compilacion_02/) · [hashes de evidencia](../evidencias/nucleo_vistas_2026-10-14/hashes_evidencia.json). El respaldo previo y v0.2 no se sobrescriben; no hay copia externa nueva.

## Límites y siguiente paso

Validación de representación y funciones portables en F411. No valida CAN eléctrico, sensores, SD real, BMS, relés, motores, límite físico de reversa, ESC ni temporización bajo carga real. La captura GRAM no mide brillo, colorimetría, ángulo de visión, refresco ni defectos del cristal. La interfaz F746 conserva su tamaño, fuentes y recursos originales: se ha hecho una adaptación visual, no una equivalencia píxel a píxel.

Legibilidad y recorrido aceptados por Armando el 14-10. Siguiente posible ampliación: escenarios de transición o ajustes del frontal de trabajo según nuevas necesidades. El ensayo integrado mantiene la necesidad de identificar/resguardar F746+QSPI, calibrar sensores y registrar por Wi-Fi; USB del ESP32 solo fuera del kart.
