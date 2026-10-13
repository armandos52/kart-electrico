# ENS-0014 · Carga completa y comprobación del SOC de la batería 16S

Registrado: 13-10-2026. **Estado: planificado, no ejecutado.**
Ventana prevista por Armando: esta semana, **12–18 de octubre de 2026** (Europe/Lisbon); día y hora pendientes.
Resultado: pendiente; no se ha acreditado el 100 % de carga.

## Objetivo y motivo

Armando solicita conservar el tutorial y anuncia que esta semana se cargará la batería para llegar al «verdadero 100 %». Se registra como intención de realizar una carga completa verificada y establecer una referencia de carga para el BMS. El porcentaje mostrado, por sí solo, no acredita la finalización; alcanzar una tensión total tampoco.

La referencia será el **final de carga definido por el perfil válido para las celdas y el conjunto real**, documentando tensión de cada celda, corriente final, tiempo, temperatura y estado de balanceo/protecciones. No se promete un SOC exacto absoluto ni se fuerza el indicador a 100 %. Una carga completa no mide por sí sola la capacidad útil en Ah ni el estado de salud.

## Configuración y antecedentes

- Usuario: 16 celdas; aproximadamente 3,2–3,3 V por celda en reposo el 13-10. No se recibieron los 16 valores precisos ni condiciones completas.
- Referencia documental Winston LFP/LYP; discrepancia 60 Ah del informe / 70 Ah de la ficha. Identidad/revisión física pendiente en [PROB-0010](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0010_identificacion_y_carga_bateria_16s.md).
- [Tutorial y gráfica conservados](../../04_CONOCIMIENTO/bateria_y_bms/TUTORIAL_CELDAS_Y_CARGA_16S.md): techo prudente propuesto 3,65 V/celda = 58,4 V en 16S, **condicionado a comprobar celda, cargador y BMS; no ajuste aplicado ni garantía de carga completa**. No utilizar 3,8 V como objetivo para perseguir el 100 %.
- Identificar y registrar cargador (modelo, tensión, corriente, perfil), BMS (modelo, versión, 16S, protecciones y balanceo), instrumentos y montaje real antes del ensayo. Límites de corriente, temperatura, corriente de terminación, duración y tolerancia de dispersión: pendientes de documentación del equipo real; no inventados.
- Si se usa telemetría del kart, anotar versiones instaladas de ESP32/STM32 y panel, commit/binario cuando se conozcan. No asumir que el panel dispone de las 16 tensiones: conservar lectura/exportación directa del BMS cuando proceda. ESP32 instalado: registro por Wi-Fi; no exigir USB en el kart.

## Procedimiento previsto y datos a conservar

1. Identificar equipos y completar criterios de carga/finalización antes de aplicar ajustes. Registrar configuración inicial y cualquier cambio autorizado; conservar el bloqueo de marcha y el camino de carga protegido por BMS.
2. Registrar fecha/hora, temperatura ambiente y de batería, tiempo de reposo, tensión total, las 16 tensiones, máxima/mínima/diferencia, SOC indicado, alarmas y estado de balanceo. Conservar datos originales y capturas sin credenciales.
3. Efectuar carga supervisada con perfil compatible CC/CV. Registrar corriente, tensión total y por celda, temperatura y eventos del BMS durante la carga, con atención al tramo final. No puentear protecciones ni subir tensión para compensar una celda que llega antes al límite.
4. Registrar motivo y hora de finalización, corriente final y duración del criterio correspondiente. Si hay corte por protección, desequilibrio, calor anormal u otra anomalía, detener y registrar como interrupción/resultado pendiente, no como carga completa satisfactoria.
5. Tras finalizar, registrar tensiones y SOC con tiempo de reposo explícito. Comparar indicación del BMS con el final de carga documentado. Sincronizar el SOC únicamente según el procedimiento del BMS identificado y dejando constancia de los valores antes/después; no modificar capacidad nominal a partir de esta carga.

Datos del futuro ensayo: conservar juntos logs/exportaciones, capturas y etiquetas en una carpeta de evidencias identificada con ENS-0014; registrar instrumento, unidades, intervalos y hashes cuando se generen. **Todavía no existen mediciones de ENS-0014.**

## Criterio de resultado

Se podrá registrar «carga completa según perfil verificado» cuando consten identidad/configuración y criterio válido de finalización cumplido, con las 16 celdas dentro de límites y ausencia de incidencias no resueltas. Registrar balanceo y dispersión observados; tolerancias aún pendientes del equipo. El 100 % del BMS será una referencia sincronizada conforme a su método, no prueba independiente de capacidad nueva o exactitud absoluta.

Si faltan datos o no se completa el criterio, resultado no concluyente o interrumpido, explicando el motivo. **Resultados positivos, negativos, Ah/Wh aportados, duración y SOC final: pendientes.**

## Trazabilidad de esta preparación

Solo documentación del plan solicitado por Armando: sin carga física, modificación de parámetros, firmware o publicación ejecutada en esta sesión. Se reutilizan fuentes y cobertura del tutorial; no se releen originales ni se amplía la lectura integral. Siguiente paso: identificar cargador/BMS/celdas y fijar los criterios pendientes para el ensayo de esta semana.
