# ENS-0006 · Modelo visual del kart a partir de fotografías

Fecha: 08-10-2026. Tipo: desarrollo y ensayo automático de software; revisión visual por el agente. Sin ensayo de banco, conducción o conexión al kart.

## Objetivo y configuración

Mejorar la correspondencia entre el modelo de la app y el kart mostrado por Armando en tres fotografías. Su solicitud de mejorar la app usando las fotos de WhatsApp autoriza la implementación antes aplazada: [DEC-0005](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0005_modelo_visual_basado_en_fotos.md).

- Versión visual `0.3.0-modelo-fotos`, cambios locales sin commit/push. Base Git observada: `968b828ea60f02d3fbfe14b155640aacc8331993`, con trabajo previo sin confirmar conservado.
- Windows, Chrome/Playwright locales y Python del entorno de trabajo. Servidor `127.0.0.1:8766`, iniciado desconectado; no había servicio respondiendo al comprobarlo. Ningún puerto USB ni TCP del ESP32 abierto.
- Three.js `r180 / 0.180.0`, módulos locales MIT. [Procedencia/adaptación](../../07_DESARROLLO/panel_local/static/vendor/README.md), [documentación oficial](https://threejs.org/docs/).
- Configuración visual: el kart fotografiado en [DOC-0044](../../03_CATALOGO/fichas/DOC-0044.md), presentado como referencia real por Armando. Fotos examinadas completas; cotas, fecha física de captura y componentes internos pendientes de verificar.
- Firmware: ninguno modificado ni grabado. ENS-0004 conserva la referencia anterior; no se verifica de nuevo.

## Procedimiento y cambios

1. Consulta focalizada de estado, índice, ficha DOC-0042 y notas de panel/3D/calibración. Sin nuevas páginas de originales históricos ni lectura integral adicional.
2. Copia de tres imágenes sin modificar Descargas. SHA-256 fuente/copia satisfactorio: [manifiesto](../evidencias/modelo_visual_2026-10-08/fotos_sha256.json). Galería con copias idénticas; originales aparte.
3. Conservación de [fuentes anteriores a esta intervención](../evidencias/modelo_visual_2026-10-08/antes/). Geometría programática: bastidor rojo/arco alto, asiento amarillo, volante/pantalla, suelo, suspensión, ruedas de tacos delante y más estrechas detrás, plataforma y cajas visibles. Sin fotogrametría ni CAD.
4. Órbita/zoom, cinco cámaras, selección/inspector, fotos y ampliación. Esquema 2D, canales, gráficas y asistente conservados. Carga aislada de módulos 3D: fallar WebGL mantiene esquema y monitorización.
5. Enlace al estado existente del panel sin algoritmo de control adicional. Giro delantero común esquemático; rodadura por RPM vigentes, pausa y movimiento reducido. Dirección ausente se rotula como pose neutra de referencia.
6. Batería Python y [ensayo web](../../07_DESARROLLO/panel_local/qa/check-modelo.cjs), con señal sintética interceptada solo en el navegador. Revisión visual de perspectiva, lateral y pantalla estrecha.

## Datos y resultados

- **21 pruebas Python satisfactorias**: [salida original](../evidencias/modelo_visual_2026-10-08/unittest.log). Lector/calibración con entradas y sockets locales falsos, sin hardware.
- **32 comprobaciones web satisfactorias**: [resultado](../evidencias/modelo_visual_2026-10-08/qa_modelo.json). Clic sobre rueda renderizada, inspector/2D, cámaras/fotos, bloqueo de conexión durante demo, animación/pausa comprobadas en píxeles de geometría, movimiento reducido, retirada de RPM antiguas y dirección por timeout CAN, bloqueo de calibración sin datos, ampliación, rutas cerradas, estado desconectado y recuperación al esquema tras fallar un recurso 3D.
- Anchos 1920, 1440, 1280, 1024, 760, 375 y 320 px sin desborde horizontal. Cero errores JavaScript y solicitudes externas en la prueba principal.
- Capturas: [panel final](../evidencias/modelo_visual_2026-10-08/panel-3d-final.png), [lateral](../evidencias/modelo_visual_2026-10-08/modelo-lateral-final.png), [demo](../evidencias/modelo_visual_2026-10-08/panel-3d-demo.png), [pantalla estrecha](../evidencias/modelo_visual_2026-10-08/panel-3d-movil.png). Los ejes/historial tras el escenario simulado no son datos reales; el servidor normal permanece desconectado.
- [Hashes entregados](../evidencias/modelo_visual_2026-10-08/archivos_sha256.json): fuentes, biblioteca, fotos servidas y capturas. Fotos excluidas de Git y sin publicar.

## Resultados negativos y correcciones

- El entorno restringido impidió DNS/sockets y temporales usados por las pruebas (`WinError 10013`/`WinError 5`). Descarga oficial y pruebas repetidas con ejecución autorizada; resultados finales conservados. No fue un fallo del kart.
- Primer render: inspector alargaba excesivamente la escena. Se acotó altura manteniendo desplazamiento del inspector; corregidos dibujo de neumáticos traseros, espacio inferior móvil y petición de favicon inexistente. [Captura de revisión](../evidencias/modelo_visual_2026-10-08/modelo-primer-render.png), distinta del resultado final.
- El comparador inicial de pausa/movimiento reducido incluía rótulos de RPM que deben seguir refrescando. Se corrigió para comparar solo geometría; ambas condiciones pasan.
- El chequeo inicial de calibración usaba `cal-begin`, selector obsoleto de una prueba anterior. Se adaptó a `cal-primary` del asistente vigente, sin modificarlo. [Último fallo previo](../evidencias/modelo_visual_2026-10-08/qa_modelo_fallo.json).

## Límites y siguiente paso

Reconstrucción visual aproximada en coordenadas sin unidad física; no se adoptan parámetros históricos como cotas medidas. Ambas ruedas delanteras usan el mismo giro ilustrativo: no hay Ackermann validado ni cinemática resuelta de suspensión. Rodadura ralentizada/limitada; sentido según marcha ordenada, sin medición Hall de sentido.

Electrónica y volúmenes bajo cubierta orientativos con aviso. CAN/IMU sin posición física asignada. Rojo/amarillo reproducen materiales, no alarmas. Fotos no verifican celdas, cableado, señales, firmware ni aptitud mecánica.

La app sigue siendo de lectura, sin nueva actuación ni calibración física validada. Próximo: contraste visual con Armando; cotas de entre ejes, vías, ruedas/alturas; ubicación comprobada de placas/BMS/Kelly; captura integrada por Wi-Fi. USB del módulo instalado no requerido. Sin copia externa configurada.
