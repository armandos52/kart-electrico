---
estado: propuesta
actualizado: 2026-10-07
resumen: Viabilidad y mini plan de un banco virtual local para observar entradas, CAN y salidas; no iniciado ni validado contra el kart.
---

# Propuesta de simulador visual del kart

Actualización posterior del 07-10-2026: Armando elige panel conectado a las placas reales y calibración en ejecución. Continuación en [propuesta técnica](PANEL_REAL_CALIBRACION_2026-10-07.md) y [DEC-0002](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0002_panel_real_y_calibracion.md). Las recomendaciones y estimaciones siguientes conservan el estudio inicial; no constituyen el plan de implementación elegido.

## Solicitud y alcance

Armando solicita estudiar la viabilidad, sistemas, tiempo y tokens antes de comenzar. Prefiere observar gráficamente las entradas/salidas de los microcontroladores, sus conexiones y el efecto de programarlos; acepta hacer pruebas reales. Esta nota propone alternativas. No autoriza ni registra una elección definitiva, no se ha creado el simulador y no se ha cambiado firmware ni hardware.

Recomendación: banco virtual local 2D, ampliable a ejecución de la misma lógica de control y después a visualización con placas reales. La simulación complementa los ensayos físicos. La prioridad es observar señales y estados; un modelo físico completo o un kart 3D no son necesarios para ese objetivo.

## Base documentada y pendientes

- Configuración de referencia: curso 2025/26. DOC-0042, página PDF 5: STM32F746G-DISCO frontal, ESP32-WROOM-32D trasero, comunicación CAN con el BMS, dos motores BLDC traseros y un controlador por motor. [Ficha](../03_CATALOGO/fichas/DOC-0042.md), [síntesis](../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md).
- Código fuente de referencia: STM32 del 10-07-2026 y ESP32 del 21-07-2026, según el [inventario](../04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md). No se asume que sea el firmware grabado actualmente. Las librerías exactas del ESP32 están pendientes de reconstruir.
- STM32: pedales, dirección, botones, Hall delanteros, pantalla TouchGFX, SD y recepción del BMS. ESP32: diferencial DIF/ESC/BLQ, PWM y dirección hacia los Kelly, Hall traseros, giroscopio y relés.
- Consulta directa de `ESP32-final.ino` 2025/26: líneas 1–110 y 299–396. PWM izquierdo GPIO32 y derecho GPIO2; relés GPIO16/17; cargador GPIO35. Constantes: PWM 5 kHz y resolución de 8 bits (líneas 31–33). Son parámetros del código, no mediciones del montaje.
- En el mismo archivo, líneas 351–368: recepción de orden CAN `0x100` con 8 bytes. Líneas 371–373 y constante de línea 52: tras haber recibido una orden, un silencio superior a 100 ms llama a `pararMotores`; ese bloque no abre los relés. Línea 382: respuesta cada 200 ms. Líneas 326–340: secuencia de relés con esperas de 2 s y 1 s. Son comportamientos de fuente, sin nueva ejecución o comprobación eléctrica.
- Consulta directa de `STM32-final/Core/Inc/main.h`, bloque Private defines: START/STOP PI0, D PC6, R PC7, freno PG6, Hall delantero izquierdo PB4 y derecho PG7. Cableado y polaridades reales pendientes.
- Declaración del usuario: 16 celdas disponibles y funcionales, reconfirmadas el 07-10. Conexión 16S activa y configuración del BMS pendientes; no cargar como actual la tabla histórica 15S.
- El comportamiento del diferencial ESC no está validado físicamente en la documentación revisada. No presentar una animación como validación de estabilidad.

Los archivos citados están bajo `02_ORIGINALES/entrega_2026-09-28/Smart Eletric Kart Cross-2223406-2223403/Smart Eletric Kart Cross-2223406-2223403/`: ESP32 en `ESP32-final/ESP32-final/ESP32-final.ino` y STM32 en `STM32-final/`. Originales conservados.

## Hipótesis de implementación

### A. Banco virtual de señales (primera entrega propuesta)

