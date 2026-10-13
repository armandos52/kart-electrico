---
estado: etapa_a_aprobada_e_implementada
actualizado: 2026-10-08
resumen: Centro físico, extremos independientes, repetibilidad y perfiles de dirección para el panel; demostración sintética, sin aplicación al kart.
---

# Propuesta de calibración de dirección

**Actualización posterior, 08-10-2026:** Armando aprueba expresamente la etapa A. Implementada en el panel como `0.2.0-direccion`, con ensayos automáticos descritos en [ENS-0005](ensayos/ENS-0005_calibracion_direccion_panel.md). Los apartados siguientes conservan la propuesta y el estado anterior a la aprobación; no deben interpretarse como bloqueo vigente. La calibración física sigue pendiente.

## Solicitud, alcance y configuración

Armando comunica el 08-10-2026 que la indicación de dirección **varía unos grados en la app del portátil sin mover las ruedas**. Aclara que la pantalla del kart no muestra ese dato y que cree que la variación se debe a la calidad del sensor. Sospecha recorridos angulares distintos hacia izquierda y derecha y desconoce si el centro está bien ajustado. No aporta intervalo numérico, captura ni duración. Son observaciones e hipótesis del usuario, no mediciones verificadas. [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md).

Pide primero propuesta y demostración gráfica; **la implementación en la app requiere su aprobación posterior**, según [DEC-0004](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0004_proponer_calibracion_antes_de_aplicar.md). Esta sesión documenta y crea una simulación aislada. No modifica el panel, conecta al kart, escribe perfiles activos, compila ni graba firmware.

Configuración de referencia: arquitectura documental STM32 → CAN 0x100 → ESP32 → telemetría Wi-Fi → panel del portátil. Última grabación registrada: `esp32_trasero-v1.2-diagnostico`, commit `905b09e`; no se identifica el binario durante la observación actual ni el STM32 instalado. Repositorio consultado: HEAD `968b828ea60f02d3fbfe14b155640aacc8331993`, con cambios locales previos en el panel, incluidos módulos nuevos. Los hashes de los fuentes realmente consultados se conservan en [evidencias](evidencias/calibracion_direccion_2026-10-08/fuentes_consultadas_sha256.json); HEAD por sí solo no describe esos cambios.

## Qué se ha comprobado en los fuentes

