---
estado: propuesta aceptada; implementación local dev6-ui, sin grabar
actualizado: 2026-10-16
resumen: Funcionamiento del frontal, pendientes y dos propuestas navegables para aprovechar la pantalla de diagnóstico.
---

# Pantalla frontal · descripción y propuesta de mejora

**Continuación 15–16-10:** Armando acepta el diseño y pide implementarlo con temas claro/oscuro. Hecho en [ENS-0022](ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md): siete páginas en firmware dev6-ui, compilación y pruebas locales; [renders del código](evidencias/frontal_dashboard_2026-10-15/GALERIA.md). Instalación pendiente por indicación de continuar sin placa. El análisis siguiente conserva la configuración y las alternativas del momento de la propuesta.

## Alcance y configuración

Armando pide describir el programa frontal, identificar las mejoras pendientes y mostrar ideas de cómo quedaría. Se mantiene la fase de desarrollo/diagnóstico de [DEC-0010](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md). La interfaz de usuario final sigue pendiente. Al redactar la propuesta inicial no había diseño adoptado; la aceptación e implementación posteriores se registran arriba, sin firmware nuevo instalado.

- **Documentado en placa:** STM32F746G Discovery con dev4, grabado y verificado en [ENS-0019](ensayos/ENS-0019_frontal_dev4_f746_real.md). Captura consultada: [framebuffer final](evidencias/frontal_f746_2026-10-15/dev4_final/pantalla.png); no es una fotografía del LCD.
- **Fuente de trabajo:** `stm32_frontal-v1.3-dev5`, corrección parcial de SD compilada y comprobada en PC en [ENS-0021](ensayos/ENS-0021_sd_reintentos_locales.md), sin grabar. No se ha conectado hardware en esta sesión.
- **ESP32 físico:** sin identificar; en el ensayo anterior recibíamos 0x123, pero no 0x124. La hipótesis de versión antigua no es identificación binaria.
- **Geometría:** interfaz horizontal de 480 × 272 píxeles; reservar filas 0–47 deja 480 × 224. El daño físico está documentado en filas 0–43, con margen conservado en [PROB-0004](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md). La banda rayada de la maqueta representa espacio excluido, sin información ni controles.

## Qué hace el programa actual

El frontal es la unidad que recoge los mandos, transmite peticiones y presenta información. El control trasero de los motores corresponde al ESP32. La pantalla forma parte de un programa mayor; cambiar su aspecto no sustituye las condiciones de control.

| Parte | Función actual y evidencia de código de trabajo |
|---|---|
| Adquisición | Lee ADC del pedal y dirección, freno, START y selector D/R. Valida las entradas y normaliza ADC; [kart_frontend.c](../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c), `read_adc` y `Kart_ControlTick`, líneas 124–200. Los rangos son referencias históricas, pendientes de calibración. |
| START/STOP | Una pulsación solicita START y la siguiente STOP. Soltar permite otra pulsación. No es confirmación física de contactor cerrado; líneas 22–45 del mismo archivo y [ENS-0013](ensayos/ENS-0013_start_stop_momentaneo.md). |
| Marcha/modo | Selección N/D/R y rotación DIF → ESC → BLQ con condiciones. D/R requiere freno, pedal libre, parada e información válida. Cambiar modo exige N, parada, pedal libre y entradas/CAN válidos; líneas 179–198. DIF/BLQ/ESC son modos del control trasero; ESC sigue experimental. |
| CAN | Envía mando 0x100 y salud de entradas 0x101; recibe velocidad/RPM/yaw 0x123 y estado extendido 0x124, además de bytes BMS 0x355/0x356. [Contrato](../07_DESARROLLO/firmware/common/kart_protocol.h) y recepción, líneas 95–122. |
| Presentación | Model copia un snapshot coherente y lo publica a la vista con separación mínima programada de 100 ms; [Model.cpp](../07_DESARROLLO/firmware/stm32_frontal/TouchGFX/gui/src/model/Model.cpp), líneas 5–12. Es una condición del código, no una medida de FPS/latencia. |
| Registro | CSV `kart_v13.csv`, cadencia nominal de filas de 200 ms y sincronización de 1 s; `Kart_SdTick`, líneas 244–285. El icono Play/Pausa controla este registro, no la marcha del kart. |

