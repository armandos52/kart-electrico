---
estado: vigente
actualizado: 2026-10-15
resumen: Estado consolidado, configuración y siguiente paso; historia enlazada por separado.
---

# Estado actual · 15-10-2026

## Aprendizaje PID e IMU · 23-10

Fase de estudio pedida por Armando. [Guía](../04_CONOCIMIENTO/software_y_telemetria/GUIA_PID_E_IMU.md) y [simulador en vista superior](../04_CONOCIMIENTO/software_y_telemetria/simulador_pid_imu.html) con fuerzas, giróscopo y acelerómetro. Hallazgo de código: ESC v1.3 = solo P con zona muerta (deja error estacionario) y consigna sin límite de adherencia. Recomendación futura: PI con anti-windup. Simulación sintética con valores supuestos; sin cambios de firmware; IMU sigue no disponible (PROB-0017).

## Frontal F746 · dev4 grabado y probado en placa

**STM32 v1.3-dev4 grabado y verificado en la F746 real (15-10)**, [ENS-0019](../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md).

- **Antes de grabar:** respaldo interna 1 MiB ×2 + QSPI 16 MiB ×3, idénticos ([RESPALDO](../07_DESARROLLO/respaldos/stm32_f746/2026-10-15/RESPALDO.md)). Firmware previo identificado: **original de la entrega `STM32-final`**, no v1.3.
- **Funcionamiento:** más de 3 h sin reinicios. F01–F03 vistos en el framebuffer y leídos por Armando: [PROB-0011](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0011_presentacion_frontal_preinstalacion.md) cerrado.
- **Táctil, modos y START:** táctil y cambio de modo DIF→ESC→BLQ funcionan; START conmuta una vez por pulsación.
- **CAN:** feedback 0x123 del ESP32 recibido; **0x124 nunca** (ESP32 probablemente v1.2 o anterior, sin identificar).
- **Problemas nuevos:** [PROB-0012](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0012_pedal_reposo_fuera_de_pedal_libre.md), el reposo del pedal (149–194 cuentas) supera el umbral de pedal libre y bloquea modos y D/R; [PROB-0013](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md), sin SD el montaje bloquea ~30 s y el error casi no se ve.
- **Franja ([PROB-0004](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md)):** daño físico confirmado en las filas 0–43; la interfaz original ya pintaba de blanco las filas 0–47.
- **Sin probar:** BMS sigue oculto; grabación SD con tarjeta y relé pendientes.

Armando confirma que estas son **pantallas de desarrollo**. Las de usuario final quedan para el cierre del proyecto ([DEC-0010](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md)).

## Nuevo banco con pantalla

**NUCLEO-F411RE + MCUFRIEND UNO/ILI9341. Banco visual v0.3 grabado/verificado:** diez escenas adaptadas del frontal, B1 avanza al pulsar/soltar. 137 comprobaciones en F411 sin fallos, RGB 3/3 y navegación B1 real registrada. [ENS-0016](../06_TRABAJO/ensayos/ENS-0016_vistas_frontal_nucleo.md), [capturas del LCD para informe](../06_TRABAJO/evidencias/nucleo_vistas_2026-10-14/GALERIA.md), [uso](../07_DESARROLLO/firmware/nucleo_f411_banco/README.md). Datos sintéticos, placa aislada por USB; no valida CAN/motores ni TouchGFX físico del kart. Respaldo triple, herramientas y banco v0.2 conservados en [ENS-0015](../06_TRABAJO/ensayos/ENS-0015_nucleo_f411_pantalla_banco.md).

Armando confirma legibilidad y recorrido completo 10→01 sin saltos en v0.3; aceptación visual/manual declarada, vinculada en ENS-0016.

## Entrega para revisión académica

