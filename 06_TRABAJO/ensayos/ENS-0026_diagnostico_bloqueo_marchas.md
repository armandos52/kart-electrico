---
estado: dev9-ui instalada y verificada; mejora declarada y limitaciones físicas abiertas
actualizado: 2026-10-21
---

# ENS-0026 · Bloqueo en N y cambio de modo · 21-10-2026

## Objetivo y configuración

Armando solicita diagnosticar el kart conectado al banco, autoriza pruebas y simplificar programación si hace falta. Confirma que queda en N y no cambia de modo; después confirma acelerador libre y atribuye su variación al ruido del kart. Se distingue esta atribución de la variación ADC medida: no se ha medido espectro ni origen eléctrico.

HEAD base `bebe4528747bc6e05dc1ba244cd3ce9a090a7942`, con cambios previos conservados. F746 dev8-ui: tabla de vectores y cadena de versión coinciden con ELF `ec8b3b9dd74297e53f2aa9b7a0c739b46de91ccad94e2b5997708d4e8a892c0d`; UID `00420026 54435003 20353036`, ST-LINK `0670FF3332584B3043206038`. No es una comparación integral de flash en esta sesión. ESP32 identificado por cabecera y 61 muestras Wi-Fi como **esp32_trasero-v1.2-diagnostico**, sesión `6fc7b05399a0f013`. Sin USB ESP32, órdenes TCP, halt ni escrituras de memoria. [Evidencias](../evidencias/marchas_2026-10-21/README.md).

## Diagnóstico inicial medido

- `observacion_host`: CAN fresco (161 ms), 0 RPM traseras, entradas válidas, cero errores CAN/ADC, START ON, selector N, marcha N. ADC pedal 190 → normalizado 155 → `pedal_free=0`.
- `serie_inicial`: 160 muestras en 7,951 s. Tras excluir primer segundo: 140 muestras ADC **178–229**, mediana 206; pedal libre **0 %**. La herramienta etiqueta automáticamente «reposo»; la confirmación explícita del operador llegó después. No usar esa etiqueta sola como prueba de posición del pie.
- `estado_selector`: 400 muestras/20 s; ADC **176–238**, mediana 208; selector D, freno 0, START 1, marcha N; CAN utilizable/parado en 400/400, errores CAN/ADC 0. No había confirmación de freno pisado en este tramo.
- Wi-Fi 11:06:46–11:06:58 Europe/Lisbon: secuencias 1036–1096, 61 muestras, `drive_inhibit=neutral`, mando CAN reciente, RPM 0 y PWM crudo 255/255 (codificación invertida de v1.2; no significa aceleración plena). Último motivo de reset **brownout_tension** y `mpu_ok=0`. No hubo reset observado dentro de esta captura; el motivo corresponde al arranque anterior. Se restauró eduroam al terminar.
- `reposo_confirmado` y `reposo_OFF_N`: dos intentos cancelados tras una muestra porque START era OFF pero el pin D seguía activo. ADC 257/259 con pedal declarado libre. No son registros completos. La discrepancia temporal entre respuesta del operador y pin no demuestra selector averiado.
- `gpio_inicial.log`: PC6/PC7 son entradas con pulldown (`MODER=0x80`, `PUPDR=0xa000`); en esa lectura ambos niveles ya eran 0. PG6 entrada con pulldown. No se modificaron registros.
- `transicion_selector_freno`: 799 muestras/40 s, START OFF, R 799/799; freno 1 en 717 y 0 en 82, ADC **178–302**, `pedal_free=0` y marcha N en todas. Usuario confirma R y freno al inicio del registro; no se atribuye cada transición a una acción sin marca temporal independiente. CAN válido/parado en todas; errores 0. Selector y freno responden.

La normalización dev8 usa `ACCEL_MIN=124`, `ACCEL_MAX=1861` y exige normalizado ≤40 para pedal libre: **ADC ≤141**. Esa condición común impide tanto entrar D/R como cambiar DIF/ESC/BLQ; el modo exige además selector físico N, aunque la marcha aceptada ya sea N. D/R requieren además freno y quietud. El síntoma queda reproducido sin quitar las otras condiciones.

