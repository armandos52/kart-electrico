---
estado: diagnóstico y propuesta; sin cambios de firmware
actualizado: 2026-10-08
resumen: Revisión de v1.2, recorrido pantalla-CAN-control, funciones efectivas, defectos reproducidos y orden propuesto de mejora.
---

# Revisión del código v1.2 · 08-10-2026

> Actualización 09-10: este diagnóstico conserva el estado histórico v1.2. El usuario autorizó después las correcciones de [v1.3](IMPLEMENTACION_ROBUSTEZ_V13.md); estos resultados no describen el fuente actual. Las referencias E/T se han fijado a copias verificadas del commit de la auditoría.

## Conclusión para Armando

Hay una base aprovechable: el kart funciona según su prueba, los tres modos están conectados en el fuente y existen cortes por freno y pérdida de órdenes. No equivale a tener todas las funciones terminadas. **Conservaría DIF como base a calibrar, BLQ como mando igual útil para comparar y la telemetría; corregiría primero los estados de parada, la validez de datos y la rampa; dejaría ESC como experimental hasta resolver cálculo y sensor.** La pantalla y la SD requieren trabajo en el STM32, independiente de la v1.2 del ESP32.

Armando pide investigar y proponer antes de actuar. En esta sesión se revisaron fuentes y se ejecutaron comprobaciones aisladas en el PC. No se modificó ni grabó firmware, ni se conectó al kart. Todavía no se recibió acceso a la pantalla mencionada ni una descripción que permita atribuir su fallo a una causa concreta.

## Configuración y alcance de evidencia

- **Declaración del usuario:** reconfirma marcha satisfactoria con v1.2; indica modos/pantalla que prácticamente no funcionan. La prueba anterior figura en [ENS-0004](ensayos/ENS-0004_prueba_real_motores_confirmada.md), sin logs ni detalle por modo.
- **ESP32 analizado:** `esp32_trasero-v1.2-diagnostico`, commit de referencia `905b09ea26a153da4ff281861f6e41fc71838462`. `src`, `include/telemetria.h`, `platformio.ini` y biblioteca CAN local no difieren de ese commit. HEAD documental al comenzar: `968b828ea60f02d3fbfe14b155640aacc8331993`; había otros cambios locales anteriores, conservados.
- **Binario de referencia:** SHA-256 `d2a1e207468e89fb3d23dee8b15c95e2eb69673d23cc29fd34d802d287021794`, según [ENS-0003](ensayos/ENS-0003_v12_diagnostico_y_panel.md). No leído de la placa en esta sesión; no se generó nueva imagen ESP32.
- **Frontal:** fuente histórico STM32/TouchGFX 2025/26; no se ha identificado el binario instalado. Los fallos descritos del frontal corresponden a este fuente, pendientes de comprobar en la placa.
- **Prueba automática:** 19 escenarios sobre los cuerpos reales del `.ino`, compilados con MSVC x64 y periféricos simulados. Se reproducen 19 observaciones, incluidas protecciones correctas y defectos. **No son 19 pruebas de seguridad superadas**, ni simulación física, ni ensayo de banco o del kart.
- [Fuentes y hashes](evidencias/AUD_V12_2026-10-08/hashes_fuentes.json), [resultados](evidencias/AUD_V12_2026-10-08/resultados.jsonl), [procedimiento y límites](evidencias/AUD_V12_2026-10-08/README.md). Sin credenciales en la evidencia.

## Referencias de código

Los rangos siguientes se refieren a estos archivos y a sus hashes conservados, no a números de línea de otra versión:

- **E:** [ESP32-final.ino de v1.2](evidencias/AUD_V12_2026-10-08/fuentes_v12/src/ESP32-final.ino), SHA-256 `b39f5164e06957cb91bb564b7e217c98bd6446bb718e4d9a42af6f8022370f5c`.
- **T:** [telemetria.cpp](evidencias/AUD_V12_2026-10-08/fuentes_v12/src/telemetria.cpp).
- **M:** [main.c del STM32 2025/26 · solo local](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/Core/Src/main.c).
- **S:** [Screen1View.cpp · solo local](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/TouchGFX/gui/src/screen1_screen/Screen1View.cpp).
- **G:** [Screen1ViewBase.cpp generado · solo local](../02_ORIGINALES/entrega_2026-09-28/Smart%20Eletric%20Kart%20Cross-2223406-2223403/Smart%20Eletric%20Kart%20Cross-2223406-2223403/STM32-final/TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp).

## Qué está activo y para qué sirve

