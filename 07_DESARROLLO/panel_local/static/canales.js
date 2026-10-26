// Canales: catálogo de presentación, espacios de trabajo, historial, tira de canales fijados y buscador Ctrl+K.
import {$, app, activeSignals, display, steeringPresentation, anomaly, saved, save, normalize, ago, word, f, TEXT, t, hasText} from './nucleo.js';
import {sparkline} from './graficas.js';

export const WINDOW_MS = 60000;
// Órdenes de software: se dibujan con trazo discontinuo. No son medidas físicas.
const ORDERS = new Set(['pwm_left_raw','pwm_right_raw','relay1','relay2','drive_direction']);
export const SPACES = {
  robustez:{name:'esp.robustez',keys:['mode','mode_applied','drive_inhibit','rearm_required','reverse_limited','speed_kph_max_wheel','reverse_limit_kph','speed_valid','mpu_ok','frontend_health_seen','frontend_health_ok','watchdog_ok','can_tx_errors','can_rx_invalid','mpu_errors'],charts:[{title:'graf.vel_umbral',keys:['speed_kph_max_wheel','reverse_limit_kph']},{title:'graf.adc',keys:['accelerator_adc_physical','steer_adc_physical']}]},
  motores:{name:'esp.motores',keys:['rpm_rl','rpm_rr','pwm_left_raw','pwm_right_raw','accelerator_pct','brake','drive_direction','drive_inhibit'],
    charts:[{title:'graf.rpm_tras',keys:['rpm_rl','rpm_rr']},{title:'graf.pwm',keys:['pwm_left_raw','pwm_right_raw']},{title:'graf.pedal',keys:['accelerator_pct']}]},
  can:{name:'esp.can',keys:['control_state','drive_inhibit','can_rx_age_ms','can_rx_count','can_init_ok','can_init_attempts','emergency_stop','start_requested'],
    charts:[{title:'graf.edad_can',keys:['can_rx_age_ms']},{title:'graf.ordenes_can',keys:['can_rx_count']},{title:'graf.loop',keys:['loop_max_us']}]},
  direccion:{name:'esp.direccion',keys:['steer_view','steer_raw','steer_deg','yaw_dps','mode','gear','rpm_rl','rpm_rr'],
    charts:[{title:'graf.direccion',keys:['steer_view']},{title:'graf.guinada',keys:['yaw_dps']},{title:'graf.rpm_tras',keys:['rpm_rl','rpm_rr']}]},
  energia:{name:'esp.energia',keys:['battery_v','battery_a','soc_pct','battery_temp_c','relay1','relay2','relay_seq','charger_connected'],
    charts:[{title:'graf.tension',keys:['battery_v']},{title:'graf.corriente',keys:['battery_a']},{title:'graf.carga',keys:['soc_pct']}]},
  esp32:{name:'esp.esp32',keys:['loop_max_us','telemetry_us_max','telemetry_dropped','wifi_clients','wifi_ap','mpu_ok','control_ready','sd_ready'],
    charts:[{title:'graf.loop',keys:['loop_max_us']},{title:'graf.coste_tel',keys:['telemetry_us_max']},{title:'graf.tramas',keys:['telemetry_dropped']}]},
};
export const layout = saved('espacios', null) || {};
layout.active = SPACES[layout.active] ? layout.active : 'motores';
layout.lists = layout.lists || {};
layout.free = layout.free || 'can_rx_age_ms';
const persist = () => save('espacios', layout);

export function shortName(key, s) { return hasText('short.'+key) ? t('short.'+key) : s?.label || key; }
export function nature(key, s) { return ORDERS.has(key) ? 'orden' : key==='steer_view' && s?.estimated ? 'estimacion' : 'medida'; }
// Señal real o dirección presentada (grados reportados, o ADC convertido con la fórmula del fuente y rotulado como estimación).
export function signal(key, signals=activeSignals()) {
  if(key!=='steer_view')return signals[key];
  const steering=steeringPresentation(), source=steering.calibrated?signals.steer_raw:signals.steer_deg?.quality==='fresh'?signals.steer_deg:signals.steer_raw||signals.steer_deg;
  const value=steering.calibrated?steering.value:steering.angle;
  return {label:steering.label||t('esp.direccion'),unit:steering.unit||'°',value,quality:value===null?(source?.quality==='stale'?'stale':'missing'):'fresh',
    age_ms:source?.age_ms??null,node:source?.node??null,sample_id:source?.sample_id,estimated:steering.estimated,calibrated:steering.calibrated,detail:steering.label};
}
export function allKeys() { return [...Object.keys(app.state?.signals || activeSignals()), 'steer_view']; }
export function currentKeys() { return layout.lists[layout.active] || SPACES[layout.active].keys; }

