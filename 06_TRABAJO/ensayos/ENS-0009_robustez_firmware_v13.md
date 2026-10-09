---
estado: código y compilaciones completados; sin grabar ni validar físicamente
actualizado: 2026-10-09
resumen: ESP32/STM32 dev2 compilados y comprobados en PC; estados, marcha atrás, pantalla, SD y panel implementados; prueba real pendiente.
---

# ENS-0009 · Implementación y comprobación de robustez v1.3

## Objetivo y autorización

Implementar todas las propuestas A01–A12 de [PROB-0008](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0008_robustez_control_modos_y_pantalla.md), por etapas, con marcha atrás limitada a 20 km/h según petición de Armando. [Seguimiento completo](../IMPLEMENTACION_ROBUSTEZ_V13.md). La declaración de hasta 50 km/h en R es del usuario; no se ha recibido medición. Se omite pre-simulación dinámica a petición del usuario; las comprobaciones aquí son unitarias de software.

## Configuración real de esta sesión

Solo PC Windows. Sin conexión a placas, sin puerto USB abierto, sin Wi-Fi del kart, sin grabación ni prueba física. ESP32 fuente de referencia `905b09e`, core Arduino 3.3.11/pioarduino 55.03.311 y bibliotecas existentes. Rama local `codex/robustez-v13`. Copia inicial frontal commit `2deb9ef`: **1673 archivos, 461160734 bytes**, copiados y verificados SHA-256 individualmente desde STM32 2025/26; [manifiesto](../../07_DESARROLLO/firmware/stm32_frontal/ORIGEN_V0.json). No acredita identidad con lo instalado.

## Etapa dev1 · ESP32

- Se separa lógica de permisos/control de los periféricos: `lib/kart_control`, `src/main.cpp`, `include/hardware_kart.h`. El `.ino` anterior queda recuperable en Git `905b09e` y en la evidencia original de auditoría; no se toca la entrega histórica.
- STOP por nivel persistente; corte coherente de PWM y relés por cargador; tracción solo tras completar 2 s de precarga + 1 s de solapamiento. Arranque exige observar START bajo antes de alto, N, freno, pedal libre y quietud estimada. Datos inválidos/timeout y doble pedal exigen rearme con N, freno y acelerador liberado.
- Rampa real en cada salida, reducciones inmediatas. En R techo de mando basado en 20 km/h y gobernador por la rueda más rápida: reduce de 18 a 20 km/h, corta a 20 y permite volver a acelerar por debajo de 19 con rampa. Todas son estimaciones por Hall; no frena pendiente/inercia.
- ESC usa yaw firmado y velocidad medida, incluye signo de R y corrección acotada. Gyro leído comprobando longitud I2C, tiempos limitados, calibración y caducidad. Ejes, ganancia y respuesta física todavía no validados.
- Hall calculado con intervalo transcurrido; calidad temporal y ausencia de pulsos bajo mando sostenido. PPR 89/68 y diámetro 0,48 m son parámetros históricos por contrastar.
- CAN drenado con presupuesto, contenido validado, contadores/recuperación y estado adicional 0x124. Extensión frontal 0x101 supervisada cuando está presente; frontal antiguo mantiene compatibilidad, **sin poder diagnosticar su ADC físico**.
- Watchdog de tarea explícito de 2 s, salidas de reposo antes de CAN y fallo de cola de telemetría protegido. No se afirma tiempo físico de parada ni inmunidad a un fallo de hardware.

## Procedimiento y resultados

