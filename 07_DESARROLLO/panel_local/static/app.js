'use strict';
const $ = id => document.getElementById(id);
const parts = {
  stm32: {title:'STM32',tag:'UNIDAD FRONTAL',note:'Mandos, pantalla táctil y microSD. Franja blanca superior declarada; extensión pendiente.',keys:['accelerator_pct','steer_deg','brake','gear','mode','sd_ready']},
  esp32: {title:'ESP32',tag:'UNIDAD TRASERA',note:'Diferencial, PWM, relés y retorno de sensores. Señales software reportadas por la placa.',keys:['can_rx_count','can_rx_age_ms','mode','pwm_left_raw','pwm_right_raw','relay1','relay2','relay_seq','charger_connected','emergency_stop','mpu_ok','loop_max_us','telemetry_us_max','telemetry_dropped','wifi_ap','wifi_clients']},
  front_left: {title:'Rueda delantera I',tag:'SENSOR HALL',note:'Velocidad medida por el controlador frontal; pulsos y calibración física por contrastar.',keys:['rpm_fl','steer_deg']},
  front_right: {title:'Rueda delantera D',tag:'SENSOR HALL',note:'Velocidad medida por el controlador frontal; pulsos y calibración física por contrastar.',keys:['rpm_fr','steer_deg']},
  rear_left: {title:'Rueda trasera I',tag:'TRACCIÓN IZQUIERDA',note:'RPM reportadas y orden PWM. Reducir par no equivale a frenado regenerativo.',keys:['rpm_rl','pwm_left_raw','gear']},
  rear_right: {title:'Rueda trasera D',tag:'TRACCIÓN DERECHA',note:'RPM reportadas y orden PWM. Reducir par no equivale a frenado regenerativo.',keys:['rpm_rr','pwm_right_raw','gear']},
  motor_left: {title:'Kelly izquierdo',tag:'ORDEN AL CONTROLADOR',note:'GPIO32 en la fuente 2025/26. PWM bruto invertido: 255 corresponde a orden de parada. Tensión física requiere medida.',keys:['pwm_left_raw','rpm_rl','brake']},
  motor_right: {title:'Kelly derecho',tag:'ORDEN AL CONTROLADOR',note:'GPIO2 en la fuente 2025/26. PWM bruto invertido: 255 corresponde a orden de parada. Tensión física requiere medida.',keys:['pwm_right_raw','rpm_rr','brake']},
  battery: {title:'Batería / BMS',tag:'ENERGÍA',note:'16 celdas declaradas por el usuario. Conexión activa, protocolo BMS y regeneración por verificar.',keys:['battery_v','battery_a','soc_pct','battery_temp_c']},
  can: {title:'Bus CAN',tag:'COMUNICACIÓN',note:'0x100: mandos. 0x123: retorno. El panel necesita diagnóstico para observar señales adicionales.',keys:['can_rx_age_ms','can_rx_count','accelerator_raw','steer_raw','gear','mode']},
  power: {title:'Relés / contactor',tag:'SALIDAS ORDENADAS',note:'GPIO16/17 en la fuente. Relé ordenado no confirma cierre físico del contactor. Energía residual pendiente de diagnóstico.',keys:['relay1','relay2','charger_connected']},
  imu: {title:'Giroscopio',tag:'DINÁMICA',note:'Guiñada reportada en °/s. El modo ESC no se considera validado físicamente por esta visualización.',keys:['yaw_dps','mpu_ok','steer_deg','mode']},
};
let state = null, token = '', selected = 'esp32', demo = false, backendOK = false, stateReceivedAt = 0;
let history = [], lastSample = null, lastMessage = 0, demoSeq = 0, demoTime = performance.now(), apiBusy = false;
let angles = {fl:0,fr:0,rl:0,rr:0}, demoFrame = null, motionPaused = false, steerAngle = 0, steerTarget = 0;
const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)');
const words = {can_wait:'Esperando CAN',mpu_init:'Inicializando sensores',running:'Control inicializado',can_init:'CAN sin inicializar',initializing:'Inicialización pendiente',no_command:'Sin órdenes del STM32',relays:'Secuencia de relés pendiente',brake:'Freno activo',pedal_zero:'Pedal en cero',neutral:'Marcha neutra',direction_change:'Cambio de marcha bloqueado por RPM',can_timeout:'Orden CAN caducada',mpu_unavailable:'ESC sin giroscopio disponible',none:'Sin bloqueo reportado',D:'Adelante',R:'Atrás',N:'Neutro',DIF:'Diferencial',ESC:'ESC',BLQ:'Bloqueado'};

