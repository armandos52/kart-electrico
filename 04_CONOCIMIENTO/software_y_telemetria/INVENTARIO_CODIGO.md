---
estado: vigente
actualizado: 2026-10-17
resumen: Inventario del código entregado por sistema y versión (Arduino 23/24, STM32 24/25 y 25/26, ESP32 25/26), su estructura y la lista de mejoras candidatas para analizar.
---

# Inventario del código del kart

**Actualización focalizada 17-10-2026:** [revisión de diferencial/reversa](../../06_TRABAJO/REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md) contrasta v1.2/dev3 y frontend dev8-ui. F746 dev8-ui instalada y verificada en ENS-0024; ESP32 físico aún sin identificar. Supervisor de R existe en dev3, no en v1.2; rampa e histéresis con nuevos casos pendientes. [ENS-0025](../../06_TRABAJO/ensayos/ENS-0025_revision_diferencial_y_reversa.md). Este inventario conserva abajo el corte histórico: sus afirmaciones de frontal sin identificar y control original no describen el estado actual. Revisión integral de sus secciones sigue pendiente.

**Vigencia:** el cuerpo de este inventario describe la entrega y la revisión del 30-09. Para el ESP32 de trabajo v1.2 y sus funciones efectivas, consultar la [revisión del 08-10](../../06_TRABAJO/REVISION_CODIGO_V12_2026-10-08.md), con fuentes/hashes y 19 casos aislados. Desde octubre ya hay dependencias fijadas, compilación/grabación y telemetría con motivo de reset; «no se ha compilado» y «faltan librerías» se conservan como situación histórica. El binario frontal instalado sigue sin identificar. Se precisan abajo corte por freno, watchdogs, bus-off y significado de ceros para no perpetuar conclusiones incorrectas.

## Resumen

- En la entrega hay **tres programas propios**. Todo lo demás son librerías, código generado, compilaciones o metadatos de los entornos de programación.
  - **Controlador frontal STM32F746G-DISCO.** Hay dos versiones: 2024/25 y 2025/26, que es la última.
  - **Controlador trasero ESP32.** 2025/26, única versión.
  - **Arduino Mega.** 2023/24, única versión. Controlaba todo el kart. Su diferencial pasó después al ESP32.
- **Últimas versiones** (fecha interna del ZIP):
  - STM32: `main.c` del 10-07-2026 20:18, compilado en `.elf` a las 21:43 del mismo día **[V]**.
  - ESP32: `ESP32-final.ino` del 21-07-2026 21:42 **[V]**.
- No hay versiones paralelas del mismo programa. Los tres ZIP coinciden archivo a archivo con sus carpetas extraídas.
- No hay código fuente de 2008–2019. Los informes remiten a CD no entregados o solo traen flujogramas.
- **La tarjeta SD la escribe el STM32, no el ESP32 [V].** `KART_LOG.CSV` es del 04-07-2026, seis días antes que el `main.c` final **[V]**. Lo generó una versión anterior del STM32, que usaba `;` y grababa cada 100 ms.
- **Arquitectura 2025/26.**
  - El STM32 lee pedales, volante y botones, muestra la pantalla y graba la SD. Cada 5 ms manda una orden por CAN al ESP32.
  - El ESP32 calcula el diferencial, mueve los dos Kelly y los relés del contactor, y mide los Hall traseros y el giróscopo. Responde cada 200 ms.
- **Debilidades comunes.**
  - No hay supervisión explícita del bucle de control en el fuente propio; IWDG/WWDG del STM32 deshabilitados y `Error_Handler` vacío. No equivale a ausencia de todos los watchdogs internos del ESP32.
  - El STM32 no detecta si dejan de llegar mensajes del ESP32 o del BMS.
  - El arranque del ESP32 se bloquea si falla el CAN.
  - La rampa de par se calcula pero no se aplica.
  - Hay código de control fuera de los bloques USER CODE, que se perdería al regenerar con CubeMX.
  - El registro en SD no guarda datos de batería.
- **Marcas.** **[V]** significa comprobado por la sesión principal en el original el 30-09-2026. El resto procede de tres exploradores, con referencia de línea; hay que comprobarlo antes de citarlo en un informe. No se ha compilado ni ejecutado nada.

