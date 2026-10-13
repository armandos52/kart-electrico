---
estado: abierto
actualizado: 2026-10-07
resumen: Energía residual y alteración del programa declaradas al abrir el contactor; rutas de alimentación y causa por comprobar.
---

# PROB-0003 · Energía residual al abrir el contactor

Configuración: montaje presente con 16 celdas declaradas por Armando el 07-10-2026. Serie activa, BMS, variadores, PCB y firmware grabado pendientes de identificación. Base documental de contraste: 2025/26, DOC-0042. Estado: síntoma declarado, causa no confirmada.

## Observación y alcance

El usuario declara que queda energía al desconectar el contactor, la atribuye a los variadores y que esto afecta al programa. Todavía no sabemos qué permanece encendido, qué tensión tiene, durante cuánto tiempo, si hay USB/cargador conectado ni cómo recupera el funcionamiento.

La existencia de condensadores en un variador es compatible con tensión residual, pero no prueba una ruta de alimentación hacia el ESP32. Puede estar relacionado con [PROB-0002](PROB-0002_esp32_no_activa_contactor_al_encender.md), bloqueo al encender recuperado retirando el módulo; no se fusionan ambos síntomas sin evidencia.

## Evidencia disponible

- **DOC-0042 PDF 43–44 (impresas 29–30):** declara optoacoplador TLP521 y filtro RC en las órdenes hacia los variadores. R = 10 kΩ, C = 3,3 µF, τ = 33 ms. Es la interfaz de señal, no un dimensionamiento de descarga del bus. Texto consultado el 07-10; figura 11 pendiente de revisión visual y contraste físico.
- **DOC-0042 PDF 49 (impresa 35):** describe DC/DC 36–72 V de entrada, 5 V/10 W de salida para la electrónica; punto real de alimentación por verificar.
- **DOC-0042 PDF 74 (impresa 60):** dos relés para atracción y mantenimiento de la bobina del contactor; resistencias 180 Ω y equivalente 90 Ω. No documenta ahí precarga ni descarga del bus.
- **DOC-0042 PDF 90–91 (impresas 76–77):** los autores describen desconexión con vibración, incluso al retirar la resistencia de bobina; el ESP32 no se bloqueó en el ensayo con PC conectado, aunque el contactor seguía soltándose. Son observaciones históricas; la explicación causal de los autores continúa siendo una hipótesis.
- **[DOC-0008](../../03_CATALOGO/fichas/DOC-0008.md), Kelly KEB V3.4, PDF 11 (impresa 10):** indica precarga en el contactor de B+. PDF 14 (impresa 13): contempla condensadores que requieren descarga tras desconectar la batería. Texto consultado, sin comprobar montaje ni trasladar el procedimiento a una prueba doméstica.
- **[DOC-0031](../../03_CATALOGO/fichas/DOC-0031.md):** manual asociado a 2025/26; búsqueda localiza también la advertencia de precarga en PDF 11. Su equivalencia completa con DOC-0008 sigue pendiente.
- **[Manual oficial Kelly KEB V3.4](https://media.kellycontroller.com/new/KellyKEBUserManualV3.4.pdf)**, consulta web 07-10-2026: contraste del principio de precarga y energía almacenada; confirmar familia/modelo antes de aplicarlo.
- **[Texas Instruments SCDA015, sección sobre protección con alimentación apagada](https://www.ti.com/document-viewer/lit/html/SCDA015)**, consulta 07-10-2026: una señal con tensión puede realimentar una alimentación apagada mediante estructuras de protección en dispositivos sin protección adecuada. Principio general; no identifica la ruta ni demuestra esa causa en el ESP32 instalado.

## Hipótesis que hay que distinguir

| Hipótesis | Qué la apoyaría | Qué falta |
|---|---|---|
| H1: carga de condensadores del bus | Tensión aguas abajo del contactor que decae tras apertura, con fuentes adicionales identificadas y aisladas en un ensayo preparado. | Capacidad, tensión frente a tiempo, circuito de descarga y relación con alimentación de lógica. |
| H2: alimentación persistente por otra ruta | Bus o lógica conservan tensión por precarga permanente, PWR independiente, USB, cargador o toma antes del contactor. | Esquema real y estado de cada fuente. Abrir un contactor no garantiza aislamiento de todas las rutas. |
| H3: realimentación por señales | Raíl de lógica parcialmente alimentado cuando su fuente principal cae, relacionado con alguna interfaz aún energizada. | Vía real. La optoaislación descrita exige revisar interfaces, masas y cableado antes de atribuirlo a los Kelly. |
| H4: apagado/arranque irregular | 5 V/3,3 V atraviesan la zona de reset sin una secuencia controlada; registro muestra reinicio o falta de inicialización. | Formas de onda, EN, motivo de reinicio y versión del programa. |
| H5: transitorio al abrir bobinas | Picos breves coinciden con el fallo, aunque no haya tensión residual prolongada. | Protección de bobina, retorno y medición temporal apropiada. Es un fenómeno distinto de la descarga lenta. |
| H6: fallo de firmware o entrada | Alimentación correcta, pero CAN no inicializa, START/IO35 tiene estado inesperado o hay otra condición lógica. | Registro de arranque y señales con el firmware real. |

Una pantalla que se apaga lentamente no basta para elegir entre estas hipótesis. Un mensaje de brownout tampoco localiza por sí solo la causa de la caída.

## Recogida de datos sin intervenir en potencia

Anotar secuencia exacta de llave, START/STOP y contactor; qué luces/pantalla permanecen activas; tiempo observado; qué falla en el siguiente encendido; USB y cargador presentes o ausentes; método usado hasta ahora para recuperar. Leer etiquetas visibles y configuración del BMS sin cambiarla. No hace falta provocar repetidamente el fallo ni conducir para esta primera descripción.

## Ensayo eléctrico pendiente con el orientador

Preparar un esquema y puntos de prueba antes de medir: tensión del pack, bus aguas abajo, entrada/salida DC/DC, 5 V, 3,3 V y EN. Instrumentos, conexiones y condiciones deben permitir medir sin riesgo de cortocircuito o movimiento inesperado; no unir la masa de un osciloscopio a puntos de potencia sin comprobar su referencia y aislamiento.

Las desconexiones selectivas, cuando sean necesarias para distinguir rutas, se preparan con alimentación aislada y energía residual verificada, nunca retirando módulos o moviendo conectores en funcionamiento. No puentear el contactor ni descargar condensadores cortocircuitándolos. Esperar un tiempo fijo no acredita ausencia de tensión.

Guardar curva o lecturas de tensión frente a tiempo, fuentes conectadas, versión, estado de CAN y salidas, motivo de reinicio y recuperación. Usar la [plantilla de ensayo](../../09_PLANTILLAS/ENSAYO.md). Un resultado negativo de una prueba no demuestra automáticamente otra hipótesis.

## Ideas de solución, sujetas al diagnóstico

- PCB con secuencia de alimentación definida, interfaces que toleren alimentación parcial y supervisor de reset.
- Separación funcional de la alimentación de lógica y de la conmutación de potencia, preservando el corte de par y el bloqueo de marcha durante carga.
- Estados de GPIO y órdenes de variador seguros antes, durante y después de reset; recuperación que requiera una habilitación deliberada.
- Precarga y descarga de bus evaluadas con el esquema completo. No confundirlas con la economización de bobina.
- Protección de bobinas/transitorios elegida teniendo en cuenta también el tiempo de liberación del contactor.
- Firmware con diagnóstico de inicio, fallo CAN visible y reintentos limitados, antigüedad de datos y registro de reinicios.

Para una descarga resistiva de un condensador **aislado de toda fuente**, V(t) = V0 × exp(−t/RC), energía inicial E = ½ CV0² y potencia inicial en la resistencia P0 = V0²/R. Son ecuaciones de estudio, no valores de componentes para montar. Faltan capacidad, tensión, umbral/tiempo objetivo, alimentación residual, tolerancias, disipación y condiciones de fallo. Una precarga permanente puede seguir aportando energía y hacer inaplicable el modelo aislado.

## Criterios de verificación propuestos

Apagado y arranque reproducibles con rutas identificadas; salidas sin órdenes de tracción durante la transición; alimentación/reset dentro de especificación; pérdida y recuperación de CAN tratadas; funcionamiento sin depender del USB; tensiones y tiempos del bus compatibles con el diseño revisado. Definir número de ciclos y límites con el orientador antes del ensayo. Ninguno comprobado todavía.

## Pendientes y decisiones

Esperar descripción detallada del síntoma e identificación del montaje. Revisar esquemas/planos y firmware real, preparar ensayo y entonces escoger intervención. **No hay causa confirmada, solución instalada, diseño de PCB ni valores de descarga aprobados.**
