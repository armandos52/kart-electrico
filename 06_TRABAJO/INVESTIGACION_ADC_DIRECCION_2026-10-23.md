# Investigación de dirección y aprovechamiento del ADC · 23-10-2026

## Objetivo y declaración de Armando

Investigar el ruido de dirección y documentar la tabla usada por el ADC, sin modificar firmware ni montaje. Armando declara observar **aproximadamente 1 V a casi 2 V con osciloscopio** y atribuye el problema al recorrido limitado del potenciómetro y a un divisor de tensión mal implementado. Propone ajustar/calibrar el potenciómetro o sustituirlo por un sensor que entregue más variación eléctrica con poco movimiento, idealmente 0–5 V.

La lectura instrumental es una **declaración del usuario**, sin captura recibida. Faltan punto de medida (cursor o entrada ADC), referencia de masa, extremos/centro, alimentación, resistencias, posición física, ruido pico a pico, ajustes de sonda y estado de motores. El divisor incorrecto es una **hipótesis por verificar**, no una causa demostrada por el código.

## Configuración y fuentes

Última instalación acreditada: frontal F746 **dev9-ui** y ESP32 **v1.2-diagnostico**, [ENS-0026](ensayos/ENS-0026_diagnostico_bloqueo_marchas.md). No se identifica nuevamente el hardware hoy. HEAD `bebe4528747bc6e05dc1ba244cd3ce9a090a7942`, con numerosos cambios previos. La cabecera local ya indica **dev11-log**, frente al dev10 descrito en el estado de entrada; no implica instalación ni validación de ese trabajo. Se contrasta la dirección con la copia preservada dev9: mismas constantes, canal, margen y normalización. No se toca el desarrollo SD ni otros cambios.

Fuentes focalizadas, con copias/hashes en [evidencias](evidencias/adc_direccion_2026-10-23/):

