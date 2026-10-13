---
estado: regresión corregida y compilada en PC; grabación y validación física pendientes
fecha: 2026-10-13
configuracion: STM32 dev2 a dev3 de trabajo; ESP32 dev3 sin cambios; versiones instaladas sin identificar
---

# ENS-0013 · START/STOP con pulsador momentáneo

## Objetivo, origen y diagnóstico

Resolver la activación/desactivación instantánea del relé declarada por Armando durante pruebas reales en «B1.3». Confirma pulsador físico que vuelve al soltarlo. Sospecha de doble pulso/contacto falso del usuario; sin registro eléctrico ni firmware exacto instalado. [PROB-0009](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0009_start_pulsador_momentaneo.md), [opciones y decisión DEC-0008](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0008_start_momentaneo_con_memoria.md).

En STM32 dev2, `Kart_ControlTick` copiaba directamente `HAL_GPIO_ReadPin(START_STOP...)` a `next.start`, después a bit 2 de 0x100. GPIO activo alto con pulldown (`Core/Src/main.c`, bloque START_STOP). ESP32 dev3 trata cualquier STOP válido como parada prioritaria (`Controller::receive`/`step`). **Soltar basta para cancelar la precarga incluso sin rebotes.** La prueba conjunta de funciones reales confirma ese defecto de la copia de trabajo; no identifica la causa física con certeza sin verificar lo instalado.

## Configuración y conservación

- PC Windows, rama `codex/robustez-v13`, HEAD inicial `4f7f1501a4c11dd52cbc74d649fbd2ede8b9bb09`; había cambios locales ajenos que se conservan. Sin nuevo commit/push.
- Fuente frontal previo `stm32_frontal-v1.3-dev2`; tras cambio `stm32_frontal-v1.3-dev3`. Núcleo trasero `esp32_trasero-v1.3-dev3` de ENS-0011, sin modificar en esta sesión. «B1.3» no prueba que esas versiones estén grabadas.
- [Copias anteriores](../evidencias/start_stop_2026-10-13/antes/) de fuente/header frontal, tests, comando de pruebas y build antes de editar. Originales históricos intactos.
- MSVC x64: funciones de frontal reales con HAL/SD simulados; integración enlaza el `kart_control.cpp` real y entrega a `Controller::receive` los ocho bytes emitidos por frontal. Sin bus físico, modelo dinámico, motores o tiempos medidos; reloj y entradas controlados, incluidos saltos temporales.
- ARM GNU 14.3.Rel1, build local existente: 151 unidades, configuración verificada, enlace con advertencias fatales y comprobación de `Reset_Handler`. No se ejecutó CubeMX/Designer.
- Sin conexión a placas, captura real, lectura de flash, programación ni cambio de red. Binarios resultantes locales en `07_DESARROLLO/respaldos/stm32/v1.3-dev3/`; no son respaldos leídos de placa ni una copia independiente.

## Cambio y método

Memoria de solicitud con START estable 40 ms, liberación estable 40 ms para habilitar siguiente pulsación, STOP al primer alto de la siguiente pulsación. Inicialización en OFF sin aceptar botón ya sostenido. Aritmética sin signo para desbordamiento de tick. Sin retardos bloqueantes; se conserva prioridad de STOP del ESP32. [Política y límites completos](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0008_start_momentaneo_con_memoria.md).

`build.py` obtiene ahora el nombre ELF/HEX de `KART_FRONTEND_VERSION`, evitando sobrescribir/rotular dev3 como dev2. Cambian únicamente fuente/header frontal, pruebas y nombre de salida; contrato CAN y estructura CSV/JSON intactos. La columna `Start` representa ahora la petición memorizada; el firmware en cada fila distingue la semántica.

Procedimiento: ejecutar 54 comprobaciones existentes; añadir 29 sobre pulsador e integración y ejecutarlas contra dev2; corregir fuente; repetir las 83; ejecutar las 56 del núcleo trasero; compilar ARM; conservar diff, fuentes, logs, trazas sintéticas, manifiestos y ELF/HEX.

Casos añadidos: botón sostenido al encender, liberación inicial insuficiente, umbral 39/40 ms, pulsación larga, soltar sin parar, rebotes de liberación y tras STOP, 100 pulsos de 5 ms, nuevo ciclo deliberado, STOP con ADC inválido, reset del estado, desbordamiento del reloj, conservación de modo, precarga/solapamiento/listo y parada, pérdida/recuperación CAN sin autoarranque. No constituyen verificación exhaustiva del hardware.

## Resultados y evidencia

