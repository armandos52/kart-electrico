---
estado: vigente
actualizado: 2026-09-30
resumen: Propuestas técnicas que aún no son decisiones.
---

# Ideas

## Propias

- **Nueva PCB de la unidad trasera (30-09-2026).**
  - Propósito: acabar con el bloqueo del ESP32 al encender y con los cortes del contactor.
  - Beneficio: montaje antivibración, sin pines de arranque ni señales de 5 V hacia el ESP32, CAN integrado (TWAI) y alimentación con reset limpio.
  - Antes, comprobar la causa con las pruebas de [[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender|PROB-0002]] y revisar la PCB actual.

- **Sensores de dinámica como en los coches (30-09-2026).**
  - **Propósito:** que el modo ESC del diferencial funcione con datos fiables. Los coches con control de estabilidad combinan tres sensores que se vigilan entre sí:
    - un giróscopo de guiñada con acelerómetro lateral (IMU);
    - un sensor de ángulo de volante;
    - la velocidad de las cuatro ruedas.
  - **Situación actual:**
    - GY-521 (MPU6050, de consumo y descatalogado) con un fallo de GND que impidió validar ESC (DOC-0042 p. 97).
    - Volante leído con un potenciómetro en el ADC.
    - Hall traseros de 89 y 68 pulsos por vuelta; los delanteros se leen por sondeo.
    - El yaw nunca llegó al registro.
  - **Posible beneficio:**
    - Calcular la guiñada de referencia con el ángulo real del volante y la velocidad de las ruedas delanteras, que no son motrices.
    - Añadir un límite de adherencia a esa referencia.
    - Comprobar los sensores entre sí y pasar a DIF si no coinciden.
    - Encaja con los «novos elementos de sensorização e controlo» del cronograma (DOC-0043).
  - **Candidatos a estudiar** (sin comparar todavía sus hojas de datos):
    - IMU de grado automoción: familias ST ASM330LHH, Bosch SMI2xx o Murata SCHA6xx.
    - Sensor magnético de ángulo sin contacto en la columna de dirección: familias ams AS5x47 o Infineon TLE5012B.
    - Ruedas fónicas con sensor Hall.
  - **Comprobar:**
    - la investigación previa de Armando;
    - los requisitos: rango en °/s, ruido, deriva con la temperatura, frecuencia de salida, conexión SPI o CAN;
    - el montaje rígido cerca del centro de gravedad;
    - una prueba en banco junto al MPU6050 actual antes de elegir;
    - las curvas ADC–ángulo por rueda de DOC-0014.
  - **Cuándo:** decidir antes de diseñar la nueva PCB trasera. No bloquea el trabajo de firmware.

## Ideas tomadas de un equipo de Formula Student EV (30-09-2026)

**Fuente.** Texto pegado por Armando: publicación de un equipo universitario tailandés de Formula Student EV, clasificado para Formula Student China (Hefei). No es documentación del kart.

**Diferencias con el kart:**
- Su coche es de alta tensión y debe pasar una inspección técnica con reglamento.
- El kart funciona a 48 V (15 celdas LiFePO4) y no tiene esa obligación, pero maneja corrientes de hasta 150 A por controlador.

Se adoptan los principios, no la complejidad. Las ideas van ordenadas por prioridad para el kart.

1. **Circuito de seguridad por hardware.** Prioridad alta.
   - *Ellos:* combinan BMS, aislamiento y plausibilidad de freno con lógica discreta y relés. Un fallo queda enclavado hasta un rearme manual, y ningún microcontrolador está en la ruta crítica.
   - *En el kart:*
     - La bobina del contactor se alimentaría a través de un lazo en serie: llave, seta de emergencia, fallo del BMS y un relé enclavado con rearme.
     - El ESP32 dejaría de ser quien mantiene cerrado el contactor, así que un reinicio suyo ya no lo soltaría.
     - Un watchdog externo cortaría las señales de par hacia los Kelly si el ESP32 deja de dar señales de vida.
     - Un contactor con economizador de fábrica, o un driver de bobina, sustituiría a los dos relés con resistencias.
   - *Ataca:*
     - [[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender|PROB-0002]];
     - el contactor que se suelta (DOC-0042 p. 90);
     - el riesgo de que el PWM quede congelado si el ESP32 se cuelga.
   - *Comprobar:* la palabra «emergência» no aparece en el texto de DOC-0042, así que hay que ver en el kart si existe una seta de emergencia.
2. **Alimentación de la lógica independiente de la potencia.** Prioridad alta.
   - *Ellos:* el panel tiene batería propia, para hacer diagnósticos sin activar la alta tensión.
   - *En el kart:* alimentar el STM32 y el ESP32 desde una batería auxiliar o un convertidor aislado que no dependa del contactor. Así también se podría diagnosticar sin conectar la potencia.
   - *Ataca:* las hipótesis 1 y 4 de PROB-0002. Con el PC conectado por USB, el ESP32 nunca se bloqueó (DOC-0042 p. 91).
