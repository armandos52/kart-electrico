---
estado: aceptada_enfoque
actualizado: 2026-10-08
resumen: Armando elige panel gráfico conectado a placas reales y ajuste de parámetros en ejecución; implementación técnica pendiente.
---

# DEC-0002 · Panel gráfico de las placas reales y calibración

Fecha: 07-10-2026. Configuración de referencia STM32/ESP32 2025/26, montaje y firmware actuales por confirmar.

## Decisión del usuario

Armando elige «Panel gráfico conectado a las placas reales» y solicita observar las entradas/salidas y realimentación, ajustar valores desde la aplicación y evitar regrabar por cada ajuste. Se continúa por este enfoque; la simulación de control en el PC deja de ser la prioridad de implementación.

Cada micro mantiene su firmware de control. La aplicación observa y calibra parámetros preparados; algoritmos nuevos requieren firmware nuevo. PC e iPad son superficies deseadas, no compatibilidad ya comprobada.

Actualización posterior del 07-10: Armando aplaza iPad y prioriza portátil por cable; Wi-Fi directo posible después. Declara laboratorio completo y osciloscopio, autoriza comenzar diseño y aclara que las placas aún no están conectadas. Creada [primera aplicación de lectura](../../07_DESARROLLO/panel_local/README.md), sin TX ni firmware cambiado; [pruebas/pendientes](../../06_TRABAJO/ARRANQUE_PANEL_PORTATIL_2026-10-07.md).

Aclaración posterior del usuario en esta sesión: «autofrenado» significa regeneración para recargar batería, compatible con el Kelly aún por comprobar. Sus ejemplos son ideas vagas que deben analizarse para seleccionar mejoras necesarias; no aprobarlos como requisitos del primer panel.

Elección visual posterior del 07-10: Armando solicita registrar un **modelo 3D sencillo basado en fotos reales**, con detalle limitado. Prioridad explícita: matemática del diferencial, datos disponibles y observar el movimiento/respuesta del sistema; la fidelidad visual es secundaria. Se acepta este alcance de diseño; sigue vigente la petición anterior de no programar por ahora. [Análisis y límites de las señales](../../06_TRABAJO/ANALISIS_VISTA_3D_DIFERENCIAL_2026-10-07.md).

Rediseño visual elegido el 08-10-2026: Armando considera el panel «demasiado genérico y por bloques» y pide acceso directo a las señales sin tener que buscarlas sección por sección. Elige: (1) **primero una vista superior 2D a escala real y después un 3D sencillo** como vista conmutable sobre los mismos datos; (2) **estilo gris claro de HMI industrial (ISA-101)**, con el color reservado a estados anormales; (3) **fotos y medidas con cinta** como referencia física, sin escaneo 3D por ahora. El plan por fases (V1 distribución en una pantalla, V2 2D a escala, V3 3D Three.js y V4 opcional de análisis externo) y las referencias consultadas están en el [diario para el relatório](../../06_TRABAJO/DIARIO_PARA_RELATORIO.md). No cambia el carácter de solo lectura ni el firmware.

## Alcance aceptado y detalles propuestos

Nueva prioridad posterior del 07-10: Armando solicita un prompt para otra sesión y declara tener el ESP32 disponible. Quiere respaldo real de lo grabado antes de tocar código/flash; después abrir el código en VS Code y preparar monitorización por Wi-Fi directo, porque USB resulta incómodo en el montaje. USB permanece para respaldo/primera grabación/recuperación; iPad y 3D aplazados. [Prompt preparado](../../06_TRABAJO/PROMPT_RESPALDO_ESP32_WIFI_VSCODE_2026-10-07.md). Esta preparación no ejecuta cambios, respaldo ni grabación. La petición previa de no programar se mantiene en esta sesión y para el 3D; la fase posterior de firmware se solicita tras respaldo verificado.

Aceptado: trabajar la idea del panel real y preparar cómo se implementaría. [Propuesta técnica](../../06_TRABAJO/PANEL_REAL_CALIBRACION_2026-10-07.md) y maqueta de interacción, sin conexión al kart.

Propuestos, pendientes de contrastar: USB serie ESP32 como primera conexión, servicio local en PC, diagnóstico CAN ampliado, catálogo de parámetros, confirmación/readback, perfiles y acceso iPad por red local. No se da por aprobada compra, instalación, configuración de red o grabación concreta, ni existencia de autofrenado implementado.

## Evidencias y relación con decisiones

- Mensaje de Armando en esta sesión del 07-10-2026: elección del panel conectado a placas reales y solicitud de ajustes interactivos.
- Mensaje posterior de Armando del 07-10-2026: «quiero un modelo 3D sencillo, basado en las fotos reales»; prioriza «la matemática detrás de eso, los datos» y ver cómo se mueve el sistema sobre el detalle visual.
- [Inventario de código](../../04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md): arquitectura y datos existentes.
- [DEC-0001](DEC-0001_versiones_de_firmware_y_ensayos.md): conservar originales, copia v0, historial y ensayos. Sigue vigente.
- [Propuesta anterior](../../06_TRABAJO/PROPUESTA_SIMULADOR_2026-10-07.md): conserva las alternativas como estudio previo. Su recomendación inicial de simulación no refleja la elección posterior del usuario.

## Próximo resultado

Confirmar firmware/cableado, preparar v0 y una captura de referencia. Primera implementación propuesta de lectura antes de habilitar calibración. No se ha cambiado firmware/hardware en esta sesión.
