# Banco NUCLEO-F411RE + pantalla MCUFRIEND UNO

Preparación del 14-10-2026. Es un destino de banco separado del frontal STM32F746G-DISCO del kart.

Retoma local del 15-10: la fuente 0.4-dev incorpora ahora el núcleo frontal dev5 y sus pruebas SD; 174 comprobaciones en PC sin fallos (120 frontales + 54 visuales), F411 compilado sin grabar. [ENS-0021](../../../06_TRABAJO/ensayos/ENS-0021_sd_reintentos_locales.md) conserva el nuevo BIN/ELF con hashes. La última imagen instalada sigue v0.3; los resultados de 155 comprobaciones siguientes corresponden al corte previo con dev4.

**Último build 0.4-dev al 16-10, sin grabar:** usa el corte frontal dev7-ui; 204 comprobaciones PC sin fallos (150 frontales + 54 visuales), flash 68.160 B y RAM estática 2.328 B. [ENS-0023](../../../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md) conserva BIN/ELF/hash. El dashboard de siete páginas pertenece a la F746; este banco conserva sus escenas. Última imagen instalada F411 sigue v0.3. Antecedente dev6-ui en ENS-0022. La fuente compartida posterior pasa a dev8-ui en ENS-0024; no se recompila F411 en esa continuación.

La Nucleo conectada se identifica por ST-LINK `0672FF564881774867244057`, COM5 y unidad `NOD_F411RE`. El MCU devuelve ID `0x431` y 512 KiB de flash. Armando declara una pantalla rotulada «MCUFRIEND 3.00 uno», conectada sobre la placa y únicamente un USB al portátil. El diagnóstico lee **ILI9341 (0x9341)** seis veces: se configura a 320×240 horizontal. La diagonal no está medida.

## Uso inmediato

Banco visual **v0.3 ya grabado y verificado**. Al arrancar ejecuta **137 comprobaciones, 0 fallos**: 83 anteriores y 54 nuevas. CPU, LCD, puerto serie y B1 son reales; CAN/ADC/SD/Hall/MPU y datos del kart están simulados.

1. **Pulsar y soltar B1 azul avanza una escena**; mantenerlo pulsado no repite. Desde la décima vuelve a la primera.
2. Orden: reposo → precarga → DIF → BLQ → ESC → reversa → frenado → pérdida CAN → error SD → cambio de modo rechazado.
3. RESET reinicia el banco y vuelve a reposo tras las pruebas. START mostrado pertenece a cada escenario sintético: B1 ahora navega. La función anterior START/STOP de v0.2 se conserva en su binario y evidencia.

La pantalla no tiene interacción táctil configurada. COM5 a 115200 bit/s muestra resultados y estados. `capturar_serie.py --port COM5 --seconds 30 --out ruta_nueva.log` guarda texto y bytes originales sin enviar comandos. Requiere pyserial, disponible en el Python de PlatformIO. No conectar esta demostración a actuadores del kart.

**Capturas para el informe:** [galería y pies de figura](../../../06_TRABAJO/evidencias/nucleo_vistas_2026-10-14/GALERIA.md), [ensayo ENS-0016](../../../06_TRABAJO/ensayos/ENS-0016_vistas_frontal_nucleo.md). Son lecturas de GRAM del LCD físico, con datos sintéticos del kart; no fotografías del cristal. Se conservan PNG nativo 320×240, ampliaciones 3× sin interpolación y RGB565/CRC/metadatos.

Para repetir, cerrar otros monitores serie y ejecutar `capturar_pantallas.py --port COM5 --out carpeta_nueva`. Verifica el número de serie, selecciona las diez vistas, lee 240 filas de 640 B con confirmación, valida CRC32 y deja reposo. Tarda aproximadamente 15 s por escena en esta sesión; mantener botones libres durante la captura. `--scenes 27` captura solo DIF y pérdida CAN (índices de 0 a 9). UART: `0`…`9` selecciona, `N` avanza, `?` informa y `C` inicia captura binaria que requiere un `A` de confirmación por fila. No enviar `C` desde un monitor de texto sin receptor compatible.

La composición procede de `Screen1View`/`KartStatus` del frontal; se reutilizan el decodificador, lógica y formateador reales con periféricos simulados. Las diez escenas son instantáneas, no una dinámica física continua. BMS permanece `--`; CAN caducado oculta velocidad. El ensayo del 14-10 no modificó el frontal ni el trasero; la revisión de fuente del 15-10 incorpora el formateador frontal dev4 y conserva ESP32 dev3.

## Herramientas

- VS Code existente + extensión oficial **STM32CubeIDE for Visual Studio Code 3.11.0** instalada.
- **STM32CubeProgrammer CLI 2.23.0** y **ST-LINK GDB Server 7.14.0+st.2** instalados por el gestor oficial Cube.
- OpenOCD portátil xPack 0.12.0-7 en `../../herramientas/openocd/`, descarga con SHA-256 verificado.
- Los drivers ST-LINK 2.2.0.0 existentes funcionan: no se reinstalaron. ST-LINK informa V2J46M33; no se actualizó.
- PlatformIO de VS Code ya existía. Este proyecto utiliza STM32duino y MCUFRIEND para el adaptador de esta pantalla; no convierte automáticamente el proyecto TouchGFX del kart.

Compilar desde esta carpeta con `platformio run`. **Compilar no graba.** La programación de esta sesión se realiza mediante CubeProgrammer seleccionando el número de serie exacto. No arrastrar el HEX del frontal F746 a `D:`.

## Conservación y límites

Tres lecturas de flash interna (dos OpenOCD, una CubeProgrammer) coinciden en 524288 bytes y SHA-256 `8d88e79ec69e18e0d3038e93306b7bb4e4ad0ef1cd04b2f73c3d7b9ce3ac3722`. [Respaldo local](../../respaldos/nucleo_f411/2026-10-14/) y [logs/hashes](../../../06_TRABAJO/evidencias/nucleo_f411_2026-10-14/). Incluye lectura de option bytes; no se modifican protecciones. No equivale a copia externa ni a respaldo de una posible SD/memoria del shield.

La F411 carece de CAN integrado y del conjunto LTDC/SDRAM/QSPI que usa el frontal. Se pueden ejecutar funciones portables, comprobar entradas y mostrar estados adaptados. La prueba no acredita comunicaciones CAN físicas, motores, relés, sensores del kart ni la pantalla F746/TouchGFX. Los datos sintéticos deben quedar identificados en pantalla y registro.

## Fuentes

- [NUCLEO-F411RE, ST](https://www.st.com/en/evaluation-tools/nucleo-f411re.html) y [STM32F411RE, ST](https://www.st.com/en/microcontrollers-microprocessors/stm32f411re.html).
- [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv), `utility/mcufriend_shield.h`: ramas `STM32F411xE` e `IS_NUCLEO64`. Bus paralelo de 8 bits; control A0/A1/A2/A3/A4. Pines de datos Arduino D8,D9,D2…D7; no es una pantalla SPI.
- [PlatformIO Nucleo F411RE](https://docs.platformio.org/en/latest/boards/ststm32/nucleo_f411re.html).
- [Ensayo ENS-0015](../../../06_TRABAJO/ensayos/ENS-0015_nucleo_f411_pantalla_banco.md) para resultados, versiones exactas y limitaciones.
