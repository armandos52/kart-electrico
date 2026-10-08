---
estado: implementación por etapas en curso; sin grabar
actualizado: 2026-10-08
resumen: Control ESP32 dev1 compilado, 46 comprobaciones dirigidas y copia STM32 v0 verificada; trabajo completo aún en curso.
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

## Limitaciones y siguiente paso

No se presenta dev1 como versión lista para grabar: falta revisión integrada, frontera de fallos hardware, frontend/SD/GUI, panel, contrato documentado y validación final de código. El código nuevo tiene umbrales de desarrollo, claramente separados de mediciones. Ausencia de pulsos no demuestra sensor conectado; los Hall no miden sentido. La prueba real posterior debe registrar versión/condiciones por Wi-Fi; USB solo con ESP32 fuera del kart. Continuar según la lista completa, sin dar por terminado el objetivo con esta primera etapa.
