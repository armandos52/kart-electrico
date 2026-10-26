# Evidencia · propuestas del frontal · 15-10-2026

## Objetivo, configuración y alcance

Describir el programa real y comparar ideas de interfaz de desarrollo solicitadas por Armando. [Propuesta y fuentes](../../PROPUESTA_PANTALLA_FRONTAL_2026-10-15.md). Solo PC Windows y Chrome; ninguna conexión con el kart. HEAD `bebe4528747bc6e05dc1ba244cd3ce9a090a7942`, con trabajo local anterior conservado. F746 documentada dev4; fuente local dev5 sin grabar; ESP32 físico sin identificar. No se modifica firmware ni se producen binarios nuevos.

Todos los datos de [maqueta.html](maqueta.html) son **sintéticos**. Edades, ADC, velocidad, RPM, filas y errores son ejemplos de presentación. No son muestras del vehículo. La maqueta no transmite órdenes ni escribe una SD. El escenario «Modo confirmado» no acredita prueba de tracción.

## Procedimiento y comprobaciones

1. Consulta focalizada de notas/código y del framebuffer final de ENS-0019. Las referencias de líneas y el alcance constan en la propuesta; las fuentes consultadas se identifican en [manifest_fuentes.json](manifest_fuentes.json).
2. Preparación de dos diseños con navegación Resumen/Ruedas, Entradas, CAN, SD y BMS. Banda superior reservada de 48 px; interfaz útil nominal de 480 × 224 px, sin contar el bisel.
3. Render con `visualize/scripts/render.py` y revisión visual en Chrome. El wrapper carga la maqueta en un iframe aislado; un pequeño script de verificación se añade solo a la vista previa para pulsar controles, medir geometría y devolver resultados por `postMessage`. No se añade ese verificador al fragmento entregado. El código ejecutado se conserva en [procedimiento_qa.js](procedimiento_qa.js).
4. Ventana de 736 px (iframe de 704 px): recorrido de las cinco páginas de cada diseño, selección de rueda, solicitud local de registro y pausa, conservación del error durante apertura, y escenarios de pedal/CAN caducado/modo confirmado. [qa_736.json](qa_736.json): 10 páginas seleccionadas, sin desbordamientos horizontales detectados; área útil de 224 px y banda de 48 px. Capturas [resumen](estado_736.png) y [ruedas](mapa_736.png).
5. Ventana de 320 px (iframe de 288 px): [primera comprobación](qa_320.json) y [captura previa](estado_320_preajuste.png). Se amplía la altura del aviso en vista estrecha para alojar mensajes multilínea. La altura de pantalla de la maqueta se adapta aquí para lectura, no pretende representar la resolución nativa.
6. Repetición tras ese ajuste: [qa_320_final.json](qa_320_final.json), 10 páginas recorridas, cinco acciones de registro/pausa/rueda correctas, cero desbordamientos horizontales detectados, tres avisos largos dentro de su contenedor, cero errores capturados y consola sin errores/advertencias. [Captura final](estado_320_final.png). La prueba inicial ya cubre el escenario «Sin estado ESP»; se añaden los otros tres mediante restauración del estado local.

Los JSON conservan el retorno original del conector, cuyo bloque de texto contiene el informe JSON. Estas son comprobaciones dirigidas del navegador y revisión de imágenes, no regresión de firmware ni prueba del panel táctil. No se verificó la interfaz del selector Tweak dentro del cliente de conversación; su ayudante es opcional y está protegido por comprobación de existencia. Sí se ejercitó el cambio de escenarios por el evento de restauración local.

## Intentos negativos y ajustes

- Primera regeneración del wrapper rechazada por archivo de destino existente. Se repitió con `--force` sobre el wrapper propio; no afecta originales.
- El conector de Chrome rechazó guardar directamente JSON/PNG en la ruta del proyecto por su configuración de raíces. Se recuperó el contenido devuelto y se guardó con herramientas locales autorizadas. No hubo solicitud de elevación ni rechazo de revisión automática de permisos.
- Se ajustó la altura del aviso a `auto` con mínimo de 34 px en anchuras estrechas; después se comprobaron los tres textos alternativos. El diseño nativo de 480 px conserva sus 224 px útiles.
- El render avisa de referencias a `window.openai`: la maqueta solo usa `widgetState` y `setWidgetState` para recordar selecciones; no contiene llamadas de red. No se publica ni se exporta como sitio.

## Resultado, decisiones y límites

Resultado positivo: dos propuestas navegables, con separación de solicitado/recibido/aplicado, datos desconocidos explícitos y registro SD nombrado. Recomendación pendiente de adopción: usar «Estado y causa» como inicio y «Kart y señales» como página adicional. No cambia DEC-0010.

Pendientes: implementación TouchGFX, conexión al snapshot, validez/edad Hall frontal, fuente de datos BMS/energía, historia de eventos, perfil de calibración, carga de CPU/RAM, tiempos de dibujo y validación en la F746. Tipografía y respuesta táctil aún no aceptadas físicamente. La maqueta no demuestra autonomía, sensores conectados, límite físico de marcha atrás, relé cerrado ni funcionamiento de motores.

Próximo paso: primera iteración Resumen/Entradas/CAN/SD con pruebas de comportamiento y posterior prueba de placa; medir pedal y completar identificación ESP/SD según las incidencias abiertas. Ensayos integrados por Wi-Fi. Fuentes y capturas locales conservadas con [SHA-256](SHA256SUMS.txt); no constituye copia independiente.
