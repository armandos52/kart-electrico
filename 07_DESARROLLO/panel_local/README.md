# Panel local del kart · diagnóstico y telemetría

Aplicación para el portátil, con modelo 3D y esquema superior basados en fotografías, señales por elemento, animaciones cualitativas, traza temporal y captura USB o TCP por Wi-Fi directo. Escucha únicamente en `127.0.0.1`. No envía bytes al micro ni modifica o graba firmware. Versión visual `0.4.0-calibracion-pasos`: conserva calibración **de la visualización en PC**, 3D y esquema fotográfico aproximado.

Corrección puntual del 08-10: pulsador en la izquierda del conductor; lados I/D y rodadura coherentes entre vistas; cámara «Arriba» con frontal arriba. Demo positiva a derecha, rueda exterior más rápida y RPM cero con pedal cero. [ENS-0008](../../06_TRABAJO/ensayos/ENS-0008_lados_y_sentido_ruedas_visual.md): 19 comprobaciones dirigidas y 32 de regresión satisfactorias. Son convenciones de visualización, no una calibración física nueva.

Corrección del 09-10: la escala común de ENS-0008 saturaba la animación desde ~45 % de pedal y la corrección posterior (3 % fijo) resultó demasiado lenta. Ahora **cada rueda gira a su velocidad real (RPM·2π/60)**: a 50 km/h son unas 9 vueltas por segundo, así que, para que no parpadee ni parezca girar al revés, los tacos y pernos se difuminan al subir de velocidad y la marca clara deja una estela proporcional al giro (≈ 110° a 50 km/h). Selector **Giro de ruedas: Real 1:1 / Lento 3 %** en la cabecera de la vista (se recuerda en el navegador). En la demo, 100 % de pedal ≈ 550 RPM ≈ 50 km/h (Ø 0,48 m, referencia del firmware sin contrastar) y se muestra el equivalente en km/h. [ENS-0010](../../06_TRABAJO/ensayos/ENS-0010_velocidad_visual_ruedas.md): contrato 48/48, Chrome 53/53, regresión 19/32/30 y 26 tests Python, todo sintético. Modelo visual de desenfoque; no es medida física.

## Esquema superior · cuarta fotografía

**Esquema**, o el enlace `http://127.0.0.1:8766/#esquema`, muestra el bastidor rojo, volante/pantalla, asiento amarillo, cubierta trasera en T, suspensiones, neumáticos y cajas según la nueva foto superior. **Foto superior** abre el original para comparar. Se conservan los doce componentes seleccionables, RPM, giro, rodadura y pausa; en pantallas estrechas el dibujo se amplía y las señales quedan accesibles en el inspector. Batería/BMS, Kelly, CAN e IMU se muestran como señales sin ubicación física asignada.

La foto es oblicua; el dibujo conserva proporciones aproximadas, no medidas. [ENS-0007](../../06_TRABAJO/ensayos/ENS-0007_esquema_superior_fotografia.md): 30 comprobaciones del esquema y 32 de regresión 3D satisfactorias con señales sintéticas. No es una validación del kart físico.

## Modelo del kart · fotografías del 08-10-2026

La vista **3D** reproduce bastidor rojo con arco alto, asiento amarillo, ruedas distintas delante/detrás, suelo, suspensión y plataforma trasera a partir de [tres fotos aportadas por Armando](../../03_CATALOGO/fichas/DOC-0044.md). Reconstrucción visual programática; faltan cotas y confirmar electrónica oculta.

- Arrastra para girar y usa la rueda del ratón para acercar. **Perspectiva, Lateral, Frente, Detrás, Arriba** recuperan cámaras conocidas. **Ampliar** da espacio al modelo.
- Pulsa una rueda, pantalla o caja para inspeccionar señales. **Inspeccionar componente** permite acceder con teclado a todas las piezas. **Esquema** conserva el 2D; **Fotos** abre referencias.
- Giro delantero común y esquemático. Rodadura a velocidad real con las RPM vigentes (con desenfoque de movimiento y estela de la marca); sentido según marcha ordenada, no medido. **Pausar animación** detiene ruedas mientras las señales se actualizan. Respeta movimiento reducido.
- CAN/IMU sin ubicación física asignada; volúmenes internos/asociaciones de placas rotulados como orientativos. Rojo/amarillo representan pintura/asiento, no alarmas.
- Three.js r180 en `static/vendor/` con licencia/procedencia; sin CDN en ejecución. Si WebGL falla, vuelve al esquema. Tres vistas del kart se publican en `static/referencias/`; la frontal con una persona permanece local. La galería muestra un aviso cuando falta una referencia. Los originales y sus hashes se conservan (DOC-0044).

