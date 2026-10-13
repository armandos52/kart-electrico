// Estado compartido y presentación de señales. Sin lógica de control: solo lectura de /api/state.
export const $ = id => document.getElementById(id);
export const app = {state:null, token:'', demo:false, backendOK:false, stateReceivedAt:0, demoFrame:null, demoSeq:0, motionPaused:false};
export const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
export const words = {can_wait:'Esperando CAN',mpu_init:'Inicializando sensores',running:'Control inicializado',can_init:'CAN sin inicializar',initializing:'Inicialización pendiente',no_command:'Sin órdenes del STM32',relays:'Secuencia de relés pendiente',brake:'Freno activo',pedal_zero:'Pedal en cero',neutral:'Marcha neutra',direction_change:'Cambio de marcha bloqueado por RPM',can_timeout:'Orden CAN caducada',mpu_unavailable:'ESC sin giroscopio disponible',none:'Sin bloqueo reportado',D:'Adelante',R:'Atrás',N:'Neutro',DIF:'Diferencial',ESC:'ESC',BLQ:'Bloqueado'};
const BINARY = new Set(['brake','relay1','relay2','charger_connected','sd_ready','emergency_stop','wifi_ap','mpu_ok','control_ready','can_init_ok','start_requested']);
Object.assign(words,{mpu_calibration:'Calibrando MPU',invalid_command:'Orden CAN inválida',start_off:'START apagado',charger:'Cargador conectado',rearm_required:'Rearme: N, freno y pedal libre',mode_change:'Cambio de modo bloqueado',speed_invalid:'Velocidad inválida',hall_fault:'Fallo Hall',frontend_invalid:'Entradas STM32 inválidas',reverse_limit:'Límite de marcha atrás',output_fault:'Fallo de salida',can_fault:'Fallo CAN',configuration:'Configuración inválida'});
for(const key of ['rearm_required','reverse_limited','speed_valid','frontend_health_seen','frontend_health_ok','watchdog_ok'])BINARY.add(key);
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
// Rodadura ilustrativa común a ambas vistas. Frente -Z (3D) / arriba (SVG).
// La parte superior del neumático avanza hacia el frontal: Rx negativa en D.
// Escala común para conservar diferencias I/D, sin aliasado del dibujo repetido.
export function wheelMotion(signals, dt) {
  const sides=['fl','fr','rl','rr'];
  const values=sides.map(side=>{const s=signals['rpm_'+side];return s?.quality==='fresh'?Math.abs(s.value):0;});
  const value=key=>signals[key]?.quality==='fresh'?signals[key].value:null;
  const direction=value('drive_direction')||value('gear');
  const scale=.9/Math.max(150,...values), step=Math.min(Math.max(dt,0),.05);
  return Object.fromEntries(sides.map((side,i)=>[side,(direction==='R'?1:-1)*values[i]*scale*step]));
}
export function display(s, key) {
  if (!s || s.quality !== 'fresh') return '—';
  if(key==='mode_applied')return ({0:'DIF',1:'ESC',2:'BLQ',255:'Sin tracción'})[s.value]??'Desconocido';
  if (typeof s.value === 'string') return words[s.value] || s.value;
  if (BINARY.has(key)) return s.value ? 'Sí / 1' : 'No / 0';
  return f(s.value) + (s.unit ? ' ' + s.unit : '');
}
export function steeringPresentation() {
  const degrees = fresh('steer_deg'), raw = fresh('steer_raw');
  const cal=app.state?.calibration;
  if(!app.demo&&cal?.active&&cal.calibrated&&raw!==null&&app.backendOK){
    const c=cal.calibrated, value=c.outside?null:(c.angle??c.percent);
    return {angle:c.angle,value,unit:c.angle===null?'%':'°',percent:c.percent,estimated:true,calibrated:true,
      outside:c.outside,target:c.target,label:c.outside?'Fuera de rango':c.angle===null?'Recorrido calibrado · PC':'Ángulo interpolado · '+({wheel_left:'rueda I',wheel_right:'rueda D',steering_wheel:'volante'}[c.target]),
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
export function normalize(text) { return String(text).normalize('NFD').replace(/\p{Diacritic}/gu,'').toLocaleLowerCase('es'); }
export function ago(ms) { return ms===null||ms===undefined?'':ms<1000?`${Math.round(ms)} ms`:`${f(ms/1000,1)} s`; }