| Función | Estado comprobado en código | Valor y tratamiento propuesto |
|---|---|---|
| Lectura de acelerador, dirección, freno, D/R y START | Activa en M:389–456; manda CAN 0x100. Los ADC transmitidos ya están normalizados | Conservar; añadir plausibilidad, caducidad y diagnóstico del ADC físico |
| DIF, modo 0 | Activo E:130–170, 556–557; reparto geométrico del mando entre ruedas | Conservar y calibrar. Es control abierto: el PWM no garantiza las RPM calculadas |
| ESC, modo 1 | Activo E:172–229, 533, 559–560, condicionado a MPU presente al inicio | Experimental. Corregir antes de evaluarlo como control de estabilidad |
| BLQ, modo 2 | Activo E:231–238, 562–563; PWM igual en ambos lados | Útil como referencia. No bloquea mecánicamente ni regula igualdad de RPM |
| Rampa | Se calcula E:536–554, pero sus resultados no alimentan las funciones de modo | Implementar su efecto real; cambiar la constante actual no suavizaría los motores |
| Límite específico de marcha atrás | `REVERSE_SPEED_LIMIT=1.0` solo se define E:38, sin uso ni unidad explícita | Definir requisito y aplicarlo; no tratar esa constante como protección existente |
| Freno | `travao==1` corta mando y direcciones E:527, 579–587 | Conservar el corte. No hay frenado eléctrico ni regeneración: salidas de freno quedan a 0 |
| Timeout CAN | E:401–403 para tras más de 100 ms desde última orden aceptada, si el bucle sigue ejecutándose | Conservar y completar recuperación/armado; no abre contactor ni enclava el fallo |
| Relés y cargador | Secuencia nominal 2 s + 1 s, E:331–377, 457–475; chequeo de cargador por ventanas de 2 s | Conservar finalidad; corregir STOP y hacer coherentes estados y PWM. GPIO leído/ordenado no prueba contactos físicos |
| Hall traseros | ISR activas E:120–121, 313–316; cálculo E:477–491 | Conservar adquisición; medir pulsos/vuelta y usar intervalo real. Hoy falta validez del sensor |
| Giroscopio | Calibración inicial de 500 muestras si responde; lectura continua E:270–283, 407–410 | Conservar sensor como candidato; vigilar errores posteriores y calibrar inmóvil |
| Wi-Fi/USB de diagnóstico | Tarea separada, cola sin espera, versiones/estados, E:249 y T | Conservar. Es lectura; no cambia modo ni calibra el control desde el PC |
| Pantalla frontal | Selección y datos activos S; confirmación de recepción/ejecución ausente | Reparar recorrido y mensajes; mostrar solicitado, recibido, aplicado y motivo de inhibición |
| SD | Código activo M:346–387, 1197–1236 | Conservar finalidad, rehacer ciclo Play/Pausa y gestión de errores |
| BMS | STM32 interpreta SOC y temperatura M:264–275; ESP32 no los consume | Verificar protocolo real y validez; autonomía necesita también tensión/corriente y registro |
| Calibración del panel PC | Asistente visual ya implementado, ENS-0005 | Útil para medir. No cambia aún las constantes de dirección del ESP32/STM32 |

## Recorrido real de los modos y explicación de la pantalla

1. G:101–107, 150–157 instala un botón transparente en la zona del modo y llama `modoTextClick()`; el método virtual sí está implementado.
2. S:139–150 permite cambiar solo si `g_velocidade_kph==0` y `g_modo_kart==2` (N). En caso contrario retorna sin aviso. Cambia cíclicamente 0 → 1 → 2 → 0.
3. M:454 empaqueta modo en bits 0–1 de byte 7 del CAN 0x100; START usa bit 2. **El ID 0x104 definido no es el camino usado.**
4. E:383–397 recibe, extrae y procesa; E:556–564 llama DIF/ESC/BLQ. El modo 3 también cae en BLQ por defecto, aunque la UI no lo genera.
5. La respuesta CAN 0x123 lleva velocidad, RPM y yaw (E:495–503), **no un acuse del modo aplicado ni el motivo de inhibición**. La etiqueta de pantalla refleja la selección local. La telemetría del ESP32 sí informa modo recibido y `drive_inhibit`, pero `mode` solo tampoco demuestra actuación.

