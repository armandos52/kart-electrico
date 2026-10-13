# Inicio de la investigación y proyecto 2026/27

Fecha: 30-09-2026. Propuesta personal recibida del usuario: [DOC-0043](../03_CATALOGO/fichas/DOC-0043.md). Este documento analiza y organiza el inicio del trabajo; las recomendaciones no sustituyen el cronograma original ni acreditan aprobación de los orientadores.

Versión desarrollada: [propuesta Word V2.0 en portugués](../07_DESARROLLO/propuesta_2026-27/Proposta_Smart_Electric_Kart_V2_2026-27.docx), creada a petición del usuario. Cuatro páginas comprobadas visualmente; borrador para revisión académica, sin modificar la fuente. El renderizador empaquetado no encontró LibreOffice; se verificó mediante exportación PDF de Microsoft Word y renderizado con pypdfium2.

## Valoración

El cronograma tiene una secuencia coherente: conocer el kart, proponer cambios, mejorar el control, mejorar la interfaz y ensayar. Sirve como planificación general, pero necesita concretar la aportación propia, los entregables y las pruebas para funcionar como propuesta de investigación. No contiene por sí solo una justificación del problema, metodología, indicadores de éxito, recursos ni gestión de riesgos.

El desajuste principal está entre los objetivos de febrero y los ensayos de mayo–junio: estimar la autonomía real y validar una mejora del diferencial exige medir antes de febrero. La documentación aparece solo al final; conviene mantenerla durante todo el trabajo. La valoración se limita al contenido y a la estructura lógica extraídos del Word; la maquetación visual no está revisada.

## Lo que propone el Word

Fuente: DOC-0043, tabla «Tarefas» y bloque de objetivos. Octubre–junio se interpreta como curso 2026/27 por la fecha del documento y la mención explícita de febrero de 2027; confirmar el año de cada columna y la fecha de evaluación final.

| Actividad declarada | Periodo marcado |
|---|---|
| Familiarización y estudio del kart | Octubre |
| Propuesta de nuevos elementos de sensorización y control | Octubre–noviembre |
| Perfeccionamiento del control con nuevas funcionalidades | Noviembre–febrero |
| Perfeccionamiento de la interfaz de usuario | Marzo–abril |
| Ensayos | Mayo–junio |
| Documentación | Mayo–junio y evaluación final |

Objetivos escritos para febrero de 2027: «Implementação do diferencial eletrónico inteligente» y «Estimação da autonomia real do Kart». La tabla extraída contiene una columna sin título entre febrero y marzo y varias filas vacías; revisar su función en Word antes de limpiar la plantilla. Revisar también las grafías del título y los nombres. No se ha modificado la fuente.

## Encaje con la documentación existente

La referencia de partida es la configuración descrita en julio de 2026, no una configuración física ya comprobada: STM32 frontal, ESP32 trasero, CAN, BMS y dos motores BLDC con sus controladores. Fuente: [estado documentado](../04_CONOCIMIENTO/integracion_del_kart/ESTADO_DOCUMENTADO_2025-26.md), DOC-0042, PDF 5.

El diferencial electrónico ya aparece desarrollado. Los autores declaran probados DIF y BLQ y describen ESC implementado sin validación experimental por la conexión GND del GY-521. Fuente: DOC-0042, PDF 97, página impresa 83; texto recuperado con el buscador local y contraste con la síntesis previa. Esto no prueba el funcionamiento del kart actual. La aportación propia debería formularse como diagnóstico, mejora y validación de una base existente, precisando qué significa «inteligente»: por ejemplo, validar ESC o investigar otra estrategia tras justificarla. Ninguna de esas opciones queda elegida aquí.

Antes de ampliar sensores hay que comprobar qué señales existen, cómo se calibran y qué limitación concreta impide el objetivo. El informe declara problemas del contactor y bloqueos del ESP32; la relación causal es una sospecha, no un diagnóstico demostrado (DOC-0042, PDF 97). También registra un descenso abrupto del SOC a cero y dificultades con el registro SD (PDF 98 y 96, respectivamente; síntesis previa). La [auditoría](AUDITORIA_2026-09-29.md) señala que KART_LOG.CSV no coincide con el firmware entregado. Estas limitaciones afectan a la repetibilidad de las pruebas y a la autonomía estimada.

## Estructura recomendada para la propuesta