## Prueba de software y límites

`reproducir_bloqueo.cpp` ejecuta la lógica frontend real con periféricos simulados: cuatro ADC medidos (178,190,206,229), freno y CAN correctos, reproducen bloqueo D y de modo; controles 141/142 y falta de freno. **11 comprobaciones / 0 fallos**: confirman el diagnóstico, no son una reparación ni una prueba física de motores. Fuente dev8 conservada en `antes/`.

Fallos de herramientas preservados: primer acceso SWD limitado `open failed`, resuelto con acceso al dispositivo del host; captura gráfica falló por Pillow ausente en el venv, pero los volcados se conservaron y PNG se decodificó con runtime integrado; registrador anterior terminó con error de consola cp1252 tras guardar CSV/resumen, datos recuperados sin repetir como medición. Nuevo `registrar_marcha_swd.py`: snapshot completo, identidad y cancelación del registro de pedal si START/selector no están OFF/N. Es solo observador: no detiene el kart.

## Medición dedicada y corrección

Operador confirmó START OFF, selector físico N, pedal libre; ambas entradas fueron comprobadas en el registro. `reposo_OFF_N_2`: **600 muestras / 30 s, ADC 156–215**, media 187,13 y desviación 9,83 cuentas; ninguna cumplía pedal libre en dev8. `fondo_1`, a fondo confirmado: **200 / 10 s, ADC 1872–1924**, media 1898,07, todas normalizadas a 4095. `retorno_1`, pie retirado confirmado: **400 / 20 s, ADC 187–245**, media 218,56, desviación 9,74; ninguna libre en dev8. START OFF y N se mantuvieron; ADC/CAN sin errores y RPM traseras 0. No prueba de tracción, recorrido mecánico lineal ni caracterización de toda condición eléctrica.

**Dev9-ui** separa la zona cero de los límites de plausibilidad: ADC válido **24–1961** (límites históricos conservados); mando 0 hasta **320** inclusive; por encima, `(ADC−320)×4095/(1861−320)` con saturación en 4095. Fondo 1861 conservado, compatible con lo observado. Se adopta 320 como **parámetro provisional de banco**, máximo de reposo observado 302 + 18 cuentas de margen; no especificación del sensor ni filtro EMI. Reduce el tramo útil inicial; requiere seguir comprobando variación bajo carga. No hay aprendizaje automático al arrancar ni retardo de filtro al soltar. El permiso de pedal libre exige señal válida y mando exactamente cero; una lectura inválida ya no aparece como «libre». ADC bruto se conserva en pantalla/telemetría. Referencia de zona cero visible en Entradas.

Pruebas PC: **166 comprobaciones, 8 fallos antes → 0 después**; 163/0 dashboard y 3/0 decodificador SWD. Intervalo completo 156–302 reproducido, límites 320/321, fallos ADC, plausibilidad 23/24 y 1961/1962, CAN caducado, freno, selector incoherente, movimiento, recorrido monótono y retorno inmediato. 124 renders sintéticos; se actualizan números de los escenarios al nuevo mapeo, sin presentarlos como capturas. Build ARM correcto; avisos de dependencias conservados. ELF SHA-256 `776318e9fba824f8afa4e814fb2cef44fc18eaec58d5197b9332f175be54a87b`, HEX `31f83c4c5f8b2f135212f22691d24e0075d8cd64609afd22d192c69bd22f3006`; interna 349.644 B, QSPI 6.351.932 B idéntica a dev8. Sin cambios en ESP32, dirección, START, permisos por freno/velocidad/CAN ni algoritmos DIF/ESC/BLQ.

Respaldo previo completo en [2026-10-21](../../07_DESARROLLO/respaldos/stm32_f746/2026-10-21/RESPALDO.md): dos lecturas internas de 1 MiB iguales; QSPI 16 MiB idéntica al respaldo completo 16-10. Ambas regiones usadas coinciden byte a byte con dev8-ui. OTP/opciones conservadas, sin desbloqueo ni borrado de originales. Lectura 11:20:19–11:23:21 local. El marcador heredado `QSPI2_FIN` del log no representa una segunda lectura nueva QSPI: se hizo una y se comparó con la anterior completa.

