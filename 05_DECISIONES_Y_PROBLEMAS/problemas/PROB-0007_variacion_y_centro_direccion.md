---
estado: abierto
actualizado: 2026-10-11
resumen: El usuario observa grados variables en el portátil con ruedas quietas; centro, recorridos y causa pendientes de medir.
---

# PROB-0007 · Variación de lectura y centro de dirección

## Declaración del usuario · 08-10-2026

Armando observa que la dirección varía «unos grados» en la **app del portátil** aunque no mueve las ruedas. La pantalla del kart no ofrece esa indicación. Cree que la causa es la calidad del sensor; sospecha que el giro máximo izquierdo y derecho difieren y que el centro puede estar mal ajustado. No se recibe captura, intervalo, duración ni condiciones de alimentación. La variación se registra como observación del usuario; su causa y la asimetría física son hipótesis.

## Evidencia de software y configuración

La [propuesta técnica](../../06_TRABAJO/PROPUESTA_CALIBRACION_DIRECCION_2026-10-08.md) enlaza los fuentes comprobados: el panel convierte el mando CAN normalizado con un centro fijo 2047 y limita la vista derivada a ±20°. El STM32 original normaliza y recorta el ADC antes de transmitirlo. Esto limita qué puede deducirse desde el portátil, pero no identifica la causa de la oscilación.

Montaje actual declarado por el usuario; firmware de referencia ESP32 v1.2/`905b09e`, sin identificación en esta observación. Firmware STM32 instalado y correspondencia con el original aún sin verificar. No hay conexión ni nuevo ensayo físico en esta sesión.

## Actualización · 11-10-2026

Armando declara que al girar a izquierda/derecha aparecía error sin explicación y solicita una guía paso a paso. Se encuentran seis registros Wi-Fi del 09-10, firmware reportado v1.2: 638 muestras, ninguna posición aceptada; dos intentos terminan por lectura normalizada 0 y cuatro por cancelación. Sin saltos de secuencia internos; variación y movimiento no separables porque falta posición física por muestra. El límite CAN puede deberse al escalado/recorte previo; no prueba avería del sensor.

[ENS-0012](../../06_TRABAJO/ensayos/ENS-0012_calibracion_por_pasos.md) corrige la guía del PC: colocación libre antes de pulsar Medir, cuenta atrás, rechazo acotado, causa visible y reintento conservando posiciones anteriores. [DEC-0007](../decisiones/DEC-0007_calibracion_con_medicion_explicita.md). Pruebas sintéticas satisfactorias; no se relajan los criterios ni se afirma que la dirección física haya quedado calibrada. Próximo: repetir por Wi-Fi, registrar posiciones inmóviles identificadas y contrastar escalado/sensor si persiste límite o variación. El problema sigue abierto.

## Diagnóstico propuesto

Registrar por Wi-Fi centro con ruedas alineadas, extremos independientes y retorno al centro desde ambos lados, conservando muestras, tiempos, pérdidas, configuración y referencias físicas. Separar fluctuación, desplazamiento de cero, repetibilidad, saturación y asimetría. Comprobar sensor/acoplamiento, alimentación/cableado/adquisición si los datos lo requieren. No atribuir el problema al sensor ni esconderlo con un filtro sin evidencia.

## Estado y criterio de cierre

Etapa A aprobada por Armando e implementada el 08-10-2026, según [DEC-0004](../decisiones/DEC-0004_proponer_calibracion_antes_de_aplicar.md) y [ENS-0005](../../06_TRABAJO/ensayos/ENS-0005_calibracion_direccion_panel.md). Para cerrar: disponer de registros reales, tolerancias acordadas, centro/referencias contrastados y resultado repetible; documentar por separado causa confirmada o limitación residual. Las pruebas de software no resuelven la causa física. Calibración del panel no corrige automáticamente el control del ESP32.
