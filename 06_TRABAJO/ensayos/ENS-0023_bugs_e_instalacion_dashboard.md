---
estado: dev7-ui grabada y verificada; interfaz aceptada por el usuario
actualizado: 2026-10-16
resumen: Cuatro defectos reproducidos y corregidos en dev7-ui; instalación autorizada en la F746 del kart.
---

# ENS-0023 · Corrección de fallos e instalación del dashboard · 16-10-2026

## Objetivo, autorización y configuración

Armando pide «pule el programa de errore bugs en busca de fallos reales». Se revisa el dashboard y el núcleo frontal de ENS-0022, conservando la base dev6-ui antes de editar. Durante la revisión declara: **«ya esta conectada la pantalla y todo encendido el carro esta con las llantas levantadas podemos instalar la nueva pantalla»**. Esta declaración autoriza la instalación y describe las condiciones del kart; no sustituye una inspección física del montaje.

Fuente nueva **`stm32_frontal-v1.3-dev7-ui`**, sobre HEAD `bebe4528747bc6e05dc1ba244cd3ce9a090a7942` con cambios locales previos conservados. PC Windows, ARM GNU 14.3.Rel1, MSVC 2022, OpenOCD xPack 0.12.0-7 y dependencias de TouchGFX existentes. No se cambia la fuente ESP32 dev3 ni se instala en él; su imagen física sigue pendiente de identificar. F411 fuente 0.4-dev compilada como regresión, sin upload. Pantallas de desarrollo según DEC-0010.

F746 conectada: ST-LINK `0670FF3332584B3043206038`, firmware V2J47M34; ID `0x10016449`, UID `0x00420026 0x54435003 0x20353036`, flash interna 1024 KiB, RDP nivel 0. QSPI ya mapeada, CCR `0x0F182DEB`. El programador informa 3,217323 V; no es una medición calibrada de alimentación. Identificación coincide con ENS-0019.

## Fallos reproducidos y cambios

| ID | Disparador y efecto anterior | Corrección y alcance |
|---|---|---|
| B01 | Un desplazamiento táctil de 1 píxel cancelaba el botón. TouchGFX entrega arrastre con umbral local cero; pulsar/soltar lejos, sin evento intermedio, podía activar el mismo botón. | Se admite desviación máxima de 8 px por eje desde el punto inicial, dentro de la misma zona. Cruzar ese límite cancela hasta levantar; también se comprueba la posición de liberación. Cancelación explícita conserva su efecto. 8 px es una tolerancia elegida, pendiente de valoración manual. |
| B02 | El filtro CAN admite tráfico del bus completo; tramas válidas ajenas con DLC distinto de 8, o extendidas, aumentaban `can_rx_errors`. | Ignorar primero formatos/identificadores ajenos. Para los cuatro IDs manejados se sigue exigiendo trama de datos estándar y DLC 8. Se conserva rechazo de tramas cortas/RTR propias. |
| B03 | Llega un pulso Hall durante la transmisión CAN, después de capturar `now`. Restar un tick posterior generaba edad `4294967295`, RPM 0 e invalidez momentánea. | Leer el reloj con la copia protegida de los campos Hall. Caso sintético tick de control 1099, pulso 1100: edad pasa de 4294967295 a 0 y RPM válida 150. Caducidad a 501 ms sigue invalidando. No es una medida física de RPM. |
| B04 | Escritura parcial de cabecera/fila deja archivo no vacío. Reintento append anterior añadía filas a ese fragmento y corrompía su estructura. | Abrir también para lectura; verificar metadatos esquema 2, columnas exactas y último salto de línea antes de añadir. Archivo incompatible/incompleto se conserva sin más escrituras, error propio 128 y aviso de revisar CSV. No se trunca ni repara automáticamente. |

Cambios en `kart_frontend.c`, `kart_dashboard.c/.h`, `kart_frontend.h` y adaptador `KartDashboard.cpp`; stubs FatFs/CAN y regresiones ampliados. [Fuentes antes/después y diff](../evidencias/frontal_bugs_2026-10-16/README.md). La cabecera CSV completa sigue siendo una sola llamada de escritura: separar metadatos y columnas habría permitido que una pausa entre ambas dejara un archivo incompleto; esa variante fue descartada antes del paquete final y se añadió regresión de reanudación.

El CSV mantiene esquema 2 y acepta archivos intactos dev4 con esas columnas. La validación no recorre todas las filas ni verifica checksum; no detecta toda corrupción posible ni garantiza resistencia a cortes eléctricos. Para recuperar un CSV rechazado, conservarlo y apartarlo con otro nombre antes de crear uno nuevo. Los fallos reales de lectura/seek conservan su FRESULT. La pausa llegada durante validación evita nuevas escrituras después de retornar.

Se conservan calibración/umbrales, permisos D/R, START físico, control trasero y formato CAN. Hall sigue con 4 PPR históricos y límite 2000 RPM. Snapshot sigue 116 bytes; temas oscuro/claro y siete páginas de ENS-0022, navegación durante marcha y tema solo en RAM. No se introducen mandos remotos de motor.

## Pruebas automáticas y compilación