**Rutas abreviadas**, todas bajo `02_ORIGINALES/entrega_2026-09-28/`:
- `P25` = `Smart Eletric Kart Cross-2223406-2223403/Smart Eletric Kart Cross-2223406-2223403/`
- `P24` = `Relatórios de anos anteriores/PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos/PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos/KartProjetoFinal/`
- `P23` = `Relatórios de anos anteriores/ProjetoKart2324/ProjetoKart2324/`

**Archivos de código propio:**
- `M` = [main.c · STM32 2025/26 · solo local](../../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/Core/Src/main.c)
- `S` = [Screen1View.cpp · pantalla 2025/26 · solo local](../../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/TouchGFX/gui/src/screen1_screen/Screen1View.cpp)
- `E` = [ESP32-final.ino · 2025/26 · solo local](../../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/ESP32-final/ESP32-final/ESP32-final.ino)
- `M24` = [main.c · STM32 2024/25 · solo local](../../02_ORIGINALES/entrega_2026-09-28/Relat%C3%B3rios%20de%20anos%20anteriores/PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos/PEEC_2425_10_Smart_Eletric_Kart_Cross_Anexos/KartProjetoFinal/Core/Src/main.c)
- `K` = [CodigoKart.ino · Arduino 2023/24 · solo local](../../02_ORIGINALES/entrega_2026-09-28/Relat%C3%B3rios%20de%20anos%20anteriores/ProjetoKart2324/ProjetoKart2324/Arduino/CodigoKart/CodigoKart.ino)

Contexto: [auditoría del 29-09](../../06_TRABAJO/AUDITORIA_2026-09-29.md) (sección 2, protocolo CAN y calibraciones), [datos y proyectos](../../03_CATALOGO/DATOS_Y_PROYECTOS.md), [propuesta 2026/27](../../06_TRABAJO/INICIO_INVESTIGACION_2026-27.md).

## 1. Sistemas y versiones

| Sistema | Función en 2025/26 | Versión | Plataforma y herramientas | Código propio | Fecha interna |
|---|---|---|---|---|---|
| Controlador frontal | Mandos, pantalla, SD, recepción del BMS | **STM32 25/26** (última) | STM32F746G-DISCO. CubeMX 6.17.0, FW_F7 1.17.4 **[V .ioc]**. TouchGFX 4.26.0 **[V]**. FreeRTOS activo **[V `M`:575]**. STM32CubeIDE (GCC 14.3) | `M` 1.439 líneas **[V]** (unas 500 propias); `S` 244 **[V]**; Model y Presenter, unas 70 | `main.c` 10-07-2026; `.ioc` 08-07-2026 **[V]** |
| | | STM32 24/25 | CubeMX 6.14.0, FW_F7 1.17.2 **[V]**. FreeRTOS generado pero sin arrancar **[V `M24`:314]**. CubeIDE (GCC 13.3) | `M24` 2.186 líneas **[V]** (unas 230 propias) | Sello del ZIP 14-07-2025, igual en todos los archivos; no fecha la edición |
| Controlador trasero | Diferencial, Kelly, relés, Hall traseros, giróscopo | **ESP32 25/26** (única) | ESP32 con Arduino. `ledcAttach` **[V `E`:279]** exige el núcleo ESP32 3.x. Las librerías `mcp_can` (API de coryjfowler, deducido) y `MPU6050` no se entregaron | `E` 516 líneas **[V]** | 21-07-2026 **[V]** |
| Control único | Todo el kart (2023/24) | Arduino Mega 23/24 (única) | Arduino. Librerías incluidas en `P23/Arduino/libraries/`: MPU6050 1.3.0, autowp-mcp2515 1.2.1, NewLiquidCrystal, Timer y FastIO | `K` 936 líneas **[V]** | 08-07-2024 **[V]** |
| Electrónica | Esquemático | Proteus 25/26 | `P25/Eletronica Kart/Eletronica Kart.pdsprj`. Solo ISIS, sin PCB | — | 21-07-2026 **[V]** |

