---
estado: vigente
actualizado: 2026-10-16
resumen: Registro de versiones de firmware del kart (DEC-0001) — qué cambia, si se grabó y con qué ensayo.
---

# Versiones de firmware

## Frontal dev12-log instalada · 23-10-2026

Oscuro fijo, botón global, autograbación microSD por sesiones y DMA con caché/búfer dedicado. Incluye filtro/curva de pedal dev10. Dev11 falló con TX_UNDERRUN en placa y se conserva como resultado negativo. Dev12 grabada/verificada y relectura interna/QSPI cero diferencias; sesiones 2/3 cerradas con 1.184/989 filas, sin pérdidas/error SD. [ENS-0029](../../06_TRABAJO/ensayos/ENS-0029_instalacion_historial_sd.md), HEX SHA-256 `a887b993f9733d45e6710770cc4b1c19d4d83b8b965ece9bd3b70ba8ad23ec1d`. 53 errores ADC posteriores pendientes de diagnóstico; no ensayo de tracción. ESP32 conservado; lector SD/replay real pendientes.

## Frontal dev8-ui instalada · 16-10-2026

**`stm32_frontal-v1.3-dev8-ui` grabada, Verified OK y captura real BMS con 87 % / 22 °C provisionales.** [ENS-0024](../../06_TRABAJO/ensayos/ENS-0024_bms_visible_diagnostico.md): mantiene flags BMS sin confirmar; muestra candidatos con rango/caducidad independiente de 2 s. Oscuro preferido por el usuario, claro opcional conservado. 163/0 dashboard, 150/0 frontal, build F746 correcto; interna 349.644 B, QSPI 6.351.932 B sin cambios. HEX SHA-256 `98b89e6db204712845bfb16550ad94a1bc4984aa9a1fe692eb7ace10db872a08`. Sin cambios de control, calibración, CAN o CSV. Dev7-ui/relectura conservadas; F411 último build usa dev7-ui, última imagen física v0.3.

## Frontal dev7-ui instalada · 16-10-2026

**`stm32_frontal-v1.3-dev7-ui` grabada, Verified OK y arranque real observado.** [ENS-0023](../../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md): respaldo doble de interna/QSPI e identidad dev4 anterior confirmada. Cuatro correcciones reproducidas: jitter táctil, contador CAN, edad Hall y protección de CSV parcial. Conserva diseño de siete páginas claro/oscuro, control y calibración. PC 150/0 frontal, 145/0 dashboard, 3/0 SWD y 204/0 compartidas; F746 y F411 compilados, F411 sin grabar. HEX SHA-256 `0aa8b6b7d38791dc4b487092c6f2d07ce0e4aaa11f41c07afddf280cdd503740`; [paquete, logs y capturas](../../06_TRABAJO/evidencias/frontal_bugs_2026-10-16/README.md). ESP32 físico sin identificar, PROB-0012/0013 abiertos. Sin commit/push nuevo; cobertura física exacta en ensayo.

## Frontal dev6-ui local · 15–16-10-2026

**`stm32_frontal-v1.3-dev6-ui` compilado, sin grabar.** [ENS-0022](../../06_TRABAJO/ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md): siete páginas de desarrollo y temas claro/oscuro, fuentes antialias, condiciones reales de D/R y edades/validez Hall. Se conserva control, calibración y CAN; snapshot ampliado 96→116 bytes con decodificador SWD compatible. CSV sigue esquema 2. 130/0 frontal, 139/0 dashboard y 3/0 SWD; 184/0 banco compartido incluyen las 130. [Fuentes, renders, ELF/HEX y hashes](../../06_TRABAJO/evidencias/frontal_dashboard_2026-10-15/README.md). Fuente dev5 base preservada; cambios concurrentes de otro chat apartados por instrucción del usuario, sin mezclarlos con esta versión. F746 física conocida sigue dev4; instalación y ensayo con motores en kart suspendido pendientes por disponibilidad. Sin commit/push nuevo.

## Frontal dev5 local · 15-10-2026