DIF y BLQ son idénticos dentro de ±200 unidades de dirección normalizada respecto a 2047 (E:137). Es aproximadamente ±4,18° en la fórmula del software, **no un ángulo físico calibrado**. ESC tampoco corrige yaw dentro de esa zona; no es un estabilizador general en recta. Fuera de ella, `calcular_angulo` satura a ±20° desde un desplazamiento de unas 958 unidades (E:123–127); parte del recorrido normalizado deja de cambiar el reparto. Debe contrastarse con [PROB-0007](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md).

Posibles explicaciones, todavía sin atribución física: bloqueo silencioso por D/R o velocidad; dato de velocidad antiguo; giroscopio ausente en ESC; comparación en zona central; fallo de táctil o imagen. La franja blanca de [PROB-0004](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md) no demuestra por sí sola fallo táctil. El resto `Screen2View.hpp` sin base generada no acredita una segunda pantalla funcional ni identifica «la otra pantalla» mencionada por Armando.

## Hallazgos prioritarios y evidencia

**A01 · Alta · STOP puede perderse.** E:466–474 solo procesa el flanco descendente si ya pasaron más de 500 ms desde `tempo_relay`; aun cuando lo ignora, actualiza `start_anterior=false`. Si STOP llega a los 100 ms y se mantiene, no lo vuelve a intentar: relé 1 queda activado y la secuencia puede avanzar. Reproducido en `STOP_temprano_perdido`. También hay una ventana tras activar relé 2, porque se reinicia `tempo_relay` E:359. Proponer petición de parada persistente con prioridad, no dependiente de conservar un flanco. No cambiar tiempos de potencia sin contrastar montaje.

**A02 · Alta · Pérdida y recuperación de CAN sin rearme.** E:394 restablece el permiso en la siguiente trama; tras timeout vuelve a mandar potencia con pedal todavía pisado. Reproducidos corte y reanudación. Proponer estado de fallo/espera y criterio explícito de rearme con mando válido y pedal liberado; requisitos por acordar. El timeout de 100 ms depende del bucle: no demuestra límite temporal si se bloquea I2C/SPI u otra llamada.

**A03 · Alta · Entradas inválidas pueden convertirse en aceleración válida.** M:401–403 recorta lecturas ADC superiores a 1861 al máximo; no distingue avería de pedal a fondo. Si `HAL_ADC_PollForConversion` falla, conserva el valor anterior y sigue enviándolo en tramas nuevas (M:399–404, 413–423). En E solo se validan éxito de lectura/ID/longitud, no el contenido: 65535 de acelerador se recorta a 100 %, freno=2 no corta, D=R=1 prioriza D y modo=3 se usa como BLQ. Reproducido en host para esos cuatro valores. No se afirma que un fallo físico concreto ya haya producido esos valores. Añadir rangos/estados válidos, antigüedad de adquisición y rechazo antes de actuar; recuperar ADC original para diagnóstico.

**A04 · Alta · Las salidas se escriben antes de completar permisos.** E:556–564 escribe PWM; después E:566–576 decide marcha/N/cambio. En N se registran escrituras de aceleración y luego de parada en la misma llamada. Es secuencia software comprobada, **no un pulso físico ni movimiento medido**. Además, al detectar cargador E:342–345 abre relés sin llamar inmediatamente a parada de PWM: hasta nueva orden CAN o timeout puede quedar la salida anterior. Reproducido con CAN reciente y sin trama nueva en esa vuelta. Proponer decidir todos los permisos y aplicar una sola salida coherente al final; revisar permiso ya en `LIGOU_RELE2` E:525, antes del estado `PRONTO`.

**A05 · Alta · Rampa inefectiva.** E:536–554 obtiene un PWM suavizado local que se descarta: cada modo recalcula directamente a partir del porcentaje. En el caso sintético de 0 a 100 % y 5 ms, `pwm_atual=254,328949` y salida=0 (255 es parada). Conectar una consigna común limitada por tiempo a todos los modos; parada por fallo/freno debe conservar prioridad inmediata. No adoptar `300 rpm/s` como aceleración física demostrada: hoy es un literal aplicado a una escala teórica de PWM.

**A06 · Alta antes de usar ESC · Cálculo y sensor incompletos.** E:181 toma valor absoluto del ángulo, por lo que el yaw ideal nunca es negativo, mientras `yaw_rate` sí tiene signo. No incorpora signo de R. Usa velocidad deducida del pedal y de 570, no velocidad medida, E:180–197. En caso espejo de dirección y yaw (±30 °/s sintéticos), los PWM obtenidos son [63,145] y [145,30], no intercambios simétricos. `mpu_ok` solo se actualiza al arrancar; la biblioteca MPU6050 instalada en `getRotation()` no devuelve al llamante éxito de la lectura I2C. Ausencia inicial detiene ESC correctamente; desconexión posterior no queda supervisada. Proponer ejes/signos/unidades definidos, referencia de velocidad válida, supervisión del sensor y ensayos de ambos giros y R. No activar automáticamente DIF como sustituto sin una política explícita.

