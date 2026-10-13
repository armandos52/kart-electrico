# DEC-0005 · Modelo visual basado en fotografías

Fecha: 08-10-2026. Estado: implementado localmente, dimensiones pendientes de contraste.

Ampliación posterior, mismo día: Armando aprueba visualmente el 3D y solicita adaptar el esquema a una nueva foto superior. [ENS-0007](../../06_TRABAJO/ensayos/ENS-0007_esquema_superior_fotografia.md) incorpora el dibujo SVG con cubierta en T, asiento, ruedas y cajas observables; componentes ocultos en tarjetas sin ubicación asignada. DOC-0044 pasa a cuatro fotos; modelo 3D conservado. Proporciones aproximadas y validación física pendientes.

Armando solicita mejorar la app de monitorización utilizando las fotos de WhatsApp de Descargas para representar un prototipo acorde al kart real. Esta petición autoriza la implementación visual antes aplazada en el [análisis del 07-10](../../06_TRABAJO/ANALISIS_VISTA_3D_DIFERENCIAL_2026-10-07.md).

Se incorpora una reconstrucción 3D programática del bastidor, asiento, ruedas y elementos visibles de [DOC-0044](../../03_CATALOGO/fichas/DOC-0044.md). Se conservan el esquema 2D, los canales, gráficas y calibración del PC. Cinco cámaras, órbita/zoom, selección y galería local. Biblioteca Three.js fijada y servida localmente para usar el panel sin Internet.

Las proporciones se estiman visualmente, en coordenadas sin unidad física. No se calculan cotas ni geometría Ackermann a partir de fotografías. Ambas ruedas delanteras comparten un giro ilustrativo; la rodadura depende de RPM vigentes, se ralentiza y su sentido depende de la marcha ordenada. Los elementos internos no identificables se señalan como orientativos; CAN/IMU no reciben una ubicación física inventada. La app continúa siendo de lectura, sin nuevas órdenes a motores ni cambios de firmware.

Motivo: mejorar reconocimiento del montaje e inspección de sus señales, manteniendo explícita la diferencia entre apariencia, orden software y medida. Fotos originales y hashes conservados; sin publicación. [ENS-0006](../../06_TRABAJO/ensayos/ENS-0006_modelo_visual_fotografias.md) registra versión, comprobaciones, fallos de ensayo y límites. Siguiente: revisar fidelidad con Armando, introducir cotas reales y contrastar animación con registro Wi-Fi del kart.
