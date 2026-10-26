// Estado compartido y presentación de señales. Sin lógica de control: solo lectura de /api/state.
export const $ = id => document.getElementById(id);
export const app = {state:null, token:'', demo:false, backendOK:false, stateReceivedAt:0, demoFrame:null, demoSeq:0, motionPaused:false, wheelScale:1};
export const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
// Idioma de la interfaz. El estado vive aquí (sin imports) para poder cargar este módulo en pruebas de Node;
// i18n.js le entrega el diccionario (cargarTextos), gestiona el selector y aplica los textos al DOM.
export const idioma = {code:'es', locale:'es-ES', textos:{}, servidor:new Map(), patrones:[]};
const COLUMNA = {es:0, pt:1, en:2};
export function cargarTextos({TEXTOS, SERVIDOR, PATRONES}) {
  idioma.textos=TEXTOS;idioma.servidor=new Map(SERVIDOR.map(row=>[row[0],row]));idioma.patrones=PATRONES;
}
export const hasText = key => Object.hasOwn(idioma.textos,key);
export function t(key, params) {
  const entry=idioma.textos[key];
  const text=entry?entry[COLUMNA[idioma.code]]||entry[0]:key;
  return params?text.replace(/\{(\w+)\}/g,(match,name)=>name in params?String(params[name]):match):text;
}
// Mensajes del servicio local (siempre en español): coincidencia exacta o patrón con parte variable.
export function srv(text) {
  if(typeof text!=='string'||!text||idioma.code==='es')return text;
  const column=COLUMNA[idioma.code], row=idioma.servidor.get(text);
  if(row)return row[column];
  for(const pattern of idioma.patrones){const match=pattern[0].exec(text);if(match)return pattern[column].replace('$1',srv(match[1]));}
  return text;
}
// Valores de texto de la telemetría (marcha, modo, fase, bloqueo); undefined si no hay traducción.
export const word = value => hasText('w.'+value)?t('w.'+value):undefined;
const BINARY = new Set(['brake','relay1','relay2','charger_connected','sd_ready','emergency_stop','wifi_ap','mpu_ok','control_ready','can_init_ok','start_requested']);
for(const key of ['rearm_required','reverse_limited','speed_valid','frontend_health_seen','frontend_health_ok','watchdog_ok'])BINARY.add(key);
export const TEXT = new Set(['gear','mode','control_state','drive_direction','drive_inhibit']);

