---
estado: abierto; implementación autorizada, frontal instalado, aceptación integral pendiente
actualizado: 2026-10-23
---

# PROB-0018 · Historial SD y reproducción incompletos

**23-10:** autorizadas implementación e instalación. Dev12-log en F746, autograbación observada con 920 filas iniciales y cero pérdidas/errores SD. Corrección real de TX_UNDERRUN al pasar a DMA; [ENS-0029](../../06_TRABAJO/ensayos/ENS-0029_instalacion_historial_sd.md). Importador/biblioteca/replay y canal diagnóstico trasero implementados en código local; cobertura nueva de replay/extremo a extremo y despliegue trasero pendientes, sin cerrar el problema. El antecedente siguiente describe el estado previo.

Armando quiere conservar todas las pruebas y reproducir sus parámetros en el panel existente. Tras la ambigüedad inicial «micro USB»/«micro SD», confirma el 22-10 **microSD en la STM32 de la pantalla y panel del portátil con vista 3D**. Declaración del usuario, sin nueva inspección física; capacidad y formato pendientes. La aclaración no equivale a aprobar el plan de implementación.

Revisión estática del código local dev10-ui: `Kart_SdTick` anexa a `kart_v13.csv` (esquema 2), nominalmente cada 200 ms; no ofrece identidad inequívoca de cada prueba/arranque. `get_fattime()` devuelve 0. El CSV no contiene todos los datos internos ESP32 visibles por Wi-Fi. Panel con captura JSONL y herramienta QA de repetición, pero sin biblioteca/importación SD/reproductor temporal de usuario. [Fuentes y hashes](../../06_TRABAJO/evidencias/plan_historial_sd_2026-10-22/fuentes.json).

No se trata de una pérdida de datos recién observada: es una diferencia funcional respecto al objetivo. La latencia/detección y el ensayo físico SD siguen en [PROB-0013](PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md). Última placa identificada dev9-ui; dev10 local sin instalar, ESP32 físico v1.2. [ENS-0026](../../06_TRABAJO/ensayos/ENS-0026_diagnostico_bloqueo_marchas.md), [ENS-0027](../../06_TRABAJO/ensayos/ENS-0027_pedal_media_ponderada.md).

Cierre propuesto: sesiones sin sobrescritura, inventario de señales y ausencias, captura autónoma, importación íntegra con originales/hashes, reproducción fiel al tiempo y ensayo de extremo a extremo documentado. [Plan priorizado](../../06_TRABAJO/PLAN_HISTORIAL_SD_Y_PANTALLA_2026-10-22.md). Sin implementación ni ensayo nuevo; no se promete capturar transitorios no muestreados ni recuperar datos nunca guardados.
