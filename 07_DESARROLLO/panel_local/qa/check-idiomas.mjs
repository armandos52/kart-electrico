// Idiomas del panel (es · pt-PT · en): diccionario completo, claves usadas y definidas, mensajes del servicio con
// traducción y comportamiento de t()/srv()/f(). Solo datos sintéticos y lectura de archivos; sin navegador ni kart.
// Uso: node qa/check-idiomas.mjs [salida.json]
import fs from 'node:fs';
import path from 'node:path';
import {execFileSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';

global.window = {matchMedia: () => ({matches: false})};
global.document = {getElementById: () => null};
global.localStorage = {getItem: () => null};
const qaDir = path.dirname(fileURLToPath(import.meta.url)), staticDir = path.join(qaDir, '..', 'static');
const read = name => fs.readFileSync(path.join(staticDir, name), 'utf8');
const load = async name => import('data:text/javascript;base64,' + Buffer.from(read(name)).toString('base64'));
const {TEXTOS, SERVIDOR, PATRONES} = await load('textos.js');
const m = await load('nucleo.js');
m.cargarTextos({TEXTOS, SERVIDOR, PATRONES});

const checks = [];
const check = (name, ok, detail = '') => checks.push({name, pass: !!ok, ...(ok ? {} : {detail})});
const params = text => [...text.matchAll(/\{(\w+)\}/g)].map(x => x[1]).sort().join(',');

// 1. Estructura del diccionario
const entries = Object.entries(TEXTOS);
check('Cada clave tiene tres textos no vacíos (es, pt, en)', entries.every(([, row]) => Array.isArray(row) && row.length === 3 && row.every(x => typeof x === 'string' && x.trim() && x === x.trim())),
  entries.filter(([, row]) => !(Array.isArray(row) && row.length === 3 && row.every(x => typeof x === 'string' && x.trim() && x === x.trim()))).map(([k]) => k).join(', '));
check('Los parámetros {…} coinciden en los tres idiomas', entries.every(([, row]) => params(row[0]) === params(row[1]) && params(row[1]) === params(row[2])),
  entries.filter(([, row]) => !(params(row[0]) === params(row[1]) && params(row[1]) === params(row[2]))).map(([k]) => k).join(', '));
const sameAsSpanish = entries.filter(([, row]) => row[0].length > 24 && row[1] === row[0] && row[2] === row[0]).map(([k]) => k);
check('Ningún texto largo queda sin traducir (ni en portugués ni en inglés)', sameAsSpanish.length === 0, sameAsSpanish.join(', '));
check('Servidor: filas [es, pt, en] completas y sin duplicados', SERVIDOR.every(r => r.length === 3 && r.every(x => typeof x === 'string' && x.trim())) && new Set(SERVIDOR.map(r => r[0])).size === SERVIDOR.length);
check('Patrones del servidor: regex con (.*) y $1 en pt y en', PATRONES.every(([re, pt, en]) => re instanceof RegExp && pt.includes('$1') && en.includes('$1')));

// 2. Claves usadas en el código y el HTML ↔ definidas en el diccionario
const sources = ['app.js', 'canales.js', 'graficas.js', 'vista_kart.js', 'esquema_kart.js', 'escena_kart.js', 'calibracion.js', 'nucleo.js', 'i18n.js', 'index.html'].map(name => [name, read(name)]);
const used = new Set();
for (const [, text] of sources) {
  for (const x of text.matchAll(/\bt\('([^']+)'/g)) used.add(x[1]);
  for (const x of text.matchAll(/\bt\(\s*"([^"]+)"/g)) used.add(x[1]);
  for (const x of text.matchAll(/data-i18n(?:-\w+)?="([^"$]+)"/g)) used.add(x[1]);
  for (const x of text.matchAll(/'((?:esp|graf|cal|plan|part|loc|dir|m3d|mapa|insp|scene)\.[\w.]+)'/g)) used.add(x[1]);  // claves guardadas en mapas (SPACES, TITLES…)
  for (const x of text.matchAll(/t\(\s*\w+\s*\?\s*'([^']+)'\s*:\s*'([^']+)'/g)) { used.add(x[1]); used.add(x[2]); }   // t(cond?'a':'b')
  for (const x of text.matchAll(/t\(\w+\?\s*'([^']+)'\s*:\s*'([^']+)'/g)) { used.add(x[1]); used.add(x[2]); }
}
for (const key of ['m3d.dir_estimados', 'm3d.dir_reportados', 'mapa.estimados', 'mapa.reportados', 'info.lectura_tcp', 'info.lectura_serie']) used.add(key);   // t(estimated?'a':'b') con expresión
const python = process.env.PYTHON || 'python';
const server = JSON.parse(execFileSync(python, ['-I', path.join(qaDir, 'extraer_mensajes.py')], {encoding: 'utf8', env: {...process.env, PYTHONIOENCODING: 'utf-8'}}));
// Familias de claves construidas al vuelo
const partIds = [...read('vista_kart.js').matchAll(/^\s{2}(\w+): piece\('(\w+)'/gm)].map(x => x[2]);
const families = [
  ...server.signal_keys.map(k => 'short.' + k),
  ...server.enums.map(v => 'w.' + v),
  ...partIds.flatMap(id => ['title', 'tag', 'note'].map(p => `part.${id}.${p}`)),
  ...['fl', 'fr', 'rl', 'rr'].flatMap(s => ['plan.rueda.' + s, 'plan.etq.' + s]),
  ...['esp32', 'battery', 'motor_left', 'motor_right', 'can', 'imu', 'power', 'stm32'].map(id => 'loc.' + id),
  ...['wheel_left', 'wheel_right', 'steering_wheel'].map(v => 'dir.' + v),
];
for (const k of [...used]) if (k.endsWith('.')) used.delete(k);   // prefijos de claves construidas al vuelo (short., w., loc.…)
check('El código usa claves que están todas definidas', [...used].every(k => k in TEXTOS), [...used].filter(k => !(k in TEXTOS)).join(', '));
const familyExpected = families.filter(k => !k.startsWith('short.'));
check('Familias dinámicas completas (w.*, part.*, plan.*, loc.*, dir.*)', familyExpected.every(k => k in TEXTOS), familyExpected.filter(k => !(k in TEXTOS)).join(', '));
check('12 piezas con título, etiqueta y nota', partIds.length === 12, String(partIds.length));
const shortKeys = Object.keys(TEXTOS).filter(k => k.startsWith('short.')).map(k => k.slice(6));
check('Nombres cortos: las 48 señales del catálogo de presentación, todas con su clave de telemetría', shortKeys.length === 48 && shortKeys.every(k => k === 'steer_view' || server.signal_keys.includes(k)), shortKeys.filter(k => k !== 'steer_view' && !server.signal_keys.includes(k)).join(', ') + ' / ' + shortKeys.length);
const reachable = new Set([...used, ...families]);
const unused = Object.keys(TEXTOS).filter(k => !reachable.has(k) && !k.startsWith('short.'));
check('No sobran claves en el diccionario', unused.length === 0, unused.join(', '));

// 3. El español incrustado en HTML (index.html y la plantilla de calibración) coincide con el diccionario
function staticSpanish(name, text) {
  const problems = [];
  for (const tag of text.matchAll(/<(\w+)([^<>]*?\sdata-i18n(?:-\w+)?="[^"]+"[^<>]*)>/g)) {
    const attrs = tag[2];
    for (const x of attrs.matchAll(/data-i18n-(aria|title|ph|alt)="([^"]+)"/g)) {
      const attr = {aria: 'aria-label', title: 'title', ph: 'placeholder', alt: 'alt'}[x[1]];
      const value = attrs.match(new RegExp(`\\s${attr}="([^"]*)"`))?.[1];
      if (value !== undefined && value !== TEXTOS[x[2]]?.[0]) problems.push(`${name} ${attr} ${x[2]}`);
    }
    const key = attrs.match(/\sdata-i18n="([^"]+)"/)?.[1];
    if (key && !key.includes('${')) {
      const rest = text.slice(tag.index + tag[0].length), inner = rest.slice(0, rest.indexOf('<'));
      if (inner && inner !== TEXTOS[key]?.[0]) problems.push(`${name} texto ${key}: «${inner}»`);
    }
  }
  return problems;
}
const html = staticSpanish('index.html', read('index.html')), cal = staticSpanish('calibracion.js', read('calibracion.js'));
check('Los textos en español del HTML coinciden con el diccionario', html.length === 0, html.join(' | '));
check('Los textos en español de la plantilla de calibración coinciden con el diccionario', cal.length === 0, cal.join(' | '));
check('El selector de idioma tiene tres botones ES · PT · EN', ['es', 'pt', 'en'].every(c => read('index.html').includes(`data-lang="${c}"`)));
check('Español por defecto: aria-pressed solo en ES', /data-lang="es"[^>]*aria-pressed="true"/.test(read('index.html')) && /data-lang="pt"[^>]*aria-pressed="false"/.test(read('index.html')) && /data-lang="en"[^>]*aria-pressed="false"/.test(read('index.html')));

// 4. Mensajes del servicio: señales, unidades, errores, eventos y guía de calibración
const exact = new Map(SERVIDOR.map(r => [r[0], r]));
const matches = text => exact.has(text) || PATRONES.some(([re]) => re.test(text));
check('Todas las etiquetas/unidades de SIGNALS tienen traducción', server.signals.every(x => exact.has(x)), server.signals.filter(x => !exact.has(x)).join(' | '));
check('Todos los mensajes del servicio (ValueError, eventos, guía, interrupciones) tienen traducción', server.messages.every(matches), server.messages.filter(x => !matches(x)).join(' | '));
check(`Cobertura: ${server.signals.length} etiquetas y ${server.messages.length} mensajes del servicio`, true);

// 5. Comportamiento
const setLang = (code, locale) => { m.idioma.code = code; m.idioma.locale = locale; };
setLang('es', 'es-ES');
check('es: t() devuelve el texto original', m.t('banco') === 'Banco preparado · tracción inhabilitada' && m.t('diag.can_wait', {n: 3}) === 'CAN sin inicializar · 3 intentos. El control sigue esperando al CAN.');
check('es: srv() no cambia el texto del servicio', m.srv('Pedal procesado') === 'Pedal procesado' && m.srv('Valor inválido: x') === 'Valor inválido: x');
check('t() con clave desconocida devuelve la clave; parámetro ausente queda visible', m.t('no.existe') === 'no.existe' && m.t('cal.ronda_n') === 'Ronda {n} de 3');
check('word() traduce valores conocidos y devuelve undefined si no los conoce', m.word('D') === 'Adelante' && m.word('inventado') === undefined);
setLang('pt', 'pt-PT');
check('pt: t() y parámetros', m.t('leer.cable') === 'Ler por cabo' && m.t('cal.ronda_n', {n: 2}) === 'Ronda 2 de 3' && m.t('estado.antiguo', {edad: '3 s'}) === 'Antigo · há 3 s');
check('pt: srv() exacto, unidades y patrones anidados', m.srv('Pedal procesado') === 'Pedal processado' && m.srv('cuentas') === 'contagens'
  && m.srv('Lectura abierta en COM4; sin TX al micro') === 'Leitura aberta em COM4; sem TX para o micro'
  && m.srv('Telemetría rechazada: Valor inválido: rpm_fl') === 'Telemetria rejeitada: Valor inválido: rpm_fl'
  && m.srv('Mensaje que no existe') === 'Mensaje que no existe' && m.srv('') === '' && m.srv(undefined) === undefined);
check('pt: valores de texto y estados binarios', m.word('D') === 'Em frente' && m.word('N') === 'Ponto morto' && m.display({quality: 'fresh', value: 255}, 'mode_applied') === 'Sem tração'
  && m.display({quality: 'fresh', value: 1}, 'rearm_required') === 'Sim / 1' && m.display({quality: 'fresh', value: 'can_timeout'}, 'drive_inhibit') === 'Ordem CAN expirada');
check('pt: números con el formato de Portugal', m.f(1234.5, 1) === (1234.5).toLocaleString('pt-PT', {maximumFractionDigits: 1}) && m.f(3.25, 1) === '3,3');
setLang('en', 'en-GB');
check('en: t(), srv() y word()', m.t('leer.wifi') === 'Read over Wi-Fi' && m.srv('Ensayo no encontrado') === 'Run not found' && m.word('R') === 'Reverse' && m.display({quality: 'fresh', value: 0}, 'brake') === 'No / 0');
check('en: números con punto decimal y separador de miles', m.f(1234.5, 1) === '1,234.5' && m.f(3.25, 1) === '3.3');
check('Cada idioma da un título distinto', new Set([0, 1, 2].map(i => TEXTOS['titulo'][i])).size === 3);

// activeSignals y steeringPresentation traducen lo que llega del servicio
m.app.backendOK = true;
m.app.stateReceivedAt = Date.now();
m.app.state = {signals: {accelerator_raw: {label: 'Acelerador recibido por CAN', unit: 'cuentas', value: 1200, quality: 'fresh', age_ms: 0, node: 'esp32'},
  steer_raw: {label: 'Dirección recibida por CAN', unit: 'cuentas', value: 2047, quality: 'fresh', age_ms: 0, node: 'esp32'}},
  calibration: {active: {id: 'a'}, calibrated: {outside: false, angle: null, percent: 12, target: 'wheel_left'}}};
setLang('pt', 'pt-PT');
const translated = m.activeSignals();
check('pt: activeSignals traduce etiqueta y unidad', translated.accelerator_raw.label === 'Acelerador recebido por CAN' && translated.accelerator_raw.unit === 'contagens');
check('pt: dirección calibrada con etiquetas traducidas', m.steeringPresentation().label === 'Curso calibrado · PC');
m.app.state.calibration.calibrated = {outside: false, angle: 5, percent: 20, target: 'wheel_left'};
check('pt: ángulo interpolado indica la rueda', m.steeringPresentation().label === 'Ângulo interpolado · roda E');
setLang('en', 'en-GB');
check('en: ángulo interpolado y fuera de rango', m.steeringPresentation().label === 'Interpolated angle · wheel L'
  && (m.app.state.calibration.calibrated = {outside: true, angle: null, percent: 0, target: 'wheel_left'}, m.steeringPresentation().label) === 'Out of range');
setLang('es', 'es-ES');
check('es: la dirección sigue como antes', (m.app.state.calibration.calibrated = {outside: false, angle: 5, percent: 20, target: 'steering_wheel'}, m.steeringPresentation().label) === 'Ángulo interpolado · volante');

const failed = checks.filter(c => !c.pass);
const result = {scope: 'Prueba automática de idiomas del panel; datos sintéticos, sin hardware', status: failed.length ? 'FAIL' : 'PASS',
  claves: entries.length, servidor: SERVIDOR.length, patrones: PATRONES.length, comprobaciones: checks.length, checks};
if (process.argv[2]) fs.writeFileSync(process.argv[2], JSON.stringify(result, null, 2));
for (const c of checks) console.log(`${c.pass ? 'ok  ' : 'FAIL'} ${c.name}${c.pass ? '' : '\n       → ' + c.detail}`);
console.log(`${checks.length - failed.length}/${checks.length} comprobaciones; ${entries.length} claves, ${SERVIDOR.length} mensajes del servicio, ${PATRONES.length} patrones. ${result.status}`);
if (failed.length) process.exitCode = 1;
