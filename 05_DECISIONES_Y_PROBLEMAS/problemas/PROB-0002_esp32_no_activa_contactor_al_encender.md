---
estado: vigente
actualizado: 2026-10-07
resumen: Al encender, a veces el ESP32 no activa el contactor y solo se recupera sacándolo de la placa. Hipótesis, pruebas para distinguirlas e ideas para una nueva PCB trasera.
---

# PROB-0002 · Al encender, el ESP32 a veces no activa el contactor

Fecha: 2026-09-30.
Estado: abierto.
Configuración: kart 2025/26 ([DOC-0042](../../03_CATALOGO/fichas/DOC-0042.md)). El ESP32-WROOM-32D está en la PCB de la unidad trasera (DOC-0042 p. 50). Se supone cargado `ESP32-final.ino` del 21-07-2026, pero no se ha comprobado qué versión está grabada.

## Síntoma y condiciones de la prueba

**Actualización 07-10-2026:** Armando declara energía residual al abrir el contactor que afecta al programa y la atribuye a los variadores. Se abre [PROB-0003](PROB-0003_energia_residual_al_abrir_contactor.md) para distinguir rutas y secuencia; no se confirma causa común. Se consultó ahora el texto completo de PDF 90–91 de DOC-0042; figuras/esquemas y montaje siguen pendientes.

**Declaración de Armando (30-09-2026):**
- Al encender el kart, a veces el ESP32 no da la señal al contactor.
- El ESP32 queda bloqueado o con un comportamiento extraño.
- Para recuperarlo hay que sacarlo de la placa. Solo entonces vuelve a funcionar.

**Sin datos todavía:**
- Con qué frecuencia ocurre.
- Si lo recupera el botón EN (reset) o apagar todo el kart.
- En qué orden se encienden la llave, el convertidor DC/DC y el contactor.
- La tensión de la batería.
- Si el cargador estaba conectado.
- Qué muestra el monitor serie.

## Evidencias

**Informe 2025/26.** Registra un fallo parecido, pero en marcha:
- El ESP32 se bloqueaba y el contactor se desconectaba por encima de 25 km/h o en suelo irregular (DOC-0042 p. 90).
- Con el PC conectado al ESP32, este nunca se bloqueó, aunque el contactor seguía desconectándose (p. 91).
- Los autores sospechan de fluctuaciones de tensión (p. 97).

Estos datos proceden de extractos del buscador; la lectura completa de esas páginas está pendiente. Relación con la hipótesis 1 de la [auditoría](../../06_TRABAJO/AUDITORIA_2026-09-29.md): causa común del contactor y del ESP32.

**Código** (`ESP32-final.ino`, comprobado el 30-09 en el original):
- **Pines de arranque.** Estos pines deciden cómo arranca el chip, y los cuatro tienen algo conectado:
  - GPIO12: marcha atrás del Kelly derecho (línea 24).
  - GPIO15: freno PWM del Kelly derecho (línea 22).
  - GPIO2: PWM del motor derecho (línea 21).
  - GPIO5: selección (CS) del chip CAN MCP2515 (línea 6).
- **Entrada del cargador, IO35.** En el código está configurada como `INPUT` (línea 233), y IO34–IO39 no tienen resistencia interna. Si lee alto durante 2 s con los relés activos, apaga los relés (línea 314).
  - Según DOC-0042 p. 78 (extracto), IO35 va al relé de carga de la caja de barras, y se le añadió una resistencia de pull-down (fig. 22).
  - La [síntesis 2025/26](../../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md) la describe como provisional (p. 97-98).
- **Arranque.**
  1. Pone los relés a LOW.
  2. Arranca el CAN con `while (CAN.begin(...) != CAN_OK)`, un bucle sin salida (línea 240). Si el MCP2515 no responde, el programa no pasa de ahí y nunca activa los relés.
  3. Solo después inicializa el giróscopo por I2C y lo calibra durante 1 s.
- **El contactor necesita START sin interrupciones.** La secuencia de relés arranca con el flanco de subida de START. En cuanto llega un mensaje `0x100` con START a 0, se apagan los dos relés (líneas 388-404). Basta un fallo breve de la señal START para soltar el contactor.
- **Mando del contactor** (DOC-0042 p. 74, leída el 30-09).
  - Los dos relés no hacen una precarga: uno aplica la tensión completa del pack para que el contactor cierre (necesita 27,5 V como mínimo); el otro lo mantiene a través de resistencias para que caliente menos. Las resistencias pasaron de 180 Ω a 90 Ω porque el contactor se soltaba con la vibración.
  - El informe dice que primero entra el relé 2 y el código enciende primero el relé 1. Hay que comprobar en el cableado cuál es cuál.
- **Cristal.** `MCP_8MHZ` corresponde al cristal de 8 MHz del módulo MCP2515 + TJA1050 más habitual, que funciona a 5 V. Es una hipótesis: no se ha visto el módulo instalado.

## Causa

Ninguna causa está confirmada. Las hipótesis van ordenadas por lo bien que explican que haya que **sacar el ESP32 de la placa** para recuperarlo.

