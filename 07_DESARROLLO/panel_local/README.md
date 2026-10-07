# Panel local del kart · 0.1

Aplicación para el portátil, con mapa interactivo del kart, señales por elemento, animaciones cualitativas, traza temporal y captura serie. Escucha únicamente en `127.0.0.1`; iPad y Wi-Fi aplazados por el usuario. No envía bytes al micro, no tiene comandos de motor/calibración y no modifica ni graba firmware.

## Arrancar

En PowerShell, desde esta carpeta:

```powershell
.\iniciar.ps1
```

Abrir `http://127.0.0.1:8765/` en el navegador. Se muestra sin conexión y no se abre automáticamente ningún puerto. El script emplea el Python disponible en este portátil; admite `-Python C:\ruta\python.exe` y `-Puerto 8766`.

La dependencia pySerial 3.5 se ha instalado localmente en `.vendor/` (fuera de Git). Para otro ordenador, instalarla con el Python elegido:

```powershell
python -m pip install --target .vendor -r requirements.txt
python server.py
```

No necesita servicios de IA ni Internet al funcionar. Interfaz, scripts y estilos son locales.

## Primera conexión física, pendiente

1. Identificar la placa y preparar alimentación/cableado en laboratorio con tracción inhabilitada; conservar condiciones de USB y alimentación para comparar los fallos existentes. Abrir un puerto puede producir un reset por DTR/RTS incluso desactivados según el driver; no se promete captura pasiva eléctricamente inocua.
2. Conectar la placa al portátil, actualizar puertos e identificar el que corresponde al USB. COM3 observado el 07-10-2026 es **Intel Active Management Technology SOL**, no el kart. No seleccionarlo como ESP32/STM32.
3. Confirmar la condición del banco en la interfaz y elegir **Leer por cable**. Configuración inicial ESP32: 115200 bit/s, 8N1, DTR/RTS desactivados; estas condiciones vienen del fuente 2025/26 y de la propuesta, no de una captura actual.
4. El código ESP32 entregado imprime mensajes de diagnóstico. Se conservan en consola y JSONL; no se convierten en rpm/PWM inventados. Las variables gráficas necesitan tramas de telemetría compatibles con [PROTOCOLO.md](PROTOCOLO.md), todavía no añadidas al firmware del kart. STM32 necesita preparar su canal de diagnóstico.
5. Desconectar desde el panel y conservar la sesión. `sesiones/*.jsonl` contiene eventos y tramas válidas, con tiempo de recepción del PC y tick de origen cuando existe. No equivale a sincronización entre relojes ni captura de flancos físicos.

El panel permite revisar qué datos reportan los micros. Formas de onda, tensiones, ruido y fallos eléctricos puntuales se comprueban con el osciloscopio disponible en el laboratorio. La disposición gráfica es esquemática; cableado y posición de componentes pendientes de confirmar físicamente.

## Demostración visual

**Vista animada de ejemplo** genera valores sintéticos únicamente en el navegador para revisar diseño e interacción. Se rotula DEMOSTRACIÓN y bloquea conexión USB. No se guarda como sesión real. Al salir, las señales sin datos reales vuelven a desconocidas. El movimiento se ralentiza para legibilidad y respeta la preferencia de movimiento reducido.

## Comprobación técnica

```powershell
python -m unittest discover -s tests -v
```

Los tests usan datos y puertos falsos; no abren placas. Las pruebas web están en `qa/check-panel.cjs` y requieren el Playwright del entorno de trabajo. No sustituyen las pruebas USB/firmware. Resultados y cobertura en [sesión del proyecto](../../06_TRABAJO/ARRANQUE_PANEL_PORTATIL_2026-10-07.md).

## Próximo desarrollo

Preparar v0 y dependencias del ESP32/STM32 según DEC-0001; añadir telemetría por pasos sin bloquear el control. Verificar variables, unidades, tiempos y origen en el laboratorio. Después elegir un parámetro realmente útil y preparar validación/confirmación/persistencia. Regeneración requiere comprobar Kelly y batería/BMS. No hay fecha ni función de regeneración implementada en este panel.