const history = new Map(), lastId = new Map();
export function getHistory(key) { if(app.replay)return (app.replay.history.get(key)||[]).filter(p=>p.t<=app.replay.epoch+(app.replay.renderedAt||0));return history.get(key) || []; }
export function clearHistory() { history.clear(); lastId.clear(); }
export function record(now=Date.now()) {
  if(app.replay)return;
  const signals=activeSignals();
  for(const key of allKeys()) {
    if(TEXT.has(key))continue;
    const s=signal(key,signals), h=history.get(key)||[];
    if(s?.quality==='fresh' && typeof s.value==='number') {
      const id=app.demo?'demo-'+app.demoSeq:s.sample_id;
      if(id!==lastId.get(key)){h.push({t:now-(s.age_ms||0),v:s.value});lastId.set(key,id);}
    } else if(h.length && h.at(-1).v!==null) h.push({t:now,v:null});
    while(h.length && h[0].t<now-WINDOW_MS)h.shift();
    history.set(key,h);
  }
}

function valueParts(key, s) {
  if(!s || s.quality!=='fresh')return ['—',''];
  if(key==='steer_view')return [f(s.value),s.unit];
  if(typeof s.value==='string')return [word(s.value)||s.value,''];
  const text=display(s,key);
  return s.unit && text.endsWith(' '+s.unit) ? [text.slice(0,-s.unit.length-1),s.unit] : [text,''];
}
function meta(key, s) {
  if(app.demo)return '';
  if(!s || s.quality==='missing')return '';
  if(s.quality==='stale')return t('meta.antiguo',{edad:ago(s.age_ms)});
  if(key==='steer_view' && s.calibrated)return s.detail;
  if(key==='steer_view' && s.estimated)return t('meta.estimacion');
  return '';
}

let stripKey='', handlers={};
export function initStrip(callbacks) {
  handlers=callbacks;
  $('espacios').replaceChildren();
  for(const [id,space] of Object.entries(SPACES)) {
    const button=document.createElement('button');button.type='button';button.dataset.space=id;button.dataset.i18n=space.name;button.textContent=t(space.name);button.setAttribute('role','tab');
    button.addEventListener('click',()=>{layout.active=id;persist();stripKey='';handlers.onSpace?.();});
    $('espacios').appendChild(button);
  }
}
export function renderStrip() {
  const keys=currentKeys(), signals=activeSignals();
  document.querySelectorAll('#espacios [data-space]').forEach(b=>{const on=b.dataset.space===layout.active;b.classList.toggle('activo',on);b.setAttribute('aria-selected',String(on));});
  const stamp=layout.active+':'+keys.join(',');
  if(stamp!==stripKey) {
    stripKey=stamp;$('canales').replaceChildren();
    for(const key of keys) {
      const row=document.createElement('div');row.className='canal';row.dataset.key=key;row.tabIndex=0;row.setAttribute('role','button');
      row.innerHTML='<span class="canal-nombre"></span><span class="canal-valor"><span class="num"></span><span class="unidad"></span></span><canvas class="spark" aria-hidden="true"></canvas><span class="canal-meta"></span><button type="button" class="quitar" data-i18n-aria="quitar.aria" aria-label="'+t('quitar.aria')+'">×</button>';
      row.addEventListener('click',event=>{if(event.target.closest('.quitar')){unpin(key);return;}showFree(key);});
      row.addEventListener('keydown',event=>{if(event.key==='Enter'||event.key===' '){event.preventDefault();showFree(key);}if(event.key==='Delete')unpin(key);});
      $('canales').appendChild(row);
    }
  }
  for(const row of $('canales').children) {
    const key=row.dataset.key, s=signal(key,signals), kind=nature(key,s), [value,unit]=valueParts(key,s);
    const level=s?.quality==='fresh'?anomaly(key,s.value):null;
    row.className=`canal ${s?.quality||'missing'} ${kind}${level?' '+level:''}${layout.free===key?' libre':''}${typeof s?.value==='string'?' texto':''}`;
    row.title=`${s?.label||key} · ${key}`;
    row.querySelector('.canal-nombre').textContent=shortName(key,s);
    row.querySelector('.num').textContent=value;row.querySelector('.unidad').textContent=unit;
    row.querySelector('.canal-meta').textContent=meta(key,s);
    const canvas=row.querySelector('.spark');
    if(TEXT.has(key))canvas.hidden=true;else sparkline(canvas,getHistory(key),kind);
  }
}
export function isPinned(key) { return currentKeys().includes(key); }
export function pin(key) { if(!isPinned(key)){layout.lists[layout.active]=[...currentKeys(),key];persist();} }
export function unpin(key) { layout.lists[layout.active]=currentKeys().filter(k=>k!==key);persist();handlers.onChange?.(); }
export function showFree(key) { layout.free=key;persist();handlers.onChange?.(); }
export function resetSpace() { delete layout.lists[layout.active];persist();stripKey='';handlers.onChange?.(); }

