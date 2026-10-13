# ENS-0007 · Esquema superior basado en una nueva fotografía

Fecha: 08-10-2026. Tipo: edición de interfaz, pruebas automáticas con datos sintéticos y revisión visual; sin ensayo físico.

## Objetivo, motivo y configuración

Armando expresa satisfacción con el 3D y pide implementar un esquema más acorde al kart real, usando una nueva foto superior ya en Descargas. Se adapta el 2D manteniendo el modelo 3D aceptado y el flujo existente de lectura.

Versión visual `0.3.1-esquema-superior`. Base Git `968b828ea60f02d3fbfe14b155640aacc8331993`, con cambios locales previos conservados; sin nuevo commit/push. Navegador Chrome/Playwright en Windows, panel local `127.0.0.1:8766`, comprobado desconectado antes de reiniciar para ampliar sus rutas estáticas. No se abren puertos del kart ni se modifica firmware/calibración.

La configuración representada es la de [DOC-0044](../../03_CATALOGO/fichas/DOC-0044.md). Nueva fuente: `WhatsApp Image 2026-10-08 at 15.42.32.jpeg`, 201371 bytes; foto superior **oblicua**, no ortofoto. Original copiado sin transformar; Descargas intacto. [SHA-256 y copia verificada](../evidencias/esquema_superior_2026-10-08/foto_sha256.json). Copia idéntica local en `static/referencias/superior.jpg`, excluida de Git junto a las anteriores.

## Método y cambios

1. Consulta focalizada de estado/mapa, índice/ficha DOC-0044, notas previas y código del panel. Revisión completa de la nueva imagen; sin abrir originales históricos.
2. [Copia previa de fuentes afectadas](../evidencias/esquema_superior_2026-10-08/antes/). Redibujo SVG programático según rasgos visibles: bastidor rojo, volante/pantalla frontal, suelo, asiento amarillo central, cubierta posterior en T, muelles, ruedas delanteras de tacos y traseras más estrechas, caja lateral izquierda y caja transparente al final.
3. Pantalla/cajas asociadas al inspector con ubicación de su contenido por confirmar. Batería/BMS, Kelly, CAN e IMU aparecen como tarjetas de señales **sin asignarles una posición física demostrada**. Se retira del centro la batería genérica del esquema antiguo, que no estaba apoyada por la foto.
4. Se mantienen los doce componentes seleccionables con ratón/teclado, RPM, rodadura, pausa, giro delantero y retirada de datos caducados. Pivotes SVG ajustados a los centros de rueda nuevos (coordenadas gráficas, no cotas físicas).
5. Modo compacto por ancho disponible, conservando acceso a señales en el inspector. Botón «Foto superior», galería de cuatro originales y enlace `/#esquema`. El 3D sigue siendo la vista por defecto sin fragmento.

## Validación y datos

- [30 comprobaciones del esquema satisfactorias](../evidencias/esquema_superior_2026-10-08/qa_esquema.json): enlace, doce selecciones por teclado, sincronización con inspector, foto, pivotes, RPM/pausa, salida de demo, timeout CAN, continuidad del 3D y anchos 1440/1024/760/375/320 px. Sin errores JavaScript/SVG ni desborde horizontal. [Script reproducible](../../07_DESARROLLO/panel_local/qa/check-esquema.cjs).
- [32 comprobaciones de regresión del 3D satisfactorias](../evidencias/esquema_superior_2026-10-08/regresion_3d/qa_modelo.json), incluido movimiento reducido, datos caducados, cuatro fotos, rutas cerradas y caída de recurso 3D. Cero solicitudes externas en la prueba principal. El test existente solo adapta la cuenta de fotografías de tres a cuatro.
- Revisión visual: [esquema superior](../evidencias/esquema_superior_2026-10-08/esquema-superior.png), [pantalla completa desconectada](../evidencias/esquema_superior_2026-10-08/esquema-desconectado.png), [móvil](../evidencias/esquema_superior_2026-10-08/esquema-movil.png) y [demo sintética](../evidencias/esquema_superior_2026-10-08/esquema-demo.png). Formas, rótulos y selección legibles tras la revisión.
- [Hashes de fuentes, copia de foto y evidencia](../evidencias/esquema_superior_2026-10-08/archivos_sha256.json). `git diff --check` sin errores; solo avisos de conversión futura LF/CRLF. No se reejecutó la batería Python: el cambio de servidor se limita a permitir un módulo y una imagen estáticos; se comprobaron mediante el navegador.
- No se observaron fallos durante las dos ejecuciones de QA. Antes de ejecutar se corrigió la composición de signos en una ruta SVG de suspensión simétrica. No hay resultados negativos de hardware porque no se conectó.

## Decisión, límites y siguiente paso

Ampliación de [DEC-0005](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0005_modelo_visual_basado_en_fotos.md): el esquema reproduce la disposición visible; los componentes ocultos no se presentan como localizados. Proporciones estimadas y leve perspectiva reinterpretada, sin cotas ni reconstrucción fotogramétrica. La dirección/rodadura siguen siendo ilustrativas, sin Ackermann medido ni dirección de giro Hall. Las etiquetas STM32/ESP32 y relés son asociaciones documentales, no identificación de contenido por la foto.

Siguiente: revisión de Armando, cotas reales y ubicación física de la electrónica; contraste de señales en el kart mediante Wi-Fi. No se publica, no se configura copia externa ni se requiere USB con el módulo instalado.
