# Estado documentado en el informe 2025/26

Fuente principal: [DOC-0042](../../03_CATALOGO/fichas/DOC-0042.md), informe de julio de 2026. Cobertura inicial: páginas PDF 1, 5, 7–8 y 96–98; las conclusiones corresponden a páginas impresas 82–84. No es una inspección del kart ni una revisión completa del informe.

## Arquitectura descrita

Según el resumen del informe, la unidad frontal utiliza una STM32F746G-DISCO para adquirir señales delanteras y gestionar el panel de instrumentos. La unidad trasera utiliza un ESP32-WROOM-32D para adquirir señales traseras y controlar los motores. STM32, ESP32 y BMS se comunican por CAN. El vehículo tiene dos motores BLDC traseros, cada uno con su controlador, y un conjunto de baterías LiFePO4 con BMS. Se describen un sistema START/STOP y un sistema de carga. Fuente: página PDF 5, verificada visualmente.

Estas afirmaciones describen la configuración del informe. Falta comprobar que corresponde al montaje que se recibe físicamente y cotejarla con el código entregado.

## Resultados y límites declarados

- El informe declara implementada la nueva arquitectura y la comunicación CAN. Fuente: PDF 96.
- Se describen tres modos de diferencial: DIF, BLQ y ESC. Las conclusiones indican que DIF y BLQ se probaron, mientras ESC no pudo validarse por una soldadura defectuosa en GND del giroscopio GY-521; sin la señal de yaw se comporta como DIF. Fuentes: PDF 96–97. No se considera validado el modo ESC.
- El registro en SD se utilizó con una tarjeta alternativa; el informe dice que la tarjeta suministrada originalmente no funcionaba. Fuente: PDF 96.
- La frenada regenerativa y la monitorización remota por Wi-Fi aparecen como desarrollos futuros, no como funciones ya verificadas. Fuente: PDF 98, verificada visualmente.

## Pendientes heredados según las conclusiones

| Asunto                         | Lo que declara el informe                                                                                                               | Fuente PDF |
| ------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------- | ---------- |
| Contactor y ESP32              | Desconexiones del contactor con vibraciones y bloqueos del ESP32; relación causal con fluctuaciones de tensión presentada como sospecha | 97         |
| Distribución eléctrica         | Tornillos deteriorados y observaciones sobre dimensiones/material de la caja del embarrado                                              | 97         |
| PCB trasera                    | Errores de dimensionamiento pendientes de corregir; detalle técnico aún por revisar                                                     | 97         |
| IO35 y giroscopio              | Pull-up provisional y soldadura de GND del GY-521 pendientes                                                                            | 97–98      |
| Dirección, frenos y neumáticos | Necesidades de ajuste o mantenimiento y neumáticos desgastados                                                                          | 98         |
| BMS                            | Descenso abrupto del SOC a cero mientras el sistema sigue permitiendo alimentación; causa no establecida                                | 98         |

Esta tabla registra las observaciones de los autores y no prescribe intervenciones. Las propuestas de solución deben evaluarse con los esquemas, parámetros, pruebas y estado real del vehículo.

## Documentación asociada

- [DOC-0041](../../03_CATALOGO/fichas/DOC-0041.md): informe Word; equivalencia completa con el PDF pendiente.
- [DOC-0028](../../03_CATALOGO/fichas/DOC-0028.md): presentación de 17 diapositivas; texto indexado.
- [Plan por lotes](../../06_TRABAJO/PLAN_DE_LECTURA.md): continuación de la lectura.
- [Dudas de versiones](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0001_versiones_documentales.md): separar configuraciones históricas.
