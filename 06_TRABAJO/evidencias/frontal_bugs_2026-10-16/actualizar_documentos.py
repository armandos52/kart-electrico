"""Actualizar entradas tras Verified OK y captura real de arranque de ENS-0023."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]

def change(path, fn):
    p = ROOT/path
    text = p.read_text(encoding='utf-8-sig')
    p.write_text(fn(text), encoding='utf-8')

def paragraph(text, start, replacement):
    a = text.index(start)
    b = text.find('\n\n', a)
    return text[:a] + replacement + text[b if b >= 0 else len(text):]

def state(s):
    a = s.index('## Frontal F746')
    b = s.index('## Nuevo banco con pantalla', a)
    s = s[:a] + '''## Frontal F746 · dev7-ui instalada

**`stm32_frontal-v1.3-dev7-ui` grabada y verificada el 16-10**, tras respaldo doble de interna 1 MiB y QSPI 16 MiB e identificación exacta del dev4 anterior. [ENS-0023](../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md), [respaldo](../07_DESARROLLO/respaldos/stm32_f746/2026-10-16/RESPALDO.md). Armando declara kart encendido y ruedas levantadas y autoriza instalar. Arranque y framebuffer real observados; detalle de validación física en el ensayo.

Siete páginas **Resumen, Entradas, Marcha, Ruedas, CAN, SD y BMS**, temas oscuro/claro; oscuro al arrancar. Navegación disponible con motores activos, START físico y permisos de marcha conservados. Continúan siendo pantallas de desarrollo ([DEC-0010](../05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0010_pantallas_desarrollo_y_usuario_final.md)); las finales quedan para el cierre.

Cuatro defectos reproducidos y corregidos: pequeños movimientos cancelaban toques, tráfico CAN ajeno contaba como error, carrera temporal invalidaba Hall y reintento SD añadía datos sobre CSV parcial. PC: **150/0 frontal, 145/0 dashboard, 3/0 SWD; 204/0 banco compartido incluyen frontal**. F746/F411 compilados; F411 no grabado. [Implementación anterior y renders sintéticos](../06_TRABAJO/ensayos/ENS-0022_dashboard_frontal_claro_oscuro.md).

Primera observación real dev7-ui: feedback 0x123 presente y estado 0x124 ausente; errores CAN/ADC cero. Pedal 139 cuentas físicas / 35 normalizadas, libre en esa muestra; freno sin pisar, selector D, marcha N y START OFF. Es una muestra, no calibración ni prueba de motores. BMS recibido pero datos no validados, ocultos. SD en pausa.

Pendientes: [PROB-0012](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0012_pedal_reposo_fuera_de_pedal_libre.md), variación del pedal respecto al umbral; [PROB-0013](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md), latencia interna SD y ensayo con tarjeta; identificación del ESP32, relé/motores y rendimiento. [Franja física dañada](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0004_franja_blanca_pantalla_frontal.md): filas 0–43; UI reserva 0–47. El nuevo diseño no repara el panel.

''' + s[b:]
    s = paragraph(s, '- **STM32: v1.3-dev4 grabado', '- **STM32: v1.3-dev7-ui grabado en F746 (16-10).** [ENS-0023](../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md). ESP32 de desarrollo continúa dev3, sin grabar; anterior dev4 y firmware original conservados.')
    s = paragraph(s, '1. Cuando haya placa y operador:', '1. Validar manualmente ambos temas y navegación de dev7-ui, medir carga/latencia y completar pedal dedicado con su ELF (ENS-0020/PROB-0012). Identificar ESP32 por Wi-Fi. Probar SD con/sin tarjeta y START/relé/motores con operador en el kart suspendido declarado. PROB-0013 sigue abierto por latencia del driver y pruebas pendientes.')
    return s

change('00_INICIO/ESTADO_ACTUAL.md', state)

def tasks(s):
    s = paragraph(s, '- [ ] Instalar y validar **dev6-ui**', '- [ ] Completar validación manual de **dev7-ui instalada**: siete páginas, ambos temas, táctil y rendimiento. [ENS-0023](ensayos/ENS-0023_bugs_e_instalacion_dashboard.md): respaldo doble, programación verificada y arranque real; usuario declara kart encendido con ruedas levantadas. Pruebas integradas por Wi-Fi, sin exigir USB del ESP32 instalado.')
    s = s.replace('Calibrar el reposo y el recorrido del pedal con dev4 instalado', 'Calibrar el reposo y el recorrido del pedal con dev7-ui instalada')
    s = paragraph(s, '- [ ] Probar las correcciones SD dev5,', '- [ ] Probar SD con/sin microSD en **dev7-ui instalada** y resolver detección/latencia del driver ([PROB-0013](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0013_sd_sin_tarjeta_bloqueo_y_error_invisible.md)). Incluye reintentos/pausa de ENS-0021 y rechazo conservador de CSV parcial de ENS-0023; no hay ensayo físico de escritura con tarjeta en este corte.')
    s = s.replace('Usar ELF dev4 conservado si sigue instalado;', 'Usar ELF dev7-ui de ENS-0023, que corresponde al instalado;')
    marker = '- [x] Implementar la ampliación de pantalla'
    a = s.index(marker)
    s = s[:a] + '- [x] Corregir cuatro fallos reproducidos e instalar dashboard **dev7-ui** en la F746: respaldo doble, verificación y arranque real. [ENS-0023](ensayos/ENS-0023_bugs_e_instalacion_dashboard.md). Validación manual y pruebas integradas pendientes arriba.\n' + s[a:]
    return s

change('06_TRABAJO/TAREAS.md', tasks)

def fwreadme(s):
    s = s.replace('# Frontal STM32 · v1.3-dev6-ui local', '# Frontal STM32 · v1.3-dev7-ui')
    s = paragraph(s, '**Fuente actual dev6-ui compilada, sin grabar**', '**Dev7-ui grabada y verificada en la F746 el 16-10**, tras respaldo doble e identificación byte a byte de dev4. [ENS-0023](../../../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md): arranque real, framebuffer y CAN observados; cuatro fallos corregidos (jitter táctil, falsos errores CAN, edad Hall y append a CSV parcial). PC: 150 comprobaciones frontales, 145 de dashboard y 3 del decodificador SWD sin fallos; 204 con banco compartido incluyen frontal. Conserva siete páginas y ambos temas de ENS-0022 y correcciones SD dev5 de ENS-0021. Calibración del pedal y latencia interna SD pendientes; consultar el ensayo para cobertura física exacta.')
    s = s.replace('Pantalla dev6-ui:', 'Pantalla dev7-ui:').replace('LCD/táctil, brillo y tiempos reales pendientes.', 'Arranque y framebuffer real observados en ENS-0023; validación manual, brillo y tiempos según cobertura del ensayo.')
    s = s.replace('Snapshot dev6-ui:', 'Snapshot dev6-ui/dev7-ui:').replace('y dev6-ui de 116;', 'y dev6-ui/dev7-ui de 116;')
    a = s.index('## Compilación local')
    s = s[:a] + 'Dev7-ui admite jitter táctil hasta 8 px por eje dentro de la zona inicial; arrastrar fuera o cancelar impide activar. Antes de añadir a un CSV existente verifica cabecera/columnas esquema 2 y final de línea. Error 128: conservar/apartar el archivo incompatible y usar uno nuevo; no lo borra ni repara. No detecta toda corrupción ni resuelve el bloqueo del driver. CAN ignora tráfico ajeno antes de validar DLC; Hall toma el reloj junto a la copia protegida de sus datos.\n\n' + s[a:]
    return s

change('07_DESARROLLO/firmware/stm32_frontal/README_ROBUSTEZ.md', fwreadme)

def versions(s):
    marker = '## Frontal dev6-ui local'
    a = s.index(marker)
    s = s[:a] + '''## Frontal dev7-ui instalada · 16-10-2026

**`stm32_frontal-v1.3-dev7-ui` grabada, Verified OK y arranque real observado.** [ENS-0023](../../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md): respaldo doble de interna/QSPI e identidad dev4 anterior confirmada. Cuatro correcciones reproducidas: jitter táctil, contador CAN, edad Hall y protección de CSV parcial. Conserva diseño de siete páginas claro/oscuro, control y calibración. PC 150/0 frontal, 145/0 dashboard, 3/0 SWD y 204/0 compartidas; F746 y F411 compilados, F411 sin grabar. HEX SHA-256 `0aa8b6b7d38791dc4b487092c6f2d07ce0e4aaa11f41c07afddf280cdd503740`; [paquete, logs y capturas](../../06_TRABAJO/evidencias/frontal_bugs_2026-10-16/README.md). ESP32 físico sin identificar, PROB-0012/0013 abiertos. Sin commit/push nuevo; cobertura física exacta en ensayo.

''' + s[a:]
    return s.replace('**Instalado ahora en la F746: STM32 v1.3-dev4**', '**Instalado el 15-10 en la F746: STM32 v1.3-dev4**')

change('07_DESARROLLO/firmware/VERSIONES.md', versions)

def bank(s):
    a = s.index('**Fuentes de desarrollo al 16-10:')
    b = s.index('\n\n', a)
    return s[:a] + '**Fuente 0.4-dev al 16-10, compilada sin grabar:** consume núcleo frontal dev7-ui; 204 comprobaciones PC sin fallos (150 frontales + 54 visuales), flash 68.160 B y RAM estática 2.328 B. [ENS-0023](../../../06_TRABAJO/ensayos/ENS-0023_bugs_e_instalacion_dashboard.md) conserva BIN/ELF/hash. El dashboard de siete páginas pertenece a la F746; este banco conserva sus escenas. Última imagen instalada F411 sigue v0.3. Antecedente dev6-ui en ENS-0022.' + s[b:]

change('07_DESARROLLO/firmware/nucleo_f411_banco/README.md', bank)
print('Estado, tareas, versiones y README actualizados; consultar ensayo para cobertura física.')