1. [Panel, `nucleo.js`](../07_DESARROLLO/panel_local/static/nucleo.js), función `steeringPresentation`, líneas 31–36: si recibe `steer_raw`, presenta `(valor − 2047) / 47.9`, limitado a ±20°. Con `steer_deg` tiene otra ruta, limitada a ±25°. Estos son criterios del código, no ángulos físicos medidos. La representación actual puede ocultar recorrido fuera de sus límites.
2. [ESP32 de trabajo](https://github.com/armandos52/kart-electrico/blob/905b09e/07_DESARROLLO/firmware/esp32_trasero/src/ESP32-final.ino), `calcular_angulo`, líneas 123–127: usa la misma relación de centro 2047 y límite ±20° para el control. **Calibrar solamente el panel no cambia el diferencial del ESP32.**
3. [STM32 original 2025/26 · solo local](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/Core/Src/main.c), líneas 407–425 y 449–450: lee ADC3/CH8, recorta la lectura entre 1042 y 3313 cuentas y la normaliza a 0–4095 antes de enviarla por CAN. `steer_raw` es, por tanto, **mando CAN normalizado**, no ADC eléctrico original. Si ese recorte ya eliminó información, el portátil no puede recuperarla mediante una nueva fórmula.
4. [Telemetría](../07_DESARROLLO/firmware/esp32_trasero/src/telemetria.cpp), líneas 191–199: publica ese mando recibido. [Periodo declarado](../07_DESARROLLO/firmware/esp32_trasero/include/telemetria.h), línea 11: 200 ms, nominalmente 5 muestras/s; no es la frecuencia de adquisición ADC ni garantiza recepción regular. La captura Wi-Fi puede cuantificar la variación que recibe el panel, pero no todos los transitorios eléctricos.
5. [DOC-0042](../03_CATALOGO/fichas/DOC-0042.md), PDF 28 / impresa 14, §3.1.4: describe potenciómetro de columna y normalización; PDF 53 / impresa 39 describe contraste con transportador sobre ruedas delanteras. Solo se recuperó texto: figura y tabla de esa página no se verificaron visualmente. Su descripción del centro no sustituye la comprobación física actual. No se adoptan datos históricos como calibración vigente.

## Flujo propuesto en el panel

Añadir «Dirección → Calibrar» con un asistente de cinco pasos:

1. **Preparar y registrar.** Kart inmóvil, electrónica alimentada y tracción físicamente inhabilitada; registrar suelo/apoyo de ruedas, carga, alimentación, temperatura si disponible y método de alineación. Conexión por Wi-Fi con ESP32 instalado; USB únicamente fuera del kart. Identificar firmware, origen, sesión y perfil existente. N/RPM cero por sí solos no prueban que la tracción esté inhabilitada.
2. **Centro físico y variación.** Colocar las ruedas en la referencia recta comprobada con útil/alineación, sin inferirla de la posición decorativa del volante. Capturar inicialmente 10 s quieto y guardar cada muestra disponible, mediana, mínimo, máximo, pico a pico y desviación estándar muestral. Ventana propuesta para diagnóstico, no tolerancia de aceptación. El cero procede de esta referencia física, no del promedio de los dos topes.
3. **Extremos independientes.** Capturar izquierda y derecha sin forzar los topes. Propuesta inicial: al menos 5 s y 25 muestras válidas diferentes por captura, ampliando si faltan datos. Registrar los valores de cada extremo, centro, signo y recorrido en cuentas. No imponer extremos angulares iguales.
4. **Repetición y grados.** Repetir al menos tres ciclos izquierda → centro → derecha → centro, distinguiendo el sentido de llegada. Comprobar si el centro y los topes se repiten. Para publicar grados, añadir mediciones físicas de referencia: indicar si corresponden al volante, a la rueda delantera izquierda o a la derecha, instrumento, resolución y condiciones. Medir ambas ruedas cuando se quiera conocer ambas; un único sensor de columna no ofrece por sí solo sus dos ángulos. Empezar por extremos y centro, contrastar puntos intermedios y usar una tabla por tramos si la linealidad no resulta suficiente.
5. **Revisar y guardar.** Presentar perfil anterior/candidato, variación en reposo, retorno al centro, asimetría en cuentas y, si existen referencias físicas, grados por lado. Guardar borrador con nombre y notas; activar explícitamente solo en la visualización del panel. Conservar versiones y permitir volver al perfil anterior. No sustituir silenciosamente un perfil por repetir el ensayo.

La futura interfaz mostrará simultáneamente «CAN normalizado», «estimación anterior», «posición calibrada por lado» y el ángulo identificado por su referencia, además del estado del dato. En el mapa no se representará un mismo ángulo como medición independiente de las dos ruedas. Sin referencias físicas, mostrar únicamente cuentas y porcentaje de recorrido, y mantener los grados como no calibrados.

## Cálculo, estabilidad y datos inválidos

Sean L, C y R las medianas capturadas en izquierda, centro y derecha, en cuentas CAN. Determinar orientación `s = signo(R − L)` y trabajar con `x = s·r`, `l = s·L`, `c = s·C`, `d = s·R`; exigir `l < c < d`. Esto admite sensores cuyo valor decrece al girar a la derecha.

- En el lado izquierdo: `u = (x − c)/(c − l)`.
- En el lado derecho: `u = (x − c)/(d − c)`.
- Mostrar `100·u %`: −100 % en izquierda, 0 % en centro y +100 % en derecha. Los recorridos `|C − L|` y `|R − C|` permanecen visibles; normalizar cada lado no demuestra simetría mecánica.
- Si existen referencias físicas válidas de magnitudes A_I y A_D, interpolar inicialmente `θ = u·A_I` en izquierda y `θ = u·A_D` en derecha. Las magnitudes son independientes y pertenecen al mismo mensurando. La interpolación es una aproximación por validar en puntos intermedios, no una medida independiente en cada posición.
- Si la lectura sale del intervalo capturado, señalar «fuera de rango» y conservar el valor original; no ocultarlo mediante saturación silenciosa ni extrapolar grados como válidos.

No confundir calibración del cero con eliminación de fluctuaciones. La distinción entre sesgo y variabilidad sigue el [manual NIST, caracterización](https://www.itl.nist.gov/div898/handbook/mpc/section1/mpc11.htm) y [calibración](https://www.itl.nist.gov/div898/handbook/mpc/section3/mpc3.htm), consultados el 08-10-2026. Una gráfica suavizada puede resultar más legible, pero no demostraría reparación del sensor.

Conservar siempre muestras sin suavizado. Después del diagnóstico, evaluar un filtro visual opcional con parámetros y retardo visibles, comparando señal original/filtrada. No introducir autoajuste continuo del cero ni zona muerta del control para ocultar la oscilación. Umbral admisible en cuentas/grados, deriva, separación mínima entre referencias y retardo permitido quedan por acordar con datos reales; no se fija una tolerancia inventada.

La adquisición debe hacerse en el servicio sobre tramas originales, no sondeando repetidamente `/api/state`: cada trama de telemetría se cuenta una vez por nodo/sesión/secuencia. Guardar `tick_ms`, hora de recepción, `can_rx_count`, edad CAN y pérdidas; no contar como nuevos datos los refrescos del navegador. Un valor igual en dos adquisiciones válidas sí es una repetición legítima. Rechazar capturas con pérdida de enlace, reinicio, cambio de fuente/firmware/perfil, CAN caducado o insuficientes muestras. Respetar el timeout CAN de 100 ms existente y distinguirlo de la caducidad del transporte. Una telemetría sin ADC original no acredita la salud de su adquisición; valores planos o saturados requieren comprobación adicional, no aceptación automática.

Si la fluctuación, deriva o retorno al centro impide cumplir tolerancias, conservar el intento como fallido/inconcluso. Hipótesis a comprobar: sensor/contacto, holgura o acoplamiento, alimentación/masa, cableado, adquisición y transformación/representación. La sospecha de calidad del sensor es de Armando; no hay causa confirmada. Si hay saturación heredada o faltan datos para distinguirla, preparar aparte diagnóstico ADC del STM32 y aprobarlo antes de modificar firmware.

## Perfiles y registro reproducible

Perfil propuesto `steering-cal-v1`: identificador/revisión, fecha Europe/Lisbon y UTC, configuración física, nodo/canal (`can_steer_normalized`), firmware/esquema y estado conocido/desconocido, hashes del software, L/C/R, orientación, unidades, ventanas y estadísticas, referencias angulares por rueda/volante cuando existan, instrumento y resolución, filtro visual, criterios acordados, validación pendiente/aprobada y notas. Conservar JSONL original, resumen CSV, perfil JSON y hashes asociados. Una nueva sesión exige confirmar compatibilidad de configuración; no reutilizar por defecto tras cambiar sensor, bieletas, geometría o firmware.

Etapa A propuesta: capturar, comparar y guardar perfiles **en PC** manteniendo la comunicación hacia el ESP32 en lectura. Etapa B futura y separada: decidir qué magnitud angular necesita el diferencial, corregir de forma coherente STM32/ESP32, versionar parámetros, validar en banco y después en kart. Un perfil visual aprobado no autoriza automáticamente cambios de control ni escritura NVS.

## Demostración, comprobación y siguiente paso

La demostración interactiva es aislada y usa L=500, C=2180, R=3500 cuentas; ángulos **inventados** −24°/+19° y secuencia sintética de variación ±48 cuentas alrededor de C (96 pico a pico). Sirve para comparar la fórmula anterior con una transformación por lados y ver que el cero no elimina variación. Sus capturas son instantáneas y simplificadas; no implementan la ventana estadística ni todos los bloqueos del asistente real. La demo asume orientación creciente; la implementación propuesta debe admitir ambas orientaciones. No crea perfiles reales.

Se conservan [previsualización de QA](evidencias/calibracion_direccion_2026-10-08/preview.html), [comprobación automática](evidencias/calibracion_direccion_2026-10-08/verificar_demo.cjs), [11 comprobaciones satisfactorias](evidencias/calibracion_direccion_2026-10-08/qa_resultado.json), [captura ancha](evidencias/calibracion_direccion_2026-10-08/demo-800.png) y [captura estrecha](evidencias/calibracion_direccion_2026-10-08/demo-320.png), ambas revisadas visualmente. Verificados centro, extremos −24°/+19°, captura inválida, recálculo al cambiar centro, fuera de rango y variación persistente; sin errores JavaScript ni desbordes/solapamiento de etiquetas a 800, 375 y 320 px. El intento inválido se rechaza deliberadamente; no es un fallo pendiente del software. Son pruebas de software con datos sintéticos, no ensayo en banco/kart ni evaluación de precisión del sensor. La comparación final de hashes confirmó que los diez archivos del panel/firmware registrados permanecieron intactos durante este trabajo; evidencia en [integridad](evidencias/calibracion_direccion_2026-10-08/integridad_final.json).

Cobertura: notas y fichas existentes, funciones de presentación del panel, fragmentos STM32 407–454, ESP32 123–127, telemetría 191–199 y periodo; texto PDF 28 y 53 de DOC-0042 recuperado con el buscador. Sin revisión integral ni nueva verificación visual del original.

Siguiente paso actualizado: etapa A aprobada e implementada; capturar por Wi-Fi los datos reales de centro, extremos y repetición, acordar tolerancias y contrastar grados físicos. La demo anterior sigue siendo sintética.
