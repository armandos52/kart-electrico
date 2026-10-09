---
estado: implementación de código completada; pruebas físicas pendientes
actualizado: 2026-10-09
resumen: Seguimiento de todas las propuestas de la revisión v1.2 y límite solicitado de marcha atrás a 20 km/h.
---

# Implementación de robustez v1.3

Armando autoriza implementar por etapas todas las propuestas de [la revisión](REVISION_CODIGO_V12_2026-10-08.md), trabajando también desde el último fuente STM32 disponible aunque no haya acceso a su placa/pantalla. Solicita limitar R a **20 km/h** y declara que actualmente alcanza a veces 50 km/h (declaración, no medición recibida). Prueba real/comparación con el frontal instalado se harán después. Pide no realizar ahora una pre-simulación: se omite simulación dinámica; compilación y pruebas unitarias dirigidas de código son verificación de implementación, no un modelo físico del kart.

No se graba hardware. Origen ESP32 `905b09e`; binario v1.2 y respaldo original conservados. El fuente STM32 disponible es 2025/26 y no se presenta como identificado en la placa. Los originales se conservan intactos. La autorización actual permite código sin esperar conexión al frontal ni nueva aprobación de cada corrección.

## Requisitos y seguimiento

| Requisito | Estado | Verificación de código y límite físico |
|---|---|---|
| A01 STOP persistente; secuencia y cargador coherentes | Implementado; ver ENS-0009 | Pruebas de transición/STOP y revisión de una única aplicación de salidas |
| A02 Timeout y rearme con pedal liberado | Implementado; ver ENS-0009 | Prueba pérdida/recuperación sin rearranque con pedal |
| A03 Contenido CAN y ADC, frescura y diagnóstico | Implementado; ver ENS-0009 | Contrato compartido, pruebas de rechazo y fuente frontal que invalida conversión fallida |
| A04 Permisos antes de PWM; parada prioritaria | Implementado; ver ENS-0009 | Código de salidas y pruebas de N/freno/fallo sin escritura transitoria de potencia |
| A05 Rampa común efectiva | Implementado; ver ENS-0009 | Prueba de tasa y parada inmediata |
| R ≤20 km/h: techo de mando y gobernador por velocidad | Implementado; ver ENS-0009 | Pruebas numéricas de límite por rueda en todos los modos; medición física posterior |
| A06 ESC firmado, velocidad medida, MPU supervisado | Implementado; ver ENS-0009 | Pruebas unitarias de simetría y fallo de lectura; ejes/ganancia por validar físicamente |
| A07 RPM con dt real, caducidad y detección de anomalías | Implementado; ver ENS-0009 | Pruebas con intervalos y calidad; PPR/geometría todavía históricos |
| A08 Watchdog, errores TX/CAN y arranque | Implementado; ver ENS-0009 | Compilación hardware y revisión de política de fallo; sin autoarranque tras reset |
| A09 SD propiedad única, pausa/reapertura/error, versión y datos | Implementado; ver ENS-0009 | Compilación frontal y pruebas de máquina de estados donde sea viable |
| A10 GUI por una tarea, datos válidos, solicitado/recibido/aplicado/motivo | Implementado; ver ENS-0009 | Fuente frontal, compilación y comprobaciones del contrato; pantalla física posterior |
| A11 Telemetría sin cola, errores visibles y sin bloquear control | Implementado; ver ENS-0009 | Guardia de recursos, pruebas y compilación |
| A12 Modularizar, constantes, USER CODE y compilación reproducible | Implementado; ver ENS-0009 | Diff original/copia, manifiesto, fuentes y binarios versionados |
| Panel PC entiende nuevos estados y datos | Implementado; ver ENS-0009 | Pruebas del lector y protocolo documentado |
| Registro y entrega para ensayo real | Implementado; ver ENS-0009 | Versiones, hashes, diario, procedimiento por Wi-Fi y lista de pendientes físicos |

## Límites de la implementación física

El límite R actuará sobre el mando y sobre la velocidad estimada con Hall. No hay frenado eléctrico implementado: cortar aceleración no garantiza detener una aceleración por pendiente/inercia. El umbral solicitado es 20 km/h, sin afirmar una velocidad máxima física validada antes del ensayo. PPR 89/68 y geometría histórica requieren contraste; ninguna corrección de software convierte esos valores en medición. La calibración y los ajustes por motor se prepararán explícitamente, sin inventar coeficientes del vehículo.

## Cronología

- 08-10: se comprueba estado actual, referencias y autorización. Hay cambios locales previos que se conservan. Inicio de separación del control ESP32 de sus periféricos para corregir lógica y comprobarla sin placa. Acceso STM32 diferido por el usuario; no bloquea programación de su copia.

## Entrega de código · 09-10-2026

ESP32 y STM32 `v1.3-dev2`, panel ampliado compatible con v1.2. [ENS-0009](ensayos/ENS-0009_robustez_firmware_v13.md) conserva compilaciones, pruebas, hashes y fallos del procedimiento. [DEC-0006](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0006_robustez_v13_y_limite_marcha_atras.md) fija políticas; [contrato](../07_DESARROLLO/firmware/common/PROTOCOLO_V13.md) detalla CAN y telemetría.

Cierre de implementación no equivale a calibración ni validación física. Pendientes externos: identidad/respaldo STM32+QSPI, escalas BMS, PPR/ángulos, polaridad Hall/MPU/entradas, carga/jitter/latencia y watchdog medidos, prueba real por Wi-Fi y comparación con v1.2. CubeMX/Designer no se ejecutaron; la compilación comprueba requisitos que no deben perderse al regenerar. ESC continúa experimental. No hay grabación, conexión al kart ni publicación.

Etapas trazables: `2deb9ef` copia frontal v0; `53ebf40` ESP32 dev1; continuación dev2 frontal/contrato/panel/correcciones finales con manifiesto de fuentes. El acceso pendiente a la pantalla no impidió completar los cambios posibles en código.