**STM32 v1.3-dev5 compilado, sin grabar**, sobre HEAD `bebe452`; la placa sigue en dev4. [ENS-0021](../../06_TRABAJO/ensayos/ENS-0021_sd_reintentos_locales.md): reintento SD 1 s después del fallo y limpieza, último error visible durante apertura y pausa comprobada antes de continuar escribiendo. 120/0 frontales y 174/0 con banco compartido; F746 y F411 compilados. [ELF/HEX, BIN F411 y SHA-256](../../06_TRABAJO/evidencias/sd_reintentos_2026-10-15/README.md). La fuente F411 sigue 0.4-dev y ahora consume núcleo dev5; su última imagen grabada sigue v0.3. Sin cambios ADC/START/control/CAN; bloqueo interno de SD y pruebas físicas pendientes. Cambios de esta retoma sin commit/push.

## Frontal F746 grabado · 15-10-2026

**Instalado el 15-10 en la F746: STM32 v1.3-dev4**, HEX `b758edd7…0316`, grabado con OpenOCD (interna + QSPI), `Verified OK` y relectura independiente sin diferencias. [ENS-0019](../../06_TRABAJO/ensayos/ENS-0019_frontal_dev4_f746_real.md).

- **Antes:** había el firmware **original de la entrega** (`STM32-final/STM32CubeIDE/Debug/STM32F746G_DISCO.hex`, coincidencia byte a byte), no una v1.3. Respaldo completo en [RESPALDO.md](../respaldos/stm32_f746/2026-10-15/RESPALDO.md).
- **ESP32 en el bus:** no emite 0x124, compatible con v1.2 o anterior; no identificado.
- **Comprobado en placa:** dev4 funciona más de 3 h sin reinicios. Se cierran F01–F03 en hardware y se abren [PROB-0012](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0012_pedal_reposo_fuera_de_pedal_libre.md) (pedal) y [PROB-0013](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md) (SD).

## Pantalla de desarrollo · 15-10-2026

**STM32 v1.3-dev4**, compilado este día y grabado después en ENS-0019: versión visible/CSV/paquete unificada, limitador R según estado CAN sin umbral supuesto y causas siempre visibles con avisos de modo aparte. [ENS-0018](../../06_TRABAJO/ensayos/ENS-0018_mejoras_pantalla_desarrollo.md): 101 comprobaciones, 13 fallos antes → 0 después; ARM ELF/HEX preservados en `07_DESARROLLO/respaldos/stm32/v1.3-dev4/`, hashes en evidencia. Control y protocolo binario intactos; ESP32 continúa dev3. Cambios locales sobre `43b0c29`, sin commit/push.

**Banco F411 v0.4-dev de fuente**, compilado sin grabar: adapta el formateador compartido y capturador, 155 comprobaciones en PC (incluyen las 101 frontales) y cinco casos de captura con serie falsa. BIN SHA-256 `2647a757e0b878465881f6929851e3f75249beccf5803891f8e6db32ab038a19`. Última imagen grabada/verificada conocida sigue v0.3 del 14-10; no se atribuyen a ella las correcciones nuevas. [DEC-0010](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md): pantallas finales pendientes para cierre del proyecto.

## Revisión frontal · 15-10-2026

Sin nueva versión ni grabación. [ENS-0017](../../06_TRABAJO/ensayos/ENS-0017_revision_preinstalacion_frontal.md): STM32 dev3 recompilado; ELF/HEX idénticos por SHA-256 a los preservados el 13-10, 83 comprobaciones frontales y 56 traseras correctas. Tres casos adicionales de presentación reproducidos en [PROB-0011](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0011_presentacion_frontal_preinstalacion.md). Ajustes y validación F746 pendientes; no inferir firmware físico del rótulo DEV2 mostrado por este código.

## Banco Nucleo F411 · 14-10-2026

**Actual en banco: v0.3 visual**, binario `vistas_v0.3_02.bin`, SHA-256 `fbf86237a4888b81c80508a69d8648d6ae25fa0596848108d972be4f159031a1`, grabado y verificado. Diez escenas adaptadas del frontal, B1 navega, 137 comprobaciones sin fallos, capturas GRAM con confirmación por fila/CRC. [ENS-0016](../../06_TRABAJO/ensayos/ENS-0016_vistas_frontal_nucleo.md). Primera compilación v0.3 y v0.2 quedan conservadas; no cambia firmware de producción F746/ESP32. Trabajo local sin nuevo commit/etiqueta.