// Buscador Ctrl+K: fija, grafica o localiza la pieza de cualquier señal.
let results=[], cursor=0;
export function openPalette() {
  $('paleta-buscar').value='';cursor=0;renderPalette();
  if(!$('paleta').open)$('paleta').showModal();
  $('paleta-buscar').focus();
}
export function renderPalette() {
  const query=normalize($('paleta-buscar').value.trim()), signals=activeSignals();
  results=allKeys().filter(key=>{const s=signal(key,signals);return !query || normalize(`${key} ${s?.label||''} ${shortName(key,s)} ${s?.unit||''} ${handlers.partOf?.(key)?.title||''}`).includes(query);});
  cursor=Math.min(cursor,Math.max(0,results.length-1));
  $('paleta-lista').replaceChildren();
  results.forEach((key,index)=>{
    const s=signal(key,signals), item=document.createElement('li');
    item.setAttribute('role','option');item.setAttribute('aria-selected',String(index===cursor));item.dataset.key=key;
    item.innerHTML='<span class="p-nombre"></span><code></code><span class="p-valor"></span><span class="p-acciones"><button type="button" data-a="fijar"></button><button type="button" data-a="graficar">'+t('paleta.graficar')+'</button><button type="button" data-a="pieza">'+t('paleta.pieza')+'</button></span>';
    item.querySelector('.p-nombre').textContent=shortName(key,s);item.querySelector('code').textContent=key;
    item.querySelector('.p-valor').textContent=valueParts(key,s).join(' ').trim();
    item.querySelector('[data-a="fijar"]').textContent=isPinned(key)?t('quitar'):t('fijar');
    item.querySelector('[data-a="pieza"]').hidden=!handlers.partOf?.(key);
    item.addEventListener('click',event=>{const action=event.target.closest('button')?.dataset.a;if(action)act(action,key);else{cursor=index;renderPalette();}});
    $('paleta-lista').appendChild(item);
  });
  $('paleta-vacio').hidden=results.length>0;
}
function act(action, key) {
  if(action==='fijar'){isPinned(key)?unpin(key):pin(key);handlers.onChange?.();renderPalette();return;}
  if(action==='graficar')showFree(key);
  if(action==='pieza')handlers.onPart?.(key);
  $('paleta').close();
}
export function initPalette() {
  $('paleta-buscar').addEventListener('input',()=>{cursor=0;renderPalette();});
  $('paleta-buscar').addEventListener('keydown',event=>{
    if(event.key==='ArrowDown'||event.key==='ArrowUp'){event.preventDefault();cursor=Math.max(0,Math.min(results.length-1,cursor+(event.key==='ArrowDown'?1:-1)));renderPalette();$('paleta-lista').children[cursor]?.scrollIntoView({block:'nearest'});}
    if(event.key==='Enter'&&results[cursor]){event.preventDefault();act(event.shiftKey?'graficar':'fijar',results[cursor]);}
  });
  $('paleta').addEventListener('click',event=>{if(event.target===$('paleta'))$('paleta').close();});
}
