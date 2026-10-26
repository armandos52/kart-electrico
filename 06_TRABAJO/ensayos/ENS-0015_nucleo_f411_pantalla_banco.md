---
estado: banco grabado, 83 comprobaciones sin fallos; B1 registrado y visual confirmada por usuario
fecha: 2026-10-14
configuracion: NUCLEO-F411RE + shield MCUFRIEND UNO ILI9341; solo USB al portátil
---

# ENS-0015 · Nucleo F411 y primera pantalla de banco

## Objetivo y declaraciones

Armando recibe del profesor una placa para probar antes del frontal del kart y pide identificarla, preparar programas/drivers y comenzar ensayos. Declara pantalla montada sobre la placa, único cable USB al portátil, rótulo «MCUFRIEND 3.00 uno». El rótulo no especifica por sí solo diagonal, controlador ni versión de firmware. No se recibió fotografía ni inspección del cableado interno.

Decisión [DEC-0009](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0009_banco_nucleo_f411.md): banco independiente, respaldo previo, adaptación de pantalla y funciones portables; no reutilizar el binario F746. No se conecta ni modifica el kart.

## Configuración identificada y conservación

| Elemento | Evidencia y alcance |
|---|---|
| Placa | Windows: unidad `D:` NOD_F411RE, ST-LINK Debug y COM5 operativos. CubeProgrammer: NUCLEO-F411RE. |
| ST-LINK | Serie `0672FF564881774867244057`, USB `0483:374B`, firmware V2J46M33. No actualización. |
| MCU | SWD: ID `0x10006431` / DEV_ID `0x431`, STM32F411xC/E revisión A, Cortex-M4, 512 KiB de flash. Referencia de placa: STM32F411RET6. |
| Alimentación | ST-LINK informa 3,241803 V; CubeProgrammer 3,24 V. Lectura del programador, no medida independiente/calibrada. USB único según usuario. |
| Pantalla | Diagnóstico real: seis lecturas consecutivas `readID=0x9341`, registro D3 incluye `9341`. Driver ILI9341, paralelo de 8 bits MCUFRIEND UNO. Biblioteca configurada a 320×240 horizontal (`rotation=1`); diagonal no medida. |
| Programa anterior | Nombre/fuente desconocidos. Flash 524288 bytes leída 3 veces con dos herramientas, mismo SHA-256; [respaldo](../../07_DESARROLLO/respaldos/nucleo_f411/2026-10-14/RESPALDO.md). RDP=0xAA/nivel 0, sin protección de sectores. |
| Base de trabajo | `main`, HEAD inicial `43b0c299af47e30aabcf9b5acd7677b86754a00b`, árbol inicialmente limpio. Firmware frontal/trasero de trabajo dev3, sin alterar sus fuentes. Firmware físico del kart sigue sin identificar. |

Las lecturas previas no detienen ni resetean deliberadamente el MCU. Se desactiva el callback OpenOCD que ajustaría DBGMCU. La primera consulta muestra estado `unknown` antes del sondeo; termina `running`, y CubeProgrammer confirma `running` antes/después. No afirmar que se midió ausencia total de perturbación temporal. Option bytes solo leídos; no se cambian. No respaldo de posible SD/memoria externa ni copia independiente del ordenador.

## Herramientas preparadas

- Drivers STMicroelectronics 2.2.0.0 existentes: conexión y lectura correctas, sin reinstalación. La consulta WMI muestra `IsSigned=false` en dos entradas ST; este campo se conserva como dato bruto y no se interpreta como prueba de firma inválida. No se desactivó verificación de firmas.
- Extensión oficial **STM32CubeIDE for Visual Studio Code 3.11.0**, componentes ST 1.5.0 y build-CMake 1.47.0, instalados en VS Code 1.141.0 existente.
- Gestor oficial Cube wrapper 0.10.4: **CubeProgrammer CLI 2.23.0** y **ST-LINK GDB Server 7.14.0+st.2**, instalados y programador probado físicamente. No se instala ni se afirma instalada la GUI Eclipse, CubeMX o TouchGFX Designer.
- OpenOCD xPack **0.12.0-7** portátil, SHA-256 del ZIP cotejado con el publicado: `6bfd3c97135aafef8affc9af1acf34fd0e2b9ca26044506f6abd7f95b7630052`.
- PlatformIO Core **6.2.0** existente; plataforma **ststm32 20.0.0**, STM32duino **3.0.0** (paquete 4.30000.0), CMSIS **6.3.0**, CMSIS-DSP **1.16.2**, GCC ARM **12.3.1** (paquete 1.120301.0), MCUFRIEND **3.1.0-Beta**, Adafruit GFX **1.12.6**, BusIO **1.17.4**. Versiones directas fijadas en `platformio.ini`; transitorias en logs/manifiestos. El GCC 14.3.Rel1 existente para el frontal no se sustituye.