[ENS-0006](../../06_TRABAJO/ensayos/ENS-0006_modelo_visual_fotografias.md): 21 pruebas Python y 32 comprobaciones web satisfactorias, con señales sintéticas y revisión visual; sin ensayo físico.

## Arrancar

Requiere Python 3.12 o posterior. Desde esta carpeta, crear un entorno e instalar la dependencia:

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
.\iniciar.ps1
```

Abrir `http://127.0.0.1:8766/`. El lanzador busca el entorno `.venv` y después Python en PATH; admite `-Python C:\ruta\python.exe` y `-Puerto 8766`. En Linux/macOS o sin PowerShell:

```sh
python -m pip install -r requirements.txt
python server.py --port 8766
```

Comienza **sin conexión**, sin abrir puertos automáticamente. Para explorar sin kart, activar **Demostración**; los datos son sintéticos. La disponibilidad de un servidor depende de que lo arranque cada usuario; las URLs de sesiones anteriores no acreditan una instancia actual.

Para adquirir datos, la fuente `socket://192.168.4.1:3333` aparece por defecto. v1.2 inicia Wi-Fi antes de esperar CAN y puede mostrar su diagnóstico sin inventar medidas de motores. La imagen original respaldada no incluye esta telemetría. COM de Intel AMT queda excluido del selector.

## Conexión con el ESP32 instalado en el kart

Armando indica que el USB no es accesible con el módulo instalado. Las pruebas integradas se registran por Wi-Fi; grabación, respaldo y recuperación por USB se hacen con el módulo fuera del kart.

1. Preparar el primer ensayo estático con tracción inhabilitada y alimentar la electrónica del kart.
2. Conectar el Wi-Fi de Windows a `kart-esp32` con la contraseña configurada en `secretos.h`. El panel funciona sin Internet.
3. Abrir `http://127.0.0.1:8766/` en el portátil. Esta es la dirección del panel; `192.168.4.1:3333` es el destino TCP del ESP32, no una página web.
4. Elegir «Wi-Fi directo ESP32», confirmar la preparación del banco y pulsar «Leer por Wi-Fi». Mantener desactivada la demostración visual.
5. Comprobar versión, estado CAN/control y recepción de mandos; conservar el JSONL. `can_wait` indica que CAN todavía no inicializa. Una red visible por sí sola no valida el control ni los motores.

El 08-10-2026 Armando confirma prueba real satisfactoria con motores tras preparar v1.2, [ENS-0004](../../06_TRABAJO/ensayos/ENS-0004_prueba_real_motores_confirmada.md). No se adjuntó registro del panel de esa prueba; la captura integrada por Wi-Fi sigue pendiente de conservar. Si el panel estaba conectado por USB, desconectarlo antes de retirar el módulo; no conectar ambas fuentes simultáneamente.

## Calibrar dirección · asistente por pasos (11-10-2026)

Abre `http://127.0.0.1:8766/` y pulsa **Calibrar dirección**. Bloques Preparar → Centro → Recorrido → Verificar → Guardar; volante ilustrativo, mensajes visibles y cuenta atrás. El dibujo indica qué hacer: no representa un ángulo medido.

