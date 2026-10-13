# ENS-0008 · Pulsador izquierdo y sentido de ruedas en la visualización

Fecha: 08-10-2026. Tipo: diagnóstico y corrección de interfaz; pruebas sintéticas y revisión visual. Sin hardware.

## Solicitud y configuración

Armando pide corregir únicamente el pulsador rojo al lado izquierdo en 3D y la dirección aparente de las ruedas en la demo. Referencia de montaje: fotos de [DOC-0044](../../03_CATALOGO/fichas/DOC-0044.md), en particular la superior ya revisada. Izquierda/derecha se definen desde el conductor mirando al frontal.

Versión visual `0.3.2-sentido-ruedas`. Base Git `968b828ea60f02d3fbfe14b155640aacc8331993`, con cambios locales previos conservados; sin commit/push. Chrome/Playwright en Windows; aplicación normal en `127.0.0.1:8766`, desconectada. Cambios en módulos JS servidos al recargar, sin reiniciar servidor, abrir puertos del kart, alterar firmware ni calibración. [Fuentes anteriores](../evidencias/sentido_ruedas_2026-10-08/antes/).

## Diagnóstico y cambios

- El modelo usa Y vertical y frente −Z. La izquierda física del conductor corresponde a X negativo, pero pulsador y etiquetas de ruedas/Kelly izquierdos estaban en X positivo. Se desplaza la caja/pulsador al lado negativo y se corrigen asociaciones I/D. La localización precisa de Kelly sigue siendo orientativa.
- Rodadura invertida: en avance, la parte superior del neumático se desplazaba hacia atrás. Se corrige el signo de rotación X en 3D y la traslación de la banda en SVG. Un cálculo común conserva sentido entre vistas y lo invierte con marcha R.
- La velocidad visual anterior podía favorecer inversión aparente por repetición de tacos. Se reduce la máxima a **0,9 rad/s de animación** y el paso gráfico a 0,05 s; escala común entre ruedas para conservar diferencias de RPM. Son parámetros de representación, no velocidades físicas del kart. La causa perceptiva no se mide como ensayo óptico.
- Demo: el reparto RPM por etiquetas I/D estaba invertido para la convención visual positiva a derecha. Ahora la rueda exterior gira más rápido, recto ambas son iguales y pedal cero produce RPM cero incluso con dirección girada. Este reparto es sintético, no reproduce ni valida el diferencial del firmware. El texto del mando indica izquierda/derecha/recto.
- La revisión de la cámara «Arriba» mostró frontal abajo y ejes de pantalla invertidos respecto al esquema. Se orienta frontal arriba e izquierda del conductor a la izquierda de pantalla. Las otras cámaras conservan su posición.
- El ángulo de ambas ruedas delanteras ya concordaba entre 2D/3D en la prueba inicial; no se invierte la señal de dirección ni se modifican calibraciones para solucionar la rodadura. Continúa siendo un ángulo común ilustrativo, sin Ackermann medido.

## Procedimiento y evidencia

1. Consulta focalizada de estado/mapa, catálogo, ficha fotográfica, notas y cinco módulos visuales; sin nuevos originales históricos.
2. [Script dirigido](../../07_DESARROLLO/panel_local/qa/check-sentido.cjs): solo en QA se inyectan referencias de lectura a geometría/cámara ya existentes para observar posiciones, vectores y rotaciones reales. La app normal no contiene estas referencias. Datos de telemetría simulados en el navegador; servidor desconectado.
3. [Antes](../evidencias/sentido_ruedas_2026-10-08/antes.json): 9 comprobaciones fallidas de 18, reproduciendo lados, rodadura, reparto y RPM con pedal cero; giros de dirección, pausa y retirada de datos antiguos ya pasaban. [Cámara anterior](../evidencias/sentido_ruedas_2026-10-08/antes-pulsador.png).
4. [Después](../evidencias/sentido_ruedas_2026-10-08/despues.json): **19 comprobaciones satisfactorias**, incluida la comprobación añadida de orientación de cámara. Avance/retroceso, izquierda/recto/derecha, sincronía SVG, pausa, movimiento reducido y caducidad correctos en el escenario sintético. Sin errores JS ni conexión al kart.
5. [Regresión 3D](../evidencias/sentido_ruedas_2026-10-08/regresion_3d/qa_modelo.json): **32 comprobaciones satisfactorias**, selección, fotos, datos caducados, anchos y recuperación del esquema. Se actualiza la expectativa del clic en la rueda visible: corresponde a la derecha del conductor, antes etiquetada como izquierda.
6. Capturas revisadas: [pulsador/cámara corregidos](../evidencias/sentido_ruedas_2026-10-08/despues-pulsador.png), [izquierda](../evidencias/sentido_ruedas_2026-10-08/despues-giro--15.png), [recto](../evidencias/sentido_ruedas_2026-10-08/despues-giro-0.png), [derecha](../evidencias/sentido_ruedas_2026-10-08/despues-giro-15.png). [Hashes](../evidencias/sentido_ruedas_2026-10-08/archivos_sha256.json) de código y evidencia.

## Decisión y límites

Se adopta una sola convención visual de lados, marcha y cámara superior, conservando alcance/modelo previo. Las fallas registradas pertenecen a la visualización, no demuestran un fallo del hardware. No se cambia señal original ni control físico. Polaridad real del sensor, proporciones, cinemática y correspondencia entre mando y ángulos físicos siguen pendientes de contraste por Wi-Fi y medidas en el kart. No hay datos nuevos de banco o conducción, nuevas fuentes fotográficas ni copia externa configurada.

Siguiente: revisión visual de Armando; contrastar estas convenciones con un ensayo físico registrado cuando se disponga de datos.