[Proyecto de banco](../../07_DESARROLLO/firmware/nucleo_f411_banco/README.md). Se compila con STM32duino/MCUFRIEND para este shield y se graba mediante CubeProgrammer, seleccionando el ST-LINK exacto. Compartir ecosistema/herramientas no hace compatibles binarios ni interfaces TouchGFX.

## Procedimiento y resultados

1. Consultar estado, mapa, plan, catálogo/ficha DOC-0034, inventario y fuente `.ioc`, linker y funciones de trabajo. Consultar documentación oficial ST y fuente MCUFRIEND para pinout NUCLEO64/F411.
2. Identificar USB, driver y flash por SWD. Leer dos veces flash interna con OpenOCD y una con CubeProgrammer Hot Plug; comparar SHA-256 `8d88e79ec69e18e0d3038e93306b7bb4e4ad0ef1cd04b2f73c3d7b9ce3ac3722` en las tres copias.
3. Compilar diagnóstico `v0.1`: 19796 B de flash según size, BIN 20240 B, RAM estática 1288 B. Guardarlo y grabar/verificar; sectores 0–1 reemplazados, sin mass erase ni option bytes. Captura RX COM5, 115200 bit/s, 12 s: seis identificaciones ILI9341 coherentes.
4. Compilar banco **v0.2**: 51832 B de flash según size, BIN 52292 B, RAM estática 1924 B (no incluye máximo de pila/heap). Grabar/verificar y reiniciar. Código declara `SystemCoreClock=100000000`; no medida de frecuencia externa.
5. Inicializar LCD, dibujar/leer tres píxeles: rojo `F800`, verde `07E0`, azul `001F`, **3/3 iguales**. Acredita ida/vuelta al controlador/memoria gráfica, no inspección óptica del panel ni prueba táctil.
6. Ejecutar en el procesador F411 las **83 comprobaciones existentes, 0 fallos**, incluyendo START momentáneo, STOP prioritario, integración frontal→núcleo trasero, entrada inválida, caducidad de feedback, estados SD y textos. Datos de CAN/ADC/SD/Hall/tiempo **simulados**. Las trazas `TRACE tick_ms` son estímulos sintéticos, no tiempos físicos de precarga.
7. Entrar en modo B1: GPIO PC13 físico → adaptador al pulsador lógico frontal → ocho bytes en memoria → `kart::Controller` → pantalla/serie. Freno, N, pedal libre y sensores/feedback simulados. No CAN eléctrico, salidas de relé/PWM ni motores. Captura RX de 180 s: **dos ciclos completos ON–OFF**, y un tercero que llega a LISTOS con botón suelto. Armando confirma el 14-10: «Se ve bien y START cambia ON/OFF». Es confirmación visual del usuario más registro instrumental de GPIO/estado, no inspección óptica del agente.

En la captura física `STATE`: primer START a tick 198840 ms; botón suelto a 198943 ms y START permanece; siguiente pulsación a 199880 ms da STOP. Segundo ciclo: START 200588 ms → STOP 201390 ms. Tercero: START 202122 ms → SOLAPE 204123 ms → LISTOS 205124 ms; no se capturó su STOP final. Son ticks del MCU en este banco; los relés son variables de software. No acreditan tiempos de contactos del kart ni precisión del antirrebote: el refresco de pantalla al detectar un flanco demora la siguiente muestra.

El banco incluye directamente `frontend_test.cpp` y sus funciones reales mediante una unidad separada, con HAL/FatFS simulados. Renombra únicamente `main`, `printf` y el reloj ficticio a nivel del adaptador; el reloj de pruebas Hall sigue siendo sintético a 216 MHz, separado del real F411. `kart_control.cpp` también se compila directamente desde la copia de trabajo. No es el firmware completo del kart ni su pantalla TouchGFX.

## Evidencia

Carpeta [nucleo_f411_2026-10-14](../evidencias/nucleo_f411_2026-10-14/):

- `usb_inicial.json`, `drivers_inicial.json`, `DETAILS.TXT` y logs `identificacion_openocd`, `respaldo_openocd`, `lectura_cubeprogrammer`.
- `hashes_respaldo.json`, instalación OpenOCD, instalación extensión ST y bundles oficiales.
- `build_diagnostico.log` (fallido), `build_diagnostico_02.log` (correcto), `flash_diagnostico.log`, `serie_diagnostico.log/.raw`.
- `build_banco_v02.log`, `banco_v02.map`, `flash_banco_v02.log`, `serie_banco_v02_arranque.log/.raw`, `serie_banco_v02_boton.log/.raw`.
- Fuentes/manifestados de esta sesión y hashes de evidencia. BIN/ELF privados en el respaldo local, diferenciados del programa anterior; sin commit/push.
- [resultado.json](../evidencias/nucleo_f411_2026-10-14/resultado.json): recuento verificado de 83 PASS, RGB 3/3, 189 registros físicos STATE, dos ciclos ON–OFF y tercero hasta LISTOS. [Fuentes conservadas](../evidencias/nucleo_f411_2026-10-14/fuentes_banco_v02/), licencias/manifiestos y [hashes de evidencia](../evidencias/nucleo_f411_2026-10-14/hashes_evidencia.json). Capturas cerradas y puerto serie liberado al terminar; banco v0.2 permanece en la placa.