Interfaz local en HTML/JavaScript con SVG o canvas: mapa del kart, placas y cables; deslizadores de acelerador y volante; freno, N/D/R y START/STOP; LEDs de estados; mensajes CAN con valores y antigüedad; PWM como porcentaje y forma de onda esquemática; relés animados; ruedas con giro cualitativo; gráficas temporales; pausa, paso a paso y repetición de escenarios.

El motor inicial representa el comportamiento documentado y distingue datos del código, entradas simuladas y hipótesis. Una relación provisional PWM→rpm se rotularía como modelo ilustrativo. El PWM es una señal de mando al controlador: no equivale directamente a par, corriente o velocidad. El estado ordenado a un relé no prueba el cierre físico del contactor.

### B. Ejecución de lógica compartida en el PC (objetivo para programar)

Crear la copia v0 según [DEC-0001](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos.md). Separar gradualmente módulos C/C++ de marchas, diferencial, secuencia de relés y vigilancia CAN de las llamadas específicas HAL/Arduino. Mantener un adaptador de hardware y otro para el PC: reloj virtual, ADC/GPIO, colas de mensajes CAN, Hall, giroscopio y salidas.

El panel enviaría entradas al ejecutable local y mostraría sus salidas, por ejemplo mediante un pequeño servicio local. Al cambiar un módulo compartido se recompilaría para PC y para la placa. Comparar ambos con los mismos escenarios y guardar trazas/versiones. Evitar dos implementaciones independientes del algoritmo que puedan divergir.

Esto ejecuta la lógica extraída del firmware; no emula automáticamente el binario completo, las interrupciones, la concurrencia FreeRTOS ni los tiempos eléctricos del CAN. Debe fijarse qué parte está compartida, cuáles son los modelos y qué solo se verifica en placa.

### C. Panel con placas reales y potencia aislada

Como alternativa o ampliación, grabar el firmware en las placas y observar datos por un canal de diagnóstico, o CAN mediante adaptador adecuado. Comprobar recursos disponibles: el STM32 entregado no tiene UART/USB de diagnóstico configurados según el inventario; no prometer conexión automática por USB. El CAN observado no revela todas las salidas: se necesitan mensajes de diagnóstico y, para verificar PWM/GPIO físicos, instrumentos o captura apropiada.

Definir el montaje de banco sin accionar la tracción, las alimentaciones y las interfaces antes del ensayo. Este método permite probar periféricos y tiempos reales. El coste de cables, adaptadores o instrumentos queda pendiente de inventario; no se recomienda comprar aún.

### D. Modelo físico más avanzado (opcional)

Motor/variadores, masa e inercias, resistencias al avance, neumáticos, dirección, batería y energía exigirían parámetros medidos y calibración con ensayos. Puede interesar después para diferencial/autonomía, pero no hace falta para ver entradas y salidas. Una escena 3D añade presentación y no garantiza precisión.

## Herramientas contrastadas

Consulta oficial del 07-10-2026:

- [Wokwi: hardware soportado](https://docs.wokwi.com/getting-started/supported-hardware): admite ESP32; los STM32 listados son C031, L031 y F103C8, no F746. Puede estudiarse para un subsistema ESP32; no asumir compatibilidad del conjunto ni del MCP2515/firmware entregado. No se propone como solución local íntegra sin verificar dependencias y condiciones.
- [TouchGFX: simulador PC](https://support.touchgfx.com/docs/development/ui-development/working-with-touchgfx/simulator): permite compilar/probar la interfaz, animaciones y estados en el PC con una capa de abstracción distinta. El rendimiento y la interacción con backend real requieren la placa. Opción complementaria para la pantalla, no simulación integral del kart.
- El proyecto conserva un esquema Proteus, según el inventario. Tener el esquema no demuestra disponer de modelos de ambos micros y periféricos ni de las licencias necesarias; evaluación pendiente, sin compra propuesta.

## Mini plan antes de construir

1. Delimitar mapa de entradas/salidas y versiones. Confirmar las placas y construir una tabla pin→señal→destino→polaridad→unidades→fuente→pendiente, distinguiendo montaje físico de código.
2. Preparar banco 2D A con controles, conexiones y trazas. Primera demo: pedal→STM32→CAN→ESP32→PWM izquierdo/derecho, START/STOP y relés. Mostrar procedencia de cada valor y supuestos de ruedas.
3. Integrar B por módulos desde v0. Preservar el comportamiento actual como referencia antes de corregirlo; registrar por separado las mejoras deseadas.
4. Ejecutar escenarios repetibles: recta/giro, DIF/BLQ/ESC, marcha sin condiciones de entrada, freno+acelerador, ADC fuera de rango, CAN perdido/retrasado, reinicio ESP32 y cargador. Un escenario puede revelar un fallo existente; no programar de antemano la respuesta deseada y atribuirla al firmware actual.
5. Contrastar señales y tiempos en el banco real C y ensayos documentados antes de extrapolar al kart. Adaptar el alcance si la visualización de placas reales resuelve la necesidad con menos trabajo.

Primer criterio de utilidad: Armando puede seguir una entrada hasta su salida, ver qué módulo la modifica y reproducir un fallo con una traza. Para B, una modificación del código compartido debe producir una diferencia observable y comprobable en el mismo escenario.

## Estimaciones preliminares

Son órdenes de magnitud propios, no mediciones ni presupuesto cerrado. Horas de trabajo técnico y verificación del conjunto, no tiempo garantizado de una respuesta de IA. Las etapas pueden repartirse entre varias sesiones. Tokens totales procesados durante asistencia (entrada, contexto reutilizado y salidas, con razonamiento según contabilización), no solo tamaño del código; las pruebas sin IA no gastan tokens.

| Etapa | Trabajo adicional orientativo | Tokens adicionales orientativos |
|---|---:|---:|
| A: mapa y banco virtual 2D inicial | 4–10 h | 50.000–150.000 |
| B: integrar lógica compartida y escenarios | 16–40 h | 150.000–500.000 |
| C: conectar placas, diagnóstico y contraste de señales | 12–32 h | 100.000–350.000 |
| D: dinámica/energía con calibración básica | 40–100 h | 300.000–1.000.000 |

A+B: aproximadamente 20–50 h y 200.000–650.000 tokens, sin C/D. Emulación completa, electrónica de potencia y 3D detallado no están presupuestados: pueden requerir varias semanas y superar ampliamente esos rangos. Herramientas/librerías ausentes, refactorización, ajustes visuales y ciclos de depuración pueden multiplicar el consumo. Reestimar tras la primera demo y medir el consumo real disponible; no convertir estos rangos en porcentajes de cuota.

El banco A/B se diseñaría para ejecutarse localmente sin servicios de IA, por lo que abrirlo, animarlo y repetir pruebas no consumiría tokens de OpenAI. Desarrollo y futuras peticiones de cambios sí. Puede plantearse sin licencias nuevas; no se ha instalado ni auditado aún toda la cadena de compilación.

[Precios y uso oficiales de Codex](https://learn.chatgpt.com/docs/pricing): el modo de acceso, plan, modelo y reparto entre tokens de entrada/caché/salida condicionan cuota y coste. No se conoce aquí el contrato ni el reparto futuro; no dar una cifra fiable en euros ni aplicar tarifas API al uso incluido por suscripción. Para API: coste = tokens de cada categoría × tarifa correspondiente, más herramientas facturables que se usen.

## Límites de la recomendación

El banco virtual ayuda a entender programación, comunicación y secuencias. No diagnostica por sí solo la energía residual, una alimentación por otra ruta, ruido, soldaduras o respuesta real de los Kelly; [PROB-0002](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0002_esp32_no_activa_contactor_al_encender.md) y [PROB-0003](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0003_energia_residual_al_abrir_contactor.md) siguen requiriendo medidas físicas. Para esos problemas conviene avanzar con pruebas reales preparadas, mientras el banco aporta visualización y pruebas de lógica.

## Cobertura de esta consulta

Se consultaron estado, mapa para localizar, índice, ficha DOC-0042, síntesis, inventario, tareas, ideas y plan de lectura. Se recuperó con el buscador solo PDF 5 de DOC-0042 (texto ya conocido, sin nueva revisión visual). Se leyeron únicamente los fragmentos de código identificados arriba; no se recorrieron librerías ni originales completos. No se cierra ningún lote ni se amplía la cobertura a lectura integral. No hubo compilación, medición en placa, commit ni publicación.