Antecedente documental: [DOC-0042](../03_CATALOGO/fichas/DOC-0042.md), **PDF 57 / impresa 43** declara ESC sin ensayo real por masa defectuosa del GY-521; **PDF 96 / impresa 82** lo reitera y describe comportamiento como DIF. Es historia de julio, no diagnóstico del hardware de octubre. Tampoco describe el nuevo bloqueo de ESC sin MPU de v1.2. El texto de PDF 32 / impresa 18 explica los botones invisibles. Solo se recuperó texto: figura y ecuaciones diagramadas no fueron revisadas visualmente en esta sesión.

**A07 · Alta para permisos; media para exactitud · Velocidad sin validez suficiente.** E:485–489 divide siempre por 500 ms aunque transcurra más: una vuelta sintética en 1000 ms da 120 rpm en lugar de 60. Ausencia de pulsos se convierte en cero sin distinguir rueda parada de sensor desconectado; se usa `rpm_medio==0` como permiso de cambio de dirección E:526. Ventana nominal 500 ms, enteros y umbral mínimo 2 pulsos añaden retardo/cuantización. M conserva velocidad de 0x123 sin timeout; la UI y cambios de marcha usan ese último entero en km/h. 0 km/h truncado no prueba inmovilidad. Medir 89/68 pulsos por vuelta, radio/diámetro y Hall delanteros (sondeo en M:1282–1301); introducir frescura y validez antes de confiar en esos permisos.

**A08 · Media · CAN y supervisión de ejecución incompletos.** Se conserva el parche de límite DLC a 8 en mcp_can y el rechazo de lectura fallida/DLC7 pasa en host. E:503 y M:456 ignoran resultado de transmisión. M no comprueba longitud del BMS ni tipo de trama en sus ramas, ni vigila su antigüedad. El STM32 sí configura `AutoBusOff=ENABLE` M:716: falta diagnóstico y política de aplicación, no toda recuperación hardware. `Error_Handler` vacío M:1416–1422; IWDG/WWDG deshabilitados. En el ESP32 no se habilita supervisión explícita de `loopTask` en el fuente; el core local inicia `loopTaskWDTEnabled=false`. **No afirmar que el ESP32 carezca de todos los watchdogs internos.** Especificar supervisión y comportamiento seguro antes de habilitar reinicios automáticos. [Referencia oficial ESP-IDF 5.5](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32/api-reference/system/wdts.html).

**A09 · Media · SD no reanuda tras Pausa en el fuente.** S:134–135 llama `Close_SD`; M:1186–1195 cierra, desmonta y deja `sd_ready=0`. `SD_Init()` se llama una sola vez al entrar en `StartSDTask` M:1199, y Play solo cambia una bandera. Además, la UI puede cerrar el archivo mientras la tarea SD escribe, se ignoran errores de escritura/sincronización y los datos compartidos no forman una instantánea protegida. El icono Play no acredita grabación. Hacer que una única tarea sea dueña de la SD, maneje montar/abrir/pausar/reintentar y publique estado/error. Añadir versión, unidades, validez y batería tras verificar protocolo.