**Binarios compilados**
- **`P25/STM32-final/STM32CubeIDE/Debug/STM32F746G_DISCO.elf`**, con `.hex` y `.map`. Se compiló el 10-07-2026 a las 21:43, 85 minutos después del `main.c` **[V]**. Es el candidato a lo que se grabó en la placa; falta comparar sus símbolos con el código.
- **`P25/STM32-final/TouchGFX/build/bin/target.elf`.** Es del 03-05-2026 **[V]** y lo compiló TouchGFX Designer. Es dos meses anterior al código final y no le corresponde.
- **`P24/Debug/KartProjetoFinal.elf`.** Tiene el mismo sello de empaquetado que el fuente, así que no se puede fechar.
- **ESP32 y Arduino:** no hay binario.

## 2. Evolución del software por curso

| Curso | Plataforma | ¿Código entregado? |
|---|---|---|
| 2008/09 | 2×PIC18F4520 (MPLAB/MCC18); LabVIEW | No (DOC-0023) |
| 2010/11 | PIC18F4520 por I2C (C18) | No. El informe remite a un CD (DOC-0024 p. 117) |
| 2013/14 | PIC (MPLAB 8.76) | No (DOC-0025 p. 119-120) |
| 2015/16 | Arduino Mega | No. Solo flujogramas (DOC-0026 p. 34-35) |
| 2018/19 | Arduino | No. Solo flujogramas (DOC-0002 p. 50-52) |
| 2023/24 | Arduino Mega | Sí: `K` |
| 2024/25 | STM32F746G-DISCO | Sí: `P24` |
| 2025/26 | STM32 + ESP32 por CAN | Sí: `P25` |

Las páginas de esta tabla proceden de una búsqueda en el texto extraído. Un listado impreso como imagen podría no haber aparecido.

## 3. Controlador frontal STM32 2025/26

**Estructura**
- El proyecto parte de la plantilla TouchGFX de la placa y se llama `STM32F746G_DISCO`. No deriva del proyecto 2024/25.
- **Tareas FreeRTOS:**
  - `defaultTask` (4 KB, prioridad normal): hace todo el control.
  - `TouchGFXTask` (16 KB): pantalla.
  - `videoTask` (4 KB): viene de la plantilla.
  - `SDTask` (8 KB): registro. No está en el `.ioc`, que además asigna otra pila a `defaultTask`.
- **`defaultTask`** (`M`:1255-1348) se repite cada 5 ms o más:
  1. Lee el acelerador y el volante por ADC3, y los botones (`M`:389-434).
  2. Detecta por sondeo los flancos de los Hall delanteros.
  3. Decide la marcha. D o R solo entran con el freno pisado y la velocidad a 0.
  4. Envía `0x100` al ESP32.
  5. Calcula las rpm delanteras.
  6. Refresca la pantalla cada 50 ms.
- **Recepción CAN** por interrupción (`M`:250-277) **[V]**: `0x123` del ESP32, y `0x355`/`0x356` del BMS. Los datos se guardan en variables globales `volatile`.
- **Pantalla.** `Screen1View` lee directamente las variables globales; Model y Presenter casi no transportan datos. Tiene dos botones:
  - Modo: rota DIF → ESC → BLQ, solo con el kart parado y en N.
  - Play/Pausa: activa o detiene la grabación en la SD.
- **SD.** Graba `kart_log.csv` con comas, cada 200 ms, y sincroniza (`f_sync`) cada segundo **[V `M`:41, 355, 369-383]**.
  - Columnas: Tick, Accel, Steer, Brake, ModeD, ModeR, Vel, DiEsq, DiDir, TrEsq, TrDir, ModoDif, Yaw.
  - No guarda SOC, temperatura, tensión ni corriente, aunque el SOC y la temperatura llegan por CAN.
- **Periféricos:** ADC3, CAN1, SDMMC1 con FATFS, LTDC, DMA2D, FMC (SDRAM), QUADSPI e I2C3 (pantalla táctil). No usa UART, timers ni USB.

**Cambios desde 2024/25**
- **Cómo era 2024/25.**
  - Un bucle `while(1)` de unos 400 ms por vuelta, sin RTOS ni pantalla.
  - Leía el BMS DALY por UART/RS485.
  - Movía los motores directamente con TIM1/TIM3, con el **mismo PWM a los dos** **[V `M24`:1937-1939]**.
- **En 2025/26** el STM32 deja de mover los motores; esa función pasa al ESP32.
- **Se abandona:**
  - la UART del BMS;
  - el USB host;
  - los timers de PWM y de captura;
  - RTC, SAI2 y SPDIFRX;
  - los archivos `adc.c` y `fmc.c`.
