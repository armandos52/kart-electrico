---
estado: implementado y comprobado en PC; compilaciones conservadas, sin grabar
actualizado: 2026-10-15
resumen: Frontal dev4 corrige versión, estado del limitador y prioridad de avisos; interfaz final aplazada por Armando.
---

# ENS-0018 · Correcciones de la pantalla de desarrollo

## Objetivo y decisión de alcance

Armando autoriza resolver F01–F03 de [PROB-0011](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0011_presentacion_frontal_preinstalacion.md) y aclara que las pantallas actuales pertenecen a la fase de desarrollo. **Las pantallas de usuario final quedan pendientes para cuando termine el proyecto**, según [DEC-0010](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md). Se corrige el diagnóstico existente sin iniciar ese rediseño.

Configuración real de sesión: PC Windows, rama `main`, HEAD `43b0c299af47e30aabcf9b5acd7677b86754a00b`, con trabajo previo conservado. Frontal pasa de **v1.3-dev3 a v1.3-dev4**. ESP32 dev3, lógica de control/adquisición y formato CAN permanecen iguales. Banco F411 adapta sus consumidores del formateador a **v0.4-dev**, compilado sin grabar; última grabación conocida del banco sigue siendo v0.3. No se abre puerto real, no se conecta al kart, no se graba F746/ESP32/F411. Sin commit/push ni copia independiente nueva.

## Cambios aplicados

| Hallazgo | Implementación y significado |
|---|---|
| F01: versión visible distinta del firmware | [kart_display.c](../../07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_display.c) deriva `STM V1.3-DEV4` de `KART_FRONTEND_VERSION`, definido una sola vez en la cabecera. Conversión a mayúsculas para los glifos existentes. El CSV y `build.py` usan la misma definición para registros/nombres ELF y HEX |
| F02: umbral fijo atribuido a cualquier controlador | `R:--` si falta estado extendido compatible, caduca el estado o falta feedback reciente. Con estado válido: `R:LIM` cuando el bit `KART_STATUS_REVERSE_LIMIT` anuncia actuación, `R:INACT` cuando no la anuncia. El motivo 17 pasa a `LIMITE R ACTIVO`. La trama 0x124 no comunica el umbral numérico ni identifica el binario: no se deduce «20 km/h» de la mera recepción. INACT no significa protección deshabilitada; describe que no está actuando en ese estado recibido |
| F03: aviso de modo ocultaba una causa de inhibición | La segunda fila conserva siempre la causa. El aviso temporal `MODO: ...` aparece en la quinta fila junto a la versión. Mientras dura sustituye únicamente el contador de filas SD; el estado/error SD sigue en la cuarta. Al caducar reaparece el contador. Se mantiene la duración existente de 3 s y no cambia el control |

Ejemplo sintético ahora comprobado:

```text
CARGADOR CONECTADO
...
STM V1.3-DEV4 MODO: PONER N
```

Se conservan las cinco filas de 40 caracteres, posiciones y glifos del frontal F746. No se añaden pantallas para conducción ni ajustes de parámetros físicos. La compilación y las pruebas de texto no constituyen observación de legibilidad/táctil real.

El banco F411 incluye los tests/formateador del frontal directamente: se actualizan sus expectativas y se elimina la sustitución de la quinta fila por el viejo literal `CORE DEV3`, que habría ocultado el aviso nuevo. `BENCH_VERSION` identifica v0.4-dev en sus mensajes; la última fila muestra la versión del núcleo frontal. El capturador acepta únicamente las dos versiones conocidas 0.3 y 0.4-dev, comprueba identidad coherente durante la captura y registra la versión recibida. No se cambia navegación, protocolo de filas/CRC ni se capturan nuevas imágenes físicas.

## Procedimiento y resultados