**A10 · Media · Pantalla con tres rutas de refresco y sin confirmación.** S:117–120 actualiza por tick; `Model`→`Presenter` vuelve a llamar, y M:1340–1343 invoca la View desde `defaultTask` a través de `atualizarTela1`. El bloqueo de framebuffer no convierte toda esta lógica compartida en una arquitectura verificada; hay riesgo de concurrencia, sin cuelgue físico atribuido. Centralizar actualización en la tarea GUI mediante mensajes/instantáneas y `Model::tick`, como recomienda [ST/TouchGFX](https://support.touchgfx.com/docs/development/ui-development/touchgfx-engine-features/backend-communication). Mostrar causas concretas de rechazo y datos ausentes/antiguos; BMS inicializado a cero no demuestra SOC real 0 %.

**A11 · Media · Fallo de recursos de telemetría durante arranque.** T:144–154 admite cola/tarea no creadas y pone `cola=nullptr`, pero E:269 llama directamente `tomarMuestra`, que alcanza `xQueueSend(cola,...)` T:225 sin guardia. La ruta no está protegida aunque `telemetria_toca` sí filtre cola nula. Hallazgo estático; inyección real de fallo de memoria pendiente. Proponer desactivar observación sin comprometer control/arranque y contabilizar errores.

**A12 · Mantenibilidad · Eliminar restos después de fijar comportamiento.** `pwm_base` calculado en E:553–554 no se utiliza; `REVERSE_SPEED_LIMIT`, `W_BITOLA`, `V_MAX_MOTOR` y `RAIO_RODA` están desconectados mientras se repiten literales; el límite de ángulo de 25° es inalcanzable tras saturar a 20°. En STM32 quedan ID 0x104 y ruta `g_can_data_buffer/g_can_received_id` que no reciben contenido (Model los entrega al Presenter, pero la View realmente usa globales); prototipos y Screen2 requieren limpieza selectiva. No borrar bibliotecas, fuentes históricos, código generado necesario ni una protección pendiente solo por estar hoy sin implementar. Recuperar bloques USER CODE y un proceso CubeMX reproducible antes de regenerar el proyecto.

## Propuesta concreta, por etapas

| Etapa | Trabajo propuesto | Criterio de cierre |
|---|---|---|
| 0. Referencia y pantalla | Identificar firmware frontal; observar modo, N/D/R, velocidad, `mpu_ok`, `can_rx_age_ms` y `drive_inhibit` de v1.2. Registrar por Wi-Fi en el kart | Distinguir botón bloqueado, selección no recibida, recepción inhibida y problema visual/táctil; conservar versión y condiciones |
| 1. Estados y protecciones | A01–A04, validez de velocidad/ADC, rearme, tratamiento de fallos; una sola función aplica salidas. Supervisión y política de relés definidas | Pruebas automáticas de fallos/transiciones y banco sin tracción que demuestren órdenes de parada y ausencia de rearranque involuntario. Después contraste temporal y eléctrico |
| 2. Control básico | Rampa efectiva común, requisito de marcha atrás, calibración de dirección y respuesta por motor; conservar DIF y BLQ identificados correctamente | Comparaciones de consigna/salida/RPM, ambos sentidos y giros; ninguna protección anulada por la rampa |
| 3. Frontal y registro | Estados claros y acuse de modo, datos caducados, GUI por una tarea, SD Play/Pausa/reintento, errores visibles | Ciclos repetidos de pausa/reinicio, pérdida de CAN y tarjeta ausente sin congelar control; binario frontal trazable |
| 4. ESC experimental | Sensor y ejes, signo del yaw y R, referencia medida, límites y ganancia fundamentada | Casos espejo, sensor desconectado después del arranque, comparación con DIF y ensayo físico instrumentado antes de declarar mejora |
| 5. Limpieza | Extraer módulos de entradas/protocolo/estado/control/actuadores/diagnóstico; constantes únicas y restos confirmados fuera | Comportamiento cubierto por pruebas, compilación reproducible, fuente y binario por versión |

No se fija aún fecha ni se bautiza una nueva versión como terminada. Cada intervención de firmware debe ser pequeña y comparada con v1.2; el respaldo original se conserva. USB se reserva para módulo fuera del kart: **ningún ensayo integrado depende de USB accesible**. No proponer prueba de fallos durante conducción; primero ensayo aislado y banco preparado, después integración controlada y registrada.

## Correcciones de notas anteriores y cobertura

El inventario del 30-09 describía el fuente histórico, no las mejoras de diagnóstico posteriores. Se aclaran tres afirmaciones: sí hay corte de aceleración por `travao==1` en ESP32; el STM32 tiene AutoBusOff habilitado; no puede inferirse «sin dato» solo porque yaw o RPM sean cero. Dependencias ESP32/compilación están fijadas desde octubre y ya existe motivo de reset en telemetría. El resumen del inventario se enlaza a esta revisión para evitar tratarlo como estado vigente de v1.2.

Cobertura: E completo (588 líneas), T completo (228), header de telemetría y PlatformIO; M:35–78,118–204,242–278,326–456,699–723,1185–1348,1416–1422; S completo, G:95–166, declaración virtual del botón, Model/Presenter y búsquedas focalizadas; mcp_can:764–785,1231–1248; MPU6050 `getRotation`; core Arduino `loopTaskWDTEnabled`. DOC-0042 texto PDF 32,57,96 mediante buscador. No se revisa íntegramente HAL/TouchGFX, ni todas las páginas históricas, ni se identifica por comparación el ELF del STM32. L1 y L4 siguen abiertos.