El control tiene un objetivo de periodo de 5 ms, con tarea de mayor prioridad que GUI y SD: [main.c](../07_DESARROLLO/firmware/stm32_frontal/Core/Src/main.c), líneas 99–125 y 924–938. No se afirma jitter medido. La separación de tareas y del modelo/vista es una base útil para ampliar la pantalla sin poner operaciones de almacenamiento dentro de los callbacks táctiles.

La vista actual presenta velocidad, N/D/R, el modo **solicitado**, freno, icono de SD y cinco líneas compactas: solicitado/recibido/aplicado, causa, CAN/MPU/freno/limitador R, BMS/SD/error y versión/contador. [Screen1View.cpp](../07_DESARROLLO/firmware/stm32_frontal/TouchGFX/gui/src/screen1_screen/Screen1View.cpp), líneas 3–29; [formateador](../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_display.c), `Kart_FormatDisplay`; [widget](../07_DESARROLLO/firmware/stm32_frontal/TouchGFX/gui/src/common/KartStatus.cpp), región 480 × 96 desde y=176.

Ya existe invalidación de datos antiguos: el código usa 500 ms para feedback y 300 ms para estado extendido; no son nuevas prestaciones propuestas ni tiempos físicos medidos. La velocidad pasa a `--` en los casos inválidos previstos. La compatibilidad con 0x123 legado no certifica por sí sola que sus sensores estén funcionando. El BMS sigue oculto por `BMS_FORMAT_CONFIRMED=0`; recibir bytes no acredita su interpretación.

## Lo que falta resolver antes de considerar el conjunto validado

| Prioridad | Pendiente real | Mejora de pantalla asociada |
|---|---|---|
| Primero | Medir pedal en posiciones conocidas y calibrar. Cuatro observaciones anteriores dieron 149–194 cuentas con reposo supuesto; el umbral actual se supera. [PROB-0012](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0012_pedal_reposo_fuera_de_pedal_libre.md). | Mostrar ADC físico, normalizado, umbral de pedal libre y condición que bloquea. No aumentar el umbral para ocultar el síntoma. |
| Primero | Identificar ESP32 y verificar cadena frontal → CAN → control → estado. No se recibió 0x124 en la última prueba. | Separar «hay feedback» de «hay estado confirmado»; mostrar edades por trama y modo solicitado/recibido/aplicado. |
| Primero | Completar SD: dev5 mejora reintento/error/pausa al retornar, pero no elimina la espera interna observada de ~30 s sin tarjeta. [PROB-0013](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md). | Botón «Registro SD», solicitud frente a estado, último error persistente, filas escritas. No traducir error genérico de disco automáticamente a «sin tarjeta». |
| Después | Validar dirección, Hall/PPR y MPU; probar modos y START con relé/motores según el protocolo previsto. | Página por señal, coherencia de datos y validez. El cero Hall delantero actual también puede representar ausencia/fallo: ampliar snapshot con edad/validez antes de mostrarlo como rueda parada. |
| Después | Confirmar protocolo y configuración del BMS. | Mostrar «sin validar» hasta entonces. SOC y temperatura por separado, cada uno con validez y edad; no inventar tensión/corriente/celdas. |
| En placa | Legibilidad, táctil útil, carga de GUI, latencias y SD con/sin tarjeta. | Zonas táctiles explícitas, tipografía y avisos probados en F746. La aceptación del banco F411 no valida esta nueva interfaz. |

Los ensayos integrados y el registro del ESP32 se prepararán por Wi-Fi. USB del ESP32 solo fuera del kart, según la restricción permanente. La geometría útil y la priorización de avisos se aplican también durante desarrollo.

## Propuesta recomendada

**Inicio «Estado y causa», con una página adicional «Kart y señales».** Se muestran como dos alternativas navegables para comparar; se recomienda combinarlas en la siguiente iteración, no exigir escoger una para descartar la otra.