| Prueba | Resultado y evidencia |
|---|---|
| Frontal antes del arreglo, primeras regresiones añadidas | **140 comprobaciones / 5 fallos**, `frontend_antes.log`. B02 dos casos, B03 uno, B04 dos. |
| Dashboard antes del arreglo | **144 / 3 fallos**, `dashboard_antes.log`. Jitter de 1/8 px y salto grande al liberar. |
| Frontal final, con controles de recuperación y pausa | **150 / 0**, `frontend_final.log`. Se conservan también resultados intermedios 149/0. |
| Dashboard final | **145 / 0**, `dashboard_despues.log`, 120 escenas sintéticas en `renders.jsonl`; error CSV 128 comprobado. |
| Banco compartido PC | **204 / 0**, incluye las 150 frontales y 54 visuales; no sumarlas como independientes. `banco_host.log`. |
| Decodificador SWD | **3 pruebas / 0 fallos**, layouts 96/116 bytes y tamaño desconocido; `swd_tests.log`. |
| F746 ARM | **BUILD OK**, ELF/HEX interna + QSPI; `build_f746.log`. Interna ocupada 349.036 B, QSPI 6.351.932 B. Avisos de dependencias conservados; no equivalen a prueba de carga/tiempos. |
| F411 ARM | **SUCCESS**, flash 68.160 B, RAM estática 2.328 B; `build_f411.log`. Sin grabar. |

HEX dev7-ui SHA-256: `0aa8b6b7d38791dc4b487092c6f2d07ce0e4aaa11f41c07afddf280cdd503740`. ELF, HEX, mapa, binarios F411 y SHA-256 se conservan en el paquete. Sin commit/push nuevo. Estas suites usan periféricos simulados; no constituyen ensayos de motores, dinámica ni mediciones de latencia.

## Instalación física y resultados

Primer acceso sin permiso ampliado al USB falla con `open failed`; repetición con acceso al ST-LINK identifica correctamente la placa. No se interpreta ese fallo como avería/desconexión ni como rechazo de aprobación automática. Logs de ambos intentos conservados.

Respaldo a 1,8 MHz SWD completado 13:21:53–13:27:35, hora local Europe/Lisbon: dos lecturas completas de interna 1 MiB y dos de QSPI 16 MiB, más lectura de OTP/option bytes; sin halt/reset ni escritura en esta fase. Tamaños y parejas idénticos; ambas regiones del HEX dev4 coinciden byte a byte con lo instalado. [Respaldo y SHA-256](../../07_DESARROLLO/respaldos/stm32_f746/2026-10-16/RESPALDO.md), separado del original del 15-10. Programación 13:27:56–13:29:15: chip/UID/RDP comprobados, interna + QSPI, **Verified OK**, reset run. Sin escritura OTP/option bytes ni cambio de protecciones.

Relectura independiente en ejecución: **0 diferencias** frente al HEX, interna 349.036 B y QSPI 6.351.932 B; [comparación](../evidencias/frontal_bugs_2026-10-16/relectura/verificacion.json). Capturas reales [arranque](../evidencias/frontal_bugs_2026-10-16/arranque_real/pantalla.png), [Ruedas](../evidencias/frontal_bugs_2026-10-16/observacion_2/pantalla.png) y [Resumen posterior](../evidencias/frontal_bugs_2026-10-16/observacion_3/pantalla.png), con JSON y volcados. El tick avanza de 16.367 a 271.537 ms entre observaciones, sin reinicio observado. Los incrementos durante esperas nominales 500 ms son 503/512/511 ms, no medidas de jitter de control. Lecturas de framebuffer no sincronizadas con pintura: aparecen cortes finos en algunos píxeles; no se atribuyen al LCD sin confirmación del operador.

En tres muestras: CAN 0x123 reciente, 0x124 nunca, CAN TX/RX y ADC errores 0; BMS 0x355/0x356 recibido pero flags de validación 0. Pedal físico 139/139/152, normalizado 35/35/66; condición libre 1/1/0, sin protocolo dedicado de posición del pie. Freno 0, selector D; N/START OFF al arrancar y después D/START ON por interacción física del usuario, sin inyección remota. Velocidad/RPM traseras 0 en esas muestras; no se acredita giro ni relé real. SD pausa, sin ensayo de escritura.

Armando responde: **«esta perfecto me quedo con e formato en oscuro el blanco no se ve bien todo funciona bien , la parte de la bateria porcentage no se ve y tampoco la temperatura»**. Confirma funcionamiento manual y prefiere oscuro; claro tiene mala legibilidad según su valoración, no medición de contraste. BMS oculto corresponde a la política anterior de formato sin confirmar, no ausencia de tramas. Se continúa en ENS-0024 para mostrar interpretación provisional de diagnóstico con caducidad y límites; dev7-ui queda conservada.

## Limitaciones y siguiente paso

PROB-0012 (pedal en reposo fuera del umbral) y PROB-0013 (latencia interna del driver SD) siguen abiertos. No se cambia el umbral sin medir recorrido/reposo. Una SD bloqueada puede tardar en atender pausa. BMS sigue condicionado al protocolo validado. Falta identificar ESP32 por Wi-Fi; su USB no es accesible instalado. Validar táctil/ambos temas y siete páginas en placa, SD con/sin tarjeta y motor/relé con operador, manteniendo START físico. No atribuir a la interfaz la eliminación del bloqueo del pedal.

Cobertura documental L4 focalizada: estado/mapa/catálogo y notas previas, frontend, renderer, adaptador, stubs y APIs locales necesarias. No nuevas páginas originales, lectura integral, copia externa ni cierre de formatos visuales/Loop.
