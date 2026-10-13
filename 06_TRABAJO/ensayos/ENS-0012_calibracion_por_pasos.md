# ENS-0012 · Calibración por pasos y diagnóstico visible

Fecha: 11-10-2026. Tipo: análisis de registros anteriores + pruebas automáticas de PC. **Sin nueva prueba en banco ni en el kart.**

## Objetivo y motivo

Armando declara que probó la calibración en tiempo real, le resultó confusa y no pudo terminar. Aclara que al girar a izquierda o derecha aparecía un error sin explicación. Solicita bloques, pasos explícitos, formas más suaves y animaciones fluidas. [PROB-0007](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md), [DEC-0007](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0007_calibracion_con_medicion_explicita.md).

## Configuración y evidencia anterior

Se localizaron **seis ensayos existentes del 09-10-2026**, fuente `socket://192.168.4.1:3333`, firmware reportado `esp32_trasero-v1.2-diagnostico`. No se abrió conexión física hoy. Montaje/sensor y posición física por muestra no identificados; no puede suponerse que todo el registro sea con ruedas quietas. `steer_raw` es CAN normalizado, no ADC físico.

[Análisis con rutas y SHA-256](../evidencias/calibracion_pasos_2026-10-11/analisis-registros-reales.json): 638 muestras, sin saltos de secuencia entre muestras contiguas de cada intento. Ninguno guardó el centro. Dos finalizaron por 0/4095 y los otros cuatro por cancelación del usuario. Los archivos originales siguen en [sesiones/calibraciones · solo local](../../07_DESARROLLO/panel_local/sesiones/calibraciones/), sin alterar.

| ID (prefijo) | Resultado anterior | Ventana final: mín.–máx. (cuentas CAN) | Pico a pico (cuentas CAN) |
|---|---|---|---|
| c5833080 | Rechazo por límite; una muestra a 0 | 0–2347 | 2347 |
| d67c7b5c | Cancelación | 7–2500 | 2493 |
| 475b4f11 | Cancelación | 2196–2582 | 386 |
| 80f25a7d | Cancelación | 2208–2546 | 338 |
| 86c72945 | Rechazo por límite; una muestra a 0 | 0–2511 | 2511 |
| 4ac2f4f2 | Cancelación | 2134–3828 | 1694 |

Son estadísticas de las ventanas finales guardadas por la versión anterior, no mediciones nuevas ni ruido estacionario acreditado. Las dos muestras a 0 permiten explicar los rechazos registrados; no prueban la causa eléctrica. Escalado/recorte frontal, sensor, cableado y movimiento son posibilidades por contrastar. La copia frontal de desarrollo normaliza y limita el ADC antes de transmitir (`Core/Src/kart_frontend.c`, función `read_adc`); no identifica el binario instalado.

## Diagnóstico de software y cambios

- La guía anterior capturaba el centro al empezar, mezclaba colocación y medición, esperaba una ventana estable de duración indefinida y convertía un 0/4095 en fallo de todo el ensayo. No mostraba siempre el detalle de la causa en la vista principal.
- Sus clases de presentación (`cal-header`, `cal-picture`, `cal-wheel`, etc.) carecían de estilos propios en el CSS actual, que conservaba principalmente estilos de un diálogo anterior.
- Panel **0.4.0-calibracion-pasos**: Preparar → Centro → Recorrido → Verificar → Guardar, con 13 posiciones distribuidas en centro inicial y tres rondas. Cada paso tiene una fase de colocación libre y un botón **Ya está · medir**. Se dibuja un volante orientativo, independiente de la lectura.
- Cuenta atrás desde muestras reales: centro ≥10 s/50 muestras, demás ≥5 s/25. Ventana explícita sin recorte deslizante. Si no es estable al terminar, se rechaza ese intento con causa; si faltan muestras, se limita la espera a duración nominal +5 s (o se pausa antes por pérdida de datos).
- Se puede repetir el paso conservando las capturas aceptadas; reconexión a la misma sesión permite reintentar, cambio de sesión/fuente/firmware exige nuevo ensayo y preparación. Motivo, lectura, variación y progreso siempre visibles.
- Se mantienen validaciones de integridad y políticas anteriores: pico a pico ≤200 cuentas, deriva ≤máx.(4; 0,30·pico a pico), revisión final de ruido ≤10 % del menor semirrecorrido, retornos/extremos ≤3 %, separación mínima 100 cuentas. No se han validado como tolerancias físicas. 0/4095 durante medición sigue siendo rechazo; durante colocación avisa sin cancelar ni aceptar una medida.
- Sin filtrado que oculte datos, sin cambios de firmware. JSONL conserva intentos rechazados y nuevas `positioning_sample`; estas últimas no entran en las estadísticas. Los perfiles ajustan solo el PC. Paleta más suave, tarjetas/botones redondeados, transiciones y movimiento reducido; ES/PT/EN conservados.

