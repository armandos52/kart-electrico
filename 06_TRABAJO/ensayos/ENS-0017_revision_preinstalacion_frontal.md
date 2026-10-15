---
estado: revisión completada; tres ajustes de interfaz y validación física pendientes
actualizado: 2026-10-15
resumen: Frontal dev3 compila y supera regresión; tres hallazgos reproducidos impiden considerarlo entrega final validada.
---

# ENS-0017 · Revisión del frontal antes de instalar

## Dictamen y alcance

**El frontal tiene una base funcional compilable para continuar en banco; todavía no está listo como versión final validada para el kart.** Armando pide analizar el código de la pantalla frontal, las nuevas mejoras y su preparación para instalar después. Se revisa el STM32F746G-DISCO/TouchGFX de trabajo **v1.3-dev3**, su adquisición/control/SD y el contrato con ESP32 dev3. El banco Nucleo F411 v0.3 es otra plataforma: sus diez escenas y aceptación visual no validan LCD, táctil, periféricos ni tiempos del F746.

Solo PC Windows, rama `main`, HEAD `43b0c29` (hash completo en la evidencia), con cambios previos de documentación/banco conservados. Sin conexión a placas ni red del kart, grabación, cambio de firmware fuente, commit o push. Las versiones físicas siguen sin identificar. No se atribuyen estos resultados al kart instalado ni se cierra PROB-0008/0009.

## Lo que ya está implementado

| Función | Evidencia de código y alcance comprobado |
|---|---|
| Velocidad/marcha grandes, modos DIF/ESC/BLQ y estado SD | [Screen1View](../../07_DESARROLLO/firmware/stm32_frontal/TouchGFX/gui/src/screen1_screen/Screen1View.cpp), `setupScreen`/`updateDisplayData`; falta observación del LCD/táctil F746 |
| Modo solicitado, recibido y aplicado separados; causas de inhibición | [Formateador](../../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_display.c), `Kart_FormatDisplay`; textos comprobados en host, con excepciones F01–F03 abajo |
| Datos CAN antiguos dejan de habilitar marcha y la velocidad se representa con `--` | [Frontend](../../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c), `Kart_ControlTick`, y View; edades nominales 500 ms para 0x123 y 300 ms para 0x124; no son latencias físicas medidas |
| START momentáneo con memoria | `read_start_button`: alto estable 40 ms para START, soltar no para, siguiente pulsación STOP al primer muestreo; liberación estable 40 ms. Umbrales de desarrollo, sin rebote real medido |
| Play/Pausa/reanudar y errores SD | `Kart_SdTick`: montaje, append, escritura completa, cierre y reintento; registros nominales cada 200 ms y sync cada 1 s. FATFS simulado en las pruebas; SD real pendiente |
| Separación GUI/control/SD y supervisión | `Model::tick` entrega snapshots desde la tarea gráfica; control y SD tienen tareas distintas; IWDG/hooks presentes. Compilación/configuración comprobadas, carga/pilas/jitter físicos pendientes |

El frontal solicita modos y presenta su estado; el diferencial, bloqueo y ESC se ejecutan en el **ESP32**. Instalar solo la pantalla no instala esas mejoras traseras. El protocolo permite equipos antiguos con prestaciones reducidas; no equivale a validar la combinación física actual.

## Hallazgos reproducidos y mejoras previas a la entrega

Registro común: [PROB-0011](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0011_presentacion_frontal_preinstalacion.md). El [log dirigido](../evidencias/revision_frontal_2026-10-15/audit_display.log) conserva entradas, textos y resultados negativos.

| ID / prioridad | Resultado observado en software | Mejora propuesta y criterio de aceptación |
|---|---|---|
| F01 / P2 | Cabecera `stm32_frontal-v1.3-dev3`, fila visible `STM V1.3-DEV2`. Ya señalado en ENS-0015; confirmado aquí | Derivar el texto visible de una única definición de versión, conservar el alfabeto admitido por `KartStatus` y verificar que cabe. CSV, pantalla y paquete deben identificar la misma versión |
| F02 / P2 | Con solo 0x123 legado, sin 0x124, aparece `CAN:OK ... R:20KM/H` | Diferenciar objetivo configurado, capacidad conocida y limitación actuando. En legado/estado desconocido no afirmar R20 disponible. La trama actual no comunica un umbral numérico: documentar pareja de versiones o ampliar contrato si se quiere confirmación explícita; nunca presentar el umbral como velocidad física garantizada |
| F03 / P2 | Tras rechazar cambio de modo en D, llega estado `CARGADOR CONECTADO`; se sigue viendo `MODO: PONER N` hasta que caduca el aviso de 3 s | Dar prioridad a pérdida de comunicación y causas de inhibición pertinentes; mostrar el rechazo de modo en otro lugar o sin sustituirlas. El caso dirigido debe mostrar el cargador inmediatamente. La causa interna permanece correcta: este hallazgo afecta al mensaje, no demuestra pérdida del corte de tracción |

Estas pruebas adicionales son criterios de aceptación de esta revisión, ausentes de la batería anterior. **No se han aplicado las correcciones**: se entrega diagnóstico y preparación de instalación, preservando dev3 y sus binarios.

## Pendientes funcionales que no son fallos nuevos