1. Con el kart parado y la tracción inhabilitada, conecta por **Wi-Fi** y desactiva la demo. Confirma ruedas rectas y preparación; pulsa **Empezar**.
2. **Coloca** las ruedas en la posición indicada. Puedes mover el volante libremente en esta fase. Pulsa **Ya está · medir 10 s** para el centro inicial.
3. **Mantén** la posición hasta que termine la cuenta atrás. Se necesitan 10 s entre muestras y 50 muestras para el centro; después 5 s y 25 muestras por posición (el tiempo puede alargarse por la recepción). Sigue izquierda → centro → derecha → centro, durante tres rondas. Cada medición empieza con tu clic; no tienes que adivinar cuándo girar.
4. Si aparece un aviso, lee su causa: límite de señal, variación, posición, recepción o sesión. **Reintentar este paso** conserva las posiciones válidas y vuelve a la fase de colocación. Ruido excesivo termina con un resultado visible; no espera indefinidamente una ventana perfecta. Si cambia el arranque/fuente/firmware, se requiere otro ensayo y confirmar preparación de nuevo.
5. Tras 13 posiciones coherentes, pulsa **Usar esta calibración**. Solo se ajusta la visualización del PC. En Detalles puedes nombrarla, añadir referencias físicas, recuperar perfiles compatibles y descargar todas las lecturas, incluidas las rechazadas.

El centro/topes y repeticiones conservan los criterios anteriores (adquisición: pico a pico ≤200 cuentas y deriva controlada; revisión: ruido ≤10 % del menor semirrecorrido, retorno/repetición ≤3 %, separación mínima 100 cuentas). Son políticas de consistencia del PC, **no tolerancias mecánicas validadas**. Una lectura 0/4095 durante una medición la rechaza; durante colocación muestra aviso sin cancelar el ensayo. No se suaviza la señal ni se ocultan muestras para conseguir un perfil aceptado.

`steer_raw` es dirección **CAN normalizada**, no ADC físico. Un límite 0/4095 puede corresponder al escalado/recorte anterior y no demuestra avería del sensor. Sin referencias físicas se muestra porcentaje por lado; los grados opcionales son interpolaciones del elemento medido, pendientes de contraste intermedio. Pérdida de enlace, reinicio o nueva conexión desactivan el perfil en uso.

Cada trama se captura una vez en el servicio, al margen del refresco del navegador. Se conservan JSONL, CSV y metadatos con hashes del código. Los datos de colocación se identifican como `positioning_sample`; no entran en las estadísticas de medición. Los ZIP de ensayo/perfil incluyen hashes. Archivos en `sesiones/calibraciones/<id>/` y `profiles/`; un cierre del servicio no reanuda borradores automáticamente. No hay copia externa configurada.

[ENS-0012](../../06_TRABAJO/ensayos/ENS-0012_calibracion_por_pasos.md): análisis de seis registros Wi-Fi existentes del 09-10 (v1.2), más pruebas sintéticas del nuevo asistente. Ningún registro histórico guardó el centro; dos alcanzaron 0. No conocemos la posición física en cada muestra. La nueva versión **no se ha probado todavía en el kart**.

## Señales y vista del kart

- Resumen superior: RPM I/D, pedal, dirección, marcha solicitada, PWM I/D y guiñada. Diagnóstico destacado y firmware/motivo de reinicio/sesión.
- Mapa: selecciona piezas para detalles. Bandas de rodadura animadas por RPM, con sentido ilustrativo según marcha ordenada; no es medida de sentido de los Hall. Sin RPM delanteras, las delanteras no ruedan.
- Dirección sin perfil: grados reportados si existen; en ESP32, mando CAN normalizado convertido con la fórmula del fuente y rotulado «estimación del mando · sin calibrar». Con perfil PC: porcentaje o grados interpolados de la referencia indicada. No se presenta como ángulo físico validado de cada rueda. Si el CAN reporta mando caducado (>100 ms), no se representa como dirección vigente.
- Tabla de todas las señales con buscador, origen, estado y antigüedad. Gráfica de cada señal numérica. Señales ausentes o antiguas no se muestran como mediciones actuales.
- Pausar animación detiene rodadura; dirección cambia sin transición. Se respeta movimiento reducido. La demo genera únicamente datos sintéticos y mantiene bloqueada la lectura del micro.
- El panel guarda JSONL de lectura real. Horas visuales Europe/Lisbon; datos originales UTC. Los GPIO/relés/PWM son órdenes software, no prueba de tensión ni contacto físico.

