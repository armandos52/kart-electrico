// Vista superior del kart: piezas seleccionables, valores sobre cada pieza, inspector y animación cualitativa.
import {$, app, reducedMotion, activeSignals, fresh, display, steeringPresentation, word, f, ago, anomaly, wheelMotion, wheelRates, wheelBlur, t} from './nucleo.js';
import {initPlan, steeringPivots, TRAIL_STEPS} from './esquema_kart.js';

// Títulos, etiquetas y notas de cada pieza: claves part.<id>.title|tag|note en textos.js (se leen al pedirlos, con el idioma vigente).
const piece = (id, keys) => ({keys, get title() { return t(`part.${id}.title`); }, get tag() { return t(`part.${id}.tag`); }, get note() { return t(`part.${id}.note`); }});
export const parts = {
  stm32: piece('stm32', ['accelerator_pct','steer_deg','brake','gear','mode','sd_ready']),
  esp32: piece('esp32', ['control_state','drive_inhibit','can_rx_count','can_rx_age_ms','mode','pwm_left_raw','pwm_right_raw','relay1','relay2','relay_seq','charger_connected','emergency_stop','mpu_ok','loop_max_us','telemetry_us_max','telemetry_dropped','wifi_ap','wifi_clients']),
  front_left: piece('front_left', ['rpm_fl','steer_deg']),
  front_right: piece('front_right', ['rpm_fr','steer_deg']),
  rear_left: piece('rear_left', ['rpm_rl','pwm_left_raw','gear']),
  rear_right: piece('rear_right', ['rpm_rr','pwm_right_raw','gear']),
  motor_left: piece('motor_left', ['pwm_left_raw','rpm_rl','brake']),
  motor_right: piece('motor_right', ['pwm_right_raw','rpm_rr','brake']),
  battery: piece('battery', ['battery_v','battery_a','soc_pct','battery_temp_c']),
  can: piece('can', ['can_rx_age_ms','can_rx_count','accelerator_raw','steer_raw','gear','mode']),
  power: piece('power', ['relay1','relay2','relay_seq','charger_connected']),
  imu: piece('imu', ['yaw_dps','mpu_ok','steer_deg','mode']),
};
export let selected = 'esp32';
let model = null, fallo3d = null;
export function partOf(key) {
  const name=key==='steer_view'?'can':Object.keys(parts).find(id=>parts[id].keys[0]===key)||Object.keys(parts).find(id=>parts[id].keys.includes(key));
  return name?{id:name,...parts[name]}:null;
}
export function selectPart(part) {
  if (!parts[part]) return;
  selected = part;
  if ($('modelo-pieza')) $('modelo-pieza').value = part;
  model?.select(part);
  document.querySelectorAll('[data-part]').forEach(el=>el.classList.toggle('selected',el.dataset.part===part));
  $('selection-title').textContent = parts[part].title;
  $('selection-tag').textContent = parts[part].tag;
  $('selection-note').textContent = parts[part].note;
  renderInspector();
}
export function renderInspector() {
  const signals = activeSignals();
  $('signals').replaceChildren();
  for (const key of parts[selected].keys) {
    const s = signals[key], level = s?.quality==='fresh'?anomaly(key,s.value):null;
    const row = document.createElement('div'); row.className = 'signal ' + (s?.quality || 'missing') + (level?' '+level:'');
    const label = document.createElement('div'); label.className='label'; label.textContent=s?.label || key;
    const value = document.createElement('div'); value.className='value'; value.textContent=display(s,key);
    const meta = document.createElement('div'); meta.className='meta';
    meta.textContent=app.demo ? '' : !s?.node ? t('insp.sin_senal') : s.quality==='stale' ? t('insp.antiguos',{edad:ago(s.age_ms)}) : '';
    row.title=s?.node?`${key} · ${s.node} · ${ago(s.age_ms)}`:key;
    row.append(label,value); if(meta.textContent)row.appendChild(meta); $('signals').appendChild(row);
  }
}
const text = (id, value) => { $(id).textContent = value; };
const shown = (key, digits=0, unit='') => { const v=fresh(key); return v===null?'—':f(v,digits)+unit; };
let steerTarget = 0;
export function renderMap() {
  if(fallo3d!==null){$('modelo-status').textContent=t('m3d.no_disponible');$('ver-esquema').title=t('m3d.no_disponible_titulo',{error:fallo3d});}
  else if(!model){text('modelo-status',t('m3d.preparando'));text('modelo-steering',t('m3d.dir_sin_datos'));text('modelo-rpm',t('m3d.rpm_vacio'));}   // hasta que la escena 3D tome el relevo
  for (const side of ['fl','fr','rl','rr']) text('rpm-'+side, fresh('rpm_'+side) === null ? '— rpm' : f(fresh('rpm_'+side),0)+' rpm');
  const voltage = fresh('battery_v'), soc = fresh('soc_pct');
  text('battery-value', (voltage===null?'—':f(voltage))+' V · '+(soc===null?'—':f(soc,0))+' %');
  const steering = steeringPresentation();
  steerTarget = steering.mapAngle ?? steering.angle ?? 0;
  text('steering-map', steering.calibrated ? (steering.outside?t('mapa.fuera'):t('mapa.recorrido',{pct:f(steering.percent)})) : steering.angle === null ? t('mapa.dir_sin_datos') : t(steering.estimated ? 'mapa.estimados' : 'mapa.reportados',{grados:f(steering.angle)}));
  const direction = fresh('drive_direction') || fresh('gear');
  text('travel-map', direction ? t('mapa.marcha',{marcha:(word(direction)||direction).toUpperCase()}) : t('mapa.marcha_sin'));
  text('val-stm32', 'pedal '+shown('accelerator_pct',0,' %'));
  const phase = fresh('control_state');
  text('val-esp32', phase ? word(phase)||phase : '—');
  $('val-esp32').classList.toggle('aviso', !!phase && phase!=='running');
  text('val-kelly-l', shown('pwm_left_raw')+' /255');
  text('val-kelly-r', shown('pwm_right_raw')+' /255');
  text('val-can', shown('can_rx_age_ms',0,' ms'));
  $('val-can').classList.toggle('alarma', (fresh('can_rx_age_ms')??0)>100);
  text('val-power', `R1 ${shown('relay1')} · R2 ${shown('relay2')}`);
  text('val-imu', shown('yaw_dps',1,' °/s'));
}
const angles = {fl:0,fr:0,rl:0,rr:0};
const treadOffsets = {fl:0,fr:0,rl:0,rr:0};
const trailCache = {};
let steerAngle = 0;
// Estela de la marca: TRAIL_STEPS tramos entre la posición actual y la de hace `arc` radianes.
// En planta se ve la parte superior del neumático: y = R·sen(ψ), visible mientras cos(ψ) > 0.
function drawTrail(side,angle,arc,dir,strength) {
  const group=$('estela-'+side);
  if(arc<=.02){if(group.getAttribute('display')!=='none')group.setAttribute('display','none');return;}
  group.removeAttribute('display');
  const rects=trailCache[side]??=[...group.children], cy=Number(group.dataset.cy), radius=Number(group.dataset.radius), n=rects.length;
  const ys=[],light=[];
  for(let k=0;k<=n;k++){const psi=angle-dir*arc*k/n;ys.push(cy+Math.sin(psi)*radius);light.push(Math.max(0,Math.cos(psi))*Math.pow(1-k/n,1.4));}
  for(let k=0;k<n;k++){
    const top=Math.min(ys[k],ys[k+1]), rect=rects[k];
    rect.setAttribute('y',top.toFixed(2));rect.setAttribute('height',Math.max(1.5,Math.abs(ys[k+1]-ys[k])+.6).toFixed(2));
    rect.setAttribute('opacity',(.85*strength*(light[k]+light[k+1])/2).toFixed(3));
  }
}
export function animate(dt) {
  model?.update(dt);
  const signals=activeSignals(), motion=wheelMotion(signals,dt), rates=wheelRates(signals), moving=!reducedMotion.matches&&!app.motionPaused;
  for(const side of ['fl','fr','rl','rr']){
    const rim=$('rim-'+side), rate=moving?rates[side]:0, blur=wheelBlur(rate), dir=Math.sign(rate);
    if(moving){
      angles[side]=(angles[side]+motion[side])%(Math.PI*2);
      // La banda avanza lo que giran su radio (en píxeles del esquema) y su ángulo.
      treadOffsets[side]=(treadOffsets[side]+motion[side]*Number(rim.dataset.radius))%18;
    }
    rim.style.transform=`translateY(${treadOffsets[side]}px)`;
    // Con el giro real, a partir de ~0,35 vueltas/s el dibujo repetido se desvanece y queda un tono uniforme.
    rim.setAttribute('opacity',blur.detail.toFixed(3));
    $('difuso-'+side).setAttribute('opacity',((1-blur.detail)*.55).toFixed(3));
    const marker=$('marca-'+side);
    marker.setAttribute('transform',`translate(0 ${Math.sin(angles[side])*Number(marker.dataset.radius)})`);
    // Con el giro rápido la marca y su estela son más tenues: debe leerse como difuminado, no como destello.
    const strength=.6+.4*blur.detail;
    marker.style.opacity=String(Math.max(0,Math.cos(angles[side]))*strength);
    drawTrail(side,angles[side],blur.arc,dir,strength);
  }
  steerAngle = reducedMotion.matches || app.motionPaused ? steerTarget : steerAngle+(steerTarget-steerAngle)*(1-Math.exp(-12*dt));
  for(const side of ['left','right'])$('steer-'+side).setAttribute('transform',`rotate(${steerAngle} ${steeringPivots[side].join(' ')})`);
}
export function initMap(onPick) {
  initPlan();
  for (const [id, part] of Object.entries(parts)) {
    const option = document.createElement('option'); option.value = id; option.dataset.i18n = `part.${id}.title`; option.textContent = part.title;
    $('modelo-pieza').appendChild(option);
  }
  $('modelo-pieza').addEventListener('change',event=>{selectPart(event.target.value);onPick?.(event.target.value);});
  function setView(view) {
    $('modelo-3d').hidden=view!=='3d'; $('kart').hidden=view==='3d';
    // SVG no implementa de forma uniforme HTMLElement.hidden.
    $('kart').style.display=view==='3d'?'none':'';
    $('ver-3d').setAttribute('aria-pressed',String(view==='3d'));
    $('ver-esquema').setAttribute('aria-pressed',String(view!=='3d'));
    $('foto-superior').hidden=view==='3d';
    history.replaceState(null,'',view==='3d'?'#3d':'#esquema');
    model?.resize();
  }
  $('ver-3d').addEventListener('click',()=>setView('3d'));
  $('ver-esquema').addEventListener('click',()=>setView('esquema'));
  for (const img of $('fotos-kart').querySelectorAll('img')) {
    const missing = () => {
      img.hidden = true;
      if (img.parentElement.querySelector('[data-photo-local]')) return;
      const note = document.createElement('p');
      note.dataset.photoLocal = '';
      note.dataset.i18n = 'fotos.local';
      note.textContent = t('fotos.local');
      img.after(note);
    };
    img.addEventListener('error', missing);
    if (img.complete && !img.naturalWidth) missing();
  }
  $('ver-fotos').addEventListener('click',()=>$('fotos-kart').showModal());
  $('foto-superior').addEventListener('click',()=>{$('fotos-kart').showModal();$('referencia-superior').scrollIntoView({block:'center'});});
  $('cerrar-fotos').addEventListener('click',()=>$('fotos-kart').close());
  $('ampliar-modelo').addEventListener('click',()=>{
    const full=document.body.classList.toggle('modelo-ampliado');
    $('ampliar-modelo').setAttribute('aria-pressed',String(full));
    $('ampliar-modelo').textContent=full?t('btn.reducir'):t('btn.ampliar');
  });
  setView(location.hash==='#esquema'?'esquema':'3d');
  window.addEventListener('hashchange',()=>setView(location.hash==='#esquema'?'esquema':'3d'));
  // Fallar WebGL o un recurso 3D no debe interrumpir la telemetría/calibración.
  import('./escena_kart.js').then(({createKartScene})=>{
    model=createKartScene(part=>{selectPart(part);onPick?.(part);});model.select(selected);
  }).catch(error=>{
    fallo3d=error.message;
    $('modelo-status').textContent=t('m3d.no_disponible');
    $('ver-3d').disabled=true;setView('esquema');
    $('ver-esquema').title=t('m3d.no_disponible_titulo',{error:error.message});
    const notice=document.createElement('p');notice.className='modelo-error';notice.dataset.i18n='m3d.no_disponible_aviso';notice.textContent=t('m3d.no_disponible_aviso');$('kart').before(notice);
  });
  document.querySelectorAll('[data-part]').forEach(el=>{
    el.addEventListener('click',()=>{selectPart(el.dataset.part);onPick?.(el.dataset.part);});
    el.addEventListener('keydown',event=>{if(event.key==='Enter'||event.key===' '){event.preventDefault();selectPart(el.dataset.part);onPick?.(el.dataset.part);}});
  });
}