1. **Contexto y punto de partida:** configuración recibida, resultados anteriores y límites que se han comprobado físicamente.
2. **Problema y pregunta de investigación:** qué comportamiento del diferencial se pretende mejorar y cómo estimar la autonomía en condiciones definidas.
3. **Objetivo general y objetivos verificables:** diagnosticar la base, seleccionar una mejora justificada, implementarla, comparar resultados y estimar la autonomía con mediciones.
4. **Alcance:** funciones incluidas, elementos reutilizados y tareas opcionales. Acordar si la interfaz forma parte del objetivo principal o es una extensión.
5. **Metodología:** revisión focalizada, inventario físico, calibración, ensayo inicial, implementación, comparación y análisis de incertidumbre.
6. **Entregables y criterios de aceptación:** evidencias concretas para cada hito; acordar umbrales con los orientadores después de medir la base.
7. **Cronograma, responsabilidades y recursos:** dependencias, disponibilidad del kart y equipos, reparto entre alumnos y revisiones con orientadores.
8. **Riesgos y alternativa de alcance:** fallos heredados, datos incompletos y retrasos; reservar tiempo de corrección y repetir ensayos cuando sea necesario.

Pregunta provisional recomendada: «¿Cómo mejorar y validar el diferencial electrónico de la configuración recibida, y estimar su autonomía real a partir de mediciones reproducibles?». Es una formulación de trabajo; necesita concretar la estrategia de control y las condiciones de circulación.

Para el diferencial, comparar la configuración inicial y la modificada bajo condiciones equivalentes; seleccionar indicadores como seguimiento de velocidad de ruedas, respuesta en curva y error de guiñada si la estrategia lo requiere. Para autonomía, registrar distancia, tiempo, tensión y corriente de batería, estado inicial de carga y condiciones de carga, recorrido y conducción. Si las señales disponibles son suficientes y están calibradas, calcular energía en Wh y consumo en Wh/km; estimar distancia con la energía utilizable medida y el consumo correspondiente. No inventar capacidad utilizable, precisión de sensores ni una cifra de autonomía; distinguir la estimación de una validación en recorrido.

## Cronograma recomendado para discutir

Esta tabla es una recomendación, no un compromiso aprobado. Mantiene el horizonte del Word y adelanta las pruebas necesarias para febrero.

| Periodo | Trabajo y dependencia | Evidencia de cierre |
|---|---|---|
| Octubre 2026 | Lectura focalizada, inventario físico y diagnóstico de alimentación, sensores y registro | Configuración de partida identificada y lista de incidencias con evidencias |
| Octubre–noviembre | Elegir aportación propia y definir protocolo de pruebas y sensorización necesaria | Alcance, indicadores y medios revisados con orientadores |
| Noviembre–diciembre | Corregir impedimentos prioritarios, calibrar y medir comportamiento inicial | Registro trazable y ensayo de referencia reproducible |
| Diciembre–enero | Implementar la mejora seleccionada y realizar pruebas por subsistema | Versión identificada y comparación preliminar con la referencia |
| Enero–febrero 2027 | Ensayos integrados y mediciones de consumo/autonomía | Informe intermedio con resultados, condiciones, incertidumbre y límites |
| Marzo–abril | Mejoras de interfaz apoyadas en resultados y repetición de pruebas afectadas | Interfaz evaluada y regresiones comprobadas |
| Mayo–junio | Validación final y consolidación del informe | Comparación final, datos y documentación reproducibles |
| Octubre–evaluación final | Documentar de forma continua y reservar margen de corrección | Bitácora, versiones y entregables actualizados |

El objetivo de febrero queda condicionado a la disponibilidad del vehículo y a resolver impedimentos de medida y funcionamiento. Confirmar si febrero requiere una implementación, una validación completa o un resultado intermedio.

## Pendientes para cerrar la propuesta

- Confirmar montaje físico, firmware y señales realmente disponibles.
- Elegir qué función concreta del diferencial constituye la aportación propia y acordar los criterios de aceptación.
- Fijar condiciones y medios para medir autonomía, incluyendo calidad del registro y energía utilizable.
- Confirmar fechas académicas, recursos, responsabilidades y alcance de la interfaz con los orientadores.

La incorporación del cronograma está completada y el original se conserva. La propuesta sigue pendiente de concreción y aprobación académica. Esta revisión no amplía la cobertura integral del informe histórico.