| Verificación | Resultado | Datos |
|---|---|---|
| Frontal previo, pruebas previas | 54, 0 fallos | [frontend_base.log](../evidencias/start_stop_2026-10-13/frontend_base.log) |
| Nuevas pruebas contra frontal dev2 | 83, **19 fallos** | [frontend_regresion_antes.log](../evidencias/start_stop_2026-10-13/frontend_regresion_antes.log) |
| Frontal dev3 e integración con núcleo ESP32 | **83, 0 fallos** | [frontend_dev3.log](../evidencias/start_stop_2026-10-13/frontend_dev3.log) |
| Regresión del núcleo ESP32 intacto | **56, 0 fallos** | [control_dev3.log](../evidencias/start_stop_2026-10-13/control_dev3.log) |
| Compilación completa STM32 | **BUILD OK**, ELF + HEX | [build_stm32_dev3.log](../evidencias/start_stop_2026-10-13/build_stm32_dev3.log) |

`arm-none-eabi-size`: text 6 599 860 B, data 804 B, bss 1 406 280 B; totales del enlace con recursos externos, no ocupación exclusiva de flash/RAM interna. Advertencias de compilación en dependencias/generados (principalmente parámetros sin uso) y dos advertencias ya existentes en arnés MSVC (sombreado/conversión); no se afirma ausencia de advertencias.

[Trazas CSV](../evidencias/start_stop_2026-10-13/trazas_sinteticas.csv) derivadas de líneas TRACE originales de ambos logs; no mediciones. A tick sintético 1640 ms la precarga está activa. Al liberar en 1645 ms, dev2 transmite START=0 y relé=0; dev3 mantiene START=1 y relé=1. En dev3 llega a solapamiento/listo y la siguiente pulsación a 4700 ms genera START=0/relé=0. Estos tiempos son entradas del test, no latencias físicas.

[Diff de la sesión](../evidencias/start_stop_2026-10-13/cambio_start.diff), [manifiesto SHA-256](../evidencias/start_stop_2026-10-13/hashes.json), [manifiesto build](../evidencias/start_stop_2026-10-13/build_hashes.json). Se conservan copias después del cambio en `despues/`, además de las anteriores, para reproducir esta versión aunque siga el desarrollo.

Incidencias del procedimiento: primer intento de copia usó rutas relativas desde `tests` y falló antes de ejecutar pruebas; creó únicamente carpetas vacías. Se verificó su ruta y ausencia de archivos, se retiraron y se repitió con rutas absolutas antes de editar. Un comando posterior de empaquetado devolvió código 1 sin salida y no creó los destinos; se sustituyó por [script de conservación](../evidencias/start_stop_2026-10-13/conservar_evidencia.py), que rechaza sobrescribir evidencia distinta. No pérdida de fuentes ni resultado de ensayo atribuible a aquellos intentos. Los 19 fallos de regresión sí son defectos reproducidos respecto al comportamiento requerido.

## Validación física siguiente (pendiente)

1. Identificar versiones reales de ESP32 (cabecera/`fw` Wi-Fi) y STM32 (pantalla/SD o lectura); conservar flash interna + QSPI frontal y compararlas con recursos de trabajo antes de seleccionar programación. La corrección se carga en STM32, no en ESP32. No sobrescribir recursos externos sin esa identificación.
2. Preparar banco inmovilizado y sin posibilidad de propulsión; N, freno y pedal libre, sensores/quietud válidos según el firmware. Registrar versión/binario/hash, montaje, tensión y condiciones disponibles. No presentar esos valores como medidos antes de recibirlos.
3. Pulsar y soltar una vez: petición START permanece y la secuencia llega a listo. Mantener pulsado varios segundos: un único evento. Soltar y volver a pulsar: STOP, también durante precarga. Repetir ciclos y arranque con botón sostenido; no debe producir marcha al encender. Comprobar el estado real de los contactos junto a las órdenes.
4. Ensayo integrado por **Wi-Fi**, registrando `start_requested`, `relay1`, `relay2`, `relay_seq`, `drive_inhibit`, `can_rx_age_ms`, versión y motivo de reset. USB ESP32 únicamente fuera del kart. Registrar resultados favorables y fallidos, duración y número real de ciclos, sin darlos por hechos aquí.
5. La telemetría periódica de 200 ms puede omitir transitorios: para acreditar rebotes/latencia, medir GPIO/CAN/salida con instrumentación adecuada en banco. Si falla con START estable, contrastar alimentación/reset/CAN/contacto antes de ampliar filtros. No adoptar un tiempo RC ni sustituir pulsador sin revisar circuito.

La implementación está terminada y comprobada en PC. Pendientes la correspondencia con el firmware instalado, grabación y aceptación física; PROB-0009 permanece abierto. Cobertura documental: índice/ficha DOC-0042, notas anteriores y código de trabajo focalizado; no nuevas páginas originales ni cierre L1/L4.
