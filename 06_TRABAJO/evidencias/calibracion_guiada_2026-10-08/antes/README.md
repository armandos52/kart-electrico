# Panel local del kart · diagnóstico y telemetría

Aplicación para el portátil, con mapa interactivo del kart, señales por elemento, animaciones cualitativas, traza temporal y captura USB o TCP por Wi-Fi directo. Escucha únicamente en `127.0.0.1`. No envía bytes al micro ni modifica o graba firmware. Versión de panel `0.2.0-direccion`: añade calibración **de la visualización en PC**, sin comandos de motor/calibración al micro.

## Arrancar

En PowerShell, desde esta carpeta:

```powershell
.\iniciar.ps1
```

Abrir `http://127.0.0.1:8765/` en el navegador. Se muestra sin conexión y no se abre automáticamente ningún puerto. El script emplea el Python disponible en este portátil; admite `-Python C:\ruta\python.exe` y `-Puerto 8766`.

La fuente `socket://192.168.4.1:3333` aparece por defecto sin argumentos adicionales. Para leer por Wi-Fi, conectar Windows a `kart-esp32`, elegir «Wi-Fi directo ESP32», confirmar la preparación del banco y pulsar «Leer por Wi-Fi». La versión v1.1 crea la red después de inicializar CAN; el respaldo original no incluye esta telemetría ni esta red. COM de Intel Active Management Technology queda excluido del selector. La vista inicial muestra el ESP32 y su diagnóstico CAN.

**Sesión actual: http://127.0.0.1:8766/**, `-Puerto 8766`, actualmente desconectada tras el diagnóstico por COM4 del módulo ESP32 solo. v1.2 inicia Wi-Fi antes del CAN y reporta su espera sin inventar medidas de motores. Usar esta instancia actualizada; la anterior en 8765 queda desconectada.

## Conexión con el ESP32 instalado en el kart

Armando indica que el USB no es accesible con el módulo instalado. Las pruebas integradas se registran por Wi-Fi; grabación, respaldo y recuperación por USB se hacen con el módulo fuera del kart.

1. Preparar el primer ensayo estático con tracción inhabilitada y alimentar la electrónica del kart.
2. Conectar el Wi-Fi de Windows a `kart-esp32` con la contraseña configurada en `secretos.h`. El panel funciona sin Internet.
3. Abrir `http://127.0.0.1:8766/` en el portátil. Esta es la dirección del panel; `192.168.4.1:3333` es el destino TCP del ESP32, no una página web.
4. Elegir «Wi-Fi directo ESP32», confirmar la preparación del banco y pulsar «Leer por Wi-Fi». Mantener desactivada la demostración visual.
5. Comprobar versión, estado CAN/control y recepción de mandos; conservar el JSONL. `can_wait` indica que CAN todavía no inicializa. Una red visible por sí sola no valida el control ni los motores.

El 08-10-2026 Armando confirma prueba real satisfactoria con motores tras preparar v1.2, [ENS-0004](../../06_TRABAJO/ensayos/ENS-0004_prueba_real_motores_confirmada.md). No se adjuntó registro del panel de esa prueba; la captura integrada por Wi-Fi sigue pendiente de conservar. Si el panel estaba conectado por USB, desconectarlo antes de retirar el módulo; no conectar ambas fuentes simultáneamente.

## Calibrar dirección · aprobado el 08-10-2026

Abre `http://127.0.0.1:8766/` y pulsa **Calibrar dirección**. El servicio queda inicialmente desconectado, sin activar ningún perfil automáticamente.

1. Con el kart inmóvil y la tracción físicamente inhabilitada, conecta por Wi-Fi y comprueba que llega telemetría ESP32 con firmware, dirección y diagnóstico CAN. Desactiva la demostración.
2. Anota configuración del sensor/montaje, método de alineación y condiciones; confirma la preparación y pulsa **Preparar nuevo ensayo**.
3. Alinea físicamente las ruedas y captura el centro durante al menos 10 s/50 muestras. Después sigue los tres ciclos de izquierda → centro desde izquierda → derecha → centro desde derecha. Cada captura posterior exige al menos 5 s/25 muestras; mantén cada posición y no fuerces los topes. Las ventanas se miden entre muestras, por lo que pueden durar algo más.
4. Revisa medianas, mínimos/máximos, pico a pico, desviación estándar y retornos. Introduce tolerancias en cuentas según la evaluación del ensayo; no hay valores físicos predeterminados. Guarda un perfil con nombre. Si no cumple los criterios, el intento queda conservado y no se guarda como perfil aceptado.
5. Selecciona el perfil, confirma compatibilidad de sensor/geometría/montaje y actívalo **solo en la sesión del panel**. Repetir ensayos genera perfiles independientes. Para recuperar uno anterior, selecciónalo y confírmalo. Pérdida de enlace, reinicio o nueva conexión desactivan el perfil.

