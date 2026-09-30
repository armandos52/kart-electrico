# Auditoría de contenido extraíble · 2026-09-29

Revisión de lo que contiene la entrega más allá del catálogo del 28-09. Se leyeron código, hojas de cálculo, capturas, presentación y el texto extraído de los 9 informes. Cinco exploradores trabajaron en paralelo y la sesión principal comprobó en el original los datos marcados **[V]**. El resto procede de la exploración y está **pendiente de comprobar** antes de citarlo en un informe. No se ha inspeccionado el kart ni ejecutado ningún programa recibido.

Abreviaturas: `P25` = `02_ORIGINALES/entrega_2026-09-28/Smart Eletric Kart Cross-2223406-2223403/Smart Eletric Kart Cross-2223406-2223403/`; `ESP` = `P25/ESP32-final/ESP32-final/ESP32-final.ino`; `STM` = `P25/STM32-final/Core/Src/main.c`.

## 1. Evolución por curso (resumen)

| Curso | Doc | Base y motores | Batería | Control | Resultado principal |
|---|---|---|---|---|---|
| 2008/09 | DOC-0023 | Banco de laboratorio, 2 motores DC 24 V (no es el kart) | 2×12 V plomo 100 Ah | 2×PIC18F4520, chopper IGBT propio | Diferencial en banco; coste 930,51 € (p. 126) |
| 2010/11 | DOC-0024 | Conversión del KartCross: 2 hub BLDC Batlight EM-2KW, 48 V **[V p. 24]** | 4×12 V plomo 75 Ah | 2×PIC18F4520 por I2C, DAC MCP4725 | Error entre ruedas en lazo abierto hasta 34 %; PID por Ziegler-Nichols (p. 83-93) |
| 2013/14 | DOC-0025 | Mismos motores; controladores reparados | Plomo recuperado; BMS propio en breadboard | PIC18F4520, marcha atrás | Kart inmóvil: par motor-controlador derecho sin reparar (p. 109) |
| 2015/16 | DOC-0026 | Motores y controladores Kelly nuevos | 4×12,6 V | Arduino Mega + shield | Diferencial en lazo abierto por ángulo de volante |
| 2018/19 | DOC-0002 | Kelly KEB | Compra de 16 celdas LiFePO4 60 Ah + BMS 123Smart | Arduino, PID de par y de crucero | Placa OUT del BMS averiada; litio sin poder cargar (p. 5, 60) |
| 2023/24 | DOC-0022 / DOC-0027 | Quanshun 48 V 3 kW, Kelly 48 V 150 A | 16 Winston LiFePO4; DALY sin CAN | Arduino Mega; diferencial Ackermann + yaw; BMS por CAN 250 kbps (IDs 90/91) | Registro de datos imposible (reset del Arduino) |
| 2024/25 | DOC-0003 | Igual | 15 celdas útiles (una averiada, p. 44) | STM32F746G-DISCO; **FreeRTOS desactivado** (`osKernelStart` comentado **[V]**); BMS DALY por UART/RS485 | Migración incompleta; mismo PWM a los dos motores, sin diferencial |
| 2025/26 | DOC-0042 | Igual; motor izquierdo invertido por software | 15 celdas, DALY 8-17S por CAN | STM32 (frontal, TouchGFX) + ESP32 (trasera) por CAN 500 kbps | DIF y BLQ probados; ESC sin validar; contactor y ESP32 fallan con vibración |

Faltan informes de 2011/12 (citado en DOC-0025, p. 111), 2014/15, 2016/17, 2017/18 y 2019–2023. Otras contradicciones históricas: ver [PROB-0001](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0001_versiones_documentales.md). DOC-0022 y DOC-0027 se diferencian sobre todo en redacción. La única diferencia de cálculo está en el Hall delantero (PDF 16).

## 2. Firmware 2025/26: lo que dice el código