- [main.c](../07_DESARROLLO/firmware/stm32_frontal/Core/Src/main.c), líneas 391–414: ADC3, 12 bits, alineación derecha, conversión individual por software, prescaler PCLK/4. Inicialización con 3 ciclos; la función de lectura cambia el muestreo a **28 ciclos**.
- [kart_frontend.c](../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c), constantes 11–15, `read_adc` y `Kart_ControlTick`: dirección canal 8; bruto, validación, recorte y normalización; sin promedio de dirección. El promedio del pedal no se aplica a dirección.
- [MSP](../07_DESARROLLO/firmware/stm32_frontal/Core/Src/stm32f7xx_hal_msp.c), líneas 104–116: dirección **PF10 / ADC3_IN8**, pedal PA0 / ADC3_IN0, modo analógico sin pull.
- [configuración HAL](../07_DESARROLLO/firmware/stm32_frontal/Core/Inc/stm32f7xx_hal_conf.h), línea 148: `VDD_VALUE=3300U`. Es una constante nominal, **no una medición ni una programación de VREF+**. No se encuentra adquisición de VREFINT en Core; la ruta de dirección trabaja con cuentas, no convierte voltios.
- [copia dev9](evidencias/marchas_2026-10-21/despues/Core/Src/kart_frontend.c), líneas 13–15, 129–144 y 172; [ESP32 v1.2](../07_DESARROLLO/firmware/esp32_trasero/src/legacy_v12.cpp), `calcular_angulo`, líneas 126–130.
- [DOC-0042](../03_CATALOGO/fichas/DOC-0042.md), PDF 28 / impresa 14, §3.1.4, texto recuperado con `buscar_documentos.py`: potenciómetro alimentado a **3,3 V**, condensador histórico de **10 nF** entre señal y masa, recorrido **0,84–2,67 V**, centro declarado **1,76 V**, extremos ADC 1042/3313. No describe aquí los valores de un divisor adicional. Montaje de julio, no confirmado para octubre. DOC-0014 se identifica como hoja histórica pendiente; no se adopta como calibración vigente ni se afirma haberla analizado.
- [ST, DS10916 Rev 5, tabla 63, páginas impresas 169–171](https://www.st.com/resource/en/datasheet/stm32f746ig.pdf): conversión entre masa y VREF+, referencia limitada por VDDA; impedancia de fuente y tiempo de muestreo condicionan el asentamiento. Consulta técnica externa, no medida del kart.

## Tabla y transformación real

No hay una tabla de voltajes programada: se usan constantes y una fórmula entera. Para dirección válida:

`N = floor((clamp(A,1042,3313)-1042) × 4095 / 2271)`

`A` es ADC físico; `N` es mando normalizado CAN. Por debajo de **942** o por encima de **3413**, la entrada se invalida; entre margen y extremo se recorta. El ESP32 v1.2 interpreta `ángulo = clamp((N−2047)/47,9, −20°, +20°)`. Estos grados son calculados, no un ángulo físico medido.

Todas las tensiones siguientes son **equivalencias calculadas suponiendo VREF+=3,300 V**, mediante `V≈A×3,3/4095`; no mediciones. Para los puntos de 1/1,5/2 V se redondea A a la cuenta más próxima.

| Punto | ADC físico | V calculados | CAN normalizado | Grados calculados v1.2 |
|---|---:|---:|---:|---:|
| Límite inferior admisible | 942 | 0.7591 | 0 | -20.00 |
| Extremo inferior calibrado | 1042 | 0.8397 | 0 | -20.00 |
| Entrada ≈1 V | 1241 | 1.0001 | 358 | -20.00 |
| Entrada ≈1,5 V | 1861 | 1.4997 | 1476 | -11.92 |
| Centro matemático aproximado | 2177 | 1.7544 | 2046 | -0.02 |
| Entrada ≈2 V | 2482 | 2.0001 | 2596 | 11.46 |
| Extremo superior calibrado | 3313 | 2.6698 | 4095 | 20.00 |
| Límite superior admisible | 3413 | 2.7504 | 4095 | 20.00 |

Los límites eléctricos del ADC y los límites de plausibilidad del software son distintos. A=4095 es fondo de escala ADC, pero inválido para esta calibración de dirección. Un cero normalizado tampoco significa necesariamente 0 V.

## Interpretación de la hipótesis

Si el recorrido 1–casi 2 V se midió **en PF10 respecto a masa analógica**, ocupa menos de unas **1241 cuentas**, aproximadamente **30,3 %** de escala completa y **54,6 %** del recorrido histórico de 2271 cuentas. Son unas 10,3 bits de cantidad ideal de códigos aprovechados, **no ENOB medido**. Si se midió antes de un divisor, hay que aplicar su relación real y la tabla aún no representa esa señal.

El escalado actual multiplica las diferencias por **4095/2271 ≈ 1,803**; cada cuenta física corresponde aproximadamente a **0,0376° calculados** fuera de las saturaciones. Como ejemplo sintético, 10 mV de fluctuación en PF10 equivaldrían a unas 12,4 cuentas y 0,47° calculados. No se ha medido esa amplitud de ruido. Normalizar un recorrido menor no recupera resolución ni elimina ruido; con los límites actuales también puede desplazar el centro y reducir el intervalo de mando.

La observación es compatible con poco aprovechamiento del recorrido y calibración inadecuada. **No demuestra que el recorrido limitado genere interferencia**, ni excluye EMI, mal contacto del cursor, masa/alimentación inestable o asentamiento del ADC. Un divisor pasivo atenúa señal; su carga e impedancia deben verificarse. Sin esquema, resistencias y medidas antes/después no puede calificarse su implementación como incorrecta.

ENS-0026 ya registró A=813 e invalidación por debajo de 942. Con 3,3 V, 813 equivale a unos 0,655 V: no coincide con un mínimo estricto de 1 V. Son observaciones de fechas/condiciones/puntos no equiparados; hay que conciliarlas, no forzar una causa común.

## Opciones y siguiente ensayo

1. Identificar alimentación y resistencia del potenciómetro, recorrido angular mecánico/eléctrico y divisor real. Medir centro y topes en cursor y PF10, además de VDDA/VREF+ y masas, conservando capturas y ajustes. No adoptar 3,3 V nominal como medida.
2. Registrar posiciones inmóviles y tres retornos desde cada lado, con motores desactivados y después bajo condiciones de banco controladas. Conservar ADC bruto, mando, validez y posición/ángulo independientes; registro integrado por Wi-Fi. USB ESP32 únicamente fuera del kart. Medir pico a pico, repetibilidad e histéresis; separar movimiento de ruido.
3. Si el acoplamiento desaprovecha recorrido, ajustar mecánica/posición para ampliar el tramo útil sin alcanzar topes internos. Después calibrar extremos y centro reales en la cadena frontal/control; la calibración visual del PC no basta. Calibrar solo por software mejora correspondencia, pero no crea códigos físicos.
4. Si no basta, estudiar sensor de recorrido angular adecuado (incluido magnético sin contacto), atendiendo a sensibilidad V/°, ruido, repetibilidad y alimentación. **0–5 V no se conecta directamente al ADC**: requiere acondicionamiento al rango permitido por VREF+/VDDA, con margen y protección. Un sensor alimentado a 5 V no necesariamente entrega 0–5 V. No se selecciona modelo ni divisor sin especificaciones/medidas.

Resultado: tabla y diagnóstico documental completados; hipótesis parcialmente compatible con el software, divisor y causa del ruido pendientes. Sin ajuste, filtro, compilación, instalación ni ensayo físico nuevo. Se mantienen abiertos PROB-0007/0015. Cierre mediante comparación antes/después bajo condiciones iguales, centro/topes correctos, señal válida y mejora cuantificada del ruido/resolución.


## Ampliación para el informe: ruido general, filtrado y placas · 23-10-2026

**Declaración adicional de Armando:** existe ruido general en el sistema, además del problema de aprovechamiento del potenciómetro. Indica que las placas realizadas no tienen GND extendido por toda la placa. Solicita registrar como mejoras el filtrado pasaaltos/pasabajos y, posteriormente, mejorar las placas, la arquitectura electrónica y el trazado de pistas con criterios eléctricos definidos, más allá de conseguir conectividad.

Se conserva como observación del usuario y propuesta de trabajo. No se han inspeccionado en esta sesión esquemas, capas de PCB ni placas físicas; la ausencia/extensión del plano de masa y su contribución al ruido están pendientes de contraste. La hipótesis del potenciómetro no sustituye ni cierra la investigación de ruido general.

**Criterios para desarrollar la propuesta:**

- Caracterizar por separado ruido en alimentación, referencia/masa y señales, con amplitudes, frecuencias y condiciones de funcionamiento. Elegir filtros según la banda útil y el acoplamiento identificado; no fijar cortes/componentes todavía.
- Estudiar pasabajos para atenuar componentes rápidas no deseadas, verificando retardo y respuesta. El pasaaltos se considerará solo donde sea necesario conservar una componente variable y eliminar continua/deriva: **no debe eliminar la componente continua de la posición del volante ni del pedal**. No se propone encadenar ambos filtros indiscriminadamente en todos los sensores.
- Revisar el plano de masa como camino de retorno continuo y de baja impedancia. Extender cobre GND por sí solo no garantiza eliminar ruido; comprobar discontinuidades, lazos y corrientes compartidas entre potencia y medida. No adoptar cortes de plano o masas separadas sin estudiar los retornos.
- Rediseñar la arquitectura y ubicación de bloques para controlar el acoplamiento entre potencia/conmutación, lógica y entradas analógicas. Revisar desacoplo junto a alimentación de circuitos, conectores, cableado y retornos de sensores.
- Trazar pistas con propósito: recorrido de ida y retorno, corriente y anchura, separación respecto a nodos ruidosos, longitud de señales sensibles y área de los lazos. Documentar el motivo de cada cambio en esquema/layout y conservar revisión anterior y nueva.

**Secuencia propuesta por Armando:** estudiar/aplicar filtrado adecuado y después mejorar placas; planificar ambos de forma coordinada, ya que un filtro no sustituye un retorno de corriente bien diseñado. No se ha aprobado ni ejecutado un diseño concreto de filtro o PCB.

Para el informe final: registrar configuración y revisión de placa, fotos/esquema/layout, instrumentos y puntos de medida, trazas originales, comparación antes/después con las mismas condiciones, reducción de ruido y efectos negativos (retardo, pérdida de información o errores). Objetivo: reducir interferencias y mejorar integridad de señal; no prometer eliminación total. Sin intervención física, firmware modificado ni nuevos resultados experimentales en esta ampliación.


### Corrección de la propuesta por Armando · 23-10-2026

Armando rectifica expresamente: **solo filtro pasabajos para el ruido general**. Se retira el pasaaltos de la propuesta vigente; su mención anterior se conserva únicamente como cronología. Se mantiene la mejora posterior de placas, arquitectura, planos de masa y retornos.

Estudiar el pasabajos en los puntos afectados, preservando la componente continua y la banda útil de cada señal. Orden, frecuencia de corte y componentes pendientes de caracterizar el ruido y verificar retardo/respuesta. No se presupone que un mismo filtro sirva para todas las señales ni que resuelva problemas de masa o interferencias dentro de la banda útil. Corrección documental, sin diseño implementado ni ensayo nuevo.
