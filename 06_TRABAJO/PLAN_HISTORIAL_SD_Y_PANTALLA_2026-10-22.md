---
estado: implementación autorizada; frontal dev12 instalado, aceptación integral pendiente
actualizado: 2026-10-23
resumen: Oscuro fijo, grabación visible, sesiones autónomas en microSD y reproducción en el panel existente.
---

# Plan de grabación e historial del kart · 22-10-2026

**Actualización 23-10:** Armando autorizó implementar todo el plan y cargar/comprobar la placa. [ENS-0029](ensayos/ENS-0029_instalacion_historial_sd.md): frontal dev12 instalado/verificado, oscuro fijo/botón y grabación/cierre reales. Biblioteca/replay y ampliación trasera en código local; ESP32 no instalado, aceptación extremo a extremo pendiente. Usuario sin lector microSD ahora. Las formulaciones siguientes de aprobación pendiente describen la propuesta original.

## Objetivo y alcance solicitado

Armando pide retirar el selector claro/oscuro de la pantalla, conservar oscuro y aprovechar ese espacio para grabar. Declara que ya dispone de la memoria instalada y quiere conservar las pruebas y reproducir sus parámetros visualmente en la aplicación existente. Pide **presentar y aprobar un plan antes de implementar**.

**Confirmación de Armando, 22-10:** se refiere a **microSD en la STM32 de la pantalla** y al **panel local del portátil con vista 3D**. Su respuesta resuelve la ambigüedad inicial entre «micro USB» y «micro SD»; es declaración del usuario, sin nueva inspección física. Capacidad, formato, contenido previo y acceso para extraerla pendientes. La microSD almacena los archivos; un lector USB del portátil sería solo el medio para copiarlos. Esta aclaración no aprueba todavía la implementación ni la grabación automática propuesta.

El resultado buscado es: **encender → registrar → cerrar y conservar → importar → elegir prueba → reproducir**. La reproducción muestra lo registrado a su cadencia, con pausas y saltos; no reconstruye señales ausentes ni una trayectoria del kart sin sensores de posición.

## Punto de partida comprobado

| Aspecto | Evidencia y límite |
|---|---|
| Configuración física más reciente identificada | F746 **dev9-ui**, grabación y relectura verificadas el 21-10; ESP32 **v1.2-diagnostico**, identificado por Wi-Fi. [ENS-0026](ensayos/ENS-0026_diagnostico_bloqueo_marchas.md). Sin conexión al kart hoy. |
| Código de trabajo frontal | **dev10-ui**, filtro del pedal compilado/probado en PC, **sin instalar**. [ENS-0027](ensayos/ENS-0027_pedal_media_ponderada.md). Conservarlo; separar su validación de la del registrador. ESP32 de trabajo dev3 tampoco equivale a versión instalada. |
| Pantalla | Siete páginas de diagnóstico, oscuro al arrancar y botón de tema de 88×58 píxeles en x=392, y=48. Grabación disponible dentro de SD. Fuente: `kart_dashboard.c`, `hit`, `draw_sd`, `KartDashboard_Draw`. Mantener filas 0–47 libres por daño del LCD. |
| Grabación actual | `kart_frontend.c`, `Kart_SdTick`: un único `kart_v13.csv`, esquema 2, anexado; muestra nominal cada **200 ms (5 Hz)** y petición de sincronización cada **1000 ms**. Son constantes del código, no rendimiento medido con tarjeta. No separa de forma inequívoca todos los reinicios/pruebas. |
| Fallo SD conocido | Montaje sin tarjeta observado de aproximadamente **30 s** en dev4. Reintentos/errores mejorados después; detección física y escritura con tarjeta pendientes. El driver local `BSP_SD_IsDetected` devuelve siempre presente. [PROB-0013](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md). |
| Formato y reloj | `ffconf.h`: exFAT y nombres largos deshabilitados; `get_fattime()` devuelve 0. No usar la fecha del archivo como hora real de ensayo. Comprobar formato compatible antes de escribir; no formatear ni borrar contenido existente. |
| Panel existente | Guarda JSONL Wi-Fi, muestra señales/gráficas/3D. `server.py` y `static/index.html` no ofrecen biblioteca/importador SD ni reproductor con búsqueda temporal. `qa/reproducir_sesion.py` repite JSONL por TCP para QA; es antecedente técnico, no el historial solicitado. |

