// Estado compartido y presentación de señales. Sin lógica de control: solo lectura de /api/state.
export const $ = id => document.getElementById(id);
export const app = {state:null, token:'', demo:false, backendOK:false, stateReceivedAt:0, demoFrame:null, demoSeq:0, motionPaused:false};
export const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
export const words = {can_wait:'Esperando CAN',mpu_init:'Inicializando sensores',running:'Control inicializado',can_init:'CAN sin inicializar',initializing:'Inicialización pendiente',no_command:'Sin órdenes del STM32',relays:'Secuencia de relés pendiente',brake:'Freno activo',pedal_zero:'Pedal en cero',neutral:'Marcha neutra',direction_change:'Cambio de marcha bloqueado por RPM',can_timeout:'Orden CAN caducada',mpu_unavailable:'ESC sin giroscopio disponible',none:'Sin bloqueo reportado',D:'Adelante',R:'Atrás',N:'Neutro',DIF:'Diferencial',ESC:'ESC',BLQ:'Bloqueado'};
const BINARY = new Set(['brake','relay1','relay2','charger_connected','sd_ready','emergency_stop','wifi_ap','mpu_ok','control_ready','can_init_ok','start_requested']);
export const TEXT = new Set(['gear','mode','control_state','drive_direction','drive_inhibit']);

export function f(value, digits=1) { return Number(value).toLocaleString('es-ES',{maximumFractionDigits:digits}); }
export function activeSignals() {
  if(app.demo)return app.demoFrame?.signals || {};
  const elapsed=Date.now()-app.stateReceivedAt;
  const signals=Object.fromEntries(Object.entries(app.state?.signals || {}).map(([key,s])=>{
    const age=s.age_ms===null?null:s.age_ms+elapsed;
    return [key,{...s,age_ms:age,quality:s.quality==='fresh'&&age>(s.stale_after_ms||1500)?'stale':s.quality}];
  }));
  if(signals.can_rx_age_ms?.quality==='fresh' && signals.can_rx_age_ms.value>100) {
    for(const key of ['accelerator_raw','accelerator_pct','steer_raw','steer_deg','brake','gear','mode']) {
      if(signals[key]?.node==='esp32' && signals[key].quality==='fresh')signals[key]={...signals[key],quality:'stale',age_ms:Math.max(signals[key].age_ms,signals.can_rx_age_ms.value)};
    }
  }
  return signals;
}
export function fresh(key) { const s = activeSignals()[key]; return s?.quality === 'fresh' ? s.value : null; }
export function display(s, key) {
  if (!s || s.quality !== 'fresh') return '—';
  if (typeof s.value === 'string') return words[s.value] || s.value;
  if (BINARY.has(key)) return s.value ? 'Sí / 1' : 'No / 0';
  return f(s.value) + (s.unit ? ' ' + s.unit : '');
}
export function steeringPresentation() {
  const degrees = fresh('steer_deg'), raw = fresh('steer_raw');
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
  if(['telemetry_dropped'].includes(key))return value>0?'aviso':null;
  if(['mpu_ok','wifi_ap','can_init_ok','control_ready'].includes(key))return value===0?'aviso':null;
  if(key==='charger_connected')return value===1?'aviso':null;
  return null;
}
export function saved(name, fallback) { try { const v=localStorage.getItem('kart.'+name); return v===null?fallback:JSON.parse(v); } catch { return fallback; } }
export function save(name, value) { try { localStorage.setItem('kart.'+name, JSON.stringify(value)); } catch {} }
export function normalize(text) { return String(text).normalize('NFD').replace(/\p{Diacritic}/gu,'').toLocaleLowerCase('es'); }
export function ago(ms) { return ms===null||ms===undefined?'':ms<1000?`${Math.round(ms)} ms`:`${f(ms/1000,1)} s`; }
