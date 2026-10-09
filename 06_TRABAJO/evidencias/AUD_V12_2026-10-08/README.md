# Evidencia de revisión de v1.2 · 08-10-2026

Objetivo: comprobar rutas de código antes de proponer modificaciones. Resultado explicado en [la revisión](../../REVISION_CODIGO_V12_2026-10-08.md); problema [PROB-0008](../../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0008_robustez_control_modos_y_pantalla.md).

## Procedimiento

1. `generar.py` lee el `.ino` de trabajo y lo copia a `control_host.cpp`; sustituye solo sus includes de dispositivo y añade stubs/casos. Los cuerpos originales de las funciones no se editan. Guarda SHA-256 de fuentes y soporte.
2. `compilar.cmd` configura MSVC 14.44.35207 de Visual Studio Build Tools 2022 y compila para Windows x64 con `/std:c++17 /EHsc /W4 /Od /utf-8`. No construye una imagen para ESP32.
3. `control_host.exe` ejecuta 19 casos deterministas de software y escribe líneas JSON. No accede a dispositivos, red, puerto serie ni motores. PWM, pines, reloj, CAN, SPI, I2C, MPU e interrupciones son simulados.
4. `resultados.jsonl` conserva los valores y si la observación se reprodujo. Una observación reproducida puede ser un defecto: no interpretar `true` como validación funcional del kart.

Desde esta carpeta, PowerShell:

```powershell
& "$env:USERPROFILE/.venvs/kart-esp32/Scripts/python.exe" ./generar.py
& $env:ComSpec /d /c compilar.cmd
& ./control_host.exe
```

## Resultados y límites

- [Resultado final](resultados.jsonl): 19 observaciones reproducidas; 0 discrepancias respecto a la observación definida. Cuatro comprueban protecciones: freno=1, ESC sin MPU inicial, CAN con error de lectura y DLC7; se comprueba también corte de timeout junto a su recuperación sin rearme.
- Defectos reproducidos incluyen rampa descartada, ausencia de límite específico en R, asimetría ESC, aceptación de valores incoherentes, orden prematuro de PWM, STOP temprano perdido, salida PWM retenida al abrir relés por cargador y ventana RPM fija.
- [Compilación](compilacion.log): satisfactoria, advertencias de conversión entre double/float/long; no son resultados de compilación Xtensa. Aritmética flotante de PC puede diferir en último bit. No se midieron latencias, señal PWM física, I2C/CAN real ni dinámica del vehículo.
- Prueba inicial: [resultados_intento1.jsonl](resultados_intento1.jsonl), 18/19 observaciones. El caso del cargador no se aisló del temporizador `static` dejado por el escenario anterior; se corrigió el banco simulado haciendo una vuelta con relés apagados antes de iniciar ese caso. **Se corrigió el ensayo, no el firmware.** Fuente del caso inicial y hashes preservados. Su nombre antiguo `marcha_R_sin_limite_1_kmh` suponía una unidad que el define no documenta; se corrigió a `marcha_R_sin_limite_especifico_aplicado` en el resultado final, sin cambiar el caso ni atribuir unidades al literal 1.0.
- El alias `python` de WindowsApps no pudo ejecutarse al consultar documentos; el Python ya instalado en `.venvs/kart-esp32` funcionó. Una búsqueda opcional de archivos de compilación frontal encontró ruta inexistente; no se extrajo conclusión sobre binario/firmware STM32.
- No se ejecutaron las bibliotecas de hardware ni telemetría real: los resultados se limitan a lógica de aplicación del `.ino`. El diagnóstico de SD/pantalla/ADC frontal es lectura estática, sin compilación ni ejecución del STM32.
- La ausencia de pulso Hall, una espera de 1000 ms y yaw ±30 °/s son entradas sintéticas. No describen mediciones del kart ni la causa de los síntomas del usuario.

Los hashes se encuentran en [hashes_fuentes.json](hashes_fuentes.json) y [hashes_resultados.json](hashes_resultados.json). El binario de este directorio es un ejecutable de comprobación para PC; no se debe grabar en una placa. El binario ESP32 de referencia permanece en su respaldo anterior, sin modificar. Firmware, panel y originales intactos; sin commit/push de esta sesión.
