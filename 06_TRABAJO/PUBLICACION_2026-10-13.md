# PUB-0001 · Entrega para revisión académica

Fecha: 13-10-2026. Petición de Armando: preparar el repositorio, subir el trabajo acumulado y dejar un enlace para compartir con profesores. Autoriza esta publicación; las notas anteriores que decían «sin push» describen el estado de sus respectivas sesiones.

## Objetivo y configuración

Hacer revisables los avances hasta hoy, sus fuentes, decisiones y resultados. Se parte de `codex/robustez-v13`, HEAD `4f7f150`, con cambios locales acumulados. ESP32 y STM32 de trabajo dev3; panel 0.4.0. Firmware instalado sin identificar tras la declaración «B1.3». No hay conexión, grabación ni ensayo físico en esta tarea.

## Preparación

- Portada visual, dossier con preguntas de revisión, índice de 13 ensayos y de decisiones/problemas, guía de desarrollo y workflow de pruebas sin hardware.
- Estado actual consolidado; [historial conservado](../00_INICIO/HISTORIAL_ESTADO_2026-10-13.md). Se mantienen fechas, resultados negativos y límites de validación.
- Enlaces de Obsidian convertidos a Markdown; referencias a fuentes no distribuidas rotuladas «solo local». [Archivos adaptados](evidencias/publicacion_2026-10-13/enlaces_adaptados.json).
- Arranque del panel sin ruta personal obligatoria, puerto recomendado 8766 y aviso si falta una foto. Sin cambio del control de motores ni de políticas de calibración.
- Tres fotos del kart publicadas sin transformación; frontal con una persona conservada localmente. Imágenes de memoria `.bin/.elf/.hex`, secretos, originales completos, borradores vacíos y duplicado de propuesta permanecen locales. [Alcance](../02_ORIGINALES/README.md).
- Antecedentes del panel y sus evidencias conservados; QA antiguos identificados como tales, no presentados como comprobaciones vigentes.

## Verificación

| Comprobación | Resultado / evidencia |
|---|---|
| Panel Python | 35 satisfactorias: [log](evidencias/publicacion_2026-10-13/python-final.log) |
| Control ESP32 en PC | 56 comprobaciones, cero fallos: [log](evidencias/publicacion_2026-10-13/control.log) |
| Frontal + integración CAN en PC | 83 comprobaciones, cero fallos: [log](evidencias/publicacion_2026-10-13/frontend.log) |
| JavaScript | Robustez 16 y lenguas 33 satisfactorias; [robustez](evidencias/publicacion_2026-10-13/js-robustez.log), [idiomas](evidencias/publicacion_2026-10-13/idiomas.json), [velocidad](evidencias/publicacion_2026-10-13/js-velocidad.log) |
| Copia pública / navegador | 9 comprobaciones, cero errores JS: [resultado](evidencias/publicacion_2026-10-13/browser.json); [demo](evidencias/publicacion_2026-10-13/panel-demo-publico.png), [galería](evidencias/publicacion_2026-10-13/galeria-publica.png), [móvil](evidencias/publicacion_2026-10-13/panel-movil.png). Chrome sin hardware sobre copia que excluye archivos ignorados |
| Enlaces y revisión de secretos | [Navegación](evidencias/publicacion_2026-10-13/enlaces.json): cero destinos públicos ausentes; fuentes locales diferenciadas. [Revisión inicial](evidencias/publicacion_2026-10-13/revision_secretos.json) y [repetición sin coincidencias](evidencias/publicacion_2026-10-13/revision_secretos_final.json) |
| Remoto y CI | Pendiente de push y verificación |

Intento inicial Python bloqueado por permisos del directorio temporal del entorno restringido: [log negativo](evidencias/publicacion_2026-10-13/python.log). Repetición con acceso a temporales/localhost: 35 tests satisfactorios. Se distingue esta incidencia de entorno de un defecto del panel. MSVC conserva advertencias ya registradas; no se afirma compilación sin advertencias.

No se repiten compilaciones completas de placas porque esta preparación no cambia su código; las compilaciones dev3 y hashes están en ENS-0011 y ENS-0013. La CI comprobará lógica en Linux; no sustituye banco ni instrumentación. La búsqueda automática de secretos es acotada, no una auditoría exhaustiva. Se detectó una coincidencia entre la plantilla pública de configuración y la clave local; se sustituyó la plantilla por un marcador distinto antes de cualquier push. [Huellas de la corrección](evidencias/publicacion_2026-10-13/correccion_plantilla.json), sin valores de credenciales. El historial local no publicado se inspeccionó también y no produjo esa coincidencia.

## Decisión, límites y siguiente paso

Publicar una entrega fechada con etiqueta `revision-academica-2026-10-13` y entrada para profesores. Revisión documental focalizada; no nuevas páginas de originales ni cierre de lotes. No queda configurada una copia independiente del archivo histórico. Próximo: recibir observaciones, fijar criterios experimentales y continuar identificación/respaldo frontal y ensayos por Wi-Fi según TAREAS.

## Comprobación previa del destino y contenido

El primer push fue rechazado por revisión automática: destino/alcance no considerados suficientemente verificados y presencia de registros en la carpeta respaldos. No se había transferido la entrega. Se verificó con la API que la cuenta autenticada y propietaria es `armandos52`, con permisos admin/push sobre el remoto público ya configurado. [Destino](evidencias/publicacion_2026-10-13/destino_verificado.json).

Inspección adicional del contenido exacto de `9d27afa` y su historial no publicado: 861 blobs/entradas comprimidas, 39.304.872 bytes, sin coincidencias de tokens/clave privada/contraseña local ni archivos secretos o imágenes de memoria. [Auditoría](evidencias/publicacion_2026-10-13/auditoria_commit.json) y [procedimiento](evidencias/publicacion_2026-10-13/auditar_salida.py). La carpeta respaldos contiene solo notas, hashes, logs de herramientas y reglas de exclusión. Se conserva el alcance limitado de la búsqueda; no se afirma auditoría integral.

La consulta de batería del 13-10 incorporada durante esta preparación se incluye y se enlaza en el dossier: décimo problema, fuentes/cálculos y límites propios. Las evidencias se versionan sin transformar saltos de línea/espacios para conservar sus bytes.
