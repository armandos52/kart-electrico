---
estado: validación local terminada; instalación y ensayo físico pendientes
actualizado: 2026-10-16
resumen: Implementación del dashboard de desarrollo F746 con siete páginas y temas claro/oscuro.
---

# ENS-0022 · Dashboard frontal claro y oscuro · 15–16-10-2026

## Objetivo y autorización

Implementar la [propuesta de pantalla](../PROPUESTA_PANTALLA_FRONTAL_2026-10-15.md) aceptada por Armando, manteniendo el propósito de desarrollo de DEC-0010. Combina «Estado y causa» como resumen y el mapa de ruedas como página adicional. El usuario solicita ambos temas y declara que el kart está suspendido, por lo que puede hacer pruebas con motores y velocidad. Es una **declaración de configuración**, no una comprobación física de esta sesión.

Al consultar disponibilidad de F746/ST-LINK y operador, responde **«Todavía no; avanza con el código»**. Se completa implementación y verificación local; no se conecta, graba ni acciona el kart. Después confirma que otro chat había editado el firmware y pide **«descarta eso»**: sus cambios se conservan en `descartado_otro_chat/` y se retoma la base dev5 preservada al comenzar. No se mezclan sus resultados con los de este ensayo.

## Configuración e identidad

- PC Windows, ARM GNU 14.3.Rel1, MSVC 2022; TouchGFX 4.26.0 y dependencias existentes. Fuentes sobre HEAD `bebe4528747bc6e05dc1ba244cd3ce9a090a7942` con trabajo previo local dev5 conservado.
- Nueva fuente y paquete: **`stm32_frontal-v1.3-dev6-ui`**. [ELF/HEX/mapa y hashes](../evidencias/frontal_dashboard_2026-10-15/README.md). No hay commit/push nuevo ni copia independiente.
- F746 física conocida: dev4 de ENS-0019; ESP32 físico sin identificar (0x123 observado, 0x124 no observado en aquel ensayo). Fuente trasera dev3 sin cambios. F411 conocido en placa: v0.3; su build local 0.4-dev sirve solo para regresión de código compartido.
- LCD nominal 480 × 272. Reserva de filas 0–47, por daño físico documentado en ENS-0019; dibujo e interacción nuevos solo entre y=48 y y=271.

## Cambios

| Página | Información y acciones |
|---|---|
| Resumen | Velocidad recibida válida, marcha frontal, modo solicitado/recibido/aplicado, petición START, freno, SD y BMS. Tocar «Solicitado» pide el siguiente modo. |
| Entradas | ADC físico/normalizado de pedal y dirección, barras, umbral de pedal libre, errores y pines de selector/START. |
| Marcha | Condiciones usadas realmente por el frontal para D/R: entradas, CAN, freno, pedal y parada. Distingue condiciones de entrada de una marcha ya seleccionada. |
| Ruedas | Cuatro RPM y selección táctil; edad/validez de Hall delantero, origen de datos y giro MPU condicionado a validez. |
| CAN | Edades 0x123/0x124, errores, relé lógico y sentido informado por ESP. No equivale a medida eléctrica del contactor. |
| SD | Petición frente a estado real, filas, último error y botón grabar/pedir pausa. Conserva correcciones dev5 y limitación del driver. |
| BMS | SOC/temperatura solo con sus flags válidos; bytes y edades 0x355/0x356. No inventa celdas, tensión ni corriente. |

Cabecera común con causa del ESP y aviso de interacción en líneas separadas. Cambiar tema está siempre accesible; arranca oscuro, selección mantenida en RAM hasta reiniciar. Navegación disponible con velocidad/motores activos; no se añade una condición de parada para consultar páginas o cambiar tema. START sigue siendo el pulsador físico. Los botones de modo/SD emiten solo la solicitud existente, una vez al soltar en la misma zona; cancelar/arrastrar descarta el gesto.

`kart_dashboard.c` genera las primitivas de todas las páginas; `KartDashboard.cpp` las adapta a TouchGFX. La View retira los widgets anteriores de su contenedor, evitando botones transparentes superpuestos. Fuentes DejaVu Sans antialias, tamaños 12/13/20/32 px; TTF, licencia, generador y métricas conservados. Glifos alpha8: 84.511 bytes. No se modifica código generado por Designer.

Se añaden al final de `KartUiSnapshot` edades/validez de Hall y condiciones/pines usados por control, sin alterar offsets previos. Tamaño de 96 a 116 bytes, comprobado en ELF ARM y mediante aserciones de compilación en PC. `observar_swd.py` admite ambos tamaños, rechaza otros. El registrador dedicado del pedal obtiene tamaño/offsets del ELF y conserva sus campos anteriores. **CSV SD sigue en esquema 2**: las nuevas extensiones diagnósticas están disponibles en snapshot/SWD, no son columnas nuevas de ese CSV.

No se cambian calibración, umbral libre (40/4095 normalizado), permisos de marcha, START, control trasero ni formato CAN. La regresión reproduce ADC físico 170 → normalizado 108 → pedal no libre → D bloqueado; es una entrada sintética basada en el rango de observaciones previo, no una medición nueva ni calibración. También comprueba que D ya seleccionado conserva marcha con movimiento, freno liberado y pedal accionado. PROB-0012 continúa abierto.

