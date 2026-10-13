# Frontal STM32 · v1.3-dev3

Actualización 13-10: pulsador físico **momentáneo**, confirmado por Armando. START/STOP tiene memoria: START tras 40 ms alto estable, soltar no para, siguiente pulsación STOP al primer alto muestreado; exige liberación estable de 40 ms entre eventos y tras inicialización. Sin `delay` ni cambio CAN/GPIO. Umbrales de desarrollo, no rebote medido. Snapshot/CSV `Start` es petición lógica, no nivel del pin. [Política DEC-0008](../../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0008_start_momentaneo_con_memoria.md), [ENS-0013](../../../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md): 83 comprobaciones frontales/integradas, 56 del ESP32 intacto y compilación ARM correctas. Fuentes y ELF/HEX preservados, sin grabación; «B1.3» declarada en kart aún sin identificación binaria. Cambiar únicamente ESP32 no instala esta corrección.

Copia de trabajo del STM32F746G Discovery/TouchGFX 2025/26, sin grabar ni identificar todavía el firmware instalado. `ORIGEN_V0.json` identifica los 1673 archivos originales verificados antes de editar. No se cambian GPIO, relojes, QSPI, SDRAM ni controlador de pantalla de la entrega.

`Core/Src/kart_frontend.c` adquiere ADC, valida CAN, publica una copia coherente y administra SD desde una única tarea. `kart_display.c` da textos comprobables. Model → Presenter → View se ejecuta solo en TouchGFX. Se retiran globales de pantalla, refresco desde otra tarea, `Close_SD` desde GUI y Screen2 huérfana. Los generados de Designer permanecen intactos; posiciones/estado se ajustan en la View propia.

Pantalla: velocidad y marcha grandes; botón DIF/ESC/BLQ a la derecha; Play/Pausa arriba a la derecha; cinco filas inferiores con solicitado/recibido/aplicado, causa, CAN/MPU/freno/umbral R, BMS/SD/error y versión/filas grabadas. Fuente compacta propia 5×7 escalada 2×, 40 columnas, región 480×96 a partir de y=176. No se presenta como verificación del LCD/táctil ni corrección de la franja blanca declarada.

Control a 5 ms con prioridad AboveNormal; GUI Normal; SD Low. Presupuestos nominales, pendientes de medir carga y jitter. ADC: 1 ms por conversión, comprobación de Config/Start/Poll/Stop, plausibilidad sobre rangos históricos ±100 cuentas; 65535 representa fallo. Referencias pedal 124–1861, dirección 1042–3313; deben calibrarse físicamente. Hall por EXTI/DWT, 4 pulsos/vuelta históricos, edad 500 ms; RPM frontal >2000 se oculta como cero y no interviene en permisos. Cero no acredita conexión del Hall.

IWDG (prescaler 64/reload 1000) nominal ~2 s, solo alimentado por control; hooks de malloc y desbordamiento de pila paran y dejan de emitir. No valida ejecución completa de GUI/SD ni detecta todos los fallos de memoria. Watchdog inicializado al comenzar la tarea; un fallo anterior puede dejar la placa detenida, con ESP32 sin mando. En fallo no se siguen enviando órdenes.

SD: archivo `kart_v13.csv`, esquema 2. Filas cada 200 ms, sync cada 1 s; firmware en cada fila, ADC físicos/normalizados, edades, modos, motivo, RPM/yaw, BMS bruto, validez y errores. Cada pausa sincroniza/cierra y cada reanudación monta/abre en append. Montaje/escritura parcial/sync/cierre fallidos se hacen visibles; reintento espaciado 1 s. Corte de alimentación todavía puede perder datos desde el último sync o dañar el sistema de archivos.

## Compilación local

Desde la raíz del proyecto, Python 3.12:

```powershell
& 'C:\Users\nandi\.venvs\kart-esp32\Scripts\python.exe' '07_DESARROLLO/firmware/stm32_frontal/tools/build.py'
```

ARM GNU 14.3.Rel1 portable en `07_DESARROLLO/herramientas/toolchains/bin`; opción `--toolchain` para otra ruta. El script compila fuentes, dependencias y startup del proyecto en `.build`, sin reutilizar `.o` históricos. Enlace fatal ante advertencias y presencia de `Reset_Handler` obligatoria; produce ELF y HEX con direcciones internas/QSPI. **No grabar solo la flash interna**: el proyecto incluye recursos externos. La comparación/lectura de flash interna y QSPI del frontal instalado está pendiente antes de elegir procedimiento de grabación.

Después de clonar, con entrega histórica disponible, `tools/restore_dependencies.py` repone solo archivos ausentes y verifica sus hashes; no sobreescribe desarrollo. Los dos archivos IWDG que faltaban se incorporaron desde [HAL oficial v1.3.3](https://github.com/STMicroelectronics/stm32f7xx-hal-driver/tree/v1.3.3), misma versión que `stm32f7xx_hal.c`; licencia y fuentes añadidas versionadas. No actualizar todo HAL por esta dependencia.

`tests/compilar.cmd` comprueba adquisición/protocolo/SD/textos y pulsador con MSVC y respuestas de periféricos controladas; también enlaza el núcleo ESP32 real para verificar las tramas emitidas y la secuencia de relés. Son pruebas de software, no simulación dinámica ni prueba del hardware. Evidencias en [ENS-0009](../../../06_TRABAJO/ensayos/ENS-0009_robustez_firmware_v13.md) y [ENS-0013](../../../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md). `build.py` nombra ELF/HEX según `KART_FRONTEND_VERSION` de la cabecera.

## CubeMX / Designer

La lógica nueva permanece fuera de generados y los enganches en USER CODE. `.ioc` actualiza pila/prioridad del control y NART. No se ha ejecutado CubeMX/Designer: no afirmar regeneración comprobada. `tools/verify_configuration.py`, ejecutado antes de compilar, detecta pérdidas de tareas, prioridad/pila, CAN, watchdog e IRQ tras regenerar. La entrega original ya tiene ajustes manuales de periféricos: regenerar primero en otra copia, revisar diff y ejecutar verificación+compilación antes de adoptar el resultado. `migrar_main_v13.py` documenta la transformación inicial y rechaza un fuente ya migrado.