## Incidencias, límites y siguiente paso

- Acceso CIM/unidad D denegado dentro del sandbox; consulta autorizada fuera de él funciona. No rechazo del hardware. `winget search STM32` no encuentra paquete. Descarga directa de página ST falla por transporte; gestor oficial Cube permite instalar sus herramientas. Primer intento de ruta de extensión sin sufijo `win32-x64` no existe, se localiza la correcta.
- Primera compilación falla porque MCUFRIEND no declara Adafruit GFX como dependencia en su manifiesto. Se añade explícitamente y recompila correctamente. Se mantiene advertencia informativa de biblioteca `Uno Shield on NUCLEO-64`. No se ocultaron errores.
- Primera línea de captura v0.2 contiene restos del diagnóstico v0.1 retenidos en el búfer serie antes del nuevo identificador. Se conservan bytes originales; no implica que ambos programas corran a la vez.
- El refresco LCD/serie puede alargar el muestreo nominal de 5 ms. Este banco comprueba función; no mide latencia máxima ni rebote eléctrico con precisión. Los milisegundos del log de PC son tiempos de recepción; los de STATE son del MCU; TRACE usa reloj simulado.
- Texto/visibilidad y B1 confirmados por Armando; quedan ensayos físicos de pulsación larga/reinicio con botón sostenido y táctil si se desea. La F411 no tiene CAN integrado; CAN físico requeriría controlador/transceptor y adaptación propios. No confundir transceptor con controlador CAN.
- Observación de fuente existente: `kart_display.c` conserva un rótulo literal `STM V1.3-DEV2` aunque la lógica dev3 esté vigente. El banco muestra su versión separada; no se corrige ni se considera identificada la versión física del kart por esa etiqueta.
- Frontal real: proyecto F746G-DISCO con memoria interna/SDRAM/QSPI/LTDC y TouchGFX; su respaldo y ensayos por Wi-Fi siguen pendientes. Ningún resultado de ENS-0015 cierra PROB-0009 en el kart.

## Fuentes y cobertura

- DOC-0034, UM1907 Rev 7, PDF p.7 recuperada con buscador: características F746G-DISCO, LCD 4,3 pulgadas RGB 480×272, ST-LINK y memorias externas. **Texto consultado; sin nueva revisión visual**. `.ioc`: MCU STM32F746NGH6, CubeMX 6.17.0, FW_F7 1.17.4, destino CubeIDE; linker de trabajo define regiones FLASH/QUADSPI/SDRAM.
- [STM32F411RE, ST](https://www.st.com/en/microcontrollers-microprocessors/stm32f411re.html), especificaciones/periféricos: Cortex-M4 hasta 100 MHz, 512 KB flash, 128 KB SRAM, sin CAN integrado. [NUCLEO-F411RE, ST](https://www.st.com/en/evaluation-tools/nucleo-f411re.html).
- [UM1724, ST, §§6–7](https://www.st.com/resource/en/user_manual/dm00105823-stlink-v2-in-circuit-debugger-programmer-for-stm8-and-stm32-stmicroelectronics.pdf), conexión ST-LINK, USER PC13 y conectores.
- [STM32CubeIDE para VS Code](https://marketplace.visualstudio.com/items?itemName=stmicroelectronics.stm32-vscode-extension), [gestor oficial de bundles](https://dev.st.com/stm32cube-docs/stm32cubecli/latest/en/docs/markup/STM32CubeCLI_Getting_Started/STM32CubeCLI_Bundles.html), [CLI CubeProgrammer 2.23](https://dev.st.com/stm32cube-docs/prog/2.23.0/en/docs/markup/CubeProg_Command_Lines.html).
- [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv): ramas `STM32F411xE`/`IS_NUCLEO64`, `readID` y ejemplo `LCD_ID_readreg`. [Plataforma Nucleo F411RE](https://docs.platformio.org/en/latest/boards/ststm32/nucleo_f411re.html).

No lectura integral de L1/L4, nuevos originales históricos incorporados, validación de formatos visuales/Loop ni copia de seguridad externa.
