---
estado: vigente
actualizado: 2026-10-07
resumen: Índice y diario de evidencias para el relatório final de ingeniería del kart.
---

# Diario para el relatório final

Regla permanente en [AGENTS.md](../AGENTS.md): documentar cambios y ensayos con datos, resultados y límites. Este diario facilita redactar después; no sustituye los registros ni transforma hipótesis en resultados.

| Fecha | Trabajo y motivo | Evidencia / resultado | Límites y siguiente paso |
|---|---|---|---|
| 07-10-2026, tarde | Preservar firmware previo a cambios | [Respaldo original](../07_DESARROLLO/respaldos/esp32/2026-10-07_180133/RESPALDO.md): tres lecturas de 4 MB con mismo SHA-256 | No identifica con certeza el fuente exacto; copia externa pendiente |
| 07-10-2026, tarde | Añadir telemetría USB/Wi-Fi de lectura | [ENS-0001](ensayos/ENS-0001_telemetria_esp32_usb_wifi.md): v1 sufrió panic; parche DLC de mcp_can en v1.1; transmisión y reconexión sin pérdidas en banco | Sin mandos CAN reales ni MPU operativo; no valida circulación |
| 07-10-2026, noche | Recuperar funcionamiento tras prueba fallida con v1.1 | [ENS-0002](ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md): usuario declara motores sin funcionar con v1.1; imagen original restaurada y verificada en 4 MB; después confirma que el kart volvió a funcionar | Comparación funcional declarada por el usuario, sin logs en el kart. La causa de la regresión no está identificada |
| 07-10-2026, noche | Regla documental y revisión v1.2/panel | [DEC-0003](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0003_trazabilidad_para_relatorio.md), regla en AGENTS; [ENS-0003](ensayos/ENS-0003_v12_diagnostico_y_panel.md), commit firmware `905b09e`: USB 321 y TCP 129/43/43 sin saltos/reinicios; diagnóstico disponible sin CAN; resumen, buscador, gráficas y animaciones, 13 tests y QA Chrome | Solo ESP32 fuera del kart por USB. Dirección ADC estimada sin calibrar. Prueba de motores/CAN/MPU/alimentación en el kart pendiente; [PROB-0006](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0006_regresion_marcha_con_firmware_telemetria.md) sigue abierto |

## Cómo utilizar las evidencias al redactar

### Restricción de acceso durante el montaje · 07-10-2026

Armando indica que no puede conectar USB cuando el ESP32 está instalado en el kart. Por ello, el ensayo integrado pendiente de v1.2 utilizará Wi-Fi directo y el registro JSONL del panel local. El USB se utiliza con el módulo fuera para grabar, respaldar, recuperar y comprobar en banco. Se incorpora esta condición a AGENTS y al procedimiento del panel. Es una declaración sobre el montaje, no un nuevo ensayo ni una confirmación de funcionamiento de los motores con v1.2.

### Criterios de redacción

- Explicar por qué se hizo cada cambio, qué se esperaba y qué ocurrió, incluyendo los intentos fallidos.
- Referenciar versiones en [VERSIONES](../07_DESARROLLO/firmware/VERSIONES.md), ensayos en `ensayos/`, problemas y decisiones en `05_DECISIONES_Y_PROBLEMAS/`.
- Conservar unidades, configuración y condiciones. Separar tiempos del firmware y del PC; capturas sintéticas sirven para evaluar interfaz, no dinámica del kart.
- Registrar las declaraciones del usuario como tales. Completar mediciones en el kart antes de escribir conclusiones sobre control, alimentación y seguridad.
- Preparar figuras y tablas desde datos conservados; ocultar credenciales y no publicar los binarios con NVS.