1. **Resumen:** velocidad recibida o `--`, marcha, modo solicitado/recibido/aplicado, petición START, freno y estado de registro. Causa principal siempre visible; aviso de interacción separado, sin tapar una inhibición más importante. No llamar «contactor cerrado» a una petición lógica.
2. **Entradas:** pedal y dirección con valor físico y normalizado, rangos/umbrales de desarrollo y condiciones cumplidas/no cumplidas. Un asistente posterior puede guiar «soltar → medir → pisar → medir → revisar», sin aplicar calibraciones automáticamente. La conversión a ángulo real exige calibración física.
3. **CAN:** edad del feedback y del estado, nunca recibido/caducado/incompatible, errores RX/TX y versión local. Para enseñar versión exacta del ESP32 en el frontal hace falta un mensaje de identificación; 0x124 solo identifica la versión del protocolo.
4. **SD:** controles nombrados, estado solicitado y observado, archivo/filas/último error, y más adelante duración y marcadores de ensayo. Abrir, reintentar, pedir pausa y estar grabando son estados distintos. La ausencia de tarjeta requiere detección válida.
5. **Ruedas:** esquema sencillo con RPM por rueda; seleccionar una abre su procedencia y calidad. Añadir edades/validez delanteras al snapshot y calibrar PPR antes de interpretar diferencias. RPM no equivale a par ni a PWM aplicado. Un ángulo de volante dibujado sería estimación mientras no se calibre.
6. **BMS y energía, condicionado:** primero validar SOC y temperatura. Tensión, corriente, potencia y energía solo con fuentes verificadas. Las 16 tensiones individuales y autonomía requieren adquisición/modelo adicionales; las tramas actuales no acreditan disponer de ellas.

Ideas posteriores con valor experimental: histórico breve de causas con tiempo de aparición, gráfica corta de pedal/dirección/RPM, selección de una señal ampliada y marcador de ensayo. Usar buffers acotados, medir coste de render y definir nuevos datos del CSV si se añaden eventos. Congelar una gráfica congelaría solo la visualización, con rótulo claro; nunca la adquisición ni el envío de mandos.

Diseño visual: pocas cifras dominantes, unidades junto al valor, mensajes en lenguaje claro, color acompañado de texto, pestañas visibles y aviso principal estable. Reservar los detalles de configuración a páginas específicas. En la maqueta, la banda de estado ocupa 24 px, causa 34 px, contenido 122 px y navegación 44 px: **224 px útiles**; el bisel es externo a la pantalla. Densidad/tamaños son una propuesta, no una certificación de legibilidad física.

## Maqueta, evidencia y límites

[Evidencias](evidencias/propuesta_frontal_2026-10-15/README.md): fragmento conservado, capturas de navegador, comprobaciones de navegación/estados y SHA-256 de las fuentes. Maqueta `frontal-diagnostico`, revisión de esta fecha; HEAD `bebe4528747bc6e05dc1ba244cd3ce9a090a7942` con cambios locales previos dev5 conservados. No se modifica firmware, control, calibración, protocolo, QSPI ni binarios.

**Todos los datos de la maqueta son sintéticos.** El escenario inicial se inspira en la falta de estado extendido documentada, pero no reproduce una captura ni un ensayo real. Selección de páginas, ruedas y solicitud de registro actúan solo dentro de la demostración. La vista estrecha se reorganiza para leerla en el chat; su altura ampliada no propone cambiar la resolución física.

Cobertura: estado, catálogo/ficha DOC-0042, notas/inventario y problemas/ensayos pertinentes; código propio de adquisición/CAN/formateo/vista/modelo, callbacks y arranque de tareas; framebuffer preservado. No nuevas páginas de originales ni lectura integral de bibliotecas. Próximo paso recomendado: implementar Resumen/Entradas/CAN/SD en la copia de trabajo, conservar la prioridad de causas y completar pruebas de regresión antes de compilar y validar en F746; seguir las mediciones pendientes sin confundir esta propuesta con su cierre.