Fuentes consultadas y hashes: [manifiesto](evidencias/plan_historial_sd_2026-10-22/fuentes.json). HEAD base `bebe4528747bc6e05dc1ba244cd3ce9a090a7942` con cambios locales previos, conservados. No se ha compilado ni ejecutado ensayo nuevo en esta revisión.

## Prioridades propuestas

### 1. Verificar la tarjeta y hacer fiable el registro

Identificar capacidad/formato/contenido y conservar una copia antes de cualquier transformación. Contrastar detección en la placa: [DOC-0034](../03_CATALOGO/fichas/DOC-0034.md), UM1907 Rev 7, PDF 21 §6.11 y PDF 29 §7.7/tabla 10 describen microSD por SDMMC1 y detección en PC13. Consulta textual; revisión de esquema, revisión física y polaridad pendientes antes de adoptar ese pin.

Separar captura y escritura: instantáneas/eventos a una cola de tamaño fijo y tarea SD de baja prioridad para vaciarla. Nada de operaciones de archivos ni esperas SD dentro del control o las interrupciones CAN. Medir tamaño de cola y presupuesto temporal; publicar desbordamientos, filas descartadas, errores y último guardado confirmado. Un fallo SD debe ser visible y no alterar los permisos de marcha.

Propuesta de cadencia: objetivo inicial **50 Hz (20 ms)** para entradas locales y eventos de cambios capturados en el ciclo que los detecta. Es un objetivo de desarrollo condicionado a pruebas, no capacidad acreditada. Conservar timestamp/edad real de las señales CAN: el feedback trasero actual cada 200 ms no se convierte en una medición a 50 Hz por repetirlo. Registrar flancos, cambios de marcha/modo/START, invalidez y errores aunque no coincidan con la muestra periódica; no prometer captura de transitorios eléctricos entre adquisiciones.

Corregir detección/esperas y diferenciar ausencia, apertura, grabación, cierre y error. Una petición de detener no muestra «Guardado» hasta completar sincronización/cierre sin fallo. Comprobar tarjeta llena, fallo de escritura, retirada y reinicio. Ante corte de alimentación puede perderse la cola/última escritura o dañarse el sistema de archivos; medir recuperación, conservar fragmentos y señalar sesión incompleta. **Sin garantía de pérdida cero** por llamar a `f_sync` periódicamente.

### 2. Oscuro fijo y botón de grabación siempre visible

Retirar el selector y la acción táctil de tema; usar oscuro en las siete páginas. Reutilizar el área superior derecha para el estado/control SD, sin invadir la zona dañada ni START/marchas.

| Estado visible | Acción y resultado |
|---|---|
| **Grabar** | Crea una sesión nueva; nunca sobrescribe una anterior. |
| **Abriendo** | Indica solicitud en curso; todavía no acredita datos guardados. Permite pedir cancelación. |
| **● REC / Detener** | Grabación activa. Pulsa para finalizar; no equivale a STOP del kart. |
| **Cerrando** | Espera sincronización/cierre; evita pulsaciones duplicadas. |
| **Guardado / Grabar** | Sesión cerrada correctamente; permite iniciar otra. |
| **SD: error** | Acceso al detalle y reintento, sin fingir grabación ni ocultar el último fallo. |

La página SD conserva duración, identificador, muestras, espacio, pérdidas y error. Probar los textos dentro de los 88×58 píxeles con renders y después en el táctil real. Texto e icono acompañan al color. La pantalla sigue siendo de desarrollo ([DEC-0010](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md)).

