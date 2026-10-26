---
estado: vigente
actualizado: 2026-10-26
resumen: Estado consolidado, configuración y siguiente paso; historia enlazada por separado.
---

# Estado actual · 26-10-2026

[Galería de pruebas de pantalla publicada](../06_TRABAJO/GALERIA_PRUEBAS_PANTALLA.md): 14 capturas F746 y galerías de banco/PC, enlazadas desde la portada; sin ensayo nuevo.

## Actualización prioritaria · 26-10-2026

[Resumen de avances al 26-10](../06_TRABAJO/AVANCES_2026-10-26.md): Armando comunica condensador de dirección entre masa y REF en lugar de masa y VCC, con pérdida aproximada de medio rango ADC. Diagnóstico declarado, sin esquema/foto ni medida antes/después aportados; reparación no acreditada. Búsqueda de IMU en curso, sin selección ni modos nuevos validados. Configuración más reciente acreditada: F746 dev12-log (ENS-0029); ESP32 v1.2-diagnostico. Las versiones inferiores describen antecedentes. [Publicación del trabajo acumulado](../06_TRABAJO/PUBLICACION_2026-10-26.md) con comprobaciones locales satisfactorias; no ensayo físico nuevo.


## Instalación frontal e historial · 23-10

**Dev12-log instalada**, oscuro fijo y botón global Grabar/Detener, autograbación microSD; conserva el filtro/curva de pedal dev10. [ENS-0029](../06_TRABAJO/ensayos/ENS-0029_instalacion_historial_sd.md): respaldo integral de dev9, dev11 instalada con fallo TX_UNDERRUN, corrección DMA dev12 y primera escritura real de 920 filas sin pérdidas ni errores SD/CAN/ADC. Continuidad/cierre comprobados: sesión 2 cerrada con 1.184 filas y sesión 3 con 989, sin pérdidas ni errores SD. Relectura final cero diferencias. Después aparecen 53 errores ADC; origen pendiente. Final Guardado/OFF/N. Usuario sin lector: archivos/replay reales pendientes. ESP32 no actualizado; última identidad v1.2-diagnostico. El plan ya fue autorizado por Armando; la biblioteca/replay y extensión CAN están en código local, con aceptación integral pendiente. Las referencias inferiores a «pendiente de aprobar» y «dev9 última instalada» quedan como antecedentes previos a esta actualización.

## Aprendizaje PID e IMU · 23-10

Fase de estudio pedida por Armando. [Guía](../04_CONOCIMIENTO/software_y_telemetria/GUIA_PID_E_IMU.md) y [simulador en vista superior](../04_CONOCIMIENTO/software_y_telemetria/simulador_pid_imu.html) con fuerzas, giróscopo y acelerómetro. Hallazgo de código: ESC v1.3 = solo P con zona muerta (deja error estacionario) y consigna sin límite de adherencia. Recomendación futura: PI con anti-windup. Simulación sintética con valores supuestos; sin cambios de firmware; IMU sigue no disponible (PROB-0017).

## Investigación de dirección · 23-10