**Antecedente, destino separado del kart.** Firmware previo conservado en tres lecturas idénticas. Diagnóstico `v0.1`: LCD MCUFRIEND ILI9341 (`0x9341`). Banco **v0.2**, después sustituido por v0.3: 83 comprobaciones de funciones frontal/trasera dev3 en F411 sin fallos, periféricos simulados, LCD/B1 físicos y visual/START ON–OFF confirmados por Armando. [ENS-0015](../../06_TRABAJO/ensayos/ENS-0015_nucleo_f411_pantalla_banco.md), [proyecto](nucleo_f411_banco/README.md), [respaldo previo](../respaldos/nucleo_f411/2026-10-14/RESPALDO.md). BIN/ELF, fuentes y hashes conservados; sin nuevo commit/etiqueta. No se grabaron las placas del kart ni se migró TouchGFX.

## Entrega pública · 13-10-2026

Corte `revision-academica-2026-10-13`: fuentes ESP32 y STM32 **v1.3-dev3**, panel **0.4.0-calibracion-pasos**. [PUB-0001](../../06_TRABAJO/PUBLICACION_2026-10-13.md) registra publicación y verificación. Las notas siguientes de «sin commit/push» conservan el contexto de cada ensayo y quedan superadas solo en su estado de publicación. No cambia qué se grabó ni acredita nuevas pruebas físicas. Tras «B1.3» declarada el 13-10, versiones de placas pendientes de identificar.

## Pulsador START · 13-10-2026

STM32 **v1.3-dev3**, memoria de START/STOP para pulsador momentáneo y antirrebote; 83 comprobaciones y compilación ARM ELF/HEX correctas. ESP32 **v1.3-dev3 sin modificar**, 56 comprobaciones satisfactorias. [ENS-0013](../../06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md) enlaza copias antes/después, diff, logs, manifiestos SHA-256 y binarios preservados en `07_DESARROLLO/respaldos/stm32/v1.3-dev3/`. Cambios locales sobre HEAD `4f7f150`, sin commit/push ni grabación. Armando declara pruebas reales «B1.3» y pulsador momentáneo, **sin identificar versiones exactas de las placas**; la confirmación física v1.2 del 11-10 queda histórica. Identificación/respaldo frontal+QSPI y ensayo por Wi-Fi pendientes.

## Continuación · 11-10-2026

ESP32 **v1.3-dev3**, compilado y sin grabar: conserva STOP ante STOP→START en el mismo lote CAN y exige un nuevo arranque deliberado. [ENS-0011](../../06_TRABAJO/ensayos/ENS-0011_stop_en_lote_can.md): 56 comprobaciones de control sin fallos, regresión reproducida antes del arreglo; fuentes/binarios y hashes enlazados. Cambios locales sobre HEAD `4f7f150`, sin commit/push. STM32 permanece en **v1.3-dev2**, con sus 54 comprobaciones repetidas. Armando confirma hoy que la versión física sigue en **v1.2** y no tiene acceso al frontal; no se ha leído ninguna placa.

Regla ([DEC-0001](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0001_versiones_de_firmware_y_ensayos.md)):
- Un cambio pequeño por commit.
- Etiqueta Git por versión grabada.
- La versión se muestra al arrancar.

## ESP32 trasero (`esp32_trasero/`)

