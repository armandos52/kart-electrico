---
estado: alcance_visual_aceptado_sin_implementacion
actualizado: 2026-10-07
resumen: Fotos, vista 3D y animación del diferencial; contraste de señales con fuentes 2025/26 y límites del panel actual.
---

# Fotografías y vista 3D del diferencial · 07-10-2026

**Actualización 08-10:** Armando solicita ahora implementar la mejora visual usando las fotos de Descargas. Tres fotografías incorporadas en [DOC-0044](../03_CATALOGO/fichas/DOC-0044.md); modelo implementado y probado en [ENS-0006](ensayos/ENS-0006_modelo_visual_fotografias.md), decisión [DEC-0005](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0005_modelo_visual_basado_en_fotos.md). Esta nueva solicitud sustituye el aplazamiento y la instrucción de no programar recogidos históricamente abajo. Medidas y validación física siguen pendientes.

## Solicitud y alcance

Armando pregunta si fotografías reales permitirían mejorar el diseño, representar en 3D el giro/diferencial y usar los datos de los códigos existentes. Solicita análisis y precio aproximado y dice expresamente «aún no programes nada». En esta consulta solo se leen fuentes y se guardan notas; no se cambia aplicación, firmware ni hardware. No se han recibido fotografías nuevas ni hay captura física de las placas.

Configuración de referencia: fuentes entregadas STM32/ESP32 2025/26. No está confirmado que coincidan con las versiones grabadas ni con las calibraciones del montaje actual. Véase [inventario](../04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md).

## Elección posterior del usuario

Armando confirma que quiere un modelo 3D sencillo basado en fotografías reales, sin demasiado detalle. La prioridad es entender la matemática del diferencial, consultar los datos disponibles y observar el movimiento y la respuesta del sistema. El aspecto visual queda en segundo plano. Registrado en [DEC-0002](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0002_panel_real_y_calibracion.md); no se inicia programación.

Criterios para el diseño posterior: ruedas y módulos seleccionables; comparación de órdenes PWM con RPM medidas; dirección, reparto del diferencial y radios calculados visibles junto a sus variables/unidades; gráficas y conexiones entre micros. Distinguir medición, orden software y estimación geométrica, conservando avisos de ausencia/caducidad del dato. Estos criterios no confirman todavía disponibilidad física de las señales.

## Resultado del contraste

La vista gráfica puede alimentarse del mismo firmware que controla el kart, sin trasladar el control al portátil ni mantener un segundo algoritmo de control. Sin embargo, **los valores de la demostración actual son sintéticos**. La selección de señales parte del inventario, pero el algoritmo de `makeDemo()` no reproduce el diferencial original ni valida su comportamiento. La [telemetría propuesta](../07_DESARROLLO/panel_local/PROTOCOLO.md) existe en el lector del PC, no en los micros. No hay datos reales mientras las placas están desconectadas.

Abreviaturas de fuentes (rutas conservadas, líneas contadas desde 1):

- [E: ESP32 entregado · solo local](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/ESP32-final/ESP32-final/ESP32-final.ino).
- [M: STM32 entregado · solo local](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/Core/Src/main.c).
- [A: aplicación existente](../07_DESARROLLO/panel_local/static/app.js).

| Dato | Evidencia en las fuentes | Qué falta para verlo como dato real |
|---|---|---|
| Acelerador, dirección, freno y marcha/modo | M:389–434 adquiere/normaliza mandos; E:355–361 recibe `0x100` | Emitir telemetría, identificar versión y contrastar sensores/calibración |
| Ángulo de dirección calculado | E:113–117 convierte ADC con `(adc−2047)/47,9`, limitado a ±20° | No equivale a dos ángulos físicos medidos de las ruedas delanteras; comprobar geometría y calibración |
| PWM izquierdo/derecho ordenado | E:154–159, 213–218 y 221–227 guarda variables y escribe salidas | Publicar variables; una orden software no demuestra tensión, par ni actuación física del Kelly |
| RPM traseras, velocidad estimada y guiñada | E:408–434 calcula RPM/velocidad y transmite `0x123`; E:377–379 lee giróscopo | Publicar por USB; verificar Hall, escalas y giróscopo. La guiñada viaja por CAN multiplicada por 10 |
| RPM delanteras y registro SD | Inventario, sección 3; M:1255–1348 y 348–383 | Instrumentar STM32 o ampliar su retorno CAN. No llegan al ESP32 mediante la trama `0x100` existente |
| SOC y temperatura batería | M:264–275 contiene decodificación BMS | Comprobar protocolo/configuración real y publicar/retransmitir; no son mediciones confirmadas |
| Tensión/corriente batería | Campos previstos por el panel; M:264–275 no los decodifica | Verificar origen y protocolo y añadir adquisición; no presentarlos como disponibles |
| Relés y conexiones entre placas | Estados software ESP32 e inventario | Distinguir orden de relé de contacto físico; verificar cableado, no deducirlo de fotos por sí solas |

A:155–160 inventa RPM, guiñada y batería para la demostración; los dos PWM de la demo son iguales incluso con giro. A:64 y la representación actual usan un ángulo visual común, no la cinemática medida de ambos brazos de dirección. Estos límites están registrados para el diseño posterior; no se modifican hoy.

