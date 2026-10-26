---
estado: dev8-ui instalada, relectura coincidente y BMS visible en captura real
actualizado: 2026-10-16
resumen: SOC y temperatura CAN visibles como interpretación provisional en Resumen/BMS; preferencia de tema oscuro.
---

# ENS-0024 · Batería visible en la pantalla de diagnóstico · 16-10-2026

## Objetivo y motivo

Después de instalar dev7-ui en [ENS-0023](ENS-0023_bugs_e_instalacion_dashboard.md), Armando responde: **«esta perfecto me quedo con e formato en oscuro el blanco no se ve bien todo funciona bien , la parte de la bateria porcentage no se ve y tampoco la temperatura»**. Aceptación manual del diseño/táctil y preferencia de oscuro; no es una medición de contraste ni de latencia. Se conserva el claro opcional solicitado inicialmente y el oscuro predeterminado. Se atiende la falta de datos BMS sin declarar validado un protocolo que continúa pendiente.

La recepción real de dev7-ui ya contenía 0x355 y 0x356, con edad inferior a 2 s; el programa los ocultaba porque `BMS_FORMAT_CONFIRMED=0`. No era pérdida de comunicación. Ejemplo de [observación 3](../evidencias/frontal_bugs_2026-10-16/observacion_3/observacion.json): `570064002e220000` y `8c14faffdc000000`. La interpretación histórica de bytes 4–5, little-endian, da 8750/100 = **87 % entero** y 220/10 = **22 °C**. El redondeo entero se conserva. Son candidatos decodificados, no SOC calibrado ni temperatura contrastada con instrumento/app.

Consulta focalizada de catálogo, ficha DOC-0035 y [inventario](../../04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md), puntos 19/23 y preguntas pendientes: manual DALY, modelo/configuración exactos sin identificar y comentario del código histórico tipo Pylontech. Buscador recupera referencias a tablas de 0x355/0x356 en DOC-0042 PDF 65 y recepción en PDF 68; no se revisa la tabla visual ni se acredita protocolo a partir del título. No nuevas páginas originales leídas integralmente.

## Configuración y cambio

**`stm32_frontal-v1.3-dev8-ui`**, sobre dev7-ui instalada, mismo HEAD base `bebe4528747bc6e05dc1ba244cd3ce9a090a7942` y herramientas de ENS-0023. F746 UID/ST-LINK iguales; kart encendido con ruedas levantadas según declaración del usuario. ESP32 sin modificar/identificar, F411 sin nueva grabación ni nuevo build en esta continuación. [Evidencias](../evidencias/bms_dashboard_2026-10-16/README.md).

- Resumen muestra `Bat* 87% 22°C` y `*BMS por validar`; BMS muestra `SOC* 87 %`, `22 °C*`, edades/bytes originales y `* Lectura provisional; formato por confirmar`.
- Cada señal se admite solo con edad ≤ **2000 ms**, constante compartida con frontend. Ausente/caducada o fuera de rango: `--`, sin conservar números antiguos. SOC y temperatura caducan independientemente.
- Antes de dividir se exige entero SOC 0–10000 y temperatura con signo −500–1500 décimas de °C. Son límites de plausibilidad heredados, **no límites de operación de la batería ni especificaciones del fabricante**. 10001/1501/−501 se rechazan antes de truncar.
- Los flags de validación siguen en cero y no cambia ningún permiso, orden CAN, calibración, protección, esquema CSV ni snapshot (116 B). No se publican valores provisionales como confirmados. Una futura señal confirmada conserva su presentación sin asterisco y también requiere frescura/rango.

Cambios limitados a renderer, nombre de versión, constante de edad compartida/comentario y pruebas. Recursos QSPI idénticos a dev7-ui, comprobados byte a byte al preparar. Sin cambios en paletas ni eliminaciones de la opción clara. No se añaden tensión, corriente o celdas inferidas.

## Verificación local y paquete

**163 comprobaciones de dashboard / 0 fallos**, incluidas 18 nuevas: recepción sin confirmación, ambas páginas/temas, caducidad exacta 2000/2001 ms, pérdida independiente, cero legítimo, valores fuera de rango antes de división, signo, extremos, ausencia y flags que no reviven datos caducados. **150 frontales / 0 fallos**. 124 renders sintéticos, cuatro nuevos basados en bytes capturados; inspección visual de Resumen y BMS oscuros. Estos renders no son capturas del LCD ni medidas de batería.

Build F746 **OK**, interna ocupada 349.644 B y QSPI 6.351.932 B. HEX SHA-256 `98b89e6db204712845bfb16550ad94a1bc4984aa9a1fe692eb7ace10db872a08`; ELF `ec8b3b9dd74297e53f2aa9b7a0c739b46de91ccad94e2b5997708d4e8a892c0d`. Logs, antes/después, diff, ELF/HEX/mapa y hashes conservados. La compilación conserva avisos de dependencias; no errores finales. No nuevo commit/push ni copia externa.

## Instalación y comprobación física

Se conserva dev7-ui completa y su relectura independiente de ENS-0023 (0 diferencias), además de respaldo doble completo dev4 y respaldo histórico. Se usa el mismo procedimiento chip/UID/RDP, programación HEX completo interna + QSPI, verificación y reset run. No se repite un respaldo completo de QSPI entre las dos grabaciones de esta misma sesión; ambas imágenes verificadas quedan disponibles para recuperación.

**Programación 13:40:00–13:41:19, hora local Europe/Lisbon: Verified OK y reset run.** [Log](../evidencias/bms_dashboard_2026-10-16/grabacion.log). Relectura independiente con CPU en ejecución: interna **349.644 B** y QSPI **6.351.932 B**, **0 diferencias** frente al HEX; [resultado y SHA-256](../evidencias/bms_dashboard_2026-10-16/relectura/verificacion.json).

**Captura real de página BMS, tema oscuro, 13:41:57:** [pantalla](../evidencias/bms_dashboard_2026-10-16/arranque_real/pantalla.png), [snapshot y registros](../evidencias/bms_dashboard_2026-10-16/arranque_real/observacion.json). Se leen `SOC* 87 %`, `22 °C*` y aviso provisional. Snapshot CAN `5700640024220000`/`8c140000dc000000`, edades 449/448 ms, flags de validación 0. Edad dibujada 334/333 ms: framebuffer y snapshot se leen secuencialmente mientras se actualizan, no son una captura simultánea. Cortes finos en el volcado de framebuffer no sincronizado conservados, sin nueva declaración de fallo visual del operador.

Arranque: tick 39.982 ms; avanza 508 ms en la espera nominal de 500 ms del observador. Errores CAN TX/RX/ADC 0; 0x123 presente, 0x124 nunca. N, START OFF, selector D, pedal 150 físico/61 normalizado y libre 0; posición del pie no confirmada, no calibración. SD en pausa, sin prueba de escritura. El sistema recibe datos y pinta el BMS; no se acredita relé ni giro de motores con estas muestras.

Resultado: SOC/temperatura visibles y caducidad/límites comprobados en PC. No equivale a validación metrológica o de protocolo BMS; ese contraste sigue pendiente. PROB-0012/0013 continúan abiertos. La preferencia oscuro/valoración del claro pertenece al operador. ESP32 y ensayos integrados se diagnostican por Wi-Fi; no se requiere USB del módulo instalado.