1. **Alimentación parásita por los pines.**
   - Algunos circuitos conectados al ESP32 pueden tener tensión antes que él, o mientras está apagado: el módulo CAN, las entradas de los Kelly, la placa de relés.
   - En ese caso entra corriente por los diodos de protección de sus pines y el chip queda medio alimentado. No ve un encendido limpio y se queda en un estado indefinido.
   - Ni el reset ni apagar el kart lo sacan de ahí, porque la corriente sigue entrando por los pines. Sacarlo de la placa corta todas esas vías.
   - Encaja con que no se bloquee con el PC conectado: el USB lo alimenta antes y de forma estable.
2. **GPIO12 en alto al encender.**
   - En el WROOM-32D, si el GPIO12 está en alto durante el arranque, el chip alimenta su memoria flash a 1,8 V en lugar de 3,3 V y no arranca.
   - El GPIO12 va a la marcha atrás del Kelly derecho. Si la interfaz o el Kelly lo llevan a alto al encender, el fallo dependería del orden de encendido, y por eso sería intermitente.
   - Riesgo añadido: durante el arranque, los GPIO 5, 14 y 15 emiten pulsos. Esos pulsos llegan al CS del CAN y al freno del Kelly derecho.
3. **Módulo CAN a 5 V conectado a un chip de 3,3 V.**
   - Si el MCP2515 funciona a 5 V, su salida MISO entra al ESP32 (GPIO19) a 5 V, por encima de lo admisible.
   - Eso puede provocar un enclavamiento del chip (latch-up) o la alimentación parásita de la hipótesis 1.
   - Si el MCP2515 no responde, el ESP32 se queda en el bucle de la línea 240. Es justo el síntoma: no activa el contactor.
4. **Tensión inestable al encender.** El convertidor DC/DC, las bobinas de relés y contactor y la precarga de los Kelly pueden producir una subida lenta de la tensión o caídas. El ESP32 arrancaría con tensión inestable y se colgaría o se reiniciaría en bucle (brown-out).
5. **Mal contacto en el zócalo.** Si el ESP32 va insertado en tiras de pines hembra, la vibración o la oxidación pueden cortar el contacto. Sacarlo y volver a ponerlo limpia el contacto. Es la explicación más sencilla.
6. **Otras vías de apagar el contactor.** No explican el bloqueo, pero sí que el contactor no entre o se suelte, también en marcha:
   - IO35: la resistencia es provisional y el cable llega hasta la caja de barras, junto a los cables de potencia, así que puede captar ruido.
   - Cualquier caída momentánea de START en el STM32 (PI0, entrada con pull-down).

## Pruebas para distinguir las hipótesis

**Revisión del plan 07-10:** primero registrar secuencia, síntomas, tiempos y fuentes conectadas según PROB-0003. La lista siguiente es un plan preliminar, no un procedimiento para ejecutar sin preparar el montaje. Evitar movimiento del kart y órdenes de tracción; mediciones, conexiones de USB y cambios de cableado requieren comprobar referencias/alimentaciones. No retirar módulos ni manipular conectores energizados. Las respuestas de cada prueba orientan, pero no prueban una causa única.

Comprobaciones candidatas, una vez preparado el ensayo:

1. **Pulsar el botón EN (reset) del módulo.**
   - Si se recupera: problema de arranque o del programa (hipótesis 2, o la 3 si el MCP2515 no responde).
   - Si no se recupera: alimentación parásita o enclavamiento (hipótesis 1 y 3).
2. **Apagar todo el kart, esperar 30 s y volver a encender.** Si así se recupera, apunta a la hipótesis 4.
3. **Conectar el USB al PC y abrir el monitor serie a 115200.** Según lo que aparezca:
   - `flash read err`: hipótesis 2.
   - `Brownout detector was triggered`: hipótesis 4.
   - `CAN init failed, retrying...` repetido: hipótesis 3.
   - Nada: bloqueo total, hipótesis 1.
4. **Medir con el multímetro:**
   - 3,3 V y 5 V en el ESP32.
   - GPIO12 justo al encender.
   - GPIO19 y la alimentación del módulo CAN.
   - IO35 con el cargador desconectado.
5. **Comparar interfaces conectadas o aisladas en ensayos preparados.** Cambiar conexiones únicamente con alimentación aislada y energía residual verificada. La recuperación puede orientar sobre una ruta; no demuestra por sí sola el mecanismo.
6. **Inspeccionar fijación y zócalo con alimentación aislada y energía residual verificada** (hipótesis 5); evitar mover la placa en funcionamiento.
7. **Anotar cada vez:**
   - si es el primer encendido del día;
   - la tensión de la batería;
   - si el cargador estaba conectado;
   - el orden de encendido.

Además, hay que revisar en los PDF de la PCB trasera (`P25/PCBs/PCB_unidade traseira_*.pdf`), en el esquemático de Proteus y en la placa real:
- cómo se alimenta el ESP32 y cómo está montado;
- a qué va conectado el GPIO12;
- el valor y el montaje de la resistencia de IO35;
- cómo se mueven los relés;
- a qué tensión funciona el módulo CAN.

