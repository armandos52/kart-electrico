// Panel local del kart: una pantalla con barra de estado, canales fijados, vista del kart y gráficas sincronizadas.
import {$, app, activeSignals, fresh, display, words, f, ago, anomaly} from './nucleo.js';
import {SPACES, WINDOW_MS, layout, signal, nature, shortName, record, clearHistory, getHistory, initStrip, renderStrip, initPalette, openPalette, renderPalette, isPinned, pin, unpin, showFree, resetSpace} from './canales.js';
import {buildCharts, drawCharts} from './graficas.js';
import {partOf, selectPart, renderInspector, renderMap, animate, initMap} from './vista_kart.js';

let lastMessage = 0, apiBusy = false, rates = [];

function diagnostic() {
  const signals = activeSignals(), inhibit=fresh('drive_inhibit'), phase=fresh('control_state'), state=app.state;
  if(app.demo)return ['Demostración: todas las señales son sintéticas.','demo'];
  if(!app.backendOK)return ['El servicio local no responde. Las señales dejan de mostrarse como vigentes.','aviso'];
  if(state?.status!=='connected')return ['Sin conexión. Conecta una fuente para ver el diagnóstico.',''];
  if(!state.stats.valid_frames)return [(state.messages||[]).some(m=>m.text.includes('CAN init failed'))?'CAN no inicializa. Con el ESP32 solo, sin MCP2515, los reintentos son esperables.':'Conexión abierta. Esperando telemetría; el firmware original solo ofrece mensajes de consola.','aviso'];
  if(!Object.values(signals).some(s=>s.quality==='fresh'))return ['No llegan muestras recientes. El estado actual del kart es desconocido.','aviso'];
  if(phase==='can_wait')return [`CAN sin inicializar · ${f(fresh('can_init_attempts')||0,0)} intentos. El control sigue esperando al CAN.`,'aviso'];
  if(phase==='mpu_init')return ['Inicializando el giroscopio. Espera a que termine el arranque.','aviso'];
  if(fresh('emergency_stop')===1 || inhibit==='can_timeout')return ['Parada por timeout CAN: han dejado de llegar órdenes válidas del STM32.','alarma'];
  if(fresh('charger_connected')===1)return ['Entrada de cargador activa. Comprueba si hay cargador o una lectura inesperada de la entrada.','aviso'];
  if(fresh('can_rx_count')===0)return ['CAN inicializado, pero todavía no llegan órdenes del STM32.','aviso'];
  if(inhibit && !['none','pedal_zero','neutral','brake'].includes(inhibit))return ['Control: '+words[inhibit]+'.','aviso'];
  return [inhibit?`Control: ${words[inhibit]}.`:'Telemetría recibida.',''];
}
function chip(label, value, level) {
  const item=document.createElement('span');item.className='indicador'+(level?' '+level:'');
  const name=document.createElement('span');name.textContent=label;const strong=document.createElement('b');strong.textContent=value;
  item.append(name,strong);return item;
}
function receptionRate() {
  const now=Date.now(), frames=app.state?.stats?.valid_frames||0;
  rates.push({t:now,frames});rates=rates.filter(r=>r.t>=now-5000);
  if(app.state?.status!=='connected'||rates.length<2)return null;
  const first=rates[0], span=(now-first.t)/1000;
  return span>0.8?(frames-first.frames)/span:null;
}
function renderHeader() {
  const state=app.state, connected = state?.status === 'connected';
  const wifi = (connected ? state?.port : $('port').value)?.startsWith('socket://');
  const transport = wifi ? 'Wi-Fi' : 'USB';
  $('connect').textContent = wifi ? 'Leer por Wi-Fi' : 'Leer por cable';
  $('mode-badge').className=app.demo?'demo':connected&&state?.stats.valid_frames?'en-linea':'';
  $('mode-badge').textContent=app.demo?'DEMO · sin hardware':!app.backendOK?'Servicio no disponible':connected?(state?.stats.valid_frames?`${transport} · lectura`:'Conexión abierta · sin telemetría'):'Sin conexión';
  $('connect').disabled=app.demo||connected||!app.backendOK||!$('port').value||!$('bench').checked||!state?.serial_available;
  $('disconnect').disabled=!connected;
  $('demo').disabled=connected;
  $('port').disabled=connected||app.demo;
  $('bench').disabled=connected||app.demo;
  $('refresh').disabled=connected||app.demo;
  $('demo').setAttribute('aria-pressed',String(app.demo));
  $('demo').textContent=app.demo?'Salir de demostración':'Demostración';
  $('demo-banner').hidden=!app.demo; $('demo-controls').hidden=!app.demo;
  document.body.classList.toggle('modo-demo',app.demo);
  $('download').hidden=!state?.session_file;
  if (app.demo) $('connection-info').textContent='Demostración aislada en el navegador. Ningún puerto abierto; no se registra como ensayo real.';
  else if (!app.backendOK) $('connection-info').textContent='No se recibe el servicio local; se han retirado los valores vigentes.';
  else $('connection-info').textContent=state?.error || (connected ? `Lectura de ${state.port}${wifi ? ' por TCP' : ' a 115200 bit/s'}. ${state.stats.valid_frames ? 'Telemetría estructurada recibida.' : 'Todavía no llegan tramas. Revisa los mensajes de arranque en la consola.'}`:wifi?'Conecta el portátil a la red kart-esp32, marca la preparación del banco y pulsa Leer por Wi-Fi. Abrir la fuente puede reiniciar la placa.': 'Selecciona el USB del ESP32 o Wi-Fi directo. Abrir el puerto puede reiniciar la placa.');
  $('session-info').textContent=state?.log_error?'Error de registro: '+state.log_error:state?.session_file?`Sesión ${state.session_file} · rechazadas ${state.stats.invalid_frames} · saltos ${state.stats.sequence_gaps}`:'Sin sesión registrada.';
  const [message,severity]=diagnostic();
  $('diagnostic').textContent=message;$('diagnostic').className='diagnostic '+severity;
  const node=state?.nodes?.esp32, rate=receptionRate(), phase=fresh('control_state'), inhibit=fresh('drive_inhibit');
  $('indicadores').replaceChildren(
    chip('Recepción',app.demo?'demo':rate===null?'—':f(rate,1)+' Hz',connected&&rate!==null&&rate<1?'aviso':null),
    chip('Saltos',String(state?.stats?.sequence_gaps??0),state?.stats?.sequence_gaps?'aviso':null),
    chip('Rechazadas',String(state?.stats?.invalid_frames??0),state?.stats?.invalid_frames?'aviso':null),
    chip('Firmware',app.demo?'—':node?.fw||'—'),
    chip('Reinicio',app.demo?'—':node?.reset_reason||'—'),
    chip('Fase',phase?words[phase]:'—',anomaly('control_state',phase)),
    chip('Bloqueo',inhibit?words[inhibit]:'—',anomaly('drive_inhibit',inhibit)));
}
function renderAllSignals() {
  if($('panel-senales').hidden)return;
  const query=$('signal-search').value.trim().toLocaleLowerCase('es'), signals=activeSignals();
  $('all-signal-rows').replaceChildren();
  for(const [key,s] of Object.entries(signals)) {
    if(!$('show-missing').checked && s.quality==='missing')continue;
    if(query && !`${key} ${s.label}`.toLocaleLowerCase('es').includes(query))continue;
    const row=document.createElement('tr');row.className=s.quality;
    const values=[s.label || key,display(s,key),s.node || '—',s.quality==='fresh'?`Vigente · ${ago(s.age_ms)}`:s.quality==='stale'?`Antiguo · hace ${ago(s.age_ms)}`:'Sin datos'];
    for(const text of values){const cell=document.createElement('td');cell.textContent=text;row.appendChild(cell);}
    const cell=document.createElement('td'), button=document.createElement('button');button.type='button';button.textContent=isPinned(key)?'Quitar':'Fijar';
    button.addEventListener('click',()=>{isPinned(key)?unpin(key):pin(key);refresh();});
    cell.appendChild(button);row.appendChild(cell);
    row.title=key;row.addEventListener('dblclick',()=>showFree(key));$('all-signal-rows').appendChild(row);
  }
}
function renderLog() {
  const messages=app.state?.messages || [];
  $('ver-consola').textContent=`Consola · ${messages.length}`;
  if (!messages.length || messages.at(-1).id === lastMessage) return;
  lastMessage=messages.at(-1).id;
  $('log').replaceChildren();
  for (const msg of messages.slice(-120)) {
    const line=document.createElement('div'); line.className='log-line '+msg.kind;
    line.textContent=`${new Date(msg.time).toLocaleTimeString('es-ES',{timeZone:'Europe/Lisbon',hour12:false,fractionalSecondDigits:3})} · ${msg.text}`; $('log').appendChild(line);
  }
  $('log').scrollTop=$('log').scrollHeight;
}
let chartsKey='';
function renderCharts() {
  const free=layout.free, s=signal(free), stamp=layout.active+':'+free;
  if(stamp!==chartsKey){
    chartsKey=stamp;
    buildCharts($('graficas'),[...SPACES[layout.active].charts,{title:'Selección · '+shortName(free,s),keys:[free],free:true}],{history:getHistory,nature,shortName,signal,windowMs:WINDOW_MS});
  }
  drawCharts();
}
function render(){record();renderHeader();renderStrip();renderInspector();renderMap();renderCharts();renderAllSignals();renderLog();if($('paleta').open)renderPalette();}
function refresh(){chartsKey='';render();}
async function poll(){
  if(apiBusy)return;apiBusy=true;
  const abort=new AbortController(), timeout=setTimeout(()=>abort.abort(),2000);
  try{
    const response=await fetch('/api/state',{cache:'no-store',signal:abort.signal});if(!response.ok)throw new Error('Estado no disponible');
    app.state=await response.json();app.stateReceivedAt=Date.now();app.token=app.state.token;app.backendOK=true;
  }catch{
    app.backendOK=false;
    if(app.state){app.state.status='disconnected';for(const s of Object.values(app.state.signals))if(s.quality==='fresh')s.quality='stale';}
  }finally{clearTimeout(timeout);apiBusy=false;if(!app.demo)render();}
}
async function ports(){
  const old=$('port').value;
  try{
    const response=await fetch('/api/ports');if(!response.ok)throw new Error();const data=await response.json();
    $('port').replaceChildren();const empty=document.createElement('option');empty.value='';empty.textContent=data.ports.length?'Selecciona la placa conectada':'Sin puertos detectados';$('port').appendChild(empty);
    for(const p of data.ports){const option=document.createElement('option');option.value=p.device;option.textContent=`${p.device} · ${p.description}`;$('port').appendChild(option);}
    const preferred=app.state?.status==='connected'?app.state.port:old;
    if(data.ports.some(p=>p.device===preferred))$('port').value=preferred;
    else $('port').value=data.ports.find(p=>p.hwid.includes('10C4:EA60'))?.device || '';
  }catch{$('connection-info').textContent='No se pudo consultar el inventario de puertos.';}
  renderHeader();
}
async function post(path,payload){
  try{
    const response=await fetch(path,{method:'POST',headers:{'Content-Type':'application/json','X-Kart-Token':app.token},body:JSON.stringify(payload)});
    const data=await response.json();if(!response.ok)throw new Error(data.error||'No se pudo realizar la operación');
    clearHistory();rates=[];await poll();
  }catch(error){$('connection-info').textContent=error.message;}
}
function makeDemo(){
  const pedal=Number($('demo-pedal').value), steer=Number($('demo-steer').value), t=(performance.now()-demoTime)/1000;
  $('demo-pedal-value').textContent=pedal+' %';$('demo-steer-value').textContent=steer+'°';
  const base=pedal*4, flutter=pedal?Math.sin(t*1.6)*5:0;
  const values={accelerator_pct:pedal,steer_deg:steer,brake:0,gear:pedal?'D':'N',mode:'DIF',rpm_fl:base,rpm_fr:base,rpm_rl:Math.max(0,base-steer*.8+flutter),rpm_rr:Math.max(0,base+steer*.8+flutter),yaw_dps:pedal?steer*.35:0,pwm_left_raw:Math.round(255-pedal*2.55),pwm_right_raw:Math.round(255-pedal*2.55),relay1:0,relay2:1,charger_connected:0,soc_pct:72,battery_v:51.8,battery_a:pedal*.6,battery_temp_c:24,sd_ready:1,can_rx_age_ms:4};
  const fallback={accelerator_pct:['Pedal procesado','%'],steer_deg:['Dirección','°'],brake:['Freno leído',''],gear:['Marcha',''],mode:['Modo diferencial',''],sd_ready:['SD disponible','']};
  app.demoSeq++;app.demoFrame={signals:{}};
  for(const [key,value] of Object.entries(values)){const meta=app.state?.signals[key]||{};app.demoFrame.signals[key]={label:meta.label||fallback[key]?.[0]||key,unit:meta.unit||fallback[key]?.[1]||'',value,quality:'fresh',age_ms:0,node:'demo',sample_id:app.demoSeq};}
}
let demoTime=performance.now(), lastAnimation=performance.now(), lastDemoRender=0;
function loop(now){
  const dt=Math.min((now-lastAnimation)/1000,.1);lastAnimation=now;
  if(app.demo&&now-lastDemoRender>180){makeDemo();render();lastDemoRender=now;}
  animate(dt);
  requestAnimationFrame(loop);
}
function toggleDrawer(which) {
  for(const name of ['senales','consola']){
    const open=name===which&&$('panel-'+name).hidden;
    $('panel-'+name).hidden=!open;$('ver-'+name).setAttribute('aria-expanded',String(open));
  }
  renderAllSignals();
}
initMap(()=>{});
initStrip({onSpace:refresh,onChange:refresh,partOf,onPart:key=>{const part=partOf(key);if(part)selectPart(part.id);}});
initPalette();
$('demo').addEventListener('click',()=>{if(app.state?.status==='connected')return;app.demo=!app.demo;demoTime=performance.now();clearHistory();if(app.demo)makeDemo();else app.demoFrame=null;render();});
$('demo-pedal').addEventListener('input',()=>{makeDemo();render();});$('demo-steer').addEventListener('input',()=>{makeDemo();render();});
$('refresh').addEventListener('click',ports);$('port').addEventListener('change',renderHeader);$('bench').addEventListener('change',renderHeader);
$('motion').addEventListener('click',()=>{app.motionPaused=!app.motionPaused;$('motion').setAttribute('aria-pressed',String(app.motionPaused));$('motion').textContent=app.motionPaused?'Reanudar animación':'Pausar animación';});
$('signal-search').addEventListener('input',renderAllSignals);$('show-missing').addEventListener('change',renderAllSignals);
$('connect').addEventListener('click',()=>post('/api/connect',{port:$('port').value,bench_ready:$('bench').checked}));
$('disconnect').addEventListener('click',()=>post('/api/disconnect',{}));
$('buscar').addEventListener('click',openPalette);
$('restablecer').addEventListener('click',resetSpace);
$('ver-senales').addEventListener('click',()=>toggleDrawer('senales'));$('ver-consola').addEventListener('click',()=>toggleDrawer('consola'));
document.addEventListener('keydown',event=>{
  if((event.ctrlKey||event.metaKey)&&event.key.toLowerCase()==='k'){event.preventDefault();openPalette();}
  else if(event.key==='Escape'&&!$('paleta').open){$('panel-senales').hidden=true;$('panel-consola').hidden=true;}
});
new ResizeObserver(()=>drawCharts()).observe($('graficas'));
selectPart('esp32');poll().then(ports);setInterval(poll,300);requestAnimationFrame(loop);
