---
estado: dev12 instalada y verificada; grabación/cierre reales comprobados, replay real pendiente
actualizado: 2026-10-23
---

# ENS-0029 · Instalación del historial microSD y oscuro fijo

## Autorización, objetivo y configuración

Armando autorizó implementar todo el plan y después solicitó «ya esta la placa lista instala la nuva verccion y conprueba que todo funcione». Instalar el frontal, observar arranque/SD y preservar resultados positivos y negativos. No se presupone autorización para acelerar físicamente el kart; este ensayo no ordena movimiento. ESP32 instalado se conserva, sin USB ni nueva grabación trasera.

Base Git `bebe4528747bc6e05dc1ba244cd3ce9a090a7942` con cambios locales previos. ST-LINK `0670FF3332584B3043206038`, F746 UID `00420026 54435003 20353036`. Respaldo de dos lecturas internas de 1 MiB iguales, QSPI completa de 16 MiB, OTP y opciones en [respaldo](../../07_DESARROLLO/respaldos/stm32_f746/2026-10-23/). Las regiones usadas coinciden byte a byte con dev9-ui. Lectura inicial START OFF, selector N, ADC pedal 151, mando cero, CAN fresco y errores CAN/ADC cero. Estas condiciones corresponden a muestras observadas, no a una caracterización mecánica.

## Implementación y pruebas previas

Oscuro fijo, botón global Grabar/Detener; autograbación independiente de START, sesiones exclusivas en LOGS, cola limitada y pérdidas visibles, datos/eventos/metadatos/cierre, rotación y reintentos. Conserva la media ponderada y curva de pedal de dev10; no modifica el rango de dirección ni el firmware del ESP32.

Antes de instalar se corrige exclusión mutua del reloj de captura/reinicio, tiempo detenido congelado y conversión decimal de 64 bits independiente de printf nano. Pruebas PC: frontal **170/0**, dashboard **163/0**, decodificador SWD **3/0**, pruebas existentes del panel **35/0**. No equivalen a aceptación de todas las funciones nuevas de replay ni a prueba física de tracción. Primera ejecución del panel falló por socket localhost denegado; repetición con acceso al host pasó. Primer acceso SWD limitado falló `open failed`; acceso al host permitido. Un comando de prueba usó directorio incorrecto; repetido correctamente, conservando logs.

## Dev11: resultado negativo conservado

Grabada interna+QSPI entre **14:37:01–14:38:21 Europe/Lisbon**, `Verified OK`, reset/run. HEX SHA-256 `d08dff586291218197861f9b7b025d29dea29aa867faf77d2ea30cdc038ef3c8`. Relectura independiente de 352.684 B internos y 6.351.932 B QSPI con **0 diferencias**.

Arranque observado: pantalla, CAN y ADC activos; SD error FatFs 1, cero filas/sesión, 3 muestras descartadas, máximo de llamada SD 55 ms. Volcado `hsd1.bin`: ErrorCode **0x10 = TX_UNDERRUN** según `stm32f7xx_ll_sdmmc.h`, tarjeta identificada. Se corrige la interpretación preliminar de recepción dada en conversación: es falta de datos en FIFO de transmisión. Son compatibles con preempción de la tarea SD por control/GUI; no se midió un trazado de planificación que pruebe el instante exacto. No se formateó ni borró la tarjeta.

## Corrección dev12