- **CAN 500 kbps** (tabla de la presentación DOC-0028, diapositiva 12 **[V]**, y código):
  - `0x100`: STM32 → ESP32, cada 5 ms. Contenido: acelerador y volante (0–4095), freno, D, R, modo de diferencial (2 bits) y START.
  - `0x123`: ESP32 → STM32, cada 200 ms. Contenido: velocidad (km/h), rpm izquierda, rpm derecha y yaw. La diapositiva indica yaw en °/s; el código envía °/s × 10 (discrepancia menor).
  - `0x355`: BMS → STM32. SOC en bytes 4–5, 0,01 %/bit.
  - `0x356`: BMS → STM32. Temperatura en bytes 4–5, 0,1 °C/bit.
  - El STM32 no envía nada al BMS y no supervisa si deja de llegar `0x123` o los mensajes del BMS.
- **Calibraciones fijas en el código**:
  - Acelerador: ADC 124–1861, es decir 0,1–1,5 V **[V `STM`:66-67]**.
  - Volante: ADC 1042–3313. Ángulo = (ADC − 2047)/47,9, limitado a ±20°.
  - Pulsos por vuelta de los Hall traseros: 89 (izquierdo) y 68 (derecho) **[V `ESP`:38-39]**.
  - Geometría: distancia entre ejes 1,3 m y semivía 0,5 m, escritas directamente en las fórmulas.
  - Rueda: 0,48 m de diámetro en el cálculo de velocidad y 0,243 m de radio en ESC, algo incoherentes entre sí.
  - Velocidad máxima: 570 rpm.
- **Seguridad implementada**:
  - Solo se puede cambiar de marcha con el freno pisado y la velocidad a 0.
  - El modo de diferencial solo se cambia parado y en N.
  - Si no llega un mando CAN en 100 ms, se paran los motores, pero los relés siguen como estaban.
  - Con el cargador conectado se corta el sistema (IO35).
  - No hay watchdog y `Error_Handler` está vacío.
- **Defectos visibles**:
  - La rampa de 300 rpm/s se calcula pero no se aplica **[V `ESP`:121, 473]**.
  - En ESC el yaw ideal siempre es ≥ 0 y el medido tiene signo.
  - Si el MCP2515 no responde, el arranque se queda bloqueado en un `while` **[V `ESP`:240]**.
  - Tras pulsar Pausa, la SD no vuelve a grabar hasta reiniciar.
  - Hay constantes y defines sin uso.
- **Frenado**: no hay frenado eléctrico ni regenerativo. Las salidas de freno del controlador se mantienen siempre a 0.

## 3. Registro KART_LOG.CSV (DAT-0001)

- **No procede del firmware entregado.** El código escribe con comas cada 200 ms; el archivo usa `;` y registra cada ~100 ms **[V]**. Hay que averiguar qué versión lo generó.
- Duración: 80,9 s. Velocidad máxima 53 km/h. Máximos traseros: 660 rpm (izquierda) y 605 rpm (derecha).
- Hubo filas en marcha D y en R. Todo el registro está en modo DIF.
- Hall delanteros, `ModoDif` y `Yaw` valen 0 en las 801 filas. El giroscopio y los Hall delanteros no aportaron datos.
- La relación de rpm izquierda/derecha es de 1,2–1,34, tanto en recta como en curva. Parece un sesgo de calibración (89 frente a 68 pulsos por vuelta), no un efecto del diferencial. Hipótesis por comprobar.
- No se sabe si el kart rodaba en el suelo o con las ruedas en el aire.

## 4. Datos que estaban sin extraer

- **Kelly, DOC-0015** **[V captura 2]**: controlador KEB48401X. Control por par, Hall a 120°, subtensión 46 V, sobretensión 60 V, regeneración desactivada, motor de 32 polos, sensor KTY84-130. No consta a qué lado corresponde. En 2025/26 se cambió a 42–57 V (DOC-0042 p. 72 **[V]**).
- **DOC-0013 (Excel)**: calibración de los controladores, teórico frente a medido. Con consigna máxima, el izquierdo da 200,5 y el derecho 176,02: el lado derecho responde menos. Incluye velocidades en curva medidas frente a teóricas y la referencia «435 rpm = 39,85 km/h».
- **DOC-0014 (Excel)**:
  - Curva ADC–ángulo por rueda y tablas de radio de giro de 0° a 25°.
  - 47 puntos de la curva del motor (par, corriente, rpm, potencia; pico ≈ 3,0 kW) con ajustes de MATLAB.