- **Batería:** `BMS_FORMAT_CONFIRMED=0` en `kart_frontend.c:20`; SOC y temperatura se ocultan deliberadamente (`BMS:--`). Antes de habilitarlos hay que identificar el BMS y verificar bytes, escala y frescura contra datos reales. No hay tensión/corriente/autonomía acreditadas en esta pantalla.
- **Franja blanca/táctil:** [PROB-0004](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md) sigue sin medida ni diagnóstico. La distribución F746 usa botones desde y=48 y cinco líneas desde y=176; no acredita que la zona afectada quede evitada.
- **Calibraciones:** pedal/dirección/PPR/MPU conservan referencias históricas o umbrales de desarrollo. El asistente del PC no aplica automáticamente una calibración al firmware frontal.
- **Compatibilidad e instalación:** identificar ambos firmwares, respaldar flash interna **y QSPI** del F746 y verificar procedimiento de recuperación. El HEX incluye recursos externos; grabar solo flash interna no completa el conjunto.
- **Cobertura real:** faltan cadena CAN física, freno/START/D/R, táctil, tarjeta real, watchdog/temporización y ensayos integrados. ESC continúa experimental y R20 limita propulsión estimada; no aporta frenado ni máximo físico garantizado.

Después de estos ajustes, una mejora opcional sería mostrar START/precarga/listo y rotular claramente la marcha solicitada. Hoy `s.gear` procede de `selected_gear` frontal; no es medición de marcha ni contacto real de relé. Una vista de diagnóstico separada puede ayudar a reducir texto durante conducción, sin bloquear la preparación del banco.

## Procedimiento y resultados del 15-10

1. Consulta focalizada de estado/mapa, catálogo/ficha DOC-0042, notas de software, problemas y ENS-0009/0013/0015/0016. Revisión de frontend/formateador/View/Model/Presenter, widget de texto, base generada, fuente Verdana, enganches de tareas, configuración, build y tests. Consulta adicional de inicialización SD, driver y linker para delimitar cobertura; no prueba de acceso DMA/cache en hardware ni lectura exhaustiva de dependencias.
2. Ejecución de `tests/compilar.cmd` frontal: **83 comprobaciones, 0 fallos**. Incluye integración funcional con núcleo real ESP32, transporte y periféricos simulados. [Log](../evidencias/revision_frontal_2026-10-15/frontend_tests.log).
3. Ejecución de `tests/compilar.cmd` trasero: **56 comprobaciones, 0 fallos**. [Log](../evidencias/revision_frontal_2026-10-15/backend_tests.log).
4. `tools/build.py`: configuración comprobada, 151 unidades del proyecto, enlace ARM correcto, ELF y HEX generados, sin upload. ARM GNU 14.3.Rel1/GCC 14.3.1. [Log](../evidencias/revision_frontal_2026-10-15/build_stm32.log). Hay advertencias de parámetros no usados en dependencias/generados y dos del arnés MSVC; no se presenta como compilación sin advertencias.
5. Nuevo [arnés de revisión](../evidencias/revision_frontal_2026-10-15/audit_display.cpp), con funciones reales y stubs existentes: **4 comprobaciones, 3 fallos de aceptación y 1 control positivo**. Reproduce F01/F02/F03; el control positivo verifica que el cargador aparece después de caducar el aviso. El retorno 1 es resultado esperado de reproducir defectos, no error de compilación. No modifica firmware.
6. **ELF y HEX recompilados coinciden byte a byte por SHA-256 con los dev3 preservados el 13-10.** Esto identifica la compilación local, no el firmware instalado. Se conservan 36 fuentes de alcance, logs, mapa y copia de resultados en [evidencia](../evidencias/revision_frontal_2026-10-15/README.md), con [manifiesto](../evidencias/revision_frontal_2026-10-15/manifest.json). El manifiesto de build enumera unidades compiladas: compilarlas no implica leerlas todas.

SHA-256 ELF: `a61913643aa0d977c20687f8803cbb64986f8b350b04c64f17140112ac12a3ee`.

SHA-256 HEX: `6455904ae01f27fca2df7e74fcae57a1a8428576b7b9382a4f1a1297e9522291`.

## Orden para preparar e instalar después

1. Resolver F01–F03 en copia de trabajo y repetir regresión, casos dirigidos y compilación. Versionar y conservar un candidato nuevo; no sobrescribir los respaldos dev3 ni identificarlo solo por una etiqueta de pantalla.
2. Identificar F746/pantalla y ESP32 instalados, leer/verificar respaldo frontal interno+QSPI, elegir y documentar pareja de versiones y recuperación. Si se actualiza ESP32, hacerlo con el módulo fuera del kart.
3. Grabar el candidato para banco F746 mediante el procedimiento verificado, incluidos los recursos necesarios de QSPI. Probar arranque, legibilidad/franja, todos los botones, datos ausentes/caducados, modos aceptados/rechazados y SD ausente/Play/Pausa/reanudar/error. Registrar también fallos y tiempos reales.
4. Comprobar entradas, polaridades, calibraciones y protocolo CAN conjunto antes del ensayo de tracción. Confirmar pulsar/soltar/segunda pulsación y pérdida/recuperación CAN sin rearranque indebido. El funcionamiento de ESC requiere validación propia.
5. Ensayo integrado en kart con registro **por Wi-Fi**, versiones/hashes y condiciones conocidas. USB ESP32 no es requisito con el módulo instalado. Solo los resultados instrumentados permitirán decidir aptitud para uso.

No se amplía lectura de PDF ni se cierran L1/L4, formatos visuales o Loop. No hay nueva copia de seguridad independiente ni mediciones del kart en esta sesión.