1. Compilación de `kart_control.cpp` y tests dirigidos con MSVC x64, sin periféricos ni modelo de vehículo: [log](datos/ENS-0009/control_intento1.log), **46 comprobaciones, 0 fallos**. Incluye rechazo de mandos, STOP antes de 500 ms, recuperación CAN, freno, rampa, R en los tres modos, simetría ESC, Hall/MPU y datos inválidos.
2. Compilación real ESP32 con `pio run -e usb`, sin `upload`: [primera compilación](datos/ENS-0009/compilacion_esp32_intento1.log) correcta. Hubo cambios de fuente durante esa compilación, por lo que se repitió tras terminar para obtener [dev1 final](datos/ENS-0009/compilacion_esp32_dev1.log), también correcta. La compilación no prueba electrónica ni ejecución real.
3. Binario y ELF dev1 conservados localmente, excluidos de Git, en `07_DESARROLLO/respaldos/esp32/v1.3-dev1/`; [hashes de fuentes/binarios](datos/ENS-0009/hashes_dev1.json). No publicar binarios con credenciales Wi-Fi embebidas. V1.2 y respaldo inicial conservados aparte.

Problemas del procedimiento: el entorno restringido impedía escribir el lock/cache de PlatformIO y la rama Git; las mismas acciones autorizadas pasaron con permisos de ejecución necesarios. No hubo rechazo de revisión automática ni cambios de permisos permanentes. La consulta DNS de descarga ARM tampoco funciona en el entorno restringido; preparación del compilador frontal en curso.

## Situación al cerrar dev1 (histórica)

Al cerrar dev1 no se presentó como versión lista para grabar: faltaba revisión integrada, frontera de fallos hardware, frontend/SD/GUI, panel, contrato documentado y validación final de código. El código nuevo tiene umbrales de desarrollo, claramente separados de mediciones. Ausencia de pulsos no demuestra sensor conectado; los Hall no miden sentido. La prueba real posterior debe registrar versión/condiciones por Wi-Fi; USB solo con ESP32 fuera del kart. Continuar según la lista completa, sin dar por terminado el objetivo con esta primera etapa.

## Etapa dev2 · frontal, contrato, panel y revisión final · 09-10-2026

Se completa la lista A01–A12 y R20 en el alcance posible sin placas. [DEC-0006](../../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0006_robustez_v13_y_limite_marcha_atras.md) registra políticas; [contrato CAN](../../07_DESARROLLO/firmware/common/PROTOCOLO_V13.md) y [README frontal](../../07_DESARROLLO/firmware/stm32_frontal/README_ROBUSTEZ.md) detallan módulos, parámetros y reproducción.

- STOP conserva prioridad incluso ante fallo ADC frontal. D→R solicitado en movimiento fuerza N; no continúa mandando D. Se añaden regresiones de ambos casos, BLQ, cambio de modo y ESC en R. Verificación de PWM de parada también puede declarar salida fallida; estados se actualizan tras error de escritura.
- ADC Config/Start/Poll/Stop comprobados y valores físicos publicados por 0x101, secuencia/frescura supervisadas. CAN valida DLC, tipo y contenido; 0x124 confirma recibido/seleccionado/aplicado e inhibición. Se conserva compatibilidad antigua con limitaciones explícitas.
- STM32 separa control, SD y GUI. Copia de estado coherente por sección crítica breve; control de mayor prioridad que GUI; SD exclusiva de su tarea. Sin llamadas GUI desde control ni escritura/cierre de tarjeta desde botones. Filas CSV incluyen firmware, unidades, valores originales, validez y edades BMS; pausa/reapertura/escritura parcial/sync/cierre/reintento tratados.
- Hall frontal usa EXTI y tiempo real DWT, con edad independiente en HAL tick para no revivir una lectura al desbordar el contador. Referencia 4 pulsos/vuelta histórica; frontal no gobierna tracción. IWDG y hooks de pila/malloc añadidos; no equivalen a validación temporal real.
- UI muestra modo solicitado/recibido/aplicado y causa legible. CAN/MPU/BMS sin dato válido no se presentan como mediciones nuevas. Estado y error SD visibles. BMS real sin escala confirmada: conservar bytes y ocultar SOC/temperatura; no inventar voltaje/corriente.
- Panel conserva v1.2, acepta nuevos motivos y 15 señales, incorpora espacio «Control v1.3», interpreta modo aplicado y marca lecturas inválidas/caducadas. Calibración visual del PC rechaza diagnóstico frontal inválido. No configura control ni envía órdenes.
- Fuente v1.2 de auditoría fijado a cinco copias de Git con hashes idénticos al análisis original, evitando que una referencia histórica pase a apuntar a código modificado. Entrega STM32 original nuevamente verificada completa.

