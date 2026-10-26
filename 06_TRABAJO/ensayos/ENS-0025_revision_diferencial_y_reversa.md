---
estado: revisión ejecutada en PC; plan pendiente de implementación
fecha: 2026-10-17
configuracion: fuentes ESP32 v1.2 y v1.3-dev3; F746 dev8-ui; periféricos simulados
---

# ENS-0025 · Diagnóstico del diferencial y reversa antes de implementar

## Objetivo, motivo y alcance

Revisar arquitectura/matemática DIF/BLQ/ESC ante reversa a unos 50 km/h indicados, y registrar EMI declarada. Armando aclara banco con ruedas elevadas, todos los modos, y pide **plan antes de implementar**. [Informe y plan](../REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md), [PROB-0014](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0014_reversa_y_arquitectura_diferencial.md) y [PROB-0015](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0015_ruido_electromagnetico_sensores.md).

## Configuración y procedimiento

- HEAD `bebe4528747bc6e05dc1ba244cd3ce9a090a7942`, cambios locales anteriores conservados. ESP32 dev3 fuente, v1.2 de commit `905b09e`; F746 dev8-ui conocida por ENS-0024. ESP32 realmente instalado sin identificar. Ninguna conexión/lectura/grabación de placas en esta sesión.
- Se conservan ocho fuentes actuales de alcance y fuente `.ino` v1.2, con SHA-256 y estado Git inicial. No se copian secretos. Se reutiliza arnés histórico de AUD_V12 con cuerpos de funciones íntegros y dispositivos simulados.
- MSVC Visual Studio Build Tools 2022, C++17, x64, `/W4 /Od /utf-8`; Python 3.12 del entorno kart-esp32. Se compilan ejecutables **para PC**, no imágenes Xtensa/ARM. Logs de compilación guardados; v1.2 conserva advertencias de conversión y el arnés añade conversión de índice de modo.
- Se repiten 56 comprobaciones existentes dev3. Arnés nuevo: dos comprobaciones de proporción al arrancar D/R, dos controles de proporción asentada y doce comprobaciones de corte/histéresis/validez en los tres modos. Otro arnés ejercita R v1.2 a pedal completo, dirección centrada, RPM sintéticas elevadas y modo R previamente seleccionado.
- Los datos son entradas deterministas, sin modelo dinámico. El valor 50 se convierte a RPM usando D=0,48 m solo para construir una entrada de test; no calibra ni reproduce la física del kart.

## Resultados

| Ensayo PC | Resultado | Interpretación |
|---|---|---|
| Regresión dev3 existente | 56; 0 fallos | Pasa el alcance previo, que no cubría los dos hallazgos nuevos |
| Diagnóstico dev3 ampliado | 16; **5 fallos**, 11 controles positivos | Dos defectos/propuestas: rampa D/R (2 casos) y memoria con pedal libre (3 modos); no cinco causas físicas |
| Diagnóstico v1.2 | 3/3 reproducciones, 0 discrepancias | En R y RPM elevadas sigue PWM 0/0 = mando completo, en DIF/ESC/BLQ |

Dev3 corta ambos mandos si una sola rueda indica 50 km/h y la otra cero, conserva corte a 19,5 con pedal sostenido e inhibe ante velocidad inválida. Su histéresis no observa 21 con pedal cero y vuelve a aplicar mando a 19,5 al pisar; rampa inicial con +20° da razón D/I 1 en vez de 0,754404. Ambos hallazgos quedan **sin corregir**, respetando el alcance solicitado.

[Datos originales, snapshots, logs, scripts y hashes](../evidencias/revision_diferencial_2026-10-17/README.md). El retorno 1 del diagnóstico dev3 es el resultado esperado de requisitos propuestos todavía incumplidos; no fallo de compilación. No se alteran binarios/firmware actuales ni se ejecutan comandos de motores.

## Cobertura y límites

Estado/mapa, catálogo/fichas DOC-0042/0031, notas software/PROB-0007/0008, ensayos previos y fuentes propias de control, adaptador, frontend, contrato y arneses. Buscador: DOC-0042 texto PDF 43,45,55–57; DOC-0031 PDF 4–5. No nueva revisión visual de esquemas/ecuaciones, bibliotecas completas, hojas/capturas Kelly ni Loop. Geometría y cálculos se contrastan contra fuente de código; no se adoptan parámetros históricos como mediciones actuales. Kelly oficial y Analog Devices para semántica de control/filtros, enlazados en informe. Sin cierre de lote ni lectura integral.

La coincidencia de síntoma con v1.2 y la ausencia histórica de 0x124 son indicios; falta identidad actual. No se mide EMI ni se confirma su causa. Ningún resultado PC valida velocidad/estabilidad física, bus, filtros, PWM analógico o sensores. No nuevo commit/push ni copia independiente de todo el archivo.

## Decisión y siguiente paso

Priorizar supervisor común de R, identidad/escalas, rampa y semántica de modos; después caracterización EMI y regulación/ESC según Kelly real. Mostrar plan antes de implementar, por indicación explícita de Armando. Validación integrada por Wi-Fi; USB ESP32 únicamente fuera del kart. Registrar por separado futuras pruebas de banco, conducción y confirmaciones del usuario.
