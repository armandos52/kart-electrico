---
estado: software_verificado_ensayo_fisico_pendiente
actualizado: 2026-10-08
resumen: Etapa A aprobada: captura y perfiles de dirección en PC; verificación automática sintética, sin tocar control/firmware.
---

# ENS-0005 · Calibración de dirección en el panel

## Objetivo y autorización

Registrar y comparar centro, extremos y retorno para investigar la variación observada en el portátil, [PROB-0007](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md). Armando responde «si apruebo» a la propuesta de implementar el asistente en PC manteniendo el control de motores; [DEC-0004](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0004_proponer_calibracion_antes_de_aplicar.md). No se interpreta como autorización de cambios de firmware.

## Configuración y versión

- **Tipo de ensayo:** pruebas automáticas Python, servidor sintético separado y revisión visual en Chrome sin ventana. Sin banco físico ni kart conectado.
- Panel `0.2.0-direccion`, esquema `steering-cal-v1`; fuentes locales sobre HEAD `968b828ea60f02d3fbfe14b155640aacc8331993`, con cambios previos del usuario conservados. Sin commit/push de esta intervención; [hashes finales](../evidencias/implementacion_calibracion_2026-10-08/sha256_final.json) identifican el contenido exacto. No se genera binario.
- [Copias previas de archivos intervenidos](../evidencias/implementacion_calibracion_2026-10-08/antes/) preservadas. Fuente ESP32 y telemetría con hashes intactos: [comprobación](../evidencias/implementacion_calibracion_2026-10-08/firmware_sin_cambios.json). Último firmware físico de referencia sigue siendo v1.2/`905b09e`, sin nueva lectura de identificación.
- App normal: localhost:8766, sin abrir USB/TCP al kart. QA: localhost:8877, `QA-SINTETICO-SIN-HARDWARE`, FW `QA-SINTETICO`, reloj de prueba acelerado e inyectado. Las rutas `/qa/*` existen exclusivamente en el servidor de pruebas, no en `server.py`. El servidor QA se cierra al terminar; el panel normal queda abierto y desconectado.

## Implementación y procedimiento

[Lógica](../../07_DESARROLLO/panel_local/calibracion.py), [interfaz](../../07_DESARROLLO/panel_local/static/calibracion.js) y [guía de uso](../../07_DESARROLLO/panel_local/README.md). Captura de tramas en el servicio antes del refresco gráfico; centro inicial 10 s y 50 muestras, tres ciclos con cuatro posiciones de 5 s y 25 muestras. Duraciones entre muestras, no temporizador de animación. Medianas, rango, pico a pico, desviación estándar muestral y error de retorno. Signo creciente/decreciente y tramos independientes; centro físico, sin promedio artificial de topes.

Los perfiles nuevos guardan configuración, referencias, tolerancias elegidas, firmware/origen, hashes del panel, muestras y referencias angulares opcionales. Activación explícita solo en PC; no se transmiten bytes de configuración al micro. Se conservan perfiles anteriores. Se desactiva al perder vigencia o cambiar conexión/sesión/firmware y se retira la confirmación previa del montaje. Grados solo si se aportan referencias físicas; interpolación intermedia pendiente. Mapa cualitativo basado en porcentaje, sin atribuir dos mediciones de ángulo a las ruedas.

Capturas rechazadas por duplicados, secuencia perdida, CAN caducado/sin renovación o contador retrocedido, reinicio y pérdida de recepción quedan registradas. Saturación 0/4095, bandas solapadas y tolerancias incumplidas bloquean guardado de perfil. El original nunca se suaviza. API protegida por token/origen local y rutas de descarga acotadas.

## Datos y resultados