- **Se añade:**
  - FreeRTOS activo;
  - TouchGFX;
  - CAN a 500 kbps con protocolo propio;
  - registro en SD;
  - Hall delanteros.

## 4. Controlador trasero ESP32 2025/26

**Estructura** (`E`, un único archivo)
- **`setup`** (`E`:230):
  - Pone los relés a 0.
  - Arranca el CAN MCP2515 a 8 MHz y 500 kbps; si falla, se queda en un bucle sin salida **[V `E`:240]**.
  - Calibra el giróscopo con 500 muestras.
  - Configura el PWM a 5 kHz y 8 bits, con 255 = parado.
  - Activa las interrupciones de los Hall.
- **`loop`** (`E`:299) no tiene periodo fijo (`delay(1)`):
  1. Vigila el cargador cada 2 s. Si IO35 está en alto, corta los relés.
  2. Ejecuta la secuencia del contactor: R1 → 2 s → R2 → 1 s → R1 apagado.
  3. Lee un mensaje CAN por vuelta. Si es `0x100`, procesa la orden.
  4. Si no llega ninguna orden en 100 ms, para los motores **[V `E`:371-373]**.
  5. Calcula las rpm traseras cada 500 ms y lee el giróscopo en cada vuelta.
  6. Envía `0x123` cada 200 ms.
- **Diferencial:**
  - `modo_dif` (`E`:120): Ackermann con 1,3 m entre ejes y 0,5 m de semivía.
  - `modo_esc` (`E`:162): Ackermann con corrección por yaw.
  - `modo_blq` (`E`:221): el mismo PWM a las dos ruedas.
- **Salida a los Kelly:**
  - PWM en el pin 32 (izquierdo) y en el pin 2 (derecho), con lógica invertida.
  - Freno siempre a 0.
  - Motor izquierdo invertido por software.
- No escribe en la SD **[V]** y no lee el BMS.

**Evolución desde el Arduino de 2023/24**

| Elemento | 2023/24 (`K`) | 2025/26 (`E`) |
|---|---|---|
| Ackermann (1,3 m entre ejes; semivía 0,5 m) | Sí | Se conserva |
| Corrección de yaw (/200) | Sí | Se conserva |
| Yaw ideal | `tan()` con el ángulo en grados sin convertir (sospecha) | Pasa a radianes, pero pierde el signo **[V `E`:171, 184]** |
| Ángulo del volante | Polinomio de 2.º grado | Lineal: (ADC − 2047)/47,9, limitado a ±20° |
| Autoblocante en recta (85 %) | Sí | Se elimina |
| Velocidad | Hall de 28 pulsos | Hall de 89/68 pulsos **[V `E`:38-39]** |
| Salida al controlador | `analogWrite` con polinomio de calibración por lado | PWM sin calibración |
| CAN | 250 kbps; solo recibe el BMS (IDs 90/91) | 500 kbps; recibe la orden `0x100` y responde con `0x123` |
| LCD, BMS, pedales y marchas | En el Arduino | Pasan al STM32 |
| Rampa de par | No hay | Se añade, pero no se aplica |

## 5. Precisiones a la auditoría del 29-09

- **El registro en SD lo escribe el STM32 [V].** Su cabecera coincide columna a columna con `KART_LOG.CSV`; solo cambian el separador y el periodo.
- **Fechas.** La fecha interna de `KART_LOG.CSV` es el 04-07-2026 09:31 y la del `main.c` final, el 10-07-2026 **[V]**. Refuerza que el registro procede de una versión anterior del STM32.
- **`ModoDif = 0` en el registro significa modo DIF,** que es el valor por defecto; no es falta de dato. Un cero de yaw o de Hall por sí solo no distingue valor real, inicialización o ausencia de dato; hacen falta validez y contexto (precisión 08-10).
- **Rueda.** 0,48 m de diámetro frente a 0,243 m de radio (0,486 m de diámetro): son dos literales distintos, con un 1,2 % de diferencia, más que una incoherencia de magnitud.
- **La SD no vuelve a grabar tras Pausa por un defecto del STM32 [V `S`:134-135, `M`:1199].** Pausa cierra y desmonta la SD, y la tarjeta solo se inicializa al arrancar la tarea.