**Recomendación pendiente de aprobación:** grabar automáticamente al arrancar cuando la SD esté disponible, incluso con START apagado, para recoger preparación y fallos previos a mover ruedas. El botón permite cerrar o comenzar otra sesión. Después de una parada manual, conservar aviso «Sin grabar» hasta nueva pulsación o nuevo encendido; no reactivar silenciosamente. No vincular inicio de registro únicamente a START, ni detenerlo al cambiar a N. Sin tarjeta no hay historia persistente: aviso visible, sin prometer recuperación posterior de lo no almacenado.

### 3. Historial autónomo y cobertura de señales

Una sesión por encendido/inicio manual, con ID único y creación exclusiva; un reinicio nunca continúa a ciegas un archivo anterior. Propuesta compatible con nombres cortos: `LOGS/S0000001/META.TXT`, `DATA.CSV`, `EVENTS.CSV`. El identificador no depende del reloj; evitar colisiones y rotar archivos antes del límite configurado, sin borrar ensayos viejos automáticamente.

Guardar versión del esquema, identidad de placas conocida/desconocida, firmware/build, parámetros de captura/calibración, unidades, contador de secuencia, tick monotónico, arranque, eventos y cierre. Conservar ADC bruto y mando procesado como magnitudes diferentes. Manejar rollover del tick y distinguirlo de reinicio. La biblioteca admite título, objetivo, condiciones y notas del operador.

Si no hay hora civil fiable, mostrar «fecha de ensayo sin confirmar», tiempo relativo y fecha de importación por separado. Anotación manual o futura sincronización incluirán origen/incertidumbre. No inferir fecha exacta por nombre del archivo. Tampoco alinear automáticamente registros STM32/ESP32 por tener ticks parecidos: requieren una referencia común o alineación declarada y comprobada.

| Familia | Primera entrega SD | Trabajo adicional para cubrir el historial completo disponible |
|---|---|---|
| Pedal/dirección | ADC físicos, mando normalizado, validez; añadir selector físico y permisos del snapshot | Mantener versión de curva/filtro; no confundir ángulo estimado con ángulo medido. |
| Marchas/START/modos | Solicitudes, freno, transiciones y bloqueos frontales | Separar solicitud frontal, recepción/aplicación trasera; v1.2 no entrega todo por CAN. |
| Ruedas | RPM delanteras/traseras, velocidad estimada, edades y validez disponible | Guardar ausencia de flags traseros como desconocida; no deducir validez IMU de yaw=0 en v1.2. |
| BMS | Bytes 0x355/0x356, edades, SOC/temperatura candidatos con etiqueta provisional | Tensión/corriente y protocolo pendiente de verificar; no fabricar valores ni 16 tensiones de celda. |
| PWM, relés, motivos/reset y diagnóstico ESP32 | Existen parcialmente en el JSONL Wi-Fi; no todos llegan hoy a la STM32 | Ampliar telemetría CAN diagnóstica ESP32→STM32 con identidad, secuencia y validez para grabarlos en SD sin portátil. |
| SD/CAN/errores | Contadores, cambios y eventos con origen/timestamp | Registrar pérdidas, cierre incompleto y periodos sin comunicación; lo no observado no se convierte en cero. |

**La ampliación trasera forma parte del objetivo completo**, después de la primera entrega funcional. Es una modificación separada, compatible con v1.2 y revisada frente al dev3 local: no instalar dev3 solo para conseguir registro, porque arrastra cambios de control y asuntos abiertos de reversa/rampa. Acordar contrato, comprobar IDs disponibles y medir carga del CAN/control. Se conserva el JSONL Wi-Fi como evidencia complementaria mientras tanto.

### 4. Importación y reproducción en nuestra aplicación

Primera vía: **detener → esperar Guardado → apagar → retirar microSD → lector del portátil → Importar pruebas**. Confirmar accesibilidad del soporte. No requiere USB del ESP32 instalado. Si retirar la tarjeta resulta inviable, revisar el transporte antes de comprometer esta vía; la descarga Wi-Fi desde la SD necesita un puente adicional, no existe por disponer ya de Wi-Fi en el ESP32.