function f(value, digits=1) { return Number(value).toLocaleString('es-ES',{maximumFractionDigits:digits}); }
function fresh(key) { const s = activeSignals()[key]; return s?.quality === 'fresh' ? s.value : null; }
function activeSignals() {
  if(demo)return demoFrame?.signals || {};
  const elapsed=Date.now()-stateReceivedAt;
  const signals=Object.fromEntries(Object.entries(state?.signals || {}).map(([key,s])=>{
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
function display(s) {
  if (!s || s.quality !== 'fresh') return '—';
  if (typeof s.value === 'string') return words[s.value] || s.value;
  if (['Freno leído','Relé 1 ordenado','Relé 2 ordenado','Cargador detectado','SD disponible'].includes(s.label)) return s.value ? 'Sí / 1' : 'No / 0';
  return f(s.value) + (s.unit ? ' ' + s.unit : '');
}
function selectPart(part) {
  if (!parts[part]) return;
  selected = part;
  document.querySelectorAll('[data-part]').forEach(el=>el.classList.toggle('selected',el.dataset.part===part));
  $('selection-title').textContent = parts[part].title;
  $('selection-tag').textContent = parts[part].tag;
  $('selection-note').textContent = parts[part].note;
  renderInspector();
}
function renderInspector() {
  const signals = activeSignals();
  $('signals').replaceChildren();
  for (const key of parts[selected].keys) {
    const s = signals[key];
    const row = document.createElement('div'); row.className = 'signal ' + (s?.quality || 'missing');
    const label = document.createElement('div'); label.className='label'; label.textContent=s?.label || key;
    const value = document.createElement('div'); value.className='value'; value.textContent=display(s);
    const meta = document.createElement('div'); meta.className='meta';
    meta.textContent=demo ? 'Datos sintéticos · sin hardware' : !s?.node ? 'Sin señal recibida' : `${s.node} · ${s.age_ms} ms desde recepción${s.quality==='stale'?' · DATOS ANTIGUOS':''}`;
    row.append(label,value,meta); $('signals').appendChild(row);
  }
}
function renderMap() {
  for (const side of ['fl','fr','rl','rr']) $('rpm-'+side).textContent=fresh('rpm_'+side) === null ? '— rpm' : f(fresh('rpm_'+side),0)+' rpm';
  const voltage = fresh('battery_v'), soc = fresh('soc_pct');
  $('battery-value').textContent=(voltage===null?'—':f(voltage))+' V · '+(soc===null?'—':f(soc,0))+' %';
  const steering = steeringPresentation();
  steerTarget = steering.angle || 0;
  $('steering-map').textContent = steering.angle === null ? 'DIRECCIÓN SIN DATOS' : `${f(steering.angle)}° · ${steering.estimated ? 'ESTIMADOS' : 'REPORTADOS'}`;
  const direction = fresh('drive_direction') || fresh('gear');
  $('travel-map').textContent = direction ? `MARCHA ${words[direction].toUpperCase()} · ORDENADA` : 'MARCHA SIN DATOS';
  $('animation-note').textContent = 'Giro ilustrativo según RPM; sentido según marcha ordenada. Los Hall traseros no miden el sentido. Dirección del mando sin calibrar cuando se indica estimada.';
}
function steeringPresentation() {
  const degrees = fresh('steer_deg'), raw = fresh('steer_raw');
  if(degrees !== null)return {angle:Math.max(-25,Math.min(25,degrees)),estimated:false};
  if(raw !== null)return {angle:Math.max(-20,Math.min(20,(raw-2047)/47.9)),estimated:true};
  return {angle:null,estimated:false};
}
function renderOverview() {
  const signals = activeSignals(), steering = steeringPresentation();
  $('overview-cards').replaceChildren();
  const cards = [['rpm_rl','RPM izquierda'],['rpm_rr','RPM derecha'],['accelerator_pct','Pedal'],['steering','Dirección'],['gear','Marcha solicitada'],['pwm_left_raw','PWM izquierdo'],['pwm_right_raw','PWM derecho'],['yaw_dps','Guiñada']];
  for(const [key,label] of cards) {
    const s = key==='steering' ? (signals.steer_deg?.quality==='fresh'?signals.steer_deg:signals.steer_raw) : signals[key];
    const card=document.createElement('div');card.className='overview-card '+(s?.quality || 'missing');
    const name=document.createElement('div');name.className='label';name.textContent=label;
    const value=document.createElement('div');value.className='value';value.textContent=key==='steering'?(steering.angle===null?'—':f(steering.angle)+'°'):display(s);
    const note=document.createElement('div');note.className='meta';note.textContent=demo?'Ejemplo sintético':key==='steering'&&steering.estimated?'Estimación del mando · sin calibrar':!s||s.quality!=='fresh'?'Sin datos vigentes':key.startsWith('pwm_')?'Orden software · 255 = parada':key==='gear'?'Orden CAN · no sentido medido':`${s.node} · ${s.age_ms} ms`;
    card.append(name,value,note);$('overview-cards').appendChild(card);
  }
  let message='Conecta una fuente para ver el diagnóstico.', severity='idle';
  const inhibit=fresh('drive_inhibit'), phase=fresh('control_state');
  if(demo){message='Demostración visual: todas las señales son sintéticas.';severity='demo';}
  else if(!backendOK){message='El servicio local no responde. Las señales dejan de mostrarse como vigentes.';severity='warning';}
  else if(state?.status==='connected') {
    if(!state.stats.valid_frames){message=(state.messages||[]).some(m=>m.text.includes('CAN init failed'))?'CAN no inicializa. Revisa la consola; con el ESP32 solo, sin módulo CAN, estos reintentos son esperables.':'Conexión abierta. Esperando telemetría; el firmware original solo ofrece mensajes de consola.';severity='warning';}
    else if(!Object.values(signals).some(s=>s.quality==='fresh')){message='No llegan muestras recientes. El estado actual del kart es desconocido.';severity='warning';}
    else if(phase==='can_wait'){message=`CAN sin inicializar · ${f(fresh('can_init_attempts')||0,0)} intentos. Wi-Fi disponible para diagnóstico; el control sigue esperando al CAN.`;severity='warning';}
    else if(phase==='mpu_init'){message='Inicializando el giroscopio. Espera a que termine el arranque.';severity='warning';}
    else if(fresh('emergency_stop')===1 || inhibit==='can_timeout'){message='Parada por timeout CAN: han dejado de llegar órdenes válidas del STM32.';severity='warning';}
    else if(fresh('charger_connected')===1){message='Entrada de cargador activa. Comprueba si hay cargador conectado o una lectura inesperada de la entrada.';severity='warning';}
    else if(fresh('can_rx_count')===0){message='CAN inicializado, pero todavía no llegan órdenes del STM32.';severity='warning';}
    else if(inhibit && !['none','pedal_zero','neutral'].includes(inhibit)){message='Control: '+words[inhibit]+'.';severity='warning';}
    else {message=inhibit?`Control: ${words[inhibit]}. Relés mostrados como órdenes, sin confirmar el contacto físico.`:'Telemetría recibida. Selecciona una pieza o consulta todas las señales.';severity='ok';}
  }
  $('diagnostic').textContent=message;$('diagnostic').className='diagnostic '+severity;
  const node=state?.nodes?.esp32;
  $('firmware-info').textContent=demo?'Modo de ejemplo · no es una prueba del kart':node?`Firmware: ${node.fw || 'sin versión'} · Reinicio: ${node.reset_reason || 'consulta la consola'} · Sesión: ${node.session}`:'Versión y motivo de reinicio: pendientes de recibir.';
}
function renderAllSignals() {
  const query=$('signal-search').value.trim().toLocaleLowerCase('es'), signals=activeSignals();
  $('all-signal-rows').replaceChildren();
  for(const [key,s] of Object.entries(signals)) {
    if(!$('show-missing').checked && s.quality==='missing')continue;
    if(query && !`${key} ${s.label}`.toLocaleLowerCase('es').includes(query))continue;
    const row=document.createElement('tr');row.className=s.quality;
    const values=[s.label || key,display(s),s.node || '—',s.quality==='fresh'?`Vigente · ${s.age_ms} ms`:s.quality==='stale'?'Datos antiguos':'Sin datos'];
    for(const text of values){const cell=document.createElement('td');cell.textContent=text;row.appendChild(cell);}
    row.title=key;$('all-signal-rows').appendChild(row);
  }
}
function renderHeader() {
  const connected = state?.status === 'connected';
  const wifi = (connected ? state?.port : $('port').value)?.startsWith('socket://');
  const transport = wifi ? 'Wi-Fi' : 'USB';
  $('connect').textContent = wifi ? 'Leer por Wi-Fi' : 'Leer por cable';
  $('mode-badge').className=demo?'demo':'';
  $('mode-badge').textContent=demo?'DEMO · sin hardware':!backendOK?'Servicio no disponible':connected?(state?.stats.valid_frames?`${transport} · lectura`:'Conexión abierta · sin telemetría'):'Sin conexión';
  $('frame-count').textContent=demo?'Animación con valores sintéticos':`${state?.stats.valid_frames || 0} tramas de telemetría`;
  $('connect').disabled=demo||connected||!backendOK||!$('port').value||!$('bench').checked||!state?.serial_available;
  $('disconnect').disabled=!connected;
  $('demo').disabled=connected;
  $('port').disabled=connected||demo;
  $('bench').disabled=connected||demo;
  $('refresh').disabled=connected||demo;
  $('demo').setAttribute('aria-pressed',String(demo));
  $('demo').textContent=demo?'Salir de demostración':'Vista animada de ejemplo';
  $('demo-banner').hidden=!demo; $('demo-controls').hidden=!demo;
  $('download').hidden=!state?.session_file;
  if (demo) $('connection-info').textContent='Demostración aislada en el navegador. Ningún puerto abierto; no se registra como ensayo real.';
  else if (!backendOK) $('connection-info').textContent='No se recibe el servicio local; se han retirado los valores vigentes.';
  else $('connection-info').textContent=state?.error || (connected ? `Lectura de ${state.port}${wifi ? ' por TCP' : ' a 115200 bit/s'}. ${state.stats.valid_frames ? 'Telemetría estructurada recibida.' : 'Todavía no llegan tramas. Revisa los mensajes de arranque en la consola.'}`:wifi?'Conecta el portátil a la red kart-esp32, marca la preparación del banco y pulsa Leer por Wi-Fi.': 'Selecciona el USB del ESP32 o Wi-Fi directo. Si no aparece el USB, revisa el cable de datos y pulsa Actualizar puertos.');
  $('session-info').textContent=state?.log_error?'Error de registro: '+state.log_error:state?.session_file?`Sesión: ${state.session_file} · Tramas rechazadas: ${state.stats.invalid_frames} · Saltos de secuencia: ${state.stats.sequence_gaps}`:'Sin sesión registrada.';
}
function renderLog() {
  const messages=state?.messages || [];
  if (!messages.length) return;
  if (messages.at(-1).id === lastMessage) return;
  lastMessage=messages.at(-1).id;
  $('log').replaceChildren();
  for (const msg of messages.slice(-80)) {
    const line=document.createElement('div'); line.className='log-line '+msg.kind;
    line.textContent=`${new Date(msg.time).toLocaleTimeString('es-ES',{timeZone:'Europe/Lisbon',hour12:false,fractionalSecondDigits:3})} · ${msg.text}`; $('log').appendChild(line);
  }
}
function sampleTrace() {
  const s=activeSignals()[$('trace-signal').value];
  const sampleKey=demo?'demo-'+demoSeq:s?.sample_id;
  const now=Date.now();
  if (s?.quality==='fresh' && typeof s.value==='number' && sampleKey!==lastSample) { history.push({t:now-(s.age_ms||0),v:s.value}); lastSample=sampleKey; }
  else if (s?.quality!=='fresh' && history.length && history.at(-1).v!==null) history.push({t:now,v:null});
  history=history.filter(p=>p.t>=now-30000).slice(-500);
  drawTrace();
}
function drawTrace() {
  const canvas=$('trace'), width=canvas.clientWidth, height=190, dpr=window.devicePixelRatio||1;
  canvas.width=Math.round(width*dpr);canvas.height=height*dpr;
  const ctx=canvas.getContext('2d');ctx.scale(dpr,dpr);ctx.clearRect(0,0,width,height);
  const left=55,right=15,top=20,bottom=34,plotWidth=Math.max(1,width-left-right),plotHeight=height-top-bottom;
  ctx.font='11px system-ui';ctx.strokeStyle='#2b4153';ctx.fillStyle='#a2b5c7';ctx.lineWidth=1;
  const vals=history.filter(p=>p.v!==null).map(p=>p.v);
  let min=vals.length?Math.min(...vals):0,max=vals.length?Math.max(...vals):1;
  const pad=Math.max((max-min)*.12,Math.max(Math.abs(max)*.05,1));min-=pad;max+=pad;
  const now=Date.now(), x=t=>left+(t-(now-30000))/30000*plotWidth, y=v=>top+(max-v)/(max-min)*plotHeight;
  for(let i=0;i<=3;i++){const yy=top+i/3*plotHeight;ctx.beginPath();ctx.moveTo(left,yy);ctx.lineTo(width-right,yy);ctx.stroke();ctx.fillText(f(max-i/3*(max-min),0),3,yy+4);}
  for(let i=0;i<=3;i++){ctx.fillText(String(-30+i*10)+' s',left+i/3*plotWidth-10,height-10);}
  ctx.save();ctx.beginPath();ctx.rect(left,top,plotWidth,plotHeight);ctx.clip();ctx.strokeStyle=demo?'#f1bc73':'#79d9ba';ctx.lineWidth=2;ctx.beginPath();let pen=false;
  for(const p of history){if(p.v===null){pen=false;continue;}if(!pen){ctx.moveTo(x(p.t),y(p.v));pen=true;}else ctx.lineTo(x(p.t),y(p.v));}ctx.stroke();ctx.restore();
  const s=activeSignals()[$('trace-signal').value];
  $('trace-info').textContent=(demo?'EJEMPLO SINTÉTICO · ':'')+(s?.quality==='fresh'?`${s.label}: ${display(s)}`:'Sin muestras vigentes de la señal seleccionada.');
  canvas.setAttribute('aria-label',$('trace-info').textContent);
}
let traceOptionsKey='';
function renderTraceOptions() {
  const numeric=Object.entries(state?.signals || {}).filter(([key])=>!['gear','mode','control_state','drive_direction','drive_inhibit'].includes(key));
  const stamp=numeric.map(([key])=>key).join(',');
  if(!stamp || stamp===traceOptionsKey)return;
  traceOptionsKey=stamp;const previous=$('trace-signal').value;
  $('trace-signal').replaceChildren();
  for(const [key,s] of numeric){const option=document.createElement('option');option.value=key;option.textContent=s.label;$('trace-signal').appendChild(option);}
  if(numeric.some(([key])=>key===previous))$('trace-signal').value=previous;
}
function render(){renderHeader();renderOverview();renderInspector();renderMap();renderAllSignals();renderLog();renderTraceOptions();sampleTrace();}
async function poll(){
  if(apiBusy)return;apiBusy=true;
  const abort=new AbortController(), timeout=setTimeout(()=>abort.abort(),2000);
  try{
    const response=await fetch('/api/state',{cache:'no-store',signal:abort.signal});if(!response.ok)throw new Error('Estado no disponible');
    state=await response.json();stateReceivedAt=Date.now();token=state.token;backendOK=true;
  }catch{
    backendOK=false;
    if(state){state.status='disconnected';for(const s of Object.values(state.signals))if(s.quality==='fresh')s.quality='stale';}
  }finally{clearTimeout(timeout);apiBusy=false;render();}
}
async function ports(){
  const old=$('port').value;
  try{
    const response=await fetch('/api/ports');if(!response.ok)throw new Error();const data=await response.json();
    $('port').replaceChildren();const empty=document.createElement('option');empty.value='';empty.textContent=data.ports.length?'Selecciona la placa conectada':'Sin puertos detectados';$('port').appendChild(empty);
    for(const p of data.ports){const option=document.createElement('option');option.value=p.device;option.textContent=`${p.device} · ${p.description}`;$('port').appendChild(option);}
    const preferred=state?.status==='connected'?state.port:old;
    if(data.ports.some(p=>p.device===preferred))$('port').value=preferred;
    else $('port').value=data.ports.find(p=>p.hwid.includes('10C4:EA60'))?.device || '';
  }catch{$('connection-info').textContent='No se pudo consultar el inventario de puertos.';}
  renderHeader();
}
async function post(path,payload){
  try{
    const response=await fetch(path,{method:'POST',headers:{'Content-Type':'application/json','X-Kart-Token':token},body:JSON.stringify(payload)});
    const data=await response.json();if(!response.ok)throw new Error(data.error||'No se pudo realizar la operación');
    history=[];lastSample=null;await poll();
  }catch(error){$('connection-info').textContent=error.message;}
}
function makeDemo(){
  const pedal=Number($('demo-pedal').value), steer=Number($('demo-steer').value), t=(performance.now()-demoTime)/1000;
  $('demo-pedal-value').textContent=pedal+' %';$('demo-steer-value').textContent=steer+'°';
  const base=pedal*4, flutter=pedal?Math.sin(t*1.6)*5:0;
  const values={accelerator_pct:pedal,steer_deg:steer,brake:0,gear:pedal?'D':'N',mode:'DIF',rpm_fl:base,rpm_fr:base,rpm_rl:Math.max(0,base-steer*.8+flutter),rpm_rr:Math.max(0,base+steer*.8+flutter),yaw_dps:pedal?steer*.35:0,pwm_left_raw:Math.round(255-pedal*2.55),pwm_right_raw:Math.round(255-pedal*2.55),relay1:0,relay2:1,charger_connected:0,soc_pct:72,battery_v:51.8,battery_a:pedal*.6,battery_temp_c:24,sd_ready:1,can_rx_age_ms:4};
  const fallback={accelerator_pct:['Pedal procesado','%'],steer_deg:['Dirección','°'],brake:['Freno leído',''],gear:['Marcha',''],mode:['Modo diferencial',''],sd_ready:['SD disponible','']};
  demoSeq++;demoFrame={signals:{}};
  for(const [key,value] of Object.entries(values)){const meta=state?.signals[key]||{};demoFrame.signals[key]={label:meta.label||fallback[key]?.[0]||key,unit:meta.unit||fallback[key]?.[1]||'',value,quality:'fresh',age_ms:0,node:'demo',sample_id:demoSeq};}
}
let lastAnimation=performance.now(), lastDemoRender=0;
function animate(now){
  const dt=Math.min((now-lastAnimation)/1000,.1);lastAnimation=now;
  if(demo&&now-lastDemoRender>180){makeDemo();render();lastDemoRender=now;}
  for(const side of ['fl','fr','rl','rr']){
    const rpm=fresh('rpm_'+side);
    const direction=fresh('drive_direction') || fresh('gear');
    if(rpm!==null&&!reducedMotion.matches&&!motionPaused)angles[side]=(angles[side]+(direction==='R'?-1:1)*Math.min(Math.abs(rpm)*.18,100)*dt)%18;
    $('rim-'+side).style.transform=`translateY(${angles[side]}px)`;
  }
  steerAngle = reducedMotion.matches || motionPaused ? steerTarget : steerAngle+(steerTarget-steerAngle)*(1-Math.exp(-12*dt));
  $('steer-left').setAttribute('transform',`rotate(${steerAngle} 179 132)`);
  $('steer-right').setAttribute('transform',`rotate(${steerAngle} 501 132)`);
  requestAnimationFrame(animate);
}
document.querySelectorAll('[data-part]').forEach(el=>{el.addEventListener('click',()=>selectPart(el.dataset.part));el.addEventListener('keydown',event=>{if(event.key==='Enter'||event.key===' '){event.preventDefault();selectPart(el.dataset.part);}});});
$('demo').addEventListener('click',()=>{if(state?.status==='connected')return;demo=!demo;demoTime=performance.now();history=[];lastSample=null;if(demo)makeDemo();else demoFrame=null;render();});
$('demo-pedal').addEventListener('input',()=>{makeDemo();render();});$('demo-steer').addEventListener('input',()=>{makeDemo();render();});
$('refresh').addEventListener('click',ports);$('port').addEventListener('change',renderHeader);$('bench').addEventListener('change',renderHeader);
$('motion').addEventListener('click',()=>{motionPaused=!motionPaused;$('motion').setAttribute('aria-pressed',String(motionPaused));$('motion').textContent=motionPaused?'Reanudar animación':'Pausar animación';});
$('signal-search').addEventListener('input',renderAllSignals);$('show-missing').addEventListener('change',renderAllSignals);
$('connect').addEventListener('click',()=>post('/api/connect',{port:$('port').value,bench_ready:$('bench').checked}));
$('disconnect').addEventListener('click',()=>post('/api/disconnect',{}));
$('trace-signal').addEventListener('change',()=>{history=[];lastSample=null;sampleTrace();});
new ResizeObserver(drawTrace).observe($('trace'));
selectPart('esp32');poll().then(ports);setInterval(poll,300);requestAnimationFrame(animate);