Validación: 13 tests Python, QA de Chrome y captura con placa real en `can_wait`. [ENS-0003](../../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md) conserva datos, capturas y límites. La prueba real con motores fue satisfactoria según Armando en ENS-0004; la verificación instrumentada del diagnóstico con CAN/STM32 en el kart sigue pendiente de datos.

La dependencia pySerial 3.5 se ha instalado localmente en `.vendor/` (fuera de Git). Para otro ordenador, instalarla con el Python elegido:

```powershell
python -m pip install --target .vendor -r requirements.txt
python server.py
```

No necesita servicios de IA ni Internet al funcionar. Interfaz, scripts y estilos son locales.

## Conexión USB de banco, con el módulo fuera del kart

1. Identificar la placa y preparar alimentación/cableado en laboratorio con tracción inhabilitada; conservar condiciones de USB y alimentación para comparar los fallos existentes. Abrir un puerto puede producir un reset por DTR/RTS incluso desactivados según el driver; no se promete captura pasiva eléctricamente inocua.
2. Conectar la placa al portátil, actualizar puertos e identificar el que corresponde al USB. COM3 observado el 07-10-2026 es **Intel Active Management Technology SOL**, no el kart. No seleccionarlo como ESP32/STM32.
3. Confirmar la condición del banco en la interfaz y elegir **Leer por cable**. Configuración ESP32 utilizada en ENS-0003: 115200 bit/s, 8N1, DTR/RTS desactivados.
4. El código ESP32 entregado imprime mensajes de diagnóstico. Se conservan en consola y JSONL; no se convierten en rpm/PWM inventados. Las variables gráficas necesitan tramas de telemetría compatibles con [PROTOCOLO.md](PROTOCOLO.md), disponibles en v1.1, pero ausentes en el respaldo original. STM32 necesita preparar su canal de diagnóstico.
5. Desconectar desde el panel y conservar la sesión. `sesiones/*.jsonl` contiene eventos y tramas válidas, con tiempo de recepción del PC y tick de origen cuando existe. No equivale a sincronización entre relojes ni captura de flancos físicos.

El panel permite revisar qué datos reportan los micros. Formas de onda, tensiones, ruido y fallos eléctricos puntuales se comprueban con el osciloscopio disponible en el laboratorio. La disposición gráfica es esquemática; cableado y posición de componentes pendientes de confirmar físicamente.

## Demostración visual

**Vista animada de ejemplo** genera valores sintéticos únicamente en el navegador para revisar diseño e interacción. Se rotula DEMOSTRACIÓN y bloquea conexión USB. No se guarda como sesión real. Al salir, las señales sin datos reales vuelven a desconocidas. El movimiento se ralentiza para legibilidad y respeta la preferencia de movimiento reducido.

## Comprobación técnica

```powershell
python -m unittest discover -s tests -v
```

Los tests usan datos y puertos falsos; no abren placas. Las pruebas web están en `qa/check-panel.cjs` y requieren el Playwright del entorno de trabajo. No sustituyen las pruebas USB/firmware. Resultados y cobertura en [sesión del proyecto](../../06_TRABAJO/ARRANQUE_PANEL_PORTATIL_2026-10-07.md).

## Próximo desarrollo

Preparar v0 y dependencias del ESP32/STM32 según DEC-0001; añadir telemetría por pasos sin bloquear el control. Verificar variables, unidades, tiempos y origen en el laboratorio. Después elegir un parámetro realmente útil y preparar validación/confirmación/persistencia. Regeneración requiere comprobar Kelly y batería/BMS. No hay fecha ni función de regeneración implementada en este panel.


## Control v1.3 · 09-10-2026

Ampliado para `esp32_trasero-v1.3-dev2`: modo recibido/aplicado, rearme, R20, validez Hall/MPU/ADC, watchdog y errores CAN. Firmware aún sin grabar; usar [ENS-0009](../../06_TRABAJO/ensayos/ENS-0009_robustez_firmware_v13.md) para versión/pruebas/límites. El espacio «Control v1.3» requiere reiniciar el servidor para cargar el lector actualizado y recargar la página. No se reinició una sesión abierta ni se conectó al kart durante esta modificación. Compatibilidad v1.2 conservada, sin alterar las vistas/calibraciones previas.