### Resultados de software (sin hardware ni modelo dinámico)

| Comprobación | Resultado | Evidencia |
|---|---|---|
| Núcleo ESP32, MSVC x64 | 51 comprobaciones, 0 fallos | [control_dev2.log](datos/ENS-0009/control_dev2.log) |
| Funciones frontales ADC/CAN/SD/textos, MSVC con respuestas de periféricos controladas | 54 comprobaciones, 0 fallos | [frontend_dev2_final.log](datos/ENS-0009/frontend_dev2_final.log) |
| Panel Python, lector/calibración/contrato, incluida conexión TCP loopback | 26 tests, OK | [panel_dev2_completo.log](datos/ENS-0009/panel_dev2_completo.log) |
| Presentación JavaScript | 16 comprobaciones, 0 fallos | [panel_js_dev2.log](datos/ENS-0009/panel_js_dev2.log) |
| ESP32 pioarduino 55.03.311/core 3.3.11 | Compilación final SUCCESS, RAM 49244 B, flash 949743 B | [compilacion_esp32_dev2_final.log](datos/ENS-0009/compilacion_esp32_dev2_final.log) |
| STM32 ARM GNU 14.3.Rel1/GCC 14.3.1, HAL 1.3.3 y dependencias originales | Compilación/enlace final correctos, Reset_Handler presente; ELF/HEX con recursos QSPI | [compilacion_stm32_dev2_entrega.log](datos/ENS-0009/compilacion_stm32_dev2_entrega.log), [secciones](datos/ENS-0009/secciones_stm32_dev2.txt) |
| Preservación de originales STM32 | 1673 hashes coincidentes | [originales_verificados.json](datos/ENS-0009/originales_verificados.json) |

Totales de secciones STM32: text 6599748 B, data 804 B, bss 1406272 B; incluyen memoria externa y framebuffer, no equivalen a ocupación de flash/RAM internas. Ver mapa y listado de secciones. Compilación no demuestra carga de CPU, jitter, latencia de parada, consumo, aislamiento eléctrico ni funcionamiento en vehículo. No se ejecutó ni presentó simulación dinámica. La pantalla física/táctil y la nueva interfaz no se han observado en la placa.

### Resultados negativos y límites del procedimiento

1. Primer intento frontal: faltaban header/fuente IWDG en la copia histórica; añadidos solo esos archivos desde HAL oficial v1.3.3 con licencia, manteniendo versión base.
2. Segundo intento enlazaba con advertencia de entrada ausente: el script omitía `S_SRCS`. **Ese binario se descartó como entregable.** Se incorporó startup, advertencias de enlace fatales y comprobación de Reset_Handler. Tercer intento detectaba erróneamente como ausente un símbolo débil `W`: corregido el verificador para aceptar `T/W`. Los intentos fallidos quedan en los logs `compilacion_stm32_intento1/2/3.log`.
3. Se repitió la compilación tras las últimas correcciones de PWM y etiqueta KM/H; solo se conservan como entrega los hashes finales. Los logs previos distinguen esos puntos intermedios.
4. Tests Python: primer intento impedía limpiar temporales del entorno restringido; se movieron a carpeta de evidencia. Después la prueba socket loopback fue bloqueada por el entorno. Ejecución autorizada con permisos necesarios completó los 26 tests. No hubo conexión al kart ni rechazo de revisión automática. Un primer comando de redirección de tests frontales calculó un directorio inexistente y no ejecutó los tests; se corrigió a ruta absoluta.
5. Persisten advertencias de compilación: operaciones sobre contadores `volatile` ESP32 bajo sección crítica (deprecación del dialecto C++), parámetros no usados en dependencias/generados STM32, dos advertencias del arnés MSVC frontal. No se ha llamado «compilación sin advertencias». Enlace STM32 sin advertencias, por requisito fatal.
6. CubeMX/Designer no se regeneraron. Lógica fuera de generados, enganches USER CODE, .ioc y comprobador precompilación preparados; la regeneración deberá revisarse en copia. La herramienta ARM portable queda fijada por hash, sin instalación global.