Revisión automática rechazó inicialmente la grabación por faltar autorización explícita de esa versión. Se informó al usuario y **Armando autorizó: «Sí, instalar dev9-ui y comprobarla en el banco»**. Primer intento rechazado no ejecutó proceso ni escribió flash. Se continúa por la vía autorizada, con guardas UID/RDP, START OFF, selector/marcha N y ADC ≤320 antes de programar.

## Instalación y ensayo posterior

**Dev9-ui grabada 11:25:25–11:26:47 local**, `Verified OK`, reset/run. Relectura independiente de ambas regiones usadas coincide byte a byte, 0 diferencias; ver `grabacion.log`, `relectura/verificacion.json`. La afirmación inicial de identificación parcial queda ampliada por el respaldo/comparación integral posterior de dev8 y esta verificación de dev9.

`dev9_reposo`: 262 muestras en aproximadamente 13 s, ADC 158–218, mando cero/pedal libre en todas. El registro previsto de 20 s se cancela al pasar START a ON en la última muestra por acción del operador; no se presenta como ensayo completo OFF/N.

`wifi_dev9_1`: 61 muestras/12 s, 11:28:13–11:28:25 local; marcha R, modo DIF, START ON, inhibición «none». Mando 871–4095, RPM izquierda 0–447/derecha 8–652; PWM crudo invertido 0–204/0–200. Misma sesión ESP32, `mpu_ok=0`. Registra movimiento de ruedas según Hall; no velocidad física sobre suelo ni par. El usuario aceleró durante su prueba: las instrucciones propuestas OFF/N no describen este tramo real.

`dev9_marchas_modos`: 1.198 muestras/60 s (11:28:58–11:29:59), START ON en todas, modo DIF en todas. Se observa D y N; no acredita el ciclo completo D/N/R ni tres cambios de modo solicitados. D entra a t≈11,57 s con condiciones válidas y vuelve a N a t≈32,814 s al comenzar a aumentar los errores ADC; aparecen flags 5 (dirección inválida) y dirección hasta **813**, por debajo de **942** admisibles (1042−100). Total final 457 errores ADC; 44 snapshots con flags 5. El control muestrea nominalmente a 200 Hz y el registrador aproximadamente a 20 Hz, por eso no hay correspondencia uno a uno de errores/snapshots. Pedal observado 163–1906, dentro de plausibilidad, CAN utilizable en todas y sin errores CAN.

Armando declara **«sí funcionó mejor»** y confirma que giraba **hacia un tope** al volver a N. La lógica de dirección inválida explica la caída a N reproducible en PC; falta medir topes para decidir si rango o señal eléctrica son incorrectos. No se amplía el rango a partir de un mínimo aislado. [PROB-0007](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md) sigue abierto.

Después informa menor sensibilidad, inicio de aceleración alrededor de ADC 800 y pantalla desconectada. Esta declaración no se presenta como umbral mecánico medido. Pide media ponderada y recuperar respuesta anterior; continuación **solo PC** en [ENS-0027](ENS-0027_pedal_media_ponderada.md), dev10 compilada sin instalar.

## Decisión y siguiente paso

Se conserva el permiso de pedal libre con mando cero, las demás protecciones y la referencia histórica de fondo. Instalación/relectura completadas; aceptación final de sensibilidad, topes y secuencia completa de modos pendiente. Separar el diagnóstico eléctrico del ruido, [brownout](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0016_brownout_reportado_esp32.md) e [IMU](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0017_imu_no_disponible.md) de la causa lógica demostrada del bloqueo.

Consulta focalizada de estado/mapa, índice/ficha DOC-0042, PROB-0012/0015, inventario y código de trabajo. Buscador: DOC-0042 PDF 29/impresa 15 (entradas digitales históricas); original STM32 `main.c` líneas 429–430 y declaraciones de pines. Texto, sin nueva revisión visual ni lectura integral. La configuración GPIO real se contrastó por registros, no se dedujo del informe.
