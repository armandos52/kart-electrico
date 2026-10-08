---
estado: en curso; autorizada, sin grabar
actualizado: 2026-10-08
resumen: Seguimiento de todas las propuestas de la revisión v1.2 y límite solicitado de marcha atrás a 20 km/h.
---

# Implementación de robustez v1.3

Armando autoriza implementar por etapas todas las propuestas de [la revisión](REVISION_CODIGO_V12_2026-10-08.md), trabajando también desde el último fuente STM32 disponible aunque no haya acceso a su placa/pantalla. Solicita limitar R a **20 km/h** y declara que actualmente alcanza a veces 50 km/h (declaración, no medición recibida). Prueba real/comparación con el frontal instalado se harán después. Pide no realizar ahora una pre-simulación: se omite simulación dinámica; compilación y pruebas unitarias dirigidas de código son verificación de implementación, no un modelo físico del kart.

No se graba hardware. Origen ESP32 `905b09e`; binario v1.2 y respaldo original conservados. El fuente STM32 disponible es 2025/26 y no se presenta como identificado en la placa. Los originales se conservan intactos. La autorización actual permite código sin esperar conexión al frontal ni nueva aprobación de cada corrección.

## Requisitos y seguimiento

| Requisito | Estado | Evidencia de cierre necesaria |
|---|---|---|
| A01 STOP persistente; secuencia y cargador coherentes | En curso | Pruebas de transición/STOP y revisión de una única aplicación de salidas |
| A02 Timeout y rearme con pedal liberado | En curso | Prueba pérdida/recuperación sin rearranque con pedal |
| A03 Contenido CAN y ADC, frescura y diagnóstico | Pendiente | Contrato compartido, pruebas de rechazo y fuente frontal que invalida conversión fallida |
| A04 Permisos antes de PWM; parada prioritaria | En curso | Código de salidas y pruebas de N/freno/fallo sin escritura transitoria de potencia |
| A05 Rampa común efectiva | En curso | Prueba de tasa y parada inmediata |
| R ≤20 km/h: techo de mando y gobernador por velocidad | En curso | Pruebas numéricas de límite por rueda en todos los modos; medición física posterior |
| A06 ESC firmado, velocidad medida, MPU supervisado | Pendiente | Pruebas unitarias de simetría y fallo de lectura; ejes/ganancia por validar físicamente |
| A07 RPM con dt real, caducidad y detección de anomalías | Pendiente | Pruebas con intervalos y calidad; PPR/geometría todavía históricos |
| A08 Watchdog, errores TX/CAN y arranque | Pendiente | Compilación hardware y revisión de política de fallo; sin autoarranque tras reset |
| A09 SD propiedad única, pausa/reapertura/error, versión y datos | Pendiente | Compilación frontal y pruebas de máquina de estados donde sea viable |
| A10 GUI por una tarea, datos válidos, solicitado/recibido/aplicado/motivo | Pendiente | Fuente frontal, compilación y comprobaciones del contrato; pantalla física posterior |
| A11 Telemetría sin cola, errores visibles y sin bloquear control | Pendiente | Guardia de recursos, pruebas y compilación |
| A12 Modularizar, constantes, USER CODE y compilación reproducible | Pendiente | Diff original/copia, manifiesto, fuentes y binarios versionados |
| Panel PC entiende nuevos estados y datos | Pendiente | Pruebas del lector y protocolo documentado |
| Registro y entrega para ensayo real | Pendiente | Versiones, hashes, diario, procedimiento por Wi-Fi y lista de pendientes físicos |

## Límites de la implementación física

El límite R actuará sobre el mando y sobre la velocidad estimada con Hall. No hay frenado eléctrico implementado: cortar aceleración no garantiza detener una aceleración por pendiente/inercia. El umbral solicitado es 20 km/h, sin afirmar una velocidad máxima física validada antes del ensayo. PPR 89/68 y geometría histórica requieren contraste; ninguna corrección de software convierte esos valores en medición. La calibración y los ajustes por motor se prepararán explícitamente, sin inventar coeficientes del vehículo.

## Cronología

- 08-10: se comprueba estado actual, referencias y autorización. Hay cambios locales previos que se conservan. Inicio de separación del control ESP32 de sus periféricos para corregir lógica y comprobarla sin placa. Acceso STM32 diferido por el usuario; no bloquea programación de su copia.
