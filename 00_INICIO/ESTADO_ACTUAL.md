---
estado: vigente
actualizado: 2026-10-13
resumen: Estado consolidado, configuración y siguiente paso; historia enlazada por separado.
---

# Estado actual · 13-10-2026

## Entrega para revisión académica

Armando solicita preparar y subir el trabajo acumulado para recibir feedback de los profesores. Portada, [dossier](REVISION_PARA_PROFESORES.md), [índice de ensayos](../06_TRABAJO/ensayos/README.md), navegación y guía de reproducción preparados. Verificación/publicación en curso; resultado trazable en [PUB-0001](../06_TRABAJO/PUBLICACION_2026-10-13.md). No implica nueva prueba física ni lectura integral de originales.

## Configuración conocida

- **Batería:** 16 celdas y 3,2–3,3 V en reposo declarados el 13-10. [Tutorial](../04_CONOCIMIENTO/bateria_y_bms/TUTORIAL_CELDAS_Y_CARGA_16S.md): SOC no deducible con precisión; techo prudente de carga propuesto 3,65 V/celda = 58,4 V, condicionado a identificación/BMS/cargador. Discrepancia 60 Ah informe / 70 Ah ficha en [PROB-0010](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0010_identificacion_y_carga_bateria_16s.md). Sin ajuste ni ensayo físico.

- **ESP32 de desarrollo: v1.3-dev3.** Conserva STOP ante STOP→START en el mismo lote CAN; [ENS-0011](../06_TRABAJO/ensayos/ENS-0011_stop_en_lote_can.md).
- **STM32 de desarrollo: v1.3-dev3.** START momentáneo con memoria, antirrebote y STOP en siguiente pulsación; [ENS-0013](../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md). 83 comprobaciones frontal/integración y 56 traseras; compilación ARM documentada, sin grabación de esta corrección.
- **Firmware físico actual sin identificar.** Última grabación documentada ESP32: v1.2 el 07-10; marcha satisfactoria declarada el 08-10. El 13-10 Armando refiere «B1.3» y relé que entra/sale; no identifica binarios. [PROB-0009](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0009_start_pulsador_momentaneo.md). Frontal/QSPI aún sin respaldo/comparación.
- **Panel: 0.4.0-calibracion-pasos.** Lectura, registro, 3D/esquema, ES/PT/EN y calibración de la visualización en PC. [Guía](../07_DESARROLLO/panel_local/README.md), [ENS-0012](../06_TRABAJO/ensayos/ENS-0012_calibracion_por_pasos.md). Ningún perfil físico validado; no se presume un servidor en marcha.

## Siguiente trabajo técnico

1. Identificar versiones de las placas y respaldar/comparar STM32 + QSPI; seleccionar grabación solo después. Validar START momentáneo primero en banco y después integrado por Wi-Fi.
2. Continuar cadena frontal → CAN → DIF/BLQ/ESC → pantalla; calibrar ADC, Hall/PPR y MPU, verificar BMS/configuración real. Límite R estimado no equivale a frenado ni garantía física; ESC experimental.
3. Repetir calibración por pasos con registro Wi-Fi. Seis intentos del 09-10 analizados: ningún centro aceptado; causa eléctrica/posición/recorte sin separar. [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md).
4. Recoger feedback académico sobre aportación, autonomía, protocolo experimental y calendario; [tareas](../06_TRABAJO/TAREAS.md).

USB ESP32 solo fuera del kart para grabación, recuperación y banco. Ensayos integrados por Wi-Fi; registrar configuración, datos originales, fallos, resultados y límites en el [diario](../06_TRABAJO/DIARIO_PARA_RELATORIO.md).

## Documentación y conservación

44 entradas catalogadas: 42 documentos históricos, propuesta personal y cuatro fotos. Extracción inicial de 1.007 páginas PDF; [lectura por lotes](../06_TRABAJO/PLAN_DE_LECTURA.md) sin cerrar. Formatos visuales y Loop pendientes. Originales y binarios conservados localmente; copia de seguridad independiente de todo el archivo pendiente.

[Historial íntegro de estados previos](HISTORIAL_ESTADO_2026-10-13.md) · [Cronología](CRONOLOGIA.md) · [Mapa](MAPA_DEL_PROYECTO.md). Las notas antiguas de “sin push” describen su fecha; la publicación actual se registra en PUB-0001.
