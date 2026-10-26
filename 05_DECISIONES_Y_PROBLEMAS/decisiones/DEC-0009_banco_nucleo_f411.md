# DEC-0009 · Banco independiente Nucleo F411 + MCUFRIEND

Fecha: 14-10-2026. Origen: Armando recibe del profesor una placa con pantalla para ensayar antes de la pantalla del kart. Confirma un único USB al portátil y rótulo «MCUFRIEND 3.00 uno».

**Decisión:** mantener un proyecto separado `nucleo_f411_banco`, respaldar el programa anterior y reutilizar las funciones/pruebas portables del kart con un adaptador para esta pantalla. No grabar el ELF/HEX del STM32F746G-DISCO en esta F411. Tampoco asumir que lo instalado en el kart coincide con dev3 de trabajo.

Evidencia: [ENS-0015](../../06_TRABAJO/ensayos/ENS-0015_nucleo_f411_pantalla_banco.md). USB y dos herramientas SWD identifican NUCLEO-F411RE, ID `0x431`, flash 512 KiB. Tres lecturas completas coinciden; el diagnóstico propio lee repetidamente LCD `0x9341`. El código frontal F746 enlaza LTDC/SDRAM/QSPI y CAN1; la F411 necesita otro adaptador y no tiene CAN integrado.

Herramientas: extensión oficial STM32CubeIDE para VS Code, CubeProgrammer CLI y ST-LINK GDB Server; OpenOCD como segundo lector. Para MCUFRIEND se usa STM32duino/PlatformIO existente, con dependencias fijadas y biblioteca que declara soporte NUCLEO-F411RE. Es el ecosistema STM32, con otro proyecto y controlador de pantalla, no el IDE Eclipse ni la interfaz TouchGFX del kart instalados por esta acción.

Criterios: seleccionar siempre el ST-LINK conocido, conservar binarios y hashes antes de cambiar, verificar la grabación, distinguir CPU/pantalla/botón físicos de CAN/ADC/SD/relés simulados y pedir comprobación visual al usuario. No conectar el banco a actuadores del kart ni interpretar pruebas de software como validación de marcha. La recuperación se realiza desde la flash original verificada, sin cambiar option bytes. Una posible SD del shield queda fuera del respaldo y no se accede a ella.

Continuación del 14-10: Armando confirma visual/START de v0.2 y solicita adaptar vistas sin táctil. [ENS-0016](../../06_TRABAJO/ensayos/ENS-0016_vistas_frontal_nucleo.md) carga v0.3: diez instantáneas de estados del frontal, B1 como navegación, datos sintéticos rotulados y capturas leídas de GRAM con CRC. Se conserva el proyecto de producción y se identifica el banco por su propio rótulo/huella. La lectura de memoria gráfica no sustituye una fotografía ni prueba de brillo/legibilidad física.

Armando confirma después legibilidad y recorrido completo sin saltos en v0.3; aceptación visual/manual registrada en ENS-0016. Siguiente: ampliación de transiciones según necesidad; táctil fuera de este ensayo. El frontal F746 real conserva su plan de respaldo interno+QSPI y pruebas integradas por Wi-Fi.
