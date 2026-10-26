import {$,app} from './nucleo.js';
import {ReplayClock,seriesFromFrames} from './replay.js';
import {clearHistory} from './canales.js';

let refresh=()=>{},poll=()=>{},generation=0,busy=false,view=null,lastRequest=0,windowEnd=-1;
const time=ms=>`${Math.floor(ms/60000)}:${(ms/1000%60).toFixed(2).padStart(5,'0')}`;
async function api(path,data){
  const response=await fetch('/api/history/'+path,data===undefined?{cache:'no-store'}:{method:'POST',headers:{'Content-Type':'application/json','X-Kart-Token':app.token},body:JSON.stringify(data)});
  const result=await response.json();if(!response.ok)throw new Error(result.error||'Error de historial');return result;
}
function message(text){$('history-message').textContent=text;}
async function list(){
  const {sessions}=await api('list');const select=$('history-list');const old=select.value;select.replaceChildren();
  for(const s of sessions){const option=document.createElement('option');option.value=s.id;option.textContent=`${s.title} · ${time(s.duration_ms)} · ${s.closed?'cerrada':'cierre sin verificar'}`;select.append(option);}
  if([...select.options].some(o=>o.value===old))select.value=old;
  $('history-open').disabled=!sessions.length;
  if(!sessions.length)message('Importa una carpeta de la microSD o una sesión CSV/JSONL. Los originales se conservarán.');
}
async function importFiles(files){
  if(!files.length)return;message('Preparando importación…');const {batch}=await api('start',{});
  let i=0;
  for(const file of files){
    if(!/\.(csv|jsonl|txt)$/i.test(file.name))continue;
    message(`Copiando ${++i}: ${file.name}`);
    const name=file.webkitRelativePath||file.name;
    const response=await fetch(`/api/history/upload?batch=${batch}&name=${encodeURIComponent(name)}`,{method:'POST',headers:{'X-Kart-Token':app.token,'Content-Type':'application/octet-stream'},body:file});
    const result=await response.json();if(!response.ok)throw new Error(result.error);
  }
  message('Validando e indexando muestras…');const result=await api('finish',{batch});await list();
  message(`${result.sessions.length} registros: ${result.sessions.filter(s=>s.duplicate).length} ya estaban importados. Consulta los avisos de cada prueba.`);
}
async function open(){
  if(app.liveState?.status==='connected'||app.state?.status==='connected')throw new Error('Desconecta la lectura en directo para abrir una reproducción.');
  const id=$('history-list').value;if(!id)return;
  const manifest=await api('manifest?id='+id);const eventResult=await api('events?id='+id);
  generation++;app.demo=false;clearHistory();view=manifest;windowEnd=-1;
  app.replay={id,epoch:Date.now(),clock:new ReplayClock(manifest.duration_ms),history:new Map(),manifest};app.replay.clock.last=performance.now();
  $('replay-controls').hidden=false;$('replay-seek').max=String(manifest.duration_ms);$('replay-title').textContent=manifest.title;
  $('replay-date').textContent=`${manifest.test_date||'Fecha de ensayo sin confirmar'} · ${manifest.date_source}`;
  $('history-notes').value=manifest.notes||'';$('history-title').value=manifest.title;
  $('history-objective').value=manifest.objective||'';$('history-conditions').value=manifest.conditions||'';$('history-date').value=manifest.test_date||'';
  $('history-export').href='/api/history/export?id='+id;$('history-export').hidden=false;
  $('history-warnings').textContent=[...manifest.warnings,...(!manifest.closed?['El cierre completo de esta sesión no está acreditado.']:[])].join('\n');
  $('history-events').replaceChildren();appendEvents(eventResult.events);$('history-more').dataset.after=eventResult.next||'';$('history-more').hidden=!eventResult.next;
  await update(true);refresh();
}
function appendEvents(events){for(const event of events){const button=document.createElement('button');button.type='button';button.textContent=`${time(event.t)} · ${event.text}`;button.onclick=()=>seek(event.t);$('history-events').append(button);}}
async function seek(ms){if(!app.replay)return;app.replay.clock.playing=false;app.replay.clock.seek(ms);generation++;windowEnd=-1;await update(true);}
async function update(force=false){
  if(!app.replay||busy)return;const r=app.replay;const at=r.clock.position;const now=performance.now();
  if(!force&&now-lastRequest<80)return;lastRequest=now;busy=true;const g=generation;
  try{
    const state=await api(`state?id=${r.id}&at=${at}`);
    let history;
    if(force||at>windowEnd||at<windowEnd-60000){
      const end=Math.min(r.clock.duration,at+5000),start=Math.max(0,end-60000);
      const result=await api(`window?id=${r.id}&start=${start}&end=${end}`);history=seriesFromFrames(result.frames,r.epoch);windowEnd=end;
    }
    if(g!==generation||app.replay!==r)return;
    if(history)r.history=history;
    // All renderers consume this frozen historical snapshot. No live calibration.
    app.state={status:'replay',signals:state.signals,frame_id:state.frame_id,nodes:{},stats:{valid_frames:state.frame_id,invalid_frames:0,sequence_gaps:0},messages:[],calibration:{active:null},serial_available:false,read_only:true};
    app.stateReceivedAt=Date.now();app.backendOK=true;r.renderedAt=at;
    $('replay-seek').value=String(at);$('replay-time').textContent=`${time(at)} / ${time(r.clock.duration)}`;
    $('replay-play').textContent=r.clock.playing?'Pausar':'Reproducir';
    $('history-raw').textContent=JSON.stringify(state.raw,null,2);refresh();
  }catch(error){if(app.replay)app.replay.clock.playing=false;message(error.message);}
  finally{busy=false;if(g!==generation&&app.replay)update(true);}
}
export function replayTick(now){if(!app.replay)return;app.replay.clock.tick(now);if(app.replay.clock.playing||app.replay.renderedAt!==app.replay.clock.position)update();}
export function initHistory(renderCallback,pollCallback){
  refresh=renderCallback;poll=pollCallback;
  const safe=fn=>async()=>{try{await fn();}catch(error){message(error.message);}};
  $('history-toggle').onclick=safe(async()=>{$('history-panel').hidden=!$('history-panel').hidden;if(!$('history-panel').hidden)await list();});
  $('history-refresh').onclick=safe(list);$('history-open').onclick=safe(open);
  for(const name of ['history-files','history-folder'])$(name).onchange=safe(async()=>{await importFiles([...$(name).files]);$(name).value='';});
  $('replay-play').onclick=()=>{const r=app.replay;if(!r)return;if(r.clock.position>=r.clock.duration)r.clock.seek(0);r.clock.last=performance.now();r.clock.playing=!r.clock.playing;update(true);};
  $('replay-rate').onchange=()=>app.replay?.clock.rate(Number($('replay-rate').value));
  $('replay-seek').oninput=()=>seek(Number($('replay-seek').value));
  $('replay-step').onclick=()=>seek((app.replay?.clock.position||0)+20);
  $('replay-exit').onclick=async()=>{generation++;app.replay=null;view=null;clearHistory();$('replay-controls').hidden=true;await poll();refresh();};
  $('history-more').onclick=safe(async()=>{const result=await api(`events?id=${view.id}&after=${$('history-more').dataset.after}`);appendEvents(result.events);$('history-more').dataset.after=result.next||'';$('history-more').hidden=!result.next;});
  $('history-save').onclick=safe(async()=>{if(!view)return;await api('annotate',{id:view.id,title:$('history-title').value,notes:$('history-notes').value,objective:$('history-objective').value,conditions:$('history-conditions').value,...($('history-date').value!==view.test_date&&$('history-date').value?{test_date:$('history-date').value}:{})});message('Anotaciones guardadas; originales conservados.');await list();});
}