## Ideas para una nueva PCB trasera

Son requisitos para discutir, no un diseño aprobado. Cada punto cubre una o varias hipótesis.

1. **Montaje antivibración** (H5):
   - ESP32 soldado, sin zócalo;
   - conectores con enclavamiento;
   - placa bien fijada;
   - barniz protector.
2. **No usar pines de arranque** (0, 2, 5, 12 y 15) para señales hacia los Kelly o los relés (H2). Todas las salidas con resistencias que fijen el estado seguro durante el arranque: relés apagados y marchas en neutro.
3. **Aislamiento:** las señales hacia los Kelly y los relés pasan por optoacopladores o transistores. Ningún punto del lado de 5, 12 o 48 V va directo a un pin del ESP32 (H1, H3).
4. **CAN integrado del ESP32 (TWAI) con transceptor de 3,3 V**, por ejemplo el SN65HVD230 (H3).
   - Elimina el MCP2515 y el problema de los 5 V.
   - Si se mantiene el MCP2515, alimentarlo a 3,3 V.
   - Terminación de 120 Ω y par trenzado.
5. **Alimentación** (H1, H4):
   - Convertidor DC/DC propio para la lógica, con filtro, protección contra picos (TVS) y condensadores de reserva.
   - Bobinas de relés y contactor alimentadas por separado de la lógica.
   - Un supervisor de tensión en EN que garantice un reset limpio.
   - El módulo CAN y el giróscopo alimentados desde el mismo raíl que el ESP32, para que se apaguen a la vez.
6. **Relés y contactor** con transistor y diodo de libre circulación.
7. **Resistencias en todas las entradas:** IO35 (cargador) y los Hall (IO34, IO4), con filtro RC (H6).
8. **Masas separadas:** potencia (Kelly, contactor) y señal, unidas en un solo punto.
9. **Diagnóstico sin desmontar:**
   - USB o puerto serie accesible;
   - LED de estado;
   - puntos de prueba en 3,3 V, 5 V, EN y GPIO clave.

Firmware relacionado, para cuando se trabaje el código: registrar y enviar el motivo de reinicio (`esp_reset_reason`), watchdog, reintentos del CAN con aviso en lugar del bucle sin salida, y filtrar las lecturas de IO35 y START. Ver las mejoras 1, 3 y 5 del [inventario del código](../../04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md).

## Intentos y resultados

**07-10-2026, noche ([ENS-0002](../../06_TRABAJO/ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md)):** Armando declara que con v1.1 los motores no funcionaron al probar en el kart y no pudo ver el panel. No hay captura en el vehículo ni confirmación de estado del contactor. Después conecta solo el módulo ESP32 fuera del kart: el error `CAN init failed` es esperable sin MCP2515 y no identifica la causa del fallo en el vehículo. **Imagen binaria original restaurada y verificada contra los 4 MB de flash**, conservando la flash previa en dos lecturas idénticas. Comprobación de motores pendiente.

**07-10-2026, banco ([ENS-0001](../../06_TRABAJO/ensayos/ENS-0001_telemetria_esp32_usb_wifi.md)):** ESP32 en su placa, alimentado por USB.

- **Hipótesis 3 reproducida en parte.** En cuatro arranques, el MCP2515 respondió tras 19 s, ~35 s y ~77 s de «Entering Configuration Mode Failure / CAN init failed», y en uno no respondió en 40 s. Mientras tanto el programa sigue en el bucle de la línea 240: no se calibra el giróscopo ni se pueden activar los relés.
- **Grabado del chip:** «Flash voltage set by a strapping pin: 3.3V», es decir, GPIO12 estaba bajo al arrancar en ese banco.
- **Respaldo de la flash original:** contenía un **coredump**, así que el ESP32 se colgó alguna vez.
  - Hay un mecanismo de cuelgue confirmado al leer mensajes CAN con DLC > 8 o lecturas SPI corruptas: [PROB-0005](PROB-0005_mcp_can_desborda_con_dlc_mayor_8.md), corregido en firmware v1.1.
  - Un cuelgue así se recupera con un reset; no explica por sí solo la necesidad de sacar el ESP32.

**Siguiente:** con el osciloscopio y mientras se reintenta, medir la alimentación del módulo CAN, GPIO19 (MISO), GPIO5 (CS), SCK y el cristal del MCP2515.

Antes de esta fecha, el único recurso conocido era sacar el ESP32 de la placa.

## Solución y prueba de verificación

Pendiente de identificar la causa.

## Pendientes

- Hacer las pruebas 1–3 la próxima vez que ocurra y anotar el resultado aquí.
- Revisar el esquemático y la PCB de la unidad trasera: prueba 7 y la lista que la sigue.
- Texto de PDF 90–91 consultado el 07-10; revisar esquemas/figuras y completar contexto de PDF 97 cuando corresponda a L1.