Durante la verificación se observaron rechazos intermitentes de conexiones HTTP al cargar `vista_kart.js` y `calibracion.js`, que dejaban la interfaz sin inicializar. Se amplió la cola de conexiones pendientes del servidor local de 5 a 32 (`PanelHTTPServer`), manteniendo bucle local y las comprobaciones de acceso. La QA final del recorrido pasó 23/23 tras reiniciar el servicio; se comprobaron además [tres aperturas independientes](../evidencias/calibracion_pasos_2026-10-11/aperturas.json). La cola limitada es una hipótesis compatible con el fallo, no una causa universal demostrada del entorno Windows.

Base Git `4f7f150`, árbol con cambios previos de otras sesiones. Cambios locales sin commit/push. [Copia anterior](../evidencias/calibracion_pasos_2026-10-11/antes/), [diff de esta intervención](../evidencias/calibracion_pasos_2026-10-11/cambios-panel.diff), [hashes](../evidencias/calibracion_pasos_2026-10-11/hashes.json). No hay binario de firmware nuevo en este ensayo.

## Procedimiento y resultados en PC

1. Analizar `run.json` y `samples.jsonl` de los seis ensayos anteriores, manteniendo metadatos y hashes.
2. Probar el servicio con reloj/tramas sintéticos: colocar antes de medir, completar los 13 pasos con variación temporal de recepción, saturación recuperable, ruido excesivo, lado incorrecto, pérdida/nueva sesión, salto de secuencia, muestras insuficientes y cancelación.
3. Servidor exclusivo QA en `127.0.0.1:8877`; nunca abre transporte físico. Chrome recorre el asistente, induce fallos, recupera posiciones, guarda un perfil sintético y descarga su ZIP. Panel normal `127.0.0.1:8766` queda desconectado y sin perfil real activo.
4. Verificar 320/375/768/1366 px, Escape/foco, idiomas y movimiento reducido. Revisar visualmente escritorio, pantalla estrecha y estados de giro/medición/error. Regresión general del modelo, esquema, cámaras, datos antiguos y controles.

| Comprobación | Resultado | Evidencia |
|---|---|---|
| Python, nueve regresiones nuevas y pruebas existentes | 35/35 | [Log](../evidencias/calibracion_pasos_2026-10-11/python-tests-http-final.log) |
| Idiomas y mensajes del servicio | 33/33 | [JSON](../evidencias/calibracion_pasos_2026-10-11/idiomas-final.json) |
| Asistente en Chrome | 23/23; sin errores JS | [JSON](../evidencias/calibracion_pasos_2026-10-11/browser-checks.json) |
| Modelo/panel en Chrome | 32/32 | [Log](../evidencias/calibracion_pasos_2026-10-11/regresion-modelo.log) |

[Giro](../evidencias/calibracion_pasos_2026-10-11/giro-izquierda.png), [medición](../evidencias/calibracion_pasos_2026-10-11/midiendo.png), [señal inestable](../evidencias/calibracion_pasos_2026-10-11/senal-inestable.png), [pantalla estrecha](../evidencias/calibracion_pasos_2026-10-11/movil.png), [panel desconectado](../evidencias/calibracion_pasos_2026-10-11/panel-desconectado.png). Las capturas con señal/perfil utilizan **datos sintéticos**. [ZIP de prueba](../evidencias/calibracion_pasos_2026-10-11/ensayo-sintetico.zip), separado de registros del kart.

Resultados negativos conservados: primer intento Python bloqueado por permisos de temporales, segundo por socket loopback y por un argumento duplicado en el ayudante de prueba (corregido); ejecución final con temporal del proyecto y permiso de socket local. Primera QA web usó una ruta de módulo incorrecta (`/static/nucleo.js`), corregida a `/nucleo.js`. Primer verificador de idiomas detectó cinco textos obsoletos y no reconoció una plantilla dinámica; se eliminaron textos sin uso, se distinguió la plantilla y se amplió extracción de `pause_step`. Los logs iniciales se conservan en la carpeta de evidencia. La herramienta DevTools no pudo escribir captura por su configuración de rutas; se inspeccionó en línea y se guardaron las capturas con el verificador Playwright local. Se conservaron también los intentos posteriores con timeout de carga (`browser-final.log`, `browser-final-verified.log`, `browser-isolated-final.log`); separar el contexto del navegador por sí solo no solucionó los rechazos de módulos. El resultado satisfactorio posterior está en `browser-queue-final.log`. No fueron averías del kart.

## Limitaciones y siguiente paso

Los registros físicos anteriores corroboran dos rechazos por límite y variación, pero no permiten separar ruido, movimiento, recorte ni avería del sensor. No se relajan criterios para forzar una aceptación. Si el tope físico produce siempre 0/4095, debe investigarse el escalado de origen y obtener ADC físico antes de considerar una calibración completa de recorrido.

Probar la versión nueva **por Wi-Fi en el kart**, inmóvil y con tracción inhabilitada. Documentar alinear → pulsar Medir → mantener, capturar centro y extremos y exportar el ensayo si hay un aviso. Contrastar físicamente centro, repetibilidad y límites. USB solo fuera del kart para banco/grabación/recuperación. No se acredita mejora física de dirección, ni lectura histórica integral, ni copia independiente.