3. **CAN aislado y protegido.** Prioridad alta, para la nueva PCB.
   - *Ellos:* transceptor aislado, convertidor aislado y diodos de protección (TVS) en las líneas CAN.
   - *En el kart:* transceptor aislado, o como mínimo de 3,3 V, con TVS y terminación. Se combina con el CAN integrado del ESP32.
   - *Ataca:*
     - la hipótesis 3 de PROB-0002;
     - el fallo causado por un mal aislamiento de los cables CAN (DOC-0042 p. 69).
4. **Plausibilidad del acelerador.** Prioridad alta. En firmware es barato.
   - *Ellos:* dos sensores de pedal que deben coincidir. El par se corta si no coinciden o si se pisan acelerador y freno a la vez.
   - *En el kart, ahora:* cortar el par si la señal sale del rango válido (0,1–1,5 V), por cable roto o cortocircuito, y si se pisan acelerador y freno a la vez.
   - *En el kart, después:* un segundo sensor en el pedal.
   - *Comprobado en `main.c`:401-403:* hoy cualquier lectura por encima del máximo, como un cortocircuito a 3,3 V, se recorta y se envía como acelerador a fondo. Es la mejora 24 del inventario.
5. **Registro de datos y GPS.** Prioridad alta.
   - *Ellos:* una placa de registro para que «cada sesión dé datos en lugar de opiniones», y un GPS para posición y velocidad.
   - *En el kart:*
     - Registrar la versión de firmware, la tensión, la corriente y el SOC. El mensaje `0x356` del BMS quizá ya trae tensión y corriente; ver el [[04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO|inventario]], mejora 12.
     - Añadir un GPS para medir la distancia sin depender de los Hall, cuyos 89/68 pulsos son dudosos.
   - *Ataca:* el ensayo de autonomía (Wh/km) de la propuesta.
6. **Fallos enclavados e interruptores de prueba.** Prioridad media.
   - *Ellos:* un fallo no se borra solo, y tienen un interruptor para demostrar cada ruta de fallo.
   - *En el kart:*
     - Los fallos de CAN, sensores o BMS quedarían enclavados hasta parar y rearmar en N. Según el código, hoy el ESP32 vuelve a mover los motores en cuanto regresa la señal CAN (comprobar).
     - Preparar una forma de provocar cada fallo en un ensayo y documentarlo con la [[09_PLANTILLAS/ENSAYO|plantilla de ensayo]].
7. **Corte por freno y corriente (BSPD simplificado).** Prioridad media.
   - *Ellos:* un corte por hardware si hay freno fuerte y corriente alta a la vez durante un tiempo.
   - *En el kart:* usar los sensores de corriente HAIS de 2023/24; su PCB se fabricó pero nunca se instaló (DOC-0027 p. 52-53). Ese sensor serviría también para medir la energía en el ensayo de autonomía.
8. **Modelar, ensayar, registrar, comparar y corregir.** Prioridad media: es el método del informe.
   - *Ellos:* modelos de dinámica y de energía validados con datos reales, y pruebas quitando una parte del controlador cada vez para ver qué aporta cada una.
   - *En el kart:*
     - un modelo de bicicleta sencillo para la guiñada de referencia;
     - un modelo de energía para la autonomía;
     - comparar DIF, ESC, BLQ y la versión nueva en condiciones equivalentes.
9. **Panel del piloto.** Prioridad baja.
   - *Ellos:* interfaz diseñada para leerse de un vistazo, eligiendo también qué no mostrar, y una carcasa robusta que se abre rápido.
   - *En el kart:* revisar la pantalla TouchGFX para que tenga pocos datos y avisos claros, y hacer una carcasa antivibración para la unidad frontal.
10. **Conmutación de potencia con MOSFET.** Prioridad baja; solo para estudiar.
    - *En el kart:* sería una alternativa al contactor mecánico. Es un diseño delicado (corriente, calor, precarga), así que solo se estudiaría con los orientadores. Antes conviene probar un contactor con especificación de vibración y un buen montaje.
11. **Estimación avanzada: filtro de Kalman, redes neuronales, optimización bayesiana.** Prioridad baja por ahora.
    - Un filtro de Kalman sencillo para estimar el SOC o la velocidad del kart podría ser un buen tema, pero no antes de tener sensores y datos fiables. Encaja con la hipótesis 3 de la auditoría (el SOC cae a 0 %).
    - Las redes neuronales y la optimización bayesiana quedan fuera de alcance hasta febrero.
12. **Documentar para la siguiente generación.** Ya adoptado en [[05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos|DEC-0001]].

Anotar cada idea con su propósito, posible beneficio y qué habría que comprobar. Una idea no es una decisión aprobada ni una especificación vigente.