### Versiones, conservación y trazabilidad

Commit local de firmware y evidencia: `8a60a6ecafe9370426286752f0471a080705573a`. El panel conserva sus cambios locales previos, con snapshot privado y hashes; no se incluyó ese conjunto previo en el commit.

[Manifiesto dev2](datos/ENS-0009/hashes_dev2.json) identifica fuentes de trabajo y binarios, sin copiar `secretos.h`. [Manifiesto de compilación STM32](datos/ENS-0009/hashes_compilacion_stm32_dev2.json) incluye todas las unidades y digest de cabeceras. Resguardos locales en `07_DESARROLLO/respaldos/esp32/v1.3-dev2/` (firmware/ELF/bootloader/particiones) y `07_DESARROLLO/respaldos/stm32/v1.3-dev2/` (ELF/HEX), excluidos de Git. No son respaldo leído de placas. El binario ESP32 puede incorporar la clave Wi-Fi: no publicarlo. V1.2 y dev1 permanecen aparte. No push ni segunda copia en otro soporte.

El panel se desarrolla sobre cambios locales previos de visualización/calibración: se conservan, no se barren para fabricar un árbol limpio. Código del panel conservado además en `07_DESARROLLO/respaldos/panel/v1.3-dev2/codigo_panel.zip` (sin sesiones/datos privados, solo código y dependencias estáticas). Sus archivos afectados y el ZIP están identificados por SHA-256 en el manifiesto; el commit de firmware no pretende incorporar silenciosamente todo ese trabajo previo.

### Siguiente ensayo físico (pendiente)

1. Identificar y respaldar STM32 y QSPI antes de adoptar el nuevo frontal; comparar con el fuente/ELF histórico. Mantener recuperación ESP32 v1.2 y original disponible. La revisión del código histórico no prueba lo que está instalado.
2. En banco con tracción preparada para ensayo: comprobar GPIO/polaridad de freno/START/cargador, ADC crudos y calibración, Hall/PPR/ruedas, escala/ejes/signo MPU y ausencia de conflicto 0x101/0x124. USB solo con ESP32 fuera del kart. No probar fallos durante conducción.
3. Con instrumentos, medir precarga/relés/PWM, tiempo real de timeout/STOP, watchdog, ausencia de autoarranque y efectos de energía residual. Probar CAN/sensor/tarjeta ausentes y vuelta deliberada a servicio. Registrar tanto fallos como resultados positivos.
4. Comparar DIF/BLQ y rampa; evaluar R inicialmente a baja velocidad y registrar por Wi-Fi velocidad independiente y Hall. El límite de 20 km/h corta propulsión estimada, no frena pendiente/inercia. Confirmar el umbral físico antes de dar por cumplida la velocidad máxima del vehículo.
5. ESC sigue experimental: revisar yaw firmado en D/R y giros espejo, ganancias y respuesta instrumentada antes de atribuir mejora de estabilidad. Verificar legibilidad/táctil/SD y protocolo BMS real. Registrar versión/hash/condiciones/datos originales/resultados/decisión en nuevo ensayo.

Se completa la implementación posible en código; PROB-0008 permanece abierto a validación física y correspondencia del frontal. No se afirma resolución de la franja blanca, calidad eléctrica, regresión v1.1 ni calibración real de dirección.