## 6. Mejoras candidatas para analizar

Están ordenadas por impacto. Son puntos de partida para el análisis, no decisiones.

**Seguridad funcional**
1. **Supervisión del control pendiente.**
   - En el STM32 están desactivados IWDG y WWDG **[V `stm32f7xx_hal_conf.h`:57, 73]**. El fuente ESP32 no habilita explícitamente watchdog de `loopTask`; no se afirma ausencia de watchdogs del core.
   - El fuente histórico no registraba motivo de reinicio; la telemetría añadida en octubre sí lo hace. Política de supervisión/recuperación pendiente, ver revisión v1.2.
2. **El STM32 no vigila si siguen llegando `0x123` ni los mensajes del BMS [V `M`:250-277].** Si el ESP32 se reinicia, la pantalla y la lógica de marchas siguen usando la última velocidad recibida, sin saber si está al día.
3. **Cuando no llega orden, el ESP32 para los motores pero no abre los relés ni el contactor [V `E`:371-373].**
4. **`Error_Handler` está vacío [V `M`:1416-1422].** Un fallo al inicializar se ignora y el programa sigue.
5. **El arranque del ESP32 se bloquea si el MCP2515 no responde [V `E`:240].**
6. **No hay frenado eléctrico.** Las salidas de freno a los Kelly están siempre a 0.

Añadida el 30-09:

24. **Lectura de acelerador fuera de rango alto se trata como fondo [V `M`:401-403].** Cualquier lectura por encima de 1861 se recorta al máximo y se envía como 100 %; un fallo que produzca esa lectura no se distingue de aceleración válida. No se comprueba plausibilidad y una conversión fallida conserva el valor anterior. **Sí existe corte por `travao==1` en ESP32** (v1.2 E:527), también con acelerador pisado; no confundirlo con frenado eléctrico. Ver A03 de la revisión del 08-10 y la idea 4 de [ideas](../../06_TRABAJO/IDEAS.md).

**Control y diferencial**

7. **La rampa de par se calcula pero no se aplica [V `E`:473-482].** Los tres modos recalculan su propio PWM.
8. **El yaw ideal es siempre ≥ 0 y el medido tiene signo [V `E`:171, 184].** En modo ESC, al girar hacia uno de los lados el error sale siempre grande.
9. **Los Hall traseros cuentan 89 y 68 pulsos por vuelta [V `E`:38-39].** Hay que comprobarlo en el kart; podría explicar la relación 1,2–1,34 entre ruedas del registro.
10. **El STM32 lee los Hall delanteros por sondeo cada 5 ms o más.** A 600 rpm el error llega al ±20 %. En 2024/25 se usaba captura por timer.
11. **La salida a los controladores no tiene calibración por lado.** En 2023/24 había polinomios, y DOC-0013 muestra que el lado derecho responde menos.
12. **El registro no guarda datos de batería [V `M`:369-383].** Sin SOC, tensión, corriente ni temperatura no se puede hacer el ensayo de autonomía de la propuesta 2026/27.
    - **Hipótesis a comprobar:** si el BMS usa el formato CAN de tipo Pylontech, el mensaje `0x356` trae también la tensión y la corriente del pack en los bytes 0–3. Hoy el STM32 solo lee la temperatura de los bytes 4–5. Serían datos disponibles sin hardware nuevo.

**Robustez del software**

13. **Hay código de control fuera de los bloques USER CODE [V `M`:1238-1255].** `StartDefaultTask` y otros bloques se perderían al regenerar con CubeMX. Además, `USER CODE END 4` aparece dos veces.
14. **Problemas del registro en SD:**
    - no se comprueban los errores de `f_write`;
    - `Close_SD` se llama desde la tarea de pantalla mientras `SDTask` puede estar escribiendo;
    - `g_sensor_data` se copia sin exclusión mutua.
15. **CAN del STM32:**
    - no se comprueba lo que devuelven `HAL_CAN_Start` ni `AddTxMessage`;
    - falta tratamiento/diagnóstico de errores en la aplicación; **AutoBusOff sí está habilitado** (`M`:716, precisión 08-10);
    - el filtro acepta todos los mensajes;
    - no se comprueba la longitud de los mensajes del BMS.