Armando reafirma **ruido general** e indica placas sin GND extendido; solicita únicamente filtrado pasabajos (rectificación del 23-10) y posterior mejora de arquitectura, retornos y pistas. [Ampliación para el informe](../06_TRABAJO/INVESTIGACION_ADC_DIRECCION_2026-10-23.md#ampliación-para-el-informe-ruido-general-filtrado-y-placas--23-10-2026). Propuesta pendiente de caracterización/revisión PCB; preservar continua de posición al elegir filtros. Sin implementación ni eficacia medida.

Armando declara ≈1–casi 2 V en osciloscopio y sospecha recorrido limitado/divisor incorrecto. [Revisión ADC y tabla](../06_TRABAJO/INVESTIGACION_ADC_DIRECCION_2026-10-23.md): dev9 preservada y fuente local coinciden en 1042–3313 cuentas, margen 942–3413; referencia 3,3 V nominal, no medida. Divisor y causa del ruido pendientes. Fuente local identificada dev11-log, sin atribuir instalación ni validar trabajo SD; última instalación acreditada dev9. Solo investigación/documentación.

## Antecedente del plan · 22-10 (autorizado e instalado después según ENS-0029)

Armando pide retirar claro/oscuro, usar ese espacio para grabar y reproducir las pruebas en la aplicación. Confirma microSD en la STM32 de la pantalla y panel 3D del portátil; capacidad/formato pendientes. [Plan priorizado](../06_TRABAJO/PLAN_HISTORIAL_SD_Y_PANTALLA_2026-10-22.md): SD fiable/sesiones y botón global → biblioteca/replay → ampliar datos traseros → ensayo integral. **Pendiente de aprobación; solo documentación, sin cambios de código ni prueba física nueva.** [DEC-0011](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0011_oscuro_fijo_y_acceso_grabacion.md), [PROB-0018](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0018_historial_sd_y_reproduccion_incompletos.md). Grabación automática al encender es recomendación, no decisión aprobada.

## Prioridad: reversa, diferencial y ruido

Armando declara **≈50 km/h indicados en R, en los tres modos, con ruedas elevadas en banco**, y ruido electromagnético que afecta dirección/otros sensores. [Plan y diagnóstico](../06_TRABAJO/REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md), [ENS-0025](../06_TRABAJO/ensayos/ENS-0025_revision_diferencial_y_reversa.md): v1.2 conserva mando pleno en R (3/3 casos PC); dev3 corta a velocidad elevada, pero quedan dos aspectos de rampa/histéresis (5 fallos en 16 casos). El 21-10 se identificó ESP32 **v1.2-diagnostico** por Wi-Fi en ENS-0026; no atribuir causa eléctrica definitiva. [PROB-0014](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0014_reversa_y_arquitectura_diferencial.md) y [PROB-0015](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0015_ruido_electromagnetico_sensores.md) abiertos. Plan de reversa pendiente; caracterizar con osciloscopio antes de elegir filtro.

## Antecedente frontal F746 · dev9/dev10, sustituido por dev12 en ENS-0029

Última instalación verificada: **dev9-ui, 21-10**, con respaldo/relectura y pedal cero provisional hasta ADC 320; ESP32 v1.2 identificado. [ENS-0026](../06_TRABAJO/ensayos/ENS-0026_diagnostico_bloqueo_marchas.md). Armando declara mejora de cambios, menor sensibilidad y retorno a N al girar hacia un tope; dirección llegó a 813, fuera del rango admisible. **Dev10-ui** añade media ponderada y recuperación de la curva histórica desde ADC 512: compilada y probada en PC, **sin instalar**, [ENS-0027](../06_TRABAJO/ensayos/ENS-0027_pedal_media_ponderada.md). No hay prueba nueva del kart el 22-10. Brownout previo e IMU no disponible registrados en PROB-0016/0017.

### Antecedente de pantalla · 16-10

**`stm32_frontal-v1.3-dev8-ui` grabada y verificada el 16-10.** [ENS-0024](../06_TRABAJO/ensayos/ENS-0024_bms_visible_diagnostico.md): SOC y temperatura ya visibles en Resumen/BMS como interpretación provisional, con rango y caducidad de 2 s. Captura real BMS: **87 % y 22 °C**, flags de validación conservados en cero. Armando confirma que el diseño/táctil funciona bien y prefiere oscuro; claro opcional conservado. Antecedente dev7-ui, cuatro bugs corregidos, respaldo doble completo y relectura en [ENS-0023](../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md). Kart encendido con ruedas levantadas según el usuario.

Siete páginas **Resumen, Entradas, Marcha, Ruedas, CAN, SD y BMS**, temas oscuro/claro; oscuro al arrancar. Navegación disponible con motores activos, START físico y permisos de marcha conservados. Continúan siendo pantallas de desarrollo ([DEC-0010](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md)); las finales quedan para el cierre.

Cuatro defectos reproducidos y corregidos: pequeños movimientos cancelaban toques, tráfico CAN ajeno contaba como error, carrera temporal invalidaba Hall y reintento SD añadía datos sobre CSV parcial. PC: **Dev8-ui: 150/0 frontal y 163/0 dashboard**; antecedente dev7-ui: 3/0 SWD y 204/0 banco compartido incluyen frontal. F746/F411 compilados; F411 no grabado. [Implementación anterior y renders sintéticos](../06_TRABAJO/ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md).

Primera observación real dev7-ui: feedback 0x123 presente y estado 0x124 ausente; errores CAN/ADC cero. Pedal 139 cuentas físicas / 35 normalizadas, libre en esa muestra; freno sin pisar, selector D, marcha N y START OFF. Es una muestra, no calibración ni prueba de motores. BMS recibido pero oculto en ese corte dev7-ui; ahora visible como provisional en dev8-ui. SD en pausa.

Pendientes: [PROB-0012](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0012_pedal_reposo_fuera_de_pedal_libre.md), variación del pedal respecto al umbral; [PROB-0013](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md), latencia interna SD y ensayo con tarjeta; identificación del ESP32, relé/motores y rendimiento. [Franja física dañada](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md): filas 0–43; UI reserva 0–47. El nuevo diseño no repara el panel.

## Nuevo banco con pantalla

**NUCLEO-F411RE + MCUFRIEND UNO/ILI9341. Banco visual v0.3 grabado/verificado:** diez escenas adaptadas del frontal, B1 avanza al pulsar/soltar. 137 comprobaciones en F411 sin fallos, RGB 3/3 y navegación B1 real registrada. [ENS-0016](../06_TRABAJO/ensayos/ENS-0016_vistas_frontal_nucleo.md), [capturas del LCD para informe](../06_TRABAJO/evidencias/nucleo_vistas_2026-10-14/GALERIA.md), [uso](../07_DESARROLLO/firmware/nucleo_f411_banco/README.md). Datos sintéticos, placa aislada por USB; no valida CAN/motores ni TouchGFX físico del kart. Respaldo triple, herramientas y banco v0.2 conservados en [ENS-0015](../06_TRABAJO/ensayos/ENS-0015_nucleo_f411_pantalla_banco.md).

Armando confirma legibilidad y recorrido completo 10→01 sin saltos en v0.3; aceptación visual/manual declarada, vinculada en ENS-0016.

## Entrega para revisión académica

Armando solicita preparar y subir el trabajo acumulado para recibir feedback de los profesores. Portada, [dossier](REVISION_PARA_PROFESORES.md), [índice de ensayos](../06_TRABAJO/ensayos/README.md), navegación y guía de reproducción preparados. Entrega subida y pruebas de GitHub Actions satisfactorias; revisión/integración en [PR #1](https://github.com/armandos52/kart-electrico/pull/1), corte `revision-academica-2026-10-13`. Resultado trazable en [PUB-0001](../06_TRABAJO/PUBLICACION_2026-10-13.md). No implica nueva prueba física ni lectura integral de originales.

## Configuración conocida

- **Batería:** 16 celdas y 3,2–3,3 V en reposo declarados el 13-10. [Tutorial](../04_CONOCIMIENTO/bateria_y_bms/TUTORIAL_CELDAS_Y_CARGA_16S.md): SOC no deducible con precisión; techo prudente de carga propuesto 3,65 V/celda = 58,4 V, condicionado a identificación/BMS/cargador. Discrepancia 60 Ah informe / 70 Ah ficha en [PROB-0010](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0010_identificacion_y_carga_bateria_16s.md). Sin ajuste ni ensayo físico.

- **ESP32 de desarrollo: v1.3-dev3.** Conserva STOP ante STOP→START en el mismo lote CAN; [ENS-0011](../06_TRABAJO/ensayos/ENS-0011_stop_en_lote_can.md).
- **STM32 físico: v1.3-dev12-log (23-10), con filtro dev10 incluido.** ENS-0029. **ESP32 físico: v1.2-diagnostico**, identificado por Wi-Fi el 21-10; fuente dev3 sin grabar. Versiones y respaldos previos conservados.

## Siguiente trabajo técnico

- **Carga esta semana (12–18 de octubre), solicitada por Armando:** [ENS-0014](../06_TRABAJO/ensayos/ENS-0014_carga_completa_bateria_16s.md) planificado para comprobar carga completa y referencia SOC con las 16 celdas, corriente final y perfil válido. Día/hora, identidad de equipos y criterios de terminación pendientes; 100 % todavía no acreditado.

1. Completar lectura independiente y replay de los archivos reales cuando haya lector microSD; validar retirada/reinserción y ensayo prolongado. Validar por separado pedal dev10 (incluido en dev12), topes/ruido de dirección y 53 errores ADC posteriores; medir latencia GUI y contrastar BMS. PROB-0013/0018 continúan abiertos por cobertura pendiente. Oscuro fijo y botón global ya instalados.

USB ESP32 solo fuera del kart para grabación, recuperación y banco. Ensayos integrados por Wi-Fi; registrar configuración, datos originales, fallos, resultados y límites en el [diario](../06_TRABAJO/DIARIO_PARA_RELATORIO.md).

## Documentación y conservación

44 entradas catalogadas: 42 documentos históricos, propuesta personal y cuatro fotos. Extracción inicial de 1.007 páginas PDF; [lectura por lotes](../06_TRABAJO/PLAN_DE_LECTURA.md) sin cerrar. Formatos visuales y Loop pendientes. Originales y binarios conservados localmente; copia de seguridad independiente de todo el archivo pendiente.

[Historial íntegro de estados previos](HISTORIAL_ESTADO_2026-10-13.md) · [Cronología](CRONOLOGIA.md) · [Mapa](MAPA_DEL_PROYECTO.md). Las notas antiguas de “sin push” describen su fecha; la publicación actual se registra en PUB-0001.
