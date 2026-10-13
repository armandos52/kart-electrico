---
estado: prototipo_local_sin_prueba_fisica
actualizado: 2026-10-07
resumen: Aplicación del portátil con mapa animado, lector USB de solo lectura y registro; placas aún desconectadas.
---

# Arranque del panel del portátil · 07-10-2026

## Alcance indicado por Armando

- Aplaza iPad para una eventual presentación final; prioridad portátil por cable. Wi-Fi directo queda como posible etapa posterior.
- Solicita representación animada del kart y selección de partes para consultar datos de los micros.
- Declara disponer de osciloscopio y acceso a un laboratorio completo/sala de electrónica; resto del instrumental sin inventariar.
- Autoriza comenzar diseño y pruebas por cable. Aclara durante la sesión que **todavía no hay ninguna placa conectada**. No se hizo conexión física, captura USB del kart ni grabación de firmware.

## Aplicación creada

[Panel local](../07_DESARROLLO/panel_local/README.md), guardado en `07_DESARROLLO/panel_local/`:

- Servidor Python solo en bucle local, lector pySerial 3.5, inventario de puertos, sesión JSONL y API de lectura. Sin envío de bytes al micro ni rutas de motor/calibración. Sin Wi-Fi, acceso remoto o cambios de firewall.
- Interfaz local HTML/CSS/JavaScript: kart esquemático seleccionable, cuatro ruedas/dirección animadas cualitativamente, STM32, ESP32, Kelly, batería/BMS, IMU, CAN y relés. Señales por componente, edad/origen, traza de 30 s y mensajes.
- Arranque desconectado. Apertura de puerto solo con selección explícita y condición de banco preparado. DTR/RTS desactivados antes de apertura; el driver puede generar un pulso/reset.
- Texto legado del ESP32 conservado como mensajes. Variables solo con [telemetría versionada](../07_DESARROLLO/panel_local/PROTOCOLO.md), aún no instrumentada en los micros. «CAN OK» no se transforma en rpm. Cable dibujado no acredita continuidad eléctrica.
- Tramas parciales no refrescan señales ausentes. Pérdida de puerto/servicio retira valores vigentes y detiene animación derivada. Caducidades visuales provisionales independientes del control CAN.
- Demostración visual explícita en navegador, con valores sintéticos, separada del USB y sin guardarse como ensayo real. No replica/valida algoritmos de firmware. Respeta movimiento reducido.
- Dependencia instalada solo en `.vendor/`, ignorada por Git; README, arranque PowerShell, protocolo y pruebas guardados.

## Infraestructura observada

Inventario pySerial: **COM3 = Intel(R) Active Management Technology - SOL**, HWID PCI Intel. No es ESP32/STM32; no se abrió. No hay puerto del kart identificado.

El servidor inicial en sandbox no fue accesible a Chrome externo y se detuvo. Se arrancó el panel local fuera del entorno restringido, autorizado, en **http://127.0.0.1:8766/**, solo localhost y sin serie. El script usa 8765 por defecto para próximos arranques. El proceso puede terminar al cerrar la ejecución; usar `iniciar.ps1` para volver a arrancar. Apertura solicitada en el navegador de Codex: respuesta `queued`, no confirmación de que el usuario lo haya visto.

## Comprobaciones

- Siete pruebas Python pasadas: fragmentación/timeouts, líneas excesivas y recuperación, valores no finitos/booleanos/rangos, rechazo atómico, duplicados/orden/rollover, cambio de arranque, antigüedad por señal y registro con tick de origen. Puerto falso sin TX, sin hardware.
- Chrome sin ventana visible: arranque desconectado, selección gráfica, demo animada, separación demo/USB, retirada de valores de demo, datos ausentes/antiguos, pérdida del servicio, POST sin token bloqueado y ausencia de ruta de actuación. Fixture interceptada únicamente en navegador.
- Sin errores JavaScript ni desborde horizontal a 1280, 1024, 760 y 375 px. Captura revisada. Resultados/capturas en `07_DESARROLLO/panel_local/qa/`, generados e ignorados por Git.
- El MCP Chrome configurado anteriormente no estaba expuesto como herramienta directa en este turno. Se usó Playwright con Chrome instalado; no se modificó su configuración MCP.

Esto acredita prototipo/lector, **no** comunicaciones físicas, firmware compilado, valores del kart, sincronización de relojes, respuesta eléctrica o control motriz. No hubo commit/publicación; originales conservados.

## Próximo ensayo físico

1. Preparar alimentación y tracción inhabilitada en laboratorio; identificar ESP32 y cable de datos.
2. Conectar y actualizar inventario; comprobar nuevo puerto. Capturar mensajes sin regrabar primero. Registrar condiciones de USB por su posible efecto en los problemas de alimentación.
3. Preparar v0/dependencias/versiones según DEC-0001 e instrumentar primeras muestras ESP32: rpm, PWM ordenado, relés, tiempo CAN y versión/sesión.
4. Contrastar variables con laboratorio, incluyendo PWM después de la interfaz y tiempos. Ampliar hacia STM32/SD/BMS sin prometer señales que no se transmiten.
5. Elegir ajustes necesarios con evidencia y preparar validación/confirmación/persistencia. Regeneración pendiente de comprobar en Kelly y batería/BMS.

## Fuentes y cobertura

Estado/mapa/índice, inventario/notas y decisiones previas. Sin nueva lectura de originales o páginas PDF. Referencia documental 2025/26; montaje y firmware grabado pendientes.

Documentación primaria: [API pySerial 3.5](https://pyserial.readthedocs.io/en/latest/pyserial_api.html) y [puertos](https://pyserial.readthedocs.io/en/latest/tools.html#serial.tools.list_ports), consulta 07-10. Posibles pulsos DTR/RTS al abrir son condición a registrar, no causa demostrada de PROB-0002/0003.