| Comprobación | Resultado y evidencia |
|---|---|
| Regresiones nuevas ejecutadas antes de corregir fuente | **101 comprobaciones, 13 fallos**: los 83 casos previos más 18 nuevos; [frontend_antes.log](../evidencias/mejoras_frontal_2026-10-15/frontend_antes.log). Los fallos reproducen F01/F02/F03, no errores de compilación |
| Frontend dev4, MSVC x64, periféricos simulados | **101 comprobaciones, 0 fallos**; [frontend_dev4.log](../evidencias/mejoras_frontal_2026-10-15/frontend_dev4.log). Versionado, CAN legado/incompatible/caducado, actuación R, cargador/CAN con aviso simultáneo, caducidad de aviso, todos los motivos y anchura/glifos |
| Escenas del banco ejecutadas en PC con MSVC | **155 comprobaciones, 0 fallos**: las mismas 101 más 54 del banco; no sumar ambos como conjuntos independientes. [banco_host.log](../evidencias/mejoras_frontal_2026-10-15/banco_host.log) conserva también las diez escenas textuales. No se ejecutaron en F411 durante esta sesión |
| STM32F746 ARM GNU 14.3.Rel1/GCC 14.3.1 | Configuración verificada, 151 unidades, enlace y ELF/HEX correctos; [build_stm32_dev4.log](../evidencias/mejoras_frontal_2026-10-15/build_stm32_dev4.log). Sin upload |
| Nucleo F411, PlatformIO ststm32 20.0.0 y dependencias fijadas existentes | **SUCCESS**, BIN/ELF; [build_banco_v04_final.log](../evidencias/mejoras_frontal_2026-10-15/build_banco_v04_final.log). RAM estática reportada 2.232 B, flash 59.792 B; no medida de memoria máxima en ejecución |
| Capturador con transporte serie falso | **5 casos, 0 fallos**: versiones antigua/nueva, metadatos/240 filas/CRC, rechazo de desconocida y versiones mezcladas; [capture_compatibility.log](../evidencias/mejoras_frontal_2026-10-15/capture_compatibility.log). Los archivos en `captura_simulada/` contienen píxeles cero sintéticos; no son capturas del LCD |

Se conservaron advertencias preexistentes de dependencias/generados ARM y dos del arnés MSVC. El primer intento de PlatformIO falló por acceso restringido a `platforms.lock` en el perfil; [log negativo](../evidencias/mejoras_frontal_2026-10-15/build_banco_v04.log). La misma compilación, autorizada con los permisos necesarios para su caché/bloqueo, terminó correctamente; no hubo rechazo de revisión automática, actualización global de dependencias ni grabación.

El ensayo anterior [ENS-0017](ENS-0017_revision_preinstalacion_frontal.md) y sus resultados negativos permanecen como evidencia de dev3. Su arnés histórico comprobaba específicamente DEV3; la regresión permanente de esta sesión verifica correspondencia de versión, sin fijar ese número antiguo. Los resúmenes vigentes se actualizan a dev4; la documentación histórica se marca como antecedente.

## Conservación y siguiente paso

[Evidencia y reproducción](../evidencias/mejoras_frontal_2026-10-15/README.md), [diff](../evidencias/mejoras_frontal_2026-10-15/cambios.patch) y [manifiesto SHA-256](../evidencias/mejoras_frontal_2026-10-15/manifest.json): nueve archivos de alcance conservados antes/después (el nuevo test solo después), logs positivos/negativos, binarios y mapa. También se guardan hashes del frontend de control, núcleo ESP32 y contrato binario sin cambios.

- STM32 dev4 ELF: `5878e195c60631893ef0cbf6968a1f5ce02e50c093bb8c3b569d78238f20069a`.
- STM32 dev4 HEX: `b758edd7d09f8a651b5111b7854037b5869d3f5960e4e48fc6b8636c65f10316`.
- F411 v0.4-dev BIN: `2647a757e0b878465881f6929851e3f75249beccf5803891f8e6db32ab038a19`.

ELF/HEX frontales preservados en `07_DESARROLLO/respaldos/stm32/v1.3-dev4/`; dev3 y v0.3 permanecen conservados. No se identifica con ello lo instalado en las placas del kart.

**F01–F03 quedan corregidos y comprobados en software; validación física pendiente.** Siguiente: identificar y respaldar F746 interno+QSPI, seleccionar candidato y comprobar LCD/táctil/SD/CAN en banco. Ensayo integrado por Wi-Fi; USB ESP32 solo fuera del kart. BMS, franja blanca, calibraciones, tiempos físicos y ESC siguen pendientes de sus ensayos. La interfaz de usuario final queda en tareas de cierre del proyecto.

Consulta focalizada L4: notas/catálogo/ficha DOC-0042 ya conocidos, formateador/cabecera/tests, dependencias del banco y capturador. No nuevas páginas de originales ni lectura integral de bibliotecas; formatos visuales/Loop siguen abiertos.
