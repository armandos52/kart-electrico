# ENS-0021 · Reintentos y pausa de SD en el frontal

Fecha: 15-10-2026. **Pruebas de software en PC; dev5 compilado, sin grabar.** Problema: [PROB-0013](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md). [Evidencias, fuentes, logs y hashes](../evidencias/sd_reintentos_2026-10-15/README.md).

## Objetivo y punto de partida

Armando pide revisar dónde quedó la otra sesión y seguir; después confirma continuar con código y pruebas locales, sin placa disponible. HEAD inicial `bebe452`; el estado inicial y los cambios anteriores sin commit se conservan en la evidencia. ENS-0019 ya acredita dev4 grabado y las tres mejoras de presentación verificadas. Se contrastan la relectura y el último snapshot conservados; no se repite una medida física. El registro dedicado del pedal quedó sin muestras, documentado en [ENS-0020](ENS-0020_registro_pedal_f746.md).

En dev4, `sd_last_try` se fija antes de `f_mount`, que tardó unos 30 s sin tarjeta en ENS-0019: al fallar, la espera de 1 s ya había pasado. El siguiente intento borra además el error. Las pruebas dirigidas identifican otra consecuencia: si se solicita pausa durante el montaje o apertura, se continúa escribiendo al retornar.

## Configuración y cambio

- Fuente F746/TouchGFX de trabajo: **stm32_frontal-v1.3-dev5**. Instalado conocido: **dev4**, intacto. ESP32 sigue dev3 de fuente, sin cambios ni nueva identificación física.
- MSVC x64 ejecuta funciones reales frontal/formateador y núcleo ESP32 con HAL/FatFs/reloj sintéticos. Se amplía FatFs simulado con duraciones y acciones durante llamadas para reproducir espera, pausa y observación de pantalla. Los 30 s y otras demoras de las pruebas son entradas sintéticas, no nuevas mediciones.
- `sd_publish` toma el tiempo del error y `sd_close` lo actualiza al finalizar sincronización/cierre/desmontaje. El reintento exige 1000 ms desde ese final, incluidos errores de apertura, escritura y sync; funciona al desbordar el contador de tiempo.
- Durante reintentos se muestra `SD:ABRE E:1` si el último error fue 1. `E` conserva el último error hasta alcanzar grabación; `ABRE` informa de un intento en curso. No significa ausencia de tarjeta identificada.
- Se vuelve a comprobar la petición de grabación después de montaje, apertura y cabecera. Una pausa evita las escrituras posteriores y cierra el archivo abierto cuando la operación pendiente devuelve el control.
- Tras apertura/cabecera se renueva la referencia temporal: el montaje lento no adelanta artificialmente la siguiente fila o sync. CSV esquema 2 y periodos nominales 200 ms/1 s conservados.
- Versión única de fuente, pantalla y CSV pasa a dev5 para distinguir la nueva compilación del dev4 instalado. No cambian ADC, umbrales, START, protocolo CAN ni control de motores.

## Procedimiento y resultados

1. Conservar fuentes anteriores e inventario de cambios preexistentes. Añadir 19 comprobaciones a las 101 existentes; reproducir contra producción dev4: **120 comprobaciones, 16 fallos**. Primer arnés y ajuste del verificador conservados; se mueve una comprobación fuera del callback simulado para que su ejecución no dependa de entrar en montaje. El resultado se mantiene en 120/16.
2. Aplicar la corrección y repetir el mismo conjunto: **120 comprobaciones, 0 fallos**. Incluye montaje/apertura lentos fallidos, escritura parcial, cierre lento, sync fallido, pausa en tres puntos, recuperación, error visible, periodos y desbordamiento del tick; regresiones previas de control/START/CAN/presentación pasan.
3. Ejecutar banco visual compartido en PC: **174 comprobaciones, 0 fallos**; son las mismas 120 más 54 visuales/navegación, no 174 independientes de las frontales.
4. Compilar F746 con ARM GNU 14.3.Rel1 y `tools/build.py`: **BUILD OK**, 151 unidades, ELF+HEX con interna/QSPI. Advertencias de dependencias y conversiones conservadas en log; no se declara compilación sin advertencias.
5. Compilar F411 que incluye las pruebas compartidas: **SUCCESS**, 62.420 B flash/2.264 B RAM. Las pruebas no se ejecutan en MCU en esta sesión. La última imagen conocida grabada en F411 sigue v0.3. Se explicita una conversión a `uint8_t` para eliminar una nueva advertencia de MSVC en `sd_pause`; se repiten los conjuntos y ambos builds sobre la fuente final. Logs iniciales y finales conservados; las advertencias restantes son anteriores al cambio.

## Límites, decisión y siguiente paso

**Corrección parcial de PROB-0013:** se corrigen reintentos, persistencia del diagnóstico y comportamiento al volver de una operación. **No se acorta ni se interrumpe `f_mount`**, que puede seguir tardando unos 30 s; una pausa debe esperar a que retorne. En el driver local, `BSP_SD_IsDetected` devuelve siempre `SD_PRESENT`; no se añade una supuesta detección física sin confirmar señal y montaje. Tampoco se modifica HAL/FatFs ni se promete recuperación del sistema de archivos tras escritura parcial/corte.

Se conserva el problema abierto hasta probar con/sin tarjeta, pausa durante apertura y durante grabación, reinserción, contenido CSV, error visible y tiempo de respuesta en la F746. Antes de grabar dev5, identificar la configuración y conservar respaldo; el paquete contiene flash interna y QSPI. La calibración del pedal exige muestras y queda pendiente. No conexión, flasheo, medida física, commit/push ni copia externa en esta retoma. Pantallas de usuario final siguen pendientes según DEC-0010.