Armando solicita preparar y subir el trabajo acumulado para recibir feedback de los profesores. Portada, [dossier](REVISION_PARA_PROFESORES.md), [índice de ensayos](../06_TRABAJO/ensayos/README.md), navegación y guía de reproducción preparados. Entrega subida y pruebas de GitHub Actions satisfactorias; revisión/integración en [PR #1](https://github.com/armandos52/kart-electrico/pull/1), corte `revision-academica-2026-10-13`. Resultado trazable en [PUB-0001](../06_TRABAJO/PUBLICACION_2026-10-13.md). No implica nueva prueba física ni lectura integral de originales.

## Configuración conocida

- **Batería:** 16 celdas y 3,2–3,3 V en reposo declarados el 13-10. [Tutorial](../04_CONOCIMIENTO/bateria_y_bms/TUTORIAL_CELDAS_Y_CARGA_16S.md): SOC no deducible con precisión; techo prudente de carga propuesto 3,65 V/celda = 58,4 V, condicionado a identificación/BMS/cargador. Discrepancia 60 Ah informe / 70 Ah ficha en [PROB-0010](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0010_identificacion_y_carga_bateria_16s.md). Sin ajuste ni ensayo físico.

- **ESP32 de desarrollo: v1.3-dev3.** Conserva STOP ante STOP→START en el mismo lote CAN; [ENS-0011](../06_TRABAJO/ensayos/ENS-0011_stop_en_lote_can.md).
- **STM32: v1.3-dev4 grabado en la F746 (15-10).** Conserva START momentáneo de [ENS-0013](../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md); presentación de [ENS-0018](../06_TRABAJO/ensayos/ENS-0018_mejoras_pantalla_desarrollo.md); validado en placa en ENS-0019. ESP32 de desarrollo continúa dev3, sin grabar.
- **ESP32 físico sin identificar.** Última grabación documentada: v1.2 el 07-10. El 15-10 no emite 0x124, compatible con v1.2 o anterior. Frontal identificado y respaldado en ENS-0019. «B1.3» del 13-10 sigue sin binario asociado. [PROB-0009](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0009_start_pulsador_momentaneo.md).
- **Panel: 0.4.0-calibracion-pasos.** Lectura, registro, 3D/esquema, ES/PT/EN y calibración de la visualización en PC. [Guía](../07_DESARROLLO/panel_local/README.md), [ENS-0012](../06_TRABAJO/ensayos/ENS-0012_calibracion_por_pasos.md). Ningún perfil físico validado; no se presume un servidor en marcha.

## Siguiente trabajo técnico

- **Carga esta semana (12–18 de octubre), solicitada por Armando:** [ENS-0014](../06_TRABAJO/ensayos/ENS-0014_carga_completa_bateria_16s.md) planificado para comprobar carga completa y referencia SOC con las 16 celdas, corriente final y perfil válido. Día/hora, identidad de equipos y criterios de terminación pendientes; 100 % todavía no acreditado.

1. Calibrar el reposo y el recorrido del pedal (PROB-0012) antes de esperar selección D/R con dev4. Identificar el ESP32 por Wi-Fi y decidir si se graba dev3. Validar START con relé por Wi-Fi y probar la SD con tarjeta y PROB-0013.
2. Continuar cadena frontal → CAN → DIF/BLQ/ESC → pantalla; calibrar ADC, Hall/PPR y MPU, verificar BMS/configuración real. Límite R estimado no equivale a frenado ni garantía física; ESC experimental.
3. Repetir calibración por pasos con registro Wi-Fi. Seis intentos del 09-10 analizados: ningún centro aceptado; causa eléctrica/posición/recorte sin separar. [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md).
4. Recoger feedback académico sobre aportación, autonomía, protocolo experimental y calendario; [tareas](../06_TRABAJO/TAREAS.md).

USB ESP32 solo fuera del kart para grabación, recuperación y banco. Ensayos integrados por Wi-Fi; registrar configuración, datos originales, fallos, resultados y límites en el [diario](../06_TRABAJO/DIARIO_PARA_RELATORIO.md).

## Documentación y conservación

44 entradas catalogadas: 42 documentos históricos, propuesta personal y cuatro fotos. Extracción inicial de 1.007 páginas PDF; [lectura por lotes](../06_TRABAJO/PLAN_DE_LECTURA.md) sin cerrar. Formatos visuales y Loop pendientes. Originales y binarios conservados localmente; copia de seguridad independiente de todo el archivo pendiente.

[Historial íntegro de estados previos](HISTORIAL_ESTADO_2026-10-13.md) · [Cronología](CRONOLOGIA.md) · [Mapa](MAPA_DEL_PROYECTO.md). Las notas antiguas de “sin push” describen su fecha; la publicación actual se registra en PUB-0001.
