---
estado: aprobado e instalado en frontal dev12-log; aceptación integral pendiente
actualizado: 2026-10-23
---

# DEC-0011 · Oscuro fijo y acceso a grabación

**23-10, actualización:** Armando autorizó «inplementa en codigo todo el plan» y luego instalación/comprobación de la placa. Oscuro fijo, botón global y autograbación implementados; dev12-log instalada con escritura microSD observada. [ENS-0029](../../06_TRABAJO/ensayos/ENS-0029_instalacion_historial_sd.md). El texto siguiente conserva la revisión previa a esa autorización. No extender esta aceptación a todos los ensayos/replay todavía pendientes.

Armando solicita conservar el modo oscuro, retirar el selector claro/oscuro y aprovechar su espacio para grabar las pruebas. También pide revisar y aprobar el plan antes de implementar. Esta petición sustituye la preferencia anterior de mantener claro opcional; no implica que el firmware ya haya cambiado.

Se conserva la fase de diagnóstico de [DEC-0010](DEC-0010_pantallas_desarrollo_y_usuario_final.md). El [plan](../../06_TRABAJO/PLAN_HISTORIAL_SD_Y_PANTALLA_2026-10-22.md) propone botón global con estado real de SD, sesiones, importación y replay. Grabación automática al encender, cadencia y transporte son recomendaciones pendientes, no decisiones aprobadas por el usuario.

Evidencia: petición del chat del 22-10 y revisión de `kart_dashboard.c` (`hit`, `draw_sd`, `KartDashboard_Draw`), [hashes](../../06_TRABAJO/evidencias/plan_historial_sd_2026-10-22/fuentes.json). Sin modificación de código ni conexión al hardware en esta revisión.