Copiar originales al archivo local con SHA-256 y detectar duplicados sin sobrescribir. Validar esquema/unidades, tamaño, filas y secuencias; aislar errores indicando líneas y conservar todo el original. Reconocer CSV actual esquema 2 y formatos históricos solo con cabecera reconocida, sin mezclar columnas por posición. Un antiguo CSV sin identificadores/fecha no garantiza separar todas las pruebas; exponer esa incertidumbre. Importar también sesiones JSONL existentes.

Añadir **Historial** al panel actual: lista de pruebas, duración, fecha/calidad de fecha, versión, resultado de cierre, avisos y notas. Reproductor con reproducir/pausar, barra temporal, salto al evento o a una muestra y velocidades 0,25×/0,5×/1×/2×/4×. **1× reproduce el tiempo del registro**, no una conexión en directo.

Compartir señales normalizadas entre tabla, gráficas, esquema y 3D; introducir una fuente histórica y reloj de reproducción independientes del reloj real. Al pausar no deben caducar artificialmente los valores; al buscar atrás reconstruir estado, edades y eventos sin residuos de muestras futuras. Las gráficas consumen el registro completo, no solo el refresco del navegador. Huecos, muestras inválidas e interpolación visual quedan identificados; nunca suavizar datos originales para ocultar errores.

Rótulo persistente **REPRODUCCIÓN · sesión …**; fuente histórica separada de directo y demo. El replay no emite órdenes al kart ni crea una nueva medición física. La calibración visual utiliza el perfil asociado al ensayo; aplicar otro perfil exige identificarlo como reinterpretación. Visualizar comportamiento registrado, sin afirmar trayectoria, par o fuerzas no medidos.

## Entregas y criterios de aceptación

| Orden | Entrega | Comprobación necesaria |
|---|---|---|
| A | SD fiable + sesiones + oscuro/botón global | Pruebas PC de estados/errores y no sobrescritura; regresión control/pantalla; compilación y renders. Luego banco con tarjeta real, pausa/cierre, reinicio y medida de latencias. |
| B | Importador, biblioteca y replay del frontal/JSONL | Casos normales, duplicados, CSV roto/esquema desconocido, tick reiniciado/rollover, archivo grande, huecos y señales ausentes. Comparar valores/timestamps antes/después; controles y 3D sincronizados en navegador. |
| C | Datos internos traseros en SD sin portátil | Contrato CAN compatible, pruebas aisladas y carga medida; versión instalada identificada y guardada. Solo marcar cobertura completa respecto al inventario de señales realmente accesibles. |
| D | Ensayo de extremo a extremo | Varias sesiones con acciones anotadas (pedal, dirección, START, marcha/modo, fallos controlados en banco); importar, reproducir y cotejar registros originales. Conservar positivos/negativos, hashes y límites. |

Probar fallos de alimentación/retirada primero con tracción inhabilitada y soporte de ensayo respaldado. Los ensayos con el kart mantienen las restricciones de reversa, dirección, brownout e IMU ya abiertas; registrar no las resuelve. Integrado por Wi-Fi, sin USB ESP32 como requisito.

No se aprueba por anticipado un binario ni se da por probada una tarjeta. Antes de instalar: identificar configuración, respaldo y build concreto con sus pruebas; separar el efecto del filtro dev10 de esta nueva función. Documentar cada entrega y ensayo en el diario conforme a la regla del proyecto.

## Decisión solicitada

Aprobar este orden **A → B → C → D**, incluyendo oscuro fijo, control global, grabación automática al encender, importación inicial por lector y reproducción en el panel existente. La descarga Wi-Fi de archivos queda como mejora posterior, condicionada a acceso físico y transporte. Soporte, ubicación y aplicación confirmados por Armando; capacidad, formato, contenido y accesibilidad pendientes, para concretar la verificación de tarjeta.

Revisión estática/documental completada; implementación y pruebas nuevas pendientes. Hallazgos enlazados en [PROB-0018](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0018_historial_sd_y_reproduccion_incompletos.md), criterio de pantalla en [DEC-0011](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0011_oscuro_fijo_y_acceso_grabacion.md). No se amplía a lectura integral de originales ni se configura una copia externa.