## Procedimiento y resultados locales

1. Preservar base y cambios concurrentes; integrar UI y extensiones de diagnóstico sin editar originales.
2. Compilar arnés frontal con ADC/GPIO/CAN/Hall/FatFs simulados y núcleo real ESP32: **130 comprobaciones, 0 fallos**. Incluye diez nuevas de observabilidad.
3. Compilar/dibujar función C del dashboard: **139 comprobaciones, 0 fallos**; 120 escenas sintéticas (seis configuraciones, dos temas, siete páginas y cuatro selecciones en Ruedas), todas dentro del área útil y con glifos/textos completos. Comprueba causa más aviso, ausencia/caducidad, SD petición/estado y eventos táctiles.
4. Decodificación SWD: **3 pruebas, 0 fallos**, para snapshot antiguo, ampliado y tamaños desconocidos. ELF ARM confirma offsets 96/104 y total 116 bytes.
5. Banco compartido en PC: **184 comprobaciones, 0 fallos**, que incluyen las 130 frontales; no sumarlas como independientes. Compilación F411 **SUCCESS**: 63.828 bytes flash y 2.304 bytes RAM estática, sin upload.
6. Compilar F746: **BUILD OK**, ELF y HEX con regiones interna/QSPI. Los avisos preexistentes de HAL/middleware constan en log; no hay error de compilación/enlace final.
7. Renderizar usando geometría C y los mismos mapas de glifos; revisar visualmente las siete páginas en ambos temas y el resumen de marcha. [Galería](../evidencias/frontal_dashboard_2026-10-15/GALERIA.md). Son renders de PC, no capturas del firmware ejecutándose en LCD.

Las suites diferentes cubren contratos distintos; estas comprobaciones no son pruebas de dinámica, transporte eléctrico, sensores ni motores. La compilación demuestra enlace y límites estáticos, no uso máximo de pila, tiempo de pintura, carga, jitter o latencia táctil.

## Resultados negativos y correcciones

- Generador inicial dependía de `fontTools`, no disponible. Se sustituye por lectura focalizada de la tabla `name` del TTF con biblioteca estándar; Pillow disponible en runtime local. No se instala una dependencia nueva.
- Primer build falla porque `invalidateRect` requiere un `Rect` mutable. Segundo, porque `Screen` no expone `removeAll`: se usa su contenedor. Logs conservados; build posterior correcto.
- Primera regresión frontal: **130 comprobaciones, 1 fallo**. El sufijo `-ui` hacía truncar el aviso largo en el formateador antiguo compartido con F411. Se compacta a `MODO:PARAR/SOLTAR PEDAL`, preservando ambas condiciones; final 130/0 y banco 184/0.
- Primer build F411 no pudo escribir la caché de PlatformIO fuera del workspace. Se repite con acceso de herramienta al directorio de caché: **SUCCESS**, sin grabación; no es fallo del firmware ni rechazo de revisión automática.
- Compilación conserva advertencia conservadora de posible truncamiento en `kart_display.c` (capacidad del buffer de versión frente a longitud real); las regresiones comprueban versión, contador máximo y aviso completo de esta versión. El arnés de renders en MSVC avisa sobre `fopen` estándar usado para escribir su JSONL local. No se confunden advertencias con errores o pruebas físicas.

## Instalación y siguiente ensayo

Pendiente por disponibilidad del usuario: identificar la imagen que siga instalada y conservar respaldo antes de grabar el HEX completo (interna + QSPI). Verificar después versión `v1.3-dev6-ui`, ambas paletas, siete pestañas, toque/cancelación y franja reservada. Medir carga GUI/control, respuesta táctil y SD con/sin tarjeta. La preferencia de tema no se escribe en flash.

Para investigar la marcha, registrar primero entradas y ADC del pedal con el ELF correspondiente. Armando autoriza pruebas reales con el kart suspendido, pero esta sesión no las ejecuta. Identificar ESP32 y registrar el ensayo integrado por **Wi-Fi**; no exigir USB del ESP32 instalado. Comprobar petición START, relé real, D/R, modos y giro solo cuando estén conectados y atendidos. El nuevo diseño permite observar esas pruebas; no garantiza que haya desaparecido el bloqueo del pedal ni que el ESP esté actualizado.

SD: el bloqueo de `f_mount` (~30 s en ENS-0019) sigue sin resolver y la pausa puede quedar pendiente hasta retornar el driver. Hall 4 PPR histórico y calibración física pendientes. Un Hall delantero ausente/caducado se muestra `--`, no cero medido. Con ESP legado, las RPM traseras proceden del feedback disponible sin validez Hall independiente. BMS continúa condicionado a validar protocolo. No se cierran PROB-0012/0013 ni DEC-0010.

Cobertura documental focalizada L4: estado/mapa, catálogo/notas anteriores, propuesta, fuentes propias y APIs locales de TouchGFX necesarias. No se abren nuevas páginas originales, no se cierra lectura integral y siguen pendientes formatos visuales/Loop.