## Animación propuesta y límites

En modo DIF, E:120–159 calcula reparto interior/exterior a partir de la dirección, con literales de 1,3 m entre ejes y 0,5 m de semivía (E:132–134), y escribe PWM distintos según lado. Son parámetros del código entregado, no medidas verificadas del kart actual. La referencia interna de velocidad de E:128 no es una RPM medida ni garantiza que la rueda alcance la consigna.

Propuesta: mostrar ruedas girando, mandos, PWM ordenado, RPM medidas, modo y guiñada; permitir seleccionar cada motor/placa/rueda. Dibujar radio/arcos y ángulos delanteros como **geometría calculada**, con etiquetas diferentes de las **mediciones**. Comparar mando con respuesta real para detectar diferencias; en BLQ hay igualdad de órdenes, no garantía de igualdad de RPM.

El yaw ideal del ESC se calcula con el valor absoluto del ángulo (E:171, 181–187), mientras la lectura del giróscopo conserva signo (E:379). Se requiere revisar convenciones izquierda/derecha y validar antes de atribuir comportamiento correcto al ESC. La animación por sí sola no lo valida.

Con dirección, RPM y guiñada podemos representar estados y una trayectoria estimada. No hay evidencia revisada de posición absoluta para mostrar el recorrido real sobre un plano. Una integración de guiñada puede derivar. Tampoco se conoce el ángulo independiente de cada rueda delantera. No atribuir al panel medición de deslizamiento, fuerzas, par real ni energía regenerada sin fuente específica.

Las muestras no capturan cada flanco PWM. E:381–384 transmite CAN cada 200 ms; el inventario documenta cálculo de RPM traseras cada 500 ms. Una animación fluida interpolaría esos estados, sin aportar precisión temporal adicional. El osciloscopio queda para señales eléctricas y transitorios.

## Fotografías y arquitectura visual

Fotos de frente, ambos lados, detrás, arriba y detalles de motores/placas permitirían mejorar silueta, colores y ubicación. Para proporciones 3D fiables se necesitan referencias de escala: entre ejes, vía y diámetro de ruedas, al menos. Para representar la dirección mecánica se necesitan medidas/vistas del mecanismo y contraste del sensor. Las fotos no garantizan por sí solas un CAD preciso ni verifican conexiones ocultas.

Propuesta técnica: conservar servicio local Python y telemetría del panel; añadir una escena Three.js en el navegador del portátil. Un modelo más fiel se podría preparar en Blender y exportar a GLB; Three.js documenta ese flujo. El 3D es una superficie adicional de presentación del mismo flujo de datos. No añade un segundo firmware ni permite cambiar algoritmos arbitrariamente sin regrabar.

Fuentes oficiales consultadas: [carga de modelos y glTF/GLB](https://threejs.org/manual/pages/loading-3d-models.html), [licencia MIT de Three.js](https://github.com/mrdoob/three.js/blob/dev/LICENSE), [Blender gratuito y abierto](https://www.blender.org/about/). No se instala software ni se seleccionan modelos comerciales.

## Estimación preliminar de trabajo adicional

Estimación propia de esfuerzo, no presupuesto contratado ni tiempo de ejecución garantizado. Supone reutilizar el panel actual, disponer de fotos/medidas y limitar revisiones. Las alternativas no se suman automáticamente.

| Alternativa visual | Esfuerzo aproximado |
|---|---|
| Mejorar vista 2D con fotos y proporciones reales | 4–10 h |
| 3D sencillo seleccionable: chasis, cuatro ruedas y módulos; animación y cámaras | 12–24 h |
| 3D reconocible del kart real a partir de fotos y medidas | 30–60 h |
| Modelo detallado para presentación con acabados realistas | 60–120 h o más |

Estas horas cubren la capa visual y su enlace al flujo de datos; no incluyen instrumentación de ambos micros, calibración física, resolución de fallos, regeneración ni ajustes bidireccionales. Presupuestar esos trabajos al confirmar las placas/firmware. Blender y Three.js permiten una opción sin pago de licencias; activos comerciales o modelado contratado tendrían presupuesto separado. No hay datos suficientes para una cifra monetaria de asistencia por IA o un total fiable de tokens. Una aplicación local convencional no necesita llamadas de IA para animar cada muestra.

Recomendación propuesta, no implementación autorizada ahora: primero establecer señales reales y versiones; después 3D sencillo con ruedas/módulos y comparación mando–respuesta; por último adaptar aspecto con fotos. Conservar acceso a las gráficas 2D, más claras para comparar mediciones.

## Cobertura y siguiente paso

Leídos de forma focalizada E:110–229 y 350–434; M:250–279 y 389–434; búsquedas en A y consulta de protocolo, inventario y notas. No se revisó integralmente ningún firmware, biblioteca ni informe y no se hizo ensayo físico. No se cierra L4.

Fidelidad visual elegida posteriormente: 3D sencillo basado en fotos, con prioridad en matemática/datos/movimiento. Pendiente recibir fotos/medidas e identificar firmware de las placas. La petición de no programar sigue vigente; esta actualización solo registra la preferencia.