- [Suite final](../evidencias/implementacion_calibracion_2026-10-08/pruebas_finales.txt): **21 tests satisfactorios** (13 del lector/diagnóstico y 8 de calibración), incluyendo TCP local que verifica ausencia de bytes transmitidos, conteo independiente del sondeo web, sensores de signo invertido, asimetría, persistencia, ZIP y fallos de captura. [Tests nuevos](../../07_DESARROLLO/panel_local/tests/test_calibration.py).
- [QA de interfaz](../evidencias/implementacion_calibracion_2026-10-08/qa_interfaz.json): **16 comprobaciones satisfactorias**, ensayo completo de 13 capturas más un rechazo deliberado, perfil guardado/activado/exportado, bloqueo por token, fuera de rango y pérdida de enlace. Sin errores JS, sin desbordes a 1280/375/320 px. [Captura PC](../evidencias/implementacion_calibracion_2026-10-08/asistente-1280.png) y [estrecha](../evidencias/implementacion_calibracion_2026-10-08/asistente-320.png), revisadas visualmente.
- [Prueba del panel normal](../evidencias/implementacion_calibracion_2026-10-08/smoke_panel.json): inicio desconectado, calibración deshabilitada sin datos y en demo; sin errores ni desbordes. [Captura](../evidencias/implementacion_calibracion_2026-10-08/panel-real-desconectado.png).
- [ZIP de ejemplo exportado](../evidencias/implementacion_calibracion_2026-10-08/exportacion_qa.zip): datos **sintéticos** alrededor de I=500, C=2180, D=3500 cuentas, fluctuación ±2; no es una medida de Armando. A propósito no incluye referencias angulares físicas. Registros QA adicionales en `07_DESARROLLO/panel_local/sesiones/qa_calibracion*`, separados de las futuras sesiones reales.
- `git diff --check` y comprobación sintáctica JS sin errores; avisos de normalización CRLF de Git, sin cambios de contenido por esa comprobación.

## Resultados negativos y correcciones

1. La primera ejecución en sandbox encontró restricciones de temporal y socket de bucle local; se ejecutó la suite con temporal dentro del proyecto y permiso de red local para los simuladores. No es un fallo medido del kart.
2. La revisión visual encontró desborde del panel a anchuras pequeñas al añadir la acción. Se habilitó ajuste de línea y columna de anchura mínima cero; comprobación final sin desborde. No se ocultó contenido para hacer pasar la prueba.
3. Una repetición de la suite detectó 0 tramas en la prueba TCP: [fallo conservado](../evidencias/implementacion_calibracion_2026-10-08/pruebas_intento_tcp.txt). El emisor sintético podía transmitir antes del vaciado inicial de pySerial. Se sincronizó el **fixture de prueba** con apertura completa, sin alterar el transporte real; suite posterior 21/21. Esto no demuestra que se conserve todo mensaje inicial del firmware al abrir el puerto.
4. Se añadió cobertura de contador CAN numérico entero expresado como float y retroceso de contador; no se permite que ese formato admitido por el parser rompa la captura.
5. La repetición de QA tras dejar deliberadamente el reloj sin datos quedó esperando la señal inicial; se canceló y se hizo explícita la reanudación del fixture al iniciar el script. La última ejecución completa es la registrada como satisfactoria.

## Límites, decisiones y siguiente paso

No hay datos reales de dirección, tolerancias físicas acordadas ni diagnóstico confirmado de calidad del sensor. Se conocen únicamente tramas normalizadas; el recorte histórico STM32 puede ocultar información. El registro Wi-Fi no mide todo el ruido eléctrico ni reemplaza la referencia física. Los perfiles son locales; no hay nueva copia externa ni sincronización pública configurada. Si el servicio se cierra durante un borrador, los archivos recibidos se conservan pero el asistente no lo reanuda automáticamente.

Etapa A terminada a nivel de software. Siguiente: con Armando y montaje preparado, conectar por Wi-Fi, alinear, capturar y evaluar repetibilidad/tolerancias. Grados físicos y cualquier cambio del control son etapas pendientes. No se declara resuelto PROB-0007 ni validada conducción por estas pruebas.