| Versión | Fecha | Cambios | Compila | Grabada en el kart | Ensayo |
|---|---|---|---|---|---|
| v0 | 2026-10-07 | Fuente entregado sin cambios (SHA-256 `fab150ea…b0a32e`, igual a `02_ORIGINALES/.../ESP32-final.ino`) + `platformio.ini` | Sí: pioarduino 55.03.311, core 3.3.11, mcp_can 1.5.1, MPU6050 1.4.5. Imagen 346 544 B | No: lo que hay en la placa es la imagen del [respaldo 2026-10-07](../respaldos/esp32/2026-10-07_180133/RESPALDO.md) (core 3.3.10, 345 808 B), compatible con v0 pero no demostrado idéntico | — |
| v1-telemetria | 2026-10-07 | Telemetría de solo lectura: SoftAP WPA2 + TCP 3333 + JSON por USB; 6 enganches `// v1` en el `.ino`, control sin cambios. Ver `esp32_trasero/README.md` | Sí: imagen 993 680 B, SHA-256 `8c69929a…6fb95f9d` (con `secretos.h` local) | Sí, 07-10 (banco), etiqueta `esp32-trasero-v1` | [ENS-0001](../../06_TRABAJO/ensayos/ENS-0001_telemetria_esp32_usb_wifi.md): **mal**. Cuelgue en `readMsgBuf` ([PROB-0005](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0005_mcp_can_desborda_con_dlc_mayor_8.md), fallo previo de mcp_can) y registros de error del core inundando el puerto serie |
| v1.1-telemetria | 2026-10-07 | mcp_can 1.5.1 en `lib/` con DLC limitado a 8 (PROB-0005); `CORE_DEBUG_LEVEL=0` como la imagen original; AP con `softAP` antes de `softAPConfig` y diagnóstico `wifi_ap`; `mpu_ok` (sin guiñada si el MPU no responde) | Sí: imagen 955 952 B | Sí, 07-10 (banco), etiqueta `esp32-trasero-v1.1`; sustituida por la imagen original en ENS-0002 | ENS-0001: USB y Wi-Fi sin saltos en banco, sin CAN real. Después el usuario declara fallo de marcha y falta de visualización en el kart; causa pendiente (ENS-0002) |
| Imagen original restaurada | 2026-10-07 | Restauración exacta del respaldo completo previo a los cambios, SHA-256 `31a14b7b…ffb01a8`; sin telemetría v1.1. No es una recompilación de v0 | No se recompila | 4 MB escritos y verificación independiente `verify-flash` correcta; sustituida después por v1.2 de diagnóstico | [ENS-0002](../../06_TRABAJO/ensayos/ENS-0002_recuperacion_esp32_tras_prueba_kart.md): usuario confirma recuperación de funcionamiento del kart; sin mediciones comparativas |
| v1.2-diagnostico | 2026-10-07 | Wi-Fi/diagnóstico antes de CAN, estados de inicio/parada/START/marcha/reset; validación de lecturas; MPU ausente sin lecturas/calibración inválidas y ESC detenido; búfer 1536/TX 2048 | Sí: imagen 956 912 B, SHA-256 `d2a1e207…87021794`; core 3.3.11 | Última imagen grabada/verificada, con módulo fuera del kart, commit `905b09e`, etiqueta `esp32-trasero-v1.2-diagnostico`; referencia de la prueba posterior, sin otra grabación comunicada | [ENS-0003](../../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md): banco USB/Wi-Fi sin CAN, 0 saltos/reinicios y panel verificado. [ENS-0004](../../06_TRABAJO/ensayos/ENS-0004_prueba_real_motores_confirmada.md): el 08-10 Armando confirma prueba real con motores satisfactoria; sin identificación de versión durante la prueba, logs ni condiciones detalladas. Caracterización instrumentada pendiente |

**Imagen extraída frente a v0:**
- La «imagen extraída» es el binario leído de la placa.
- v0 es el fuente de la entrega.
- Restaurar la placa al estado inicial se hace con la imagen extraída, no recompilando v0.


## Desarrollo v1.3 · 08–09-10-2026 (sin grabación)

| Versión | Alcance | Verificación / conservación |
|---|---|---|
| STM32 v0 de trabajo | Copia íntegra de 2025/26; 1673 archivos verificados | Commit `2deb9ef`, ORIGEN_V0.json; no identifica instalado |
| ESP32 v1.3-dev1 | Núcleo modular, estados, rampa, R20, sensores/watchdog | Commit `53ebf40`; 46 comprobaciones y compilación; backup dev1 preservado |
| ESP32 v1.3-dev2 | Revisión final STOP/ADC, D-R/ESC/BLQ, contrato, PWM y telemetría | 51 comprobaciones; compila, RAM 49244 B/flash 949743 B; binario+ELF local |
| STM32 v1.3-dev2 | ADC/CAN, GUI coherente, SD, Hall EXTI, IWDG, configuración/build | 54 comprobaciones; ELF/HEX compilados con startup y QSPI; hardware pendiente |
| Panel: compatibilidad v1.3 | 15 señales nuevas, motivos/modo aplicado, validez y calibración | 26 tests Python/16 JS; cambios locales previos conservados, SHA-256 de archivos afectados |

[ENS-0009](../../06_TRABAJO/ensayos/ENS-0009_robustez_firmware_v13.md), [hashes](../../06_TRABAJO/ensayos/datos/ENS-0009/hashes_dev2.json) y [protocolo](common/PROTOCOLO_V13.md). Las compilaciones no son firmware leído de placas. R limita propulsión estimada; ESC experimental, física/calibraciones/BMS pendientes. No push ni copia independiente configurada. V1.2 continúa como última grabación conocida en el kart.