16. **La pantalla se actualiza desde tres sitios,** y uno de ellos está en otra tarea (`defaultTask`).
17. **ESP32:** lee un solo mensaje CAN por vuelta de `loop`, y consulta el giróscopo por I2C en cada vuelta sin tratar errores.

**Mantenibilidad**

18. **Números mágicos repetidos:** 570, 200, 0,243, 0,48, 1,3, 0,5 y las calibraciones del ADC. Hay constantes definidas que no se usan: `REVERSE_SPEED_LIMIT`, `W_BITOLA`, `RAIO_RODA`…
19. **Código muerto en `M`:**
    - prototipos sin definición;
    - variables sin uso;
    - `g_can_data_buffer`, que nunca se escribe;
    - `Screen2View.hpp` sin su base generada.
20. **Los comentarios de `M`:53-56 contradicen el conexionado.** Dicen PC7 = D y PC6 = R; `main.h` y el `.ioc` dicen lo contrario. El código usa las definiciones de `main.h`.
21. **Todo está concentrado en pocos archivos.** El ESP32 es un único archivo y el control del STM32 está entero en `main.c`. No hay pruebas.
22. **Faltan las librerías del ESP32,** así que no se puede recompilar exactamente la misma versión.
23. **El comentario del BMS dice «Pylontech» [V `M`:264], pero los informes hablan de un DALY.** Los identificadores `0x355` y `0x356` corresponden al protocolo CAN de tipo Pylontech, que muchos BMS ofrecen como modo compatible. Hay que comprobar el reparto de bytes con la configuración del DALY instalado.

**Versiones anteriores (solo como referencia)**
- **2024/25:**
  - el buffer del BMS tiene 8 bytes y recibe 13 **[V `M24`:121, 278]**;
  - el ADC espera sin límite de tiempo;
  - PC6 y PC7 se usan a la vez como USART6 y como LED.
- **2023/24:**
  - la protección por baja tensión entra en un bucle sin salida **[V `K`:666-668]**;
  - lee el CAN y escribe por `Serial` dentro de una interrupción.

## 7. Dudas abiertas

- ¿Qué firmware estaba grabado en el kart? ¿El `.elf` del 10-07-2026 corresponde al fuente entregado? Se puede comprobar comparando los símbolos del `.map`.
- ¿Qué versión del STM32, anterior al 10-07, generó `KART_LOG.CSV`? ¿El kart rodaba en el suelo durante ese registro?
- ¿Qué versiones exactas de `mcp_can`, `MPU6050` y del núcleo ESP32 se usaron?
- ¿Son reales los 89 y 68 pulsos por vuelta?
- ¿Qué formato real tienen `0x355` y `0x356` en el BMS instalado?

## 8. Lo que no hace falta leer

| Qué | Dónde |
|---|---|
| Librerías del fabricante: HAL, CMSIS, BSP, FreeRTOS 10.2.0, FatFS, TouchGFX 4.26 con sus herramientas (`.rb`, `.exe`, `.dll`, `.lib`, `.a`) | `Drivers/`, `Middlewares/` |
| Código generado | `TouchGFX/generated/`, `TouchGFX/target/generated/`, `FATFS/Target/`, `LIBJPEG/` |
| Compilaciones | `STM32CubeIDE/Debug/`, `TouchGFX/build/`, `P24/Debug/` |
| Plantillas de IAR, Keil y Makefile sin uso | `EWARM/`, `MDK-ARM/`, `gcc/` |
| Librerías Arduino: 18 ejemplos `.ino`, 10 `.pde` y la documentación HTML de NewLiquidCrystal | `P23/Arduino/libraries/` |
| Archivos auxiliares de macOS (270) | `__MACOSX/` |

## 9. Orden propuesto para el análisis

1. **Protocolo CAN y vigilancia de fallos** entre los dos controladores (puntos 1–5). Es lo que decide si el kart se detiene con seguridad.
2. **`defaultTask` del STM32 y `loop` del ESP32.** Temporización real y reparto de funciones.
3. **Diferencial del ESP32.** Modos, rampa y signo del yaw, frente al diferencial inteligente de la propuesta 2026/27.
4. **Registro en SD.** Fiabilidad, y columnas que faltan para medir la autonomía.
5. **Reorganizar el código** antes de añadir funciones: módulos, constantes y bloques USER CODE.