export function f(value, digits=1) { return Number(value).toLocaleString(idioma.locale,{maximumFractionDigits:digits}); }
export function activeSignals() {
  if(app.demo)return app.demoFrame?.signals || {};
  const elapsed=Date.now()-app.stateReceivedAt;
  const signals=Object.fromEntries(Object.entries(app.state?.signals || {}).map(([key,s])=>{
    const age=s.age_ms===null?null:s.age_ms+elapsed;
    return [key,{...s,label:srv(s.label),unit:srv(s.unit),age_ms:age,quality:s.quality==='fresh'&&age>(s.stale_after_ms||1500)?'stale':s.quality}];
  }));
  if(signals.can_rx_age_ms?.quality==='fresh' && signals.can_rx_age_ms.value>100) {
    for(const key of ['accelerator_raw','accelerator_pct','steer_raw','steer_deg','brake','gear','mode']) {
      if(signals[key]?.node==='esp32' && signals[key].quality==='fresh')signals[key]={...signals[key],quality:'stale',age_ms:Math.max(signals[key].age_ms,signals.can_rx_age_ms.value)};
    }
  }
  return signals;
}
export function fresh(key) { const s = activeSignals()[key]; return s?.quality === 'fresh' ? s.value : null; }
// Rodadura común a ambas vistas. Frente -Z (3D) / arriba (SVG).
// La parte superior del neumático avanza hacia el frontal: Rx negativa en D.
// Giro REAL: velocidad angular = RPM·2π/60 de cada rueda (app.wheelScale=1). A 50 km/h
// (Ø 0,48 m) son ~9 vueltas por segundo: a 30–60 fps los tacos parpadean o parecen girar al
// revés, así que el dibujo no se acelera «de verdad» sin más: wheelBlur() desvanece los detalles
// y alarga la estela de la marca, como el desenfoque de movimiento de una cámara real.
// No normalizar por la rueda más rápida: ocultaría la aceleración y acoplaría I/D.
// app.wheelScale=.03 reproduce el modo lento anterior (giro nítido y 33 veces más lento).
export const EXPOSURE = 1/30;          // exposición simulada del desenfoque, en segundos
const REV = Math.PI*2, BLUR_START = .35, BLUR_FULL = 1.8;   // vueltas/s: nítido → sin detalles
export function wheelRates(signals) {
  const sides=['fl','fr','rl','rr'];
  const value=key=>signals[key]?.quality==='fresh'?signals[key].value:null;
  const direction=value('drive_direction')||value('gear');
  const scale=REV/60*app.wheelScale;
  return Object.fromEntries(sides.map(side=>{
    const s=signals['rpm_'+side], rpm=s?.quality==='fresh'&&Number.isFinite(s.value)?Math.abs(s.value):0;
    return [side,(direction==='R'?1:-1)*rpm*scale];
  }));
}
export function wheelMotion(signals, dt) {
  const step=Number.isFinite(dt)?Math.min(Math.max(dt,0),.1):0, rates=wheelRates(signals);
  return Object.fromEntries(Object.entries(rates).map(([side,rate])=>[side,rate*step]));
}
// Desenfoque para una rueda que gira a `rate` rad/s: detail = visibilidad de tacos/pernos/radios
// (1 nítido → 0 borroso), arc = giro de la marca durante la exposición, que forma su estela.
export function wheelBlur(rate) {
  const speed=Number.isFinite(rate)?Math.abs(rate):0, revs=speed/REV;
  const t=Math.min(1,Math.max(0,(revs-BLUR_START)/(BLUR_FULL-BLUR_START)));
  return {detail:1-t*t*(3-2*t), arc:Math.min(REV*.97,speed*EXPOSURE)};
}
export function display(s, key) {
  if (!s || s.quality !== 'fresh') return '—';
  if(key==='mode_applied')return ({0:'DIF',1:'ESC',2:'BLQ',255:t('modo.sin_traccion')})[s.value]??t('modo.desconocido');
  if (typeof s.value === 'string') return word(s.value) || s.value;
  if (BINARY.has(key)) return s.value ? t('bin.si') : t('bin.no');
  return f(s.value) + (s.unit ? ' ' + s.unit : '');
}
export function steeringPresentation() {
  const degrees = fresh('steer_deg'), raw = fresh('steer_raw');
  const cal=app.state?.calibration;
  if(!app.demo&&cal?.active&&cal.calibrated&&raw!==null&&app.backendOK){
    const c=cal.calibrated, value=c.outside?null:(c.angle??c.percent);
    return {angle:c.angle,value,unit:c.angle===null?'%':'°',percent:c.percent,estimated:true,calibrated:true,
      outside:c.outside,target:c.target,label:c.outside?t('dir.fuera'):c.angle===null?t('dir.recorrido'):t('dir.interpolado',{objetivo:t('dir.'+c.target)}),
      // El esquema solo representa sentido/recorrido; no geometría medida de las dos ruedas.
      mapAngle:c.outside?0:Math.max(-25,Math.min(25,c.percent*.25))};
  }
  if(degrees !== null)return {angle:Math.max(-25,Math.min(25,degrees)),estimated:false};
  if(raw !== null)return {angle:Math.max(-20,Math.min(20,(raw-2047)/47.9)),estimated:true};
  return {angle:null,estimated:false};
}
// Valores que merecen color (ISA-101: gris cuando todo es normal). Criterio de presentación, no de seguridad.
export function anomaly(key, value) {
  if(value===null || value===undefined)return null;
  if(key==='emergency_stop')return value===1?'alarma':null;
  if(key==='can_rx_age_ms')return value>100?'alarma':null;
  if(key==='drive_inhibit')return value==='can_timeout'?'alarma':['none','pedal_zero','neutral','brake'].includes(value)?null:'aviso';
  if(key==='control_state')return value==='running'?null:'aviso';
  if(['rearm_required','reverse_limited'].includes(key))return value===1?'aviso':null;
  if(['speed_valid','frontend_health_ok','watchdog_ok'].includes(key))return value===0?'aviso':null;
  if(['telemetry_dropped','can_tx_errors','can_rx_invalid','mpu_errors'].includes(key))return value>0?'aviso':null;
  if(['mpu_ok','wifi_ap','can_init_ok','control_ready'].includes(key))return value===0?'aviso':null;
  if(key==='charger_connected')return value===1?'aviso':null;
  return null;
}
export function saved(name, fallback) { try { const v=localStorage.getItem('kart.'+name); return v===null?fallback:JSON.parse(v); } catch { return fallback; } }
export function save(name, value) { try { localStorage.setItem('kart.'+name, JSON.stringify(value)); } catch {} }
export function normalize(text) { return String(text).normalize('NFD').replace(/\p{Diacritic}/gu,'').toLocaleLowerCase(idioma.locale); }
export function ago(ms) { return ms===null||ms===undefined?'':ms<1000?`${Math.round(ms)} ms`:`${f(ms/1000,1)} s`; }
