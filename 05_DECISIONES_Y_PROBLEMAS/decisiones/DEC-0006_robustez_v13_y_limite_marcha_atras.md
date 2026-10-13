---
estado: autorizada e implementada en código; contraste físico pendiente
fecha: 2026-10-09
---

# DEC-0006 · Robustez v1.3 y marcha atrás

Armando autorizó después de la auditoría implementar todas las propuestas por etapas, incluidas mejoras en el frontal aunque aún no esté conectado. Pidió R limitada a 20 km/h y declaró que a veces llega a 50 km/h; no se recibió medición. Se difiere comparación con el firmware frontal instalado y ensayo real; no se realiza ahora simulación dinámica.

Se conserva v1.2 y el respaldo original. Rama local `codex/robustez-v13`, desarrollo ESP32 y copia frontal separados, sin grabación ni push. [Implementación y pruebas](../../06_TRABAJO/ensayos/ENS-0009_robustez_firmware_v13.md), [alcance completo](../../06_TRABAJO/IMPLEMENTACION_ROBUSTEZ_V13.md), [contrato CAN](../../07_DESARROLLO/firmware/common/PROTOCOLO_V13.md).

Decisiones de software:

- START necesita observar bajo→alto, N, freno, pedal libre y quietud estimada; STOP es persistente. Datos inválidos/timeout/doble pedal exigen rearme. Un contactor ya cerrado se conserva ante pérdida de mando para evitar ciclado bajo carga; PWM/dirección se anulan. STOP/cargador/salida fallida lo abren.
- Mando R limitado por rueda, gobernador según rueda más rápida: reduce entre 18–20 km/h, corta a 20, permite recuperar por debajo de 19 con rampa. No implementa frenado eléctrico; sin geometría/PPR medidos y sin frenado no se garantiza velocidad física máxima en pendiente/inercia.
- DIF se conserva como reparto abierto; BLQ es igualdad de esfuerzo, no de RPM ni bloqueo mecánico. ESC se corrige, pero sigue experimental hasta validar ejes, signo, ganancia y comportamiento físico.
- ADC/ángulos/PPR/gains quedan explícitos como parámetros históricos o de desarrollo; no se inventa calibración. El panel calibra su visualización, no reescribe control.
- BMS: conservar bruto/edad, ocultar cifras cuya escala real no está acreditada. No presentar 0 % al arrancar como lectura de batería.
- Mantener compatibilidad CAN antigua declarando observabilidad reducida; tras detectar extensión exigir continuidad/frescura. Un ID nuevo requiere confirmar ausencia de conflicto real.

Revisión posterior: ensayo de banco, identificación/respaldo del frontal, secuencia funcional y registro integrado por Wi-Fi. USB del ESP32 solo fuera del kart. Mantener PROB-0008 abierto hasta comprobar los síntomas y resultados físicos, sin atribuir por estos cambios una solución demostrada a PROB-0003/0004/0006/0007.