Sin medidas físicas se muestran cuentas y porcentaje por lado. Opcionalmente registra magnitudes angulares izquierda/derecha, elemento medido (rueda I, rueda D o volante), instrumento y resolución. Los grados son interpolaciones pendientes de comprobar en puntos intermedios. El mapa representa el recorrido de forma esquemática; no ofrece dos ángulos de rueda independientes. La estimación anterior sigue visible para comparar. No se añade suavizado ni corrección automática del centro.

Cada trama se captura una sola vez en el servicio, independientemente del refresco del navegador. Se rechazan datos caducados, saltos/repeticiones, cambio de sesión, falta de renovación CAN y pérdida de recepción. Saturación 0/4095, bandas solapadas o tolerancias incumplidas impiden guardar un perfil aceptado. Una señal sin fluctuación no demuestra que el sensor esté bien: hay que contrastar físicamente.

**Descargar ensayo y muestras** conserva también intentos rechazados. **Descargar perfil y evidencia** incluye JSONL original, resumen CSV, metadatos, perfil y SHA-256. Almacenamiento local: `sesiones/calibraciones/<id>/`; perfiles inmutables en `sesiones/calibraciones/profiles/`. Los ensayos se guardan durante la adquisición, pero un borrador interrumpido por cierre del servicio no se reanuda automáticamente: sus archivos siguen en esa carpeta. No se configura aquí una copia externa.

Validación en [ENS-0005](../../06_TRABAJO/ensayos/ENS-0005_calibracion_direccion_panel.md): 21 tests Python y 16 comprobaciones Chrome con señal sintética, sin hardware. Ensayo físico de dirección pendiente.

## Señales y vista del kart

- Resumen superior: RPM I/D, pedal, dirección, marcha solicitada, PWM I/D y guiñada. Diagnóstico destacado y firmware/motivo de reinicio/sesión.
- Mapa: selecciona piezas para detalles. Bandas de rodadura animadas por RPM, con sentido ilustrativo según marcha ordenada; no es medida de sentido de los Hall. Sin RPM delanteras, las delanteras no ruedan.
- Dirección sin perfil: grados reportados si existen; en ESP32, mando CAN normalizado convertido con la fórmula del fuente y rotulado «estimación del mando · sin calibrar». Con perfil PC: porcentaje o grados interpolados de la referencia indicada. No se presenta como ángulo físico validado de cada rueda. Si el CAN reporta mando caducado (>100 ms), no se representa como dirección vigente.
- Tabla de todas las señales con buscador, origen, estado y antigüedad. Gráfica de cada señal numérica. Señales ausentes o antiguas no se muestran como mediciones actuales.
- Pausar animación detiene rodadura; dirección cambia sin transición. Se respeta movimiento reducido. La demo genera únicamente datos sintéticos y mantiene bloqueada la lectura del micro.
- El panel guarda JSONL de lectura real. Horas visuales Europe/Lisbon; datos originales UTC. Los GPIO/relés/PWM son órdenes software, no prueba de tensión ni contacto físico.

Validación: 13 tests Python, QA de Chrome y captura con placa real en `can_wait`. [ENS-0003](../../06_TRABAJO/ensayos/ENS-0003_v12_diagnostico_y_panel.md) conserva datos, capturas y límites. La prueba real con motores fue satisfactoria según Armando en ENS-0004; la verificación instrumentada del diagnóstico con CAN/STM32 en el kart sigue pendiente de datos.

La dependencia pySerial 3.5 se ha instalado localmente en `.vendor/` (fuera de Git). Para otro ordenador, instalarla con el Python elegido:

```powershell
python -m pip install --target .vendor -r requirements.txt
python server.py
```

No necesita servicios de IA ni Internet al funcionar. Interfaz, scripts y estilos son locales.

## Conexión USB de banco, con el módulo fuera del kart

1. Identificar la placa y preparar alimentación/cableado en laboratorio con tracción inhabilitada; conservar condiciones de USB y alimentación para comparar los fallos existentes. Abrir un puerto puede producir un reset por DTR/RTS incluso desactivados según el driver; no se promete captura pasiva eléctricamente inocua.
2. Conectar la placa al portátil, actualizar puertos e identificar el que corresponde al USB. COM3 observado el 07-10-2026 es **Intel Active Management Technology SOL**, no el kart. No seleccionarlo como ESP32/STM32.
3. Confirmar la condición del banco en la interfaz y elegir **Leer por cable**. Configuración ESP32 utilizada en ENS-0003: 115200 bit/s, 8N1, DTR/RTS desactivados.
4. El código ESP32 entregado imprime mensajes de diagnóstico. Se conservan en consola y JSONL; no se convierten en rpm/PWM inventados. Las variables gráficas necesitan tramas de telemetría compatibles con [PROTOCOLO.md](PROTOCOLO.md), disponibles en v1.1, pero ausentes en el respaldo original. STM32 necesita preparar su canal de diagnóstico.
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
