---
estado: preparado_para_otra_sesion
actualizado: 2026-10-07
resumen: Prompt de continuidad: respaldo físico verificado del ESP32 antes de cambios, VS Code y monitorización por Wi-Fi directo.
---

# Prompt para continuar con el ESP32 en otra sesión

Preparado por solicitud de Armando el 07-10-2026. No se conecta, respalda, modifica ni graba ninguna placa durante esta preparación. VS Code está localizado mediante `code.cmd`; no se han comprobado sus extensiones. El puerto actual del ESP32 aún no está identificado. Esta solicitud prioriza Wi-Fi para monitorización después del respaldo; el 3D permanece aplazado.

Copiar el siguiente bloque en una sesión con acceso a `C:\kart proyecto`:

```text
Continúa mi proyecto en C:\kart proyecto, en español. Quiero trabajar con mi ESP32 en Visual Studio Code y añadir monitorización por Wi-Fi directo para evitar depender del USB durante las pruebas. Antes de tocar el código o grabar nada, quiero un respaldo real y verificado del firmware que está actualmente en la placa. Trabaja por etapas y muéstrame los archivos, diferencias y resultados.

1. Recupera el contexto sin repetir todo el análisis. Lee AGENTS.md, 00_INICIO/ESTADO_ACTUAL.md, las decisiones DEC-0001 y DEC-0002, 04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md y 07_DESARROLLO/panel_local/README.md y PROTOCOLO.md. Usa el mapa y catálogo solo para localizar lo necesario. El fuente de referencia es:
02_ORIGINALES/entrega_2026-09-28/Smart Eletric Kart Cross-2223406-2223403/Smart Eletric Kart Cross-2223406-2223403/ESP32-final/ESP32-final/ESP32-final.ino
No modifiques originales ni asumas que ese fuente coincide con lo grabado. Ya existe un panel Python/HTML/JavaScript de lectura; reutilízalo. El último arranque fue localhost:8766: comprueba si sigue activo. Su demo es sintética y los micros todavía no tienen el emisor de telemetría del panel. No rehagas el panel ni hagas ahora el 3D.

2. Empieza por el respaldo físico. Tengo el ESP32 disponible y laboratorio/osciloscopio. Identifica placa, puerto USB, chip y tamaño real de flash. COM3 observado anteriormente era Intel AMT SOL, no el kart. Coordina conmigo la preparación del banco, alimentación y tracción inhabilitada antes de abrir el puerto: lectura/bootloader pueden resetear la placa. Cierra otros monitores que ocupen el puerto. Revisa versiones y ayuda de las herramientas; no uses tamaños, pines o puertos supuestos.

Con esptool, lee TODA la flash desde dirección 0 sin borrarla ni regrabarla. Guarda el respaldo original en 07_DESARROLLO/respaldos/esp32/<fecha-hora>/ junto a tamaño, SHA-256, identificación de placa/chip, versión de herramienta, comandos y logs. Verifica con una segunda lectura, manteniendo la aplicación detenida para evitar escrituras entre ambas; compara tamaño y hash. Si hay diferencias, investiga y no declares el respaldo verificado. Comprueba el contenido/particiones cuando sea posible. Conserva una segunda copia, preferiblemente en otro soporte cuando tengamos destino disponible; no inventes que existe una copia independiente. Documenta cómo restaurarlo a ESTA placa, sin restaurar ahora ni prometer una recuperación ensayada. El binario no recupera el fuente editable ni respalda eFuses/RAM. Comprueba restricciones de lectura/cifrado sin cambiar eFuses ni forzar protecciones. Si no se logra un respaldo fiable, no modifiques código ni flash y explica el bloqueo.

3. Solo después del respaldo verificado, conserva la referencia del fuente y crea/reutiliza 07_DESARROLLO/firmware/esp32_trasero/ según DEC-0001, con huellas, historial y VERSIONES.md. Identifica el repositorio existente; conserva cambios ajenos y evita Git anidado. Distingue v0 de la entrega de la imagen realmente extraída. Abre el proyecto de trabajo en VS Code y explícame sus partes. El código usa ledcAttach y ledcWrite por PIN, de Arduino-ESP32 3.x; verifica un entorno compatible. Prefiero PlatformIO si lo admite; si no, usa VS Code con Arduino CLI compatible sin cambiar la lógica PWM para adaptarla a un core antiguo. Fija versiones de core, herramientas y bibliotecas MCP_CAN/MPU6050. Compila la referencia antes de añadir funciones; no sustituyas el programa del kart por un ejemplo Wi-Fi vacío.

4. Añade monitorización de SOLO LECTURA en una versión nueva: ESP32 como punto de acceso Wi-Fi protegido y portátil conectado directamente, sin router ni nube. Publica la IP real y prepara salida TCP de diagnósticos/telemetría para verla en el terminal de VS Code; si usamos PlatformIO, configura su monitor socket://IP:PUERTO. Adapta el transporte del panel existente al mismo flujo JSON versionado. No expongas el servidor local del portátil a toda la red sin necesidad. No pongas contraseñas en Git o logs.

Reutiliza PROTOCOLO.md: versión, nodo, sesión de arranque, secuencia, tick y señales parciales, con unidades/origen/caducidad. Publica solo variables que existan: mandos recibidos por CAN, modo, PWM ordenado, RPM traseras, guiñada y estados de relés/diagnóstico. Para datos del STM32/BMS explica qué retorno falta. No inventes tensión, corriente, ángulos físicos o datos ausentes. Mantén USB como recuperación y diagnóstico. Diferencia monitorización Wi-Fi de actualización OTA y depuración con breakpoints; OTA queda para después. Antes de cambiar la conexión Wi-Fi del portátil, comprueba cómo conservar Internet para la asistencia si lo necesitamos: la red directa del ESP32 por sí sola no aporta Internet; no cambies redes ni compres adaptadores sin coordinarlo conmigo.

Conserva CAN, pines, PWM, diferencial y temporización. La telemetría debe tener buffers limitados y no bloquear por clientes lentos, ausencia de Wi-Fi o desconexiones. Mide su efecto sobre el control; el timeout CAN del fuente es 100 ms, no un margen libre para tareas de red. No añadas control remoto de motores, calibración, regeneración ni cambios del ESC en esta fase. Si detectas errores previos, regístralos separadamente.

5. Entrega código completo guardado y abierto en VS Code, configuración reproducible, comandos de compilación/monitorización y diferencias explicadas. Después del respaldo y de las comprobaciones, coordina la grabación por cable y el ensayo físico conmigo. Comprueba datos reales y pérdida/reconexión Wi-Fi; perder observación no demuestra que se haya parado el kart. Registra versión grabada, ensayo, resultados y límites; actualiza estado/tareas. No declares que funciona en la placa solo porque compila.

Prioridad futura del panel: 3D sencillo basado en fotos reales, matemática del diferencial, datos y movimiento por encima de acabados visuales. Ahora céntrate en respaldo, entorno y monitorización real. Solicita solo información que falte para avanzar y continúa con las tareas independientes.
```

## Referencias verificadas al preparar este prompt

- [Espressif: read-flash](https://docs.espressif.com/projects/esptool/en/latest/esp32/esptool/basic-commands.html#read-flash-contents-read-flash): dirección, longitud y fichero; admite `ALL`. Elegir sintaxis conforme a la versión instalada, no ejecutar todavía. Respaldo verificado por lecturas no equivale a restauración ensayada.
- [Espressif: SoftAP](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html): el ESP32 puede crear el punto de acceso y publicar su IP.
- [PlatformIO: monitor TCP](https://docs.platformio.org/en/latest/core/userguide/device/cmd_monitor.html): soporta `socket://host:port`. Es un cliente; se necesita implementar antes el emisor TCP en el ESP32.
- [Espressif: migración 2.x a 3.0](https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html): `ledcAttach` nuevo y API por pin. Fuente entregado líneas 279–286: usa esa API. No se ha establecido aún una versión concreta de PlatformIO/core ni compilado.

Cobertura de esta preparación: notas, ficha DOC-0032 para identificación, fuente ESP32 líneas 1–60 y búsquedas focalizadas de Wi-Fi/LEDC/Serial; documentación oficial. No hubo lectura integral, copia binaria, instalación, conexión ni ensayo físico.
