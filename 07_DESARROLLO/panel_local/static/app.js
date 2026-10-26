// Panel local del kart: una pantalla con barra de estado, canales fijados, vista del kart y gráficas sincronizadas.
import {$, app, activeSignals, fresh, display, word, f, ago, anomaly, saved, save, idioma, t, srv} from './nucleo.js';
import {initIdioma, onIdioma} from './i18n.js';
import {SPACES, WINDOW_MS, layout, signal, nature, shortName, record, clearHistory, getHistory, initStrip, renderStrip, initPalette, openPalette, renderPalette, isPinned, pin, unpin, showFree, resetSpace} from './canales.js';
import {buildCharts, drawCharts} from './graficas.js';
import {partOf, selectPart, selected, renderInspector, renderMap, animate, initMap} from './vista_kart.js';
import {initCalibration, renderCalibration} from './calibracion.js';
import {initHistory,replayTick} from './historial.js';

let lastMessage = 0, apiBusy = false, rates = [];
// Demo sintética: pedal 100 % ≈ 550 RPM de rueda ≈ 50 km/h con el Ø 0,48 m que usa el firmware (pendiente de contraste).
const DEMO_RPM_POR_PCT = 5.5, DEMO_KMH_POR_RPM = Math.PI*.48*60/1000;

function diagnostic() {
  const signals=activeSignals(), inhibit=fresh('drive_inhibit'), phase=fresh('control_state'), state=app.state;
  if(app.replay)return ['Reproducción histórica · '+app.replay.manifest.title,'demo'];
  if(app.demo)return [t('diag.demo'),'demo'];
  if(!app.backendOK)return [t('diag.servicio'),'aviso'];
  if(state?.status!=='connected')return [t('diag.sin_conexion'),''];
  if(!state.stats.valid_frames)return [(state.messages||[]).some(m=>m.text.includes('CAN init failed'))?t('diag.can_init_falla'):t('diag.esperando'),'aviso'];
  if(!Object.values(signals).some(s=>s.quality==='fresh'))return [t('diag.sin_muestras'),'aviso'];
  if(phase==='can_wait')return [t('diag.can_wait',{n:f(fresh('can_init_attempts')||0,0)}),'aviso'];
  if(phase==='mpu_init')return [t('diag.mpu_init'),'aviso'];
  if(fresh('emergency_stop')===1 || inhibit==='can_timeout')return [t('diag.timeout'),'alarma'];
  if(fresh('charger_connected')===1)return [t('diag.cargador'),'aviso'];
  if(fresh('can_rx_count')===0)return [t('diag.sin_ordenes'),'aviso'];
  if(inhibit && !['none','pedal_zero','neutral','brake'].includes(inhibit))return [t('diag.control',{estado:word(inhibit)||inhibit}),'aviso'];
  return [inhibit?t('diag.control',{estado:word(inhibit)||inhibit}):t('diag.telemetria'),''];
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
  $('connect').textContent = wifi ? t('leer.wifi') : t('leer.cable');
  $('mode-badge').className=app.demo?'demo':connected&&state?.stats.valid_frames?'en-linea':'';
  $('mode-badge').textContent=app.demo?t('badge.demo'):!app.backendOK?t('badge.servicio'):connected?(state?.stats.valid_frames?t('badge.lectura',{transporte:transport}):t('badge.sin_telemetria')):t('badge.sin_conexion');
  $('connect').disabled=app.demo||connected||!app.backendOK||!$('port').value||!$('bench').checked||!state?.serial_available;
  $('disconnect').disabled=!connected;
  $('demo').disabled=connected;
  $('port').disabled=connected||app.demo;
  $('bench').disabled=connected||app.demo;
  $('refresh').disabled=connected||app.demo;
  $('demo').setAttribute('aria-pressed',String(app.demo));
  $('demo').textContent=app.demo?t('demo.salir'):t('demo');
  $('motion').textContent=app.motionPaused?t('btn.reanudar'):t('btn.pausar');
  $('ampliar-modelo').textContent=document.body.classList.contains('modelo-ampliado')?t('btn.reducir'):t('btn.ampliar');
  $('demo-banner').hidden=!app.demo; $('demo-controls').hidden=!app.demo;
  document.body.classList.toggle('modo-demo',app.demo);
  $('download').hidden=!state?.session_file;
  if (app.demo) $('connection-info').textContent=t('info.demo');
  else if (!app.backendOK) $('connection-info').textContent=t('info.sin_servicio');
  else $('connection-info').textContent=state?.error || (connected ? `${t(wifi?'info.lectura_tcp':'info.lectura_serie',{puerto:state.port})} ${state.stats.valid_frames ? t('info.telemetria_ok') : t('info.sin_tramas')}`:wifi?t('info.wifi_listo'):t('info.usb_listo'));
  $('session-info').textContent=state?.log_error?t('sesion.error_registro',{error:state.log_error}):state?.session_file?t('sesion.activa',{archivo:state.session_file,rechazadas:state.stats.invalid_frames,saltos:state.stats.sequence_gaps}):t('sesion.sin');
  if(app.replay){
    for(const id of ['connect','demo','port','bench','refresh'])$(id).disabled=true;
    $('mode-badge').textContent='REPRODUCCIÓN';$('mode-badge').className='demo';
    $('connection-info').textContent='Datos de una prueba guardada · tiempo de reproducción independiente';
    $('session-info').textContent=app.replay.manifest.title;
    $('download').hidden=true;
  }
  const [message,severity]=diagnostic();
  $('diagnostic').textContent=message;$('diagnostic').className='diagnostic '+severity;
  const node=state?.nodes?.esp32, rate=receptionRate(), phase=fresh('control_state'), inhibit=fresh('drive_inhibit');
  $('indicadores').replaceChildren(
    chip(t('chip.recepcion'),app.demo?t('chip.demo'):rate===null?'—':f(rate,1)+' Hz',connected&&rate!==null&&rate<1?'aviso':null),
    chip(t('chip.saltos'),String(state?.stats?.sequence_gaps??0),state?.stats?.sequence_gaps?'aviso':null),
    chip(t('chip.rechazadas'),String(state?.stats?.invalid_frames??0),state?.stats?.invalid_frames?'aviso':null),
    chip(t('chip.firmware'),app.demo?'—':node?.fw||'—'),
    chip(t('chip.reinicio'),app.demo?'—':node?.reset_reason||'—'),
    chip(t('chip.fase'),phase?word(phase)||phase:'—',anomaly('control_state',phase)),
    chip(t('chip.bloqueo'),inhibit?word(inhibit)||inhibit:'—',anomaly('drive_inhibit',inhibit)));
}
function renderAllSignals() {
  if($('panel-senales').hidden)return;
  const query=$('signal-search').value.trim().toLocaleLowerCase(idioma.locale), signals=activeSignals();
  $('all-signal-rows').replaceChildren();
  for(const [key,s] of Object.entries(signals)) {
    if(!$('show-missing').checked && s.quality==='missing')continue;
    if(query && !`${key} ${s.label}`.toLocaleLowerCase(idioma.locale).includes(query))continue;
    const row=document.createElement('tr');row.className=s.quality;
    const values=[s.label || key,display(s,key),s.node || '—',s.quality==='fresh'?t('estado.vigente',{edad:ago(s.age_ms)}):s.quality==='stale'?t('estado.antiguo',{edad:ago(s.age_ms)}):t('estado.sin_datos')];
    for(const text of values){const cell=document.createElement('td');cell.textContent=text;row.appendChild(cell);}
    const cell=document.createElement('td'), button=document.createElement('button');button.type='button';button.textContent=isPinned(key)?t('quitar'):t('fijar');
    button.addEventListener('click',()=>{isPinned(key)?unpin(key):pin(key);refresh();});
    cell.appendChild(button);row.appendChild(cell);
    row.title=key;row.addEventListener('dblclick',()=>showFree(key));$('all-signal-rows').appendChild(row);
  }
}
function renderLog() {
  const messages=app.state?.messages || [];
  $('ver-consola').textContent=`${t('consola')} · ${messages.length}`;
  if (!messages.length && !$('log').querySelector('.log-line')) $('log').textContent=t('consola.vacia');
  if (!messages.length || messages.at(-1).id === lastMessage) return;
  lastMessage=messages.at(-1).id;
  $('log').replaceChildren();
  for (const msg of messages.slice(-120)) {
    const line=document.createElement('div'); line.className='log-line '+msg.kind;
    line.textContent=`${new Date(msg.time).toLocaleTimeString(idioma.locale,{timeZone:'Europe/Lisbon',hour12:false,fractionalSecondDigits:3})} · ${srv(msg.text)}`; $('log').appendChild(line);
  }
  $('log').scrollTop=$('log').scrollHeight;
}
let chartsKey='';
function renderCharts() {
  const free=layout.free, s=signal(free), stamp=layout.active+':'+free;
  if(stamp!==chartsKey){
    chartsKey=stamp;
    buildCharts($('graficas'),[...SPACES[layout.active].charts.map(chart=>({...chart,title:t(chart.title)})),{title:t('graf.seleccion',{nombre:shortName(free,s)}),keys:[free],free:true}],{history:getHistory,nature,shortName,signal,windowMs:WINDOW_MS});
  }
  drawCharts();
}
function render(){record();renderHeader();renderStrip();renderInspector();renderMap();renderCharts();renderAllSignals();renderLog();renderCalibration();if($('paleta').open)renderPalette();}
function refresh(){chartsKey='';render();}
async function poll(){
  if(apiBusy)return;apiBusy=true;
  const abort=new AbortController(), timeout=setTimeout(()=>abort.abort(),2000);
  try{
    const response=await fetch('/api/state',{cache:'no-store',signal:abort.signal});if(!response.ok)throw new Error('Estado no disponible');
    const next=await response.json();
    if(app.state?.calibration?.active?.id!==next.calibration?.active?.id){clearHistory();chartsKey='';}
    app.liveState=next;app.token=next.token;
    if(app.replay)return;
    app.state=next;app.stateReceivedAt=Date.now();app.token=app.state.token;app.backendOK=true;
  }catch{
    app.backendOK=false;
    if(app.state){app.state.status='disconnected';for(const s of Object.values(app.state.signals))if(s.quality==='fresh')s.quality='stale';}
  }finally{clearTimeout(timeout);apiBusy=false;if(!app.demo)render();}
}
// Inventario de puertos: respuesta de /api/ports (null hasta la primera consulta). Se redibuja también al cambiar de idioma.
let portData=null;
function renderPorts(preferred, useDefault) {
  const list=portData?.ports||[];
  $('port').replaceChildren();
  const empty=document.createElement('option');empty.value='';
  empty.textContent=portData===null?t('puerto.buscando'):list.length?t('puerto.selecciona'):t('puerto.ninguno');$('port').appendChild(empty);
  for(const p of list){const option=document.createElement('option');option.value=p.device;option.textContent=`${p.device} · ${srv(p.description)}`;$('port').appendChild(option);}
  if(list.some(p=>p.device===preferred))$('port').value=preferred;
  else $('port').value=useDefault?list.find(p=>p.hwid.includes('10C4:EA60'))?.device || '':'';
}
async function ports(){
  const old=$('port').value;
  try{
    const response=await fetch('/api/ports');if(!response.ok)throw new Error();
    portData=await response.json();
    renderPorts(app.state?.status==='connected'?app.state.port:old,true);
  }catch{$('connection-info').textContent=t('puerto.error');}
  renderHeader();
}
async function post(path,payload){
  try{
    const response=await fetch(path,{method:'POST',headers:{'Content-Type':'application/json','X-Kart-Token':app.token},body:JSON.stringify(payload)});
    const data=await response.json();if(!response.ok)throw new Error(srv(data.error)||t('post.error'));
    clearHistory();rates=[];await poll();
  }catch(error){$('connection-info').textContent=error.message;}
}
function makeDemo(){
  const pedal=Number($('demo-pedal').value), steer=Number($('demo-steer').value), gear=$('demo-gear').value;
  $('demo-pedal-value').textContent=pedal+' % · ≈ '+Math.round(pedal*DEMO_RPM_POR_PCT*DEMO_KMH_POR_RPM)+' km/h';$('demo-steer-value').textContent=steer+'° · '+(steer<0?t('demo.izquierda'):steer>0?t('demo.derecha'):t('demo.recto'));
  const base=pedal*DEMO_RPM_POR_PCT;
  // Convención visual de demo: positivo = derecha; rueda I exterior más rápida.
  // El reparto desaparece con pedal cero; no reproduce el algoritmo del firmware.
  const split=base*steer/100;
  const values={accelerator_pct:pedal,steer_deg:steer,brake:0,gear,mode:'DIF',rpm_fl:base,rpm_fr:base,rpm_rl:Math.max(0,base+split),rpm_rr:Math.max(0,base-split),yaw_dps:pedal?steer*.35*(gear==='R'?-1:1):0,pwm_left_raw:Math.round(255-pedal*2.55),pwm_right_raw:Math.round(255-pedal*2.55),relay1:0,relay2:1,charger_connected:0,soc_pct:72,battery_v:51.8,battery_a:pedal*.6,battery_temp_c:24,sd_ready:1,can_rx_age_ms:4};
  const fallback={accelerator_pct:['Pedal procesado','%'],steer_deg:['Dirección','°'],brake:['Freno leído',''],gear:['Marcha',''],mode:['Modo diferencial',''],sd_ready:['SD disponible','']};
  app.demoSeq++;app.demoFrame={signals:{}};
  for(const [key,value] of Object.entries(values)){const meta=app.state?.signals[key]||{};app.demoFrame.signals[key]={label:srv(meta.label||fallback[key]?.[0]||key),unit:srv(meta.unit||fallback[key]?.[1]||''),value,quality:'fresh',age_ms:0,node:'demo',sample_id:app.demoSeq};}
}
let lastAnimation=performance.now(), lastDemoRender=0;
function loop(now){
  const dt=Math.min((now-lastAnimation)/1000,.1);lastAnimation=now;
  if(app.demo&&now-lastDemoRender>180){makeDemo();render();lastDemoRender=now;}
  replayTick(now);
  animate(app.replay?(app.replay.clock.playing?dt*app.replay.clock.speed:0):dt);
  requestAnimationFrame(loop);
}
function toggleDrawer(which) {
  for(const name of ['senales','consola']){
    const open=name===which&&$('panel-'+name).hidden;
    $('panel-'+name).hidden=!open;$('ver-'+name).setAttribute('aria-expanded',String(open));
  }
  renderAllSignals();
}
initIdioma();
initMap(()=>{});
initCalibration(poll);
initHistory(refresh,poll);
initStrip({onSpace:refresh,onChange:refresh,partOf,onPart:key=>{const part=partOf(key);if(part)selectPart(part.id);}});
initPalette();
// Cambio de idioma: lo que se construye por código se rehace; el resto lo cubre applyDom() en i18n.js.
onIdioma(()=>{
  lastMessage=0;
  renderPorts($('port').value,false);
  selectPart(selected);
  if(app.demo)makeDemo();
  refresh();
});
$('demo').addEventListener('click',()=>{if(app.replay||app.state?.status==='connected')return;app.demo=!app.demo;clearHistory();if(app.demo)makeDemo();else app.demoFrame=null;render();});
$('demo-pedal').addEventListener('input',()=>{makeDemo();render();});$('demo-steer').addEventListener('input',()=>{makeDemo();render();});
$('demo-gear').addEventListener('change',()=>{makeDemo();render();});
// Giro de ruedas: real 1:1 por defecto; «Lento» conserva el 3 % anterior. Preferencia local del navegador.
app.wheelScale=[1,.03].includes(saved('giro',1))?saved('giro',1):1;$('giro-escala').value=String(app.wheelScale);
$('giro-escala').addEventListener('change',event=>{app.wheelScale=Number(event.target.value)||1;save('giro',app.wheelScale);});
$('refresh').addEventListener('click',ports);$('port').addEventListener('change',renderHeader);$('bench').addEventListener('change',renderHeader);
$('motion').addEventListener('click',()=>{app.motionPaused=!app.motionPaused;$('motion').setAttribute('aria-pressed',String(app.motionPaused));$('motion').textContent=app.motionPaused?t('btn.reanudar'):t('btn.pausar');});
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
