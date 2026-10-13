// Vista superior del kart: piezas seleccionables, valores sobre cada pieza, inspector y animación cualitativa.
import {$, app, reducedMotion, activeSignals, fresh, display, steeringPresentation, words, f, ago, anomaly, wheelMotion} from './nucleo.js';
import {initPlan, steeringPivots} from './esquema_kart.js';

export const parts = {
  stm32: {title:'STM32',tag:'Unidad frontal',note:'Mandos, pantalla táctil y microSD. Franja blanca superior declarada; extensión pendiente.',keys:['accelerator_pct','steer_deg','brake','gear','mode','sd_ready']},
  esp32: {title:'ESP32',tag:'Unidad trasera',note:'Diferencial, PWM, relés y retorno de sensores. Señales software reportadas por la placa.',keys:['control_state','drive_inhibit','can_rx_count','can_rx_age_ms','mode','pwm_left_raw','pwm_right_raw','relay1','relay2','relay_seq','charger_connected','emergency_stop','mpu_ok','loop_max_us','telemetry_us_max','telemetry_dropped','wifi_ap','wifi_clients']},
  front_left: {title:'Rueda delantera I',tag:'Sensor Hall',note:'Velocidad medida por el controlador frontal; pulsos y calibración física por contrastar.',keys:['rpm_fl','steer_deg']},
  front_right: {title:'Rueda delantera D',tag:'Sensor Hall',note:'Velocidad medida por el controlador frontal; pulsos y calibración física por contrastar.',keys:['rpm_fr','steer_deg']},
  rear_left: {title:'Rueda trasera I',tag:'Tracción izquierda',note:'RPM reportadas y orden PWM. Reducir par no equivale a frenado regenerativo.',keys:['rpm_rl','pwm_left_raw','gear']},
  rear_right: {title:'Rueda trasera D',tag:'Tracción derecha',note:'RPM reportadas y orden PWM. Reducir par no equivale a frenado regenerativo.',keys:['rpm_rr','pwm_right_raw','gear']},
  motor_left: {title:'Kelly izquierdo',tag:'Orden al controlador',note:'GPIO32 en la fuente 2025/26. PWM bruto invertido: 255 corresponde a orden de parada. Tensión física requiere medida.',keys:['pwm_left_raw','rpm_rl','brake']},
  motor_right: {title:'Kelly derecho',tag:'Orden al controlador',note:'GPIO2 en la fuente 2025/26. PWM bruto invertido: 255 corresponde a orden de parada. Tensión física requiere medida.',keys:['pwm_right_raw','rpm_rr','brake']},
  battery: {title:'Batería / BMS',tag:'Energía',note:'16 celdas declaradas por el usuario. Conexión activa, protocolo BMS y regeneración por verificar.',keys:['battery_v','battery_a','soc_pct','battery_temp_c']},
  can: {title:'Bus CAN',tag:'Comunicación',note:'0x100: mandos. 0x123: retorno. El panel necesita diagnóstico para observar señales adicionales.',keys:['can_rx_age_ms','can_rx_count','accelerator_raw','steer_raw','gear','mode']},
  power: {title:'Relés / contactor',tag:'Salidas ordenadas',note:'GPIO16/17 en la fuente. Relé ordenado no confirma cierre físico del contactor. Energía residual pendiente de diagnóstico.',keys:['relay1','relay2','relay_seq','charger_connected']},
  imu: {title:'Giroscopio',tag:'Dinámica',note:'Guiñada reportada en °/s. El modo ESC no se considera validado físicamente por esta visualización.',keys:['yaw_dps','mpu_ok','steer_deg','mode']},
};
export let selected = 'esp32';
let model = null;
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
    meta.textContent=app.demo ? '' : !s?.node ? 'Sin señal recibida' : s.quality==='stale' ? `DATOS ANTIGUOS · hace ${ago(s.age_ms)}` : '';
    row.title=s?.node?`${key} · ${s.node} · ${ago(s.age_ms)}`:key;
    row.append(label,value); if(meta.textContent)row.appendChild(meta); $('signals').appendChild(row);
  }
}
const text = (id, value) => { $(id).textContent = value; };
const shown = (key, digits=0, unit='') => { const v=fresh(key); return v===null?'—':f(v,digits)+unit; };
let steerTarget = 0;
export function renderMap() {
  for (const side of ['fl','fr','rl','rr']) text('rpm-'+side, fresh('rpm_'+side) === null ? '— rpm' : f(fresh('rpm_'+side),0)+' rpm');
  const voltage = fresh('battery_v'), soc = fresh('soc_pct');
  text('battery-value', (voltage===null?'—':f(voltage))+' V · '+(soc===null?'—':f(soc,0))+' %');
  const steering = steeringPresentation();
  steerTarget = steering.mapAngle ?? steering.angle ?? 0;
  text('steering-map', steering.calibrated ? (steering.outside?'FUERA DE RANGO':`${f(steering.percent)} % · RECORRIDO / ESQUEMA`) : steering.angle === null ? 'DIRECCIÓN SIN DATOS' : `${f(steering.angle)}° · ${steering.estimated ? 'ESTIMADOS' : 'REPORTADOS'}`);
  const direction = fresh('drive_direction') || fresh('gear');
  text('travel-map', direction ? `MARCHA ${words[direction].toUpperCase()} · ORDENADA` : 'MARCHA SIN DATOS');
  text('val-stm32', 'pedal '+shown('accelerator_pct',0,' %'));
  const phase = fresh('control_state');
  text('val-esp32', phase ? words[phase] : '—');
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
let steerAngle = 0;
export function animate(dt) {
  model?.update(dt);
  const motion=wheelMotion(activeSignals(),dt);
  for(const side of ['fl','fr','rl','rr']){
    if(!reducedMotion.matches&&!app.motionPaused){
      angles[side]=(angles[side]+motion[side])%(Math.PI*2);
      treadOffsets[side]=(treadOffsets[side]+motion[side]*20)%18;
    }
    $('rim-'+side).style.transform=`translateY(${treadOffsets[side]}px)`;
    const marker=$('marca-'+side);
    marker.setAttribute('transform',`translate(0 ${Math.sin(angles[side])*Number(marker.dataset.radius)})`);
    marker.style.opacity=String(Math.max(0,Math.cos(angles[side])));
  }
  steerAngle = reducedMotion.matches || app.motionPaused ? steerTarget : steerAngle+(steerTarget-steerAngle)*(1-Math.exp(-12*dt));
  for(const side of ['left','right'])$('steer-'+side).setAttribute('transform',`rotate(${steerAngle} ${steeringPivots[side].join(' ')})`);
}
export function initMap(onPick) {
  initPlan();
  for (const [id, part] of Object.entries(parts)) {
    const option = document.createElement('option'); option.value = id; option.textContent = part.title;
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
  $('ver-fotos').addEventListener('click',()=>$('fotos-kart').showModal());
  $('foto-superior').addEventListener('click',()=>{$('fotos-kart').showModal();$('referencia-superior').scrollIntoView({block:'center'});});
  $('cerrar-fotos').addEventListener('click',()=>$('fotos-kart').close());
  $('ampliar-modelo').addEventListener('click',()=>{
    const full=document.body.classList.toggle('modelo-ampliado');
    $('ampliar-modelo').setAttribute('aria-pressed',String(full));
    $('ampliar-modelo').textContent=full?'Reducir':'Ampliar';
  });
  setView(location.hash==='#esquema'?'esquema':'3d');
  window.addEventListener('hashchange',()=>setView(location.hash==='#esquema'?'esquema':'3d'));
  // Fallar WebGL o un recurso 3D no debe interrumpir la telemetría/calibración.
  import('./escena_kart.js').then(({createKartScene})=>{
    model=createKartScene(part=>{selectPart(part);onPick?.(part);});model.select(selected);
  }).catch(error=>{
    $('modelo-status').textContent='3D no disponible. Puedes usar el esquema y las señales.';
    $('ver-3d').disabled=true;setView('esquema');
    $('ver-esquema').title='3D no disponible: '+error.message;
    const notice=document.createElement('p');notice.className='modelo-error';notice.textContent='3D no disponible en este navegador. Esquema y monitorización disponibles.';$('kart').before(notice);
  });
  document.querySelectorAll('[data-part]').forEach(el=>{
    el.addEventListener('click',()=>{selectPart(el.dataset.part);onPick?.(el.dataset.part);});
    el.addEventListener('keydown',event=>{if(event.key==='Enter'||event.key===' '){event.preventDefault();selectPart(el.dataset.part);onPick?.(el.dataset.part);}});
  });
}