- **DOC-0036 (Fichas.xlsx)**: no es de calibración. Es el pinout y los colores de cable de los conectores de 2025/26.
- **Proteus `Eletronica Kart.pdsprj`**: es un ZIP con un único esquemático ISIS, sin PCB ni lista de materiales. Se pueden recuperar nombres de red, pines y valores; las conexiones exactas requieren Proteus.
- **SolidWorks (caja de baterías 23/24)**: versión 2023 SP2.1 Educational. Sin el programa solo se leen las cotas de los PDF exportados.
- **PCB en PDF**: son vectoriales y no tienen texto. Las referencias de componentes requieren verlas o abrir los `.brd` de Eagle, que no se entregaron.
- **DOC-0028**: presentación de 264 MB con fotos del kart, esquemas, la tabla CAN y un vídeo MOV de ensayos (diapositiva 16).
- **Word DOC-0021 y DOC-0041**: 63 y 84 imágenes. Las tablas de DOC-0042 son imágenes: 12–21 (CAN, BMS), 22–30 (pinout del cableado) y 31–32 (ensayos del diferencial).
- **Loop**: formato interno de Microsoft («Prague»). No se puede recuperar sin la aplicación Loop.
- **No hay fotos ni vídeos sueltos** en la entrega.

## 5. Hipótesis de diagnóstico (no son hechos)

1. **Contactor y ESP32 con causa común.** El contactor sigue soltándose con vibración incluso con la bobina a la tensión total del pack (DOC-0042 p. 90 **[V]**). El ESP32 no se bloquea con el PC conectado (p. 91). Los relés del contactor dependen de salidas del ESP32. Hipótesis: un reinicio o una caída de la alimentación de 5 V del ESP32, o un conector o masa afectados por la vibración, desconecta los relés y con ellos el contactor. Comprobación propuesta:
   - Registrar el motivo de reinicio del ESP32 (`esp_reset_reason`).
   - Observar el rail de 5 V con osciloscopio durante un ensayo.
   - Probar el contactor con los relés alimentados de forma independiente.
2. **Carga detenida en 52,5 V.** Dos celdas llegan a 3,8 V y el resto se queda en 3,3 V (p. 71 **[V]**). Encaja con un desequilibrio entre celdas: 3,3 V está en la meseta de LiFePO4, así que esas celdas no están llenas. No hace falta suponer envejecimiento. Además, 57 V entre 15 celdas son 3,8 V por celda, justo el límite documentado (DOC-0027 p. 20), sin margen. Comprobar la configuración del BMS (número de celdas, tablas 19–20 en imagen), medir la capacidad de cada celda y valorar un equilibrado superior.
3. **«Descarga completa en una noche» (p. 72 **[V]**).** El consumo de un BMS no descarga 60 Ah en una noche. Es más probable un salto en la estimación del SOC que una descarga real. Coincide con la conclusión «el SOC cae a 0 % y sigue habiendo alimentación». Comprobar midiendo la tensión del pack cuando el BMS marque 0 %.

## 6. Material reutilizable de informes antiguos

- Dimensionamiento de par y pendiente (DOC-0024 p. 20-21).
- Presupuestos y requisitos de batería (DOC-0002 p. 27-29).
- Plan de ensayo de autonomía con tres estilos de conducción, nunca ejecutado (DOC-0002 p. 42).
- PID por Ziegler-Nichols (DOC-0024 p. 89-90).
- Diferencial por corriente (DOC-0002 p. 48).
- Sensores de corriente HAIS con PCB fabricada en 23/24 y nunca instalada (DOC-0027 p. 52-53).
- La autonomía, la velocidad máxima en suelo y el consumo **no se han medido nunca** en ningún curso.

## Límites

Las cifras sin **[V]** proceden de la exploración y deben comprobarse en la página citada antes de usarlas. Siguen sin revisar visualmente las tablas en imagen de DOC-0042, las PCB, los planos y el vídeo.