Sustituye sondeo de FIFO por DMA de un sector con espera cooperativa, plazo de 500 ms por petición, abortado/limpieza al fallar y callbacks sin colas antiguas. Búfer exclusivo de 512 B alineado a 32 B en SDRAM (`0xC013EC00` en este ELF); limpiar caché antes de escribir e invalidar antes/después de leer. Referencia técnica: [ST AN4839, coherencia CPU/DMA](https://www.st.com/resource/en/application_note/an4839-level-1-cache-on-stm32f7-series-and-stm32h7-series-stmicroelectronics.pdf), §2.2. No se cambia reloj SD ni prioridades de control. La inicialización HAL conserva sus límites internos: no se garantiza cota global de 500 ms para montaje.

Build ARM correcto y frontal 170/0. HEX SHA-256 `a887b993f9733d45e6710770cc4b1c19d4d83b8b965ece9bd3b70ba8ad23ec1d`, ELF `b5c9e28880810eef7c6b082d841101ae149db017f70d51711cdc81621e8929e7`. Grabación con `Verified OK` y reset/run **14:44:08**. Relectura independiente de 353.084 B internos y 6.351.932 B QSPI: **0 diferencias**, [verificación](../evidencias/instalacion_dev11_2026-10-23/dev12/relectura/verificacion.json). Se conserva dev11 y su evidencia; dev12 no es prueba de firmware trasero nuevo. El comentario inicial del adaptador se rectificó después de compilar (TX underrun en vez de RX overrun), sin cambio de instrucciones ejecutables.

## Resultado físico dev12

- Primera observación a tick 18.527 ms: sesión 1, 920 filas, cero pérdidas/errores SD/CAN/ADC, 30.554.304 KiB libres informados por FatFs. Grabación automática sin START. Máximo acumulado de llamada SD 203 ms; incluye montaje, no es latencia medida del control ni una cota futura.
- Serie `continuidad`: **1.198 muestras SWD / 59,996 s**. El operador ya había pasado a sesión 2 al comenzar: 240 filas iniciales → **1.184 filas**, cierre «Guardado», cola cero y tiempo congelado en 23.665 ms. 378 snapshots grabando y 820 guardado; errores SD/CAN/ADC y pérdidas cero en todas, cola máxima 3. Armando confirma pulsación Detener. Este minuto observado no equivale a un minuto entero grabando ni acredita físicamente el contenido de cada fila en el medio.
- Serie `reanudacion`: prevista 60 s, cancelada a **179 muestras / aproximadamente 9 s** por pin D activo en una muestra, START OFF y marcha N. No se presenta como ensayo completo en N ni se atribuye el pin a ruido/acción concreta sin confirmación.
- Después: **sesión 3**, **989 filas**, duración registrada 18.920 ms, estado Guardado, cero pérdidas/errores SD, cola cero, archivos cerrados. Usuario confirma Grabar/Detener. Demuestra incremento de identificador y cierre por firmware; falta lectura independiente de originales para acreditar contenido/preservación byte a byte de sesiones anteriores.
- En la observación posterior aparecen **53 errores ADC acumulados**, sin errores CAN; lectura instantánea válida y pedal cero. Contador permanece en 53 en la observación final. No se midió el origen ni cada muestra inválida: no afirmar defecto SD, ni dar ruido/topes por resueltos. La captura final muestra Entradas, no acredita navegación a la pestaña SD pese a haberse solicitado.
- Estado final: **Guardado**, autograbación manualmente detenida, START OFF, selector/marcha N. No se realizó prueba de tracción ni se actualizó el ESP32. Se conserva captura digital del framebuffer.

## Pendientes concretos

Armando declara **no tener lector microSD ahora**. Quedan pendientes copia/hash de archivos reales, reproducción en portátil y contraste de cronología/señales; tampoco se ensayaron retirada, reinserción, tarjeta llena, corte de alimentación ni ensayo prolongado/rotación física. No se formatea la tarjeta. Autograbación volverá a activarse al reiniciar; durante este arranque queda en Guardado hasta pulsar Grabar. Extensión diagnóstica del ESP32 solo compilada, no instalada: 0x124/0x125 ausentes en estas muestras, con valores desconocidos visibles en el frontal.

## Evidencias y límites

[Carpeta de datos, binarios, fuentes, logs y capturas](../evidencias/instalacion_dev11_2026-10-23/). El framebuffer es captura digital del contenido enviado al LCD, no fotografía ni validación de brillo/táctil. No se atribuye fecha de ensayo a archivos sin reloj RTC. Persisten la validación BMS provisional, ruido/topes de dirección, reversa v1.2 e IMU previamente pendientes.
