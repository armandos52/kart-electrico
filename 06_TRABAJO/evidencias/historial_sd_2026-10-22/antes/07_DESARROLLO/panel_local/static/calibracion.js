// Un paso cada vez: el usuario coloca la dirección y después confirma la medición.
import {$,app,f,idioma,t,srv} from './nucleo.js';
import {applyDom} from './i18n.js';
let refresh,busy=false,lastConnection='',lastActive=null;
const TITLES={center:'cal.t.center',left:'cal.t.left',right:'cal.t.right',center_left:'cal.t.center_again',center_right:'cal.t.center_again'};
const NAMES={center:'cal.n.center',left:'cal.n.left',right:'cal.n.right',center_left:'cal.n.center_left',center_right:'cal.n.center_right'};
const running=c=>c?.guide?.state==='running';
const value=v=>typeof v==='number'?f(v,1):'—';
function text(id,value){if($(id).textContent!==value)$(id).textContent=value;}
export function initCalibration(poll){
  refresh=poll;
  const dialog=document.createElement('dialog');dialog.id='calibracion';dialog.setAttribute('aria-labelledby','cal-title');
  dialog.innerHTML=`<header class="cal-header"><div><span class="cal-eyebrow" data-i18n="cal.ui.asistente">ASISTENTE DE DIRECCIÓN</span><h1 id="cal-title" data-i18n="calibrar">Calibrar dirección</h1></div><button id="cal-close" type="button" aria-label="Cerrar calibración" data-i18n-aria="cal.cerrar_aria" data-i18n="cal.cerrar">Cerrar</button></header>
  <div class="cal-body">
    <ol class="cal-stages" aria-label="Progreso" data-i18n-aria="cal.progreso">${['cal.ui.preparar','cal.ui.centro','cal.ui.recorrido','cal.ui.verificar','cal.ui.guardar'].map((key,i)=>`<li><b>${String(i+1).padStart(2,'0')}</b><span data-i18n="${key}">${t(key)}</span></li>`).join('')}</ol>
    <div class="cal-workspace">
    <div class="cal-visual"><div id="cal-picture" class="cal-picture" aria-hidden="true"><div class="cal-orbit"></div><svg class="cal-wheel" viewBox="0 0 200 200"><circle cx="100" cy="100" r="79" fill="none" stroke="currentColor" stroke-width="16"/><path d="M24 89L80 112L88 179M176 89L120 112L112 179" fill="none" stroke="currentColor" stroke-width="13" stroke-linejoin="round"/><circle cx="100" cy="109" r="22" fill="currentColor"/><path d="M100 15V30" stroke="#78bcae" stroke-width="8" stroke-linecap="round"/></svg><div id="cal-arrow">↑</div></div><span class="cal-caption" data-i18n="cal.ui.dibujo">Movimiento indicado · dibujo orientativo</span><div id="cal-position-chips" class="cal-position-chips" aria-hidden="true"><span>←</span><span>↑</span><span>→</span><span>↑</span></div></div>
    <div class="cal-task">
    <div class="cal-steps"><span id="cal-round" data-i18n="cal.ronda_antes">Antes de empezar</span><span id="cal-count"></span></div>
    <div class="cal-instruction" aria-live="polite" aria-atomic="true"><h2 id="cal-heading" data-i18n="cal.indicacion">Una indicación cada vez</h2><p id="cal-help" data-i18n="cal.ayuda">Tú mueves el volante. El programa comprueba y guarda cada posición.</p></div>
    <div id="cal-start"><label class="cal-check"><input id="cal-ready" type="checkbox"> <span data-i18n="cal.listo_check">Ruedas rectas, kart parado y tracción inhabilitada.</span></label></div>
    <div id="cal-measuring" hidden><div class="cal-time"><span data-i18n="cal.ui.midiendo">Midiendo esta posición</span><strong id="cal-timer"></strong></div><progress id="cal-progress" max="100" value="0" aria-label="Comprobando posición" data-i18n-aria="cal.comprobando_aria"></progress></div>
    <p id="cal-feedback" role="status"></p>
    <div id="cal-issue" class="cal-issue" role="alert" hidden><strong id="cal-issue-title"></strong><p id="cal-issue-detail"></p><span id="cal-issue-next"></span></div>
    <p id="cal-error" role="alert" hidden></p>
    <div class="cal-main-actions"><button id="cal-primary" class="primary" type="button" data-i18n="cal.empezar">Empezar</button><button id="cal-stop" class="cal-quiet" type="button" hidden data-i18n="cal.cancelar">Cancelar</button></div>
    </div></div>
    <div class="cal-readings"><div><span data-i18n="cal.ui.enlace">Lectura del kart</span><strong id="cal-link"></strong></div><div><span data-i18n="cal.ui.senal">Señal de dirección</span><strong id="cal-raw"></strong></div><div><span data-i18n="cal.ui.variacion">Variación en esta medición</span><strong id="cal-noise"></strong></div><div><span data-i18n="cal.ui.guardadas">Posiciones guardadas</span><strong id="cal-saved"></strong></div></div>
    <p id="cal-connection" class="cal-caption"></p>
    <details id="cal-details"><summary data-i18n="cal.detalles">Detalles y calibraciones anteriores</summary>
      <div class="cal-detail-content">
      <p id="cal-technical"></p><p id="cal-summary"></p>
      <a id="cal-export-run" download="ensayo-direccion.zip" hidden data-i18n="cal.descargar_lecturas">Descargar todas las lecturas</a>
      <div class="cal-table"><table><thead><tr><th data-i18n="cal.col.posicion">Posición</th><th data-i18n="cal.col.ronda">Ronda</th><th data-i18n="cal.col.lectura">Lectura</th><th data-i18n="cal.col.variacion">Variación</th><th data-i18n="cal.col.resultado">Resultado</th></tr></thead><tbody id="cal-rows"></tbody></table></div>
      <details><summary data-i18n="cal.notas">Notas del ensayo (opcional)</summary><div class="cal-fields"><label><span data-i18n="cal.montaje">Montaje o sensor</span><input id="cal-config" maxlength="600" placeholder="Sin indicar" data-i18n-ph="cal.sin_indicar"></label><label><span data-i18n="cal.condiciones">Condiciones</span><input id="cal-conditions" maxlength="600" placeholder="Sin indicar" data-i18n-ph="cal.sin_indicar"></label><label><span data-i18n="cal.nombre">Nombre de la calibración</span><input id="cal-name" maxlength="100" placeholder="Fecha automática" data-i18n-ph="cal.fecha_auto"></label></div></details>
      <details><summary data-i18n="cal.medidas">Añadir medidas físicas en grados (opcional)</summary><p data-i18n="cal.medidas_nota">Sin medidas físicas, se muestra porcentaje de recorrido. Los grados se interpolan para el elemento que indiques.</p><div class="cal-fields">
        <label><span data-i18n="cal.elemento">Elemento medido</span><select id="cal-target"><option value="" data-i18n="cal.solo_pct">Solo porcentaje</option><option value="wheel_left" data-i18n="cal.rueda_izq">Rueda izquierda</option><option value="wheel_right" data-i18n="cal.rueda_der">Rueda derecha</option><option value="steering_wheel" data-i18n="cal.volante">Volante</option></select></label>
        <label><span data-i18n="cal.max_izq">Máximo a izquierda (°)</span><input id="cal-angle-left" type="number" min="0.01" step="any"></label><label><span data-i18n="cal.max_der">Máximo a derecha (°)</span><input id="cal-angle-right" type="number" min="0.01" step="any"></label><label><span data-i18n="cal.instrumento">Instrumento</span><input id="cal-instrument" maxlength="200"></label><label><span data-i18n="cal.resolucion">Resolución (°)</span><input id="cal-resolution" type="number" min="0.001" step="any"></label>
      </div></details>
      <label class="cal-profile-label"><span data-i18n="cal.guardadas">Calibraciones guardadas</span><select id="cal-profile"><option value="" data-i18n="cal.selecciona">Selecciona una calibración</option></select></label><p id="cal-profile-detail"></p>
      <label class="cal-check"><input id="cal-compatible" type="checkbox"> <span data-i18n="cal.compatible">El sensor y el montaje siguen igual.</span></label>
      <div class="cal-detail-actions"><button id="cal-activate" type="button" data-i18n="cal.usar_sel">Usar la seleccionada</button><button id="cal-deactivate" type="button" data-i18n="cal.quitar_activa">Quitar calibración activa</button><a id="cal-export-profile" download="perfil-direccion.zip" hidden data-i18n="cal.descargar_perfil">Descargar perfil</a></div>
      <p id="cal-active-name"></p>
      </div>
    </details>
  </div>`;
  document.body.appendChild(dialog);
  applyDom(dialog);
  $('abrir-calibracion').addEventListener('click',()=>{dialog.showModal();renderCalibration();});
  async function close(){if(busy)return;if(running(app.state?.calibration)&&!await send('guided-cancel',{}))return;dialog.close();}
  $('cal-close').addEventListener('click',close);
  dialog.addEventListener('cancel',e=>{e.preventDefault();close();});
  $('cal-ready').addEventListener('change',renderCalibration);
  $('cal-stop').addEventListener('click',()=>send('guided-cancel',{}));
  $('cal-primary').addEventListener('click',()=>{
    const c=app.state?.calibration,g=c?.guide;
    if(g?.state==='saved'){dialog.close();return;}
    if(g?.state==='paused'&&c.same_context){send('guided-resume',{});return;}
    if(g?.state==='running'&&g.phase==='positioning'){send('guided-measure',{});return;}
    if(g?.state==='complete'&&c.same_context){
      const num=id=>$(id).value.trim()===''?null:Number($(id).value);
      send('guided-use',{name:$('cal-name').value,angles:$('cal-target').value?{target:$('cal-target').value,left:num('cal-angle-left'),right:num('cal-angle-right'),resolution:num('cal-resolution'),instrument:$('cal-instrument').value}:null});return;
    }
    send('guided-begin',{prepared:$('cal-ready').checked,configuration:$('cal-config').value,conditions:$('cal-conditions').value});
  });
  $('cal-activate').addEventListener('click',()=>send('activate',{id:$('cal-profile').value,compatible:$('cal-compatible').checked}));
  $('cal-deactivate').addEventListener('click',()=>send('deactivate',{}));
  $('cal-profile').addEventListener('change',()=>{$('cal-compatible').checked=false;renderCalibration();});
  $('cal-compatible').addEventListener('change',renderCalibration);
  $('cal-details').addEventListener('toggle',renderCalibration);
}
async function send(action,payload){
  if(busy||app.demo)return false;
  busy=true;$('cal-error').hidden=true;renderCalibration();
  try{
    const response=await fetch('/api/calibration/'+action,{method:'POST',headers:{'Content-Type':'application/json','X-Kart-Token':app.token},body:JSON.stringify(payload)});
    const data=await response.json();if(!response.ok)throw Error(srv(data.error)||t('cal.error_generico'));
    await refresh();return true;
  }catch(error){text('cal-error',error.message);$('cal-error').hidden=false;return false;}
  finally{busy=false;renderCalibration();}
}
export function renderCalibration(){
  if(!$('calibracion'))return;
  const c=app.state?.calibration,g=c?.guide,state=g?.state,live=!app.demo&&app.backendOK&&c?.ready;
  const connection=[app.state?.port,app.state?.nodes?.esp32?.session,app.state?.nodes?.esp32?.fw].join('|');
  if(connection!==lastConnection){$('cal-compatible').checked=false;$('cal-ready').checked=false;}
  if(lastActive&&!c?.active)$('cal-compatible').checked=false;
  lastConnection=connection;lastActive=c?.active?.id||null;
  $('abrir-calibracion').disabled=app.demo;
  if(!$('calibracion').open)return;
  const changed=!!g&&['paused','complete'].includes(state)&&!c?.same_context;
  const starting=!g||['cancelled','failed'].includes(state)||changed,work=state==='running',holding=work&&g.phase==='holding';
  const done=c?.run?.captures?.length||0,next=c?.next;
  text('cal-round',work?(next?.cycle?t('cal.ronda_n',{n:next.cycle}):t('cal.ronda_centro')):state==='complete'?t('cal.ronda_fin'):state==='saved'?t('cal.ronda_guardada'):state==='paused'?t('cal.ronda_pausa'):t('cal.ronda_antes'));
  text('cal-count',g?t('cal.posiciones',{n:done,total:c.total_steps}):'');
  let heading=t('cal.indicacion'),help=t('cal.ayuda');
  if(work){heading=holding?t('cal.ui.quieta'):TITLES[next?.kind]?t(TITLES[next.kind]):t('cal.comprobando');help=holding?t('cal.ui.no_girar'):t(next?.kind==='left'||next?.kind==='right'?'cal.ui.coloca_tope':'cal.ui.coloca_centro');}
  if(state==='complete'){heading=t('cal.h.preparada');help=t('cal.ayuda_preparada');}
  if(state==='saved'){heading=c?.active?.id===g.profile_id?t('cal.h.todo_listo'):t('cal.h.guardada');help=c?.active?.id===g.profile_id?t('cal.ayuda_todo_listo'):t('cal.ayuda_guardada');}
  if(state==='paused'){heading=t('cal.ui.pausa');help=t('cal.ui.conservadas');}
  if(state==='failed'){heading=t('cal.h.repetir');help=srv(g.message);}
  if(state==='cancelled'){heading=t('cal.h.reiniciar');help=t('cal.ayuda_reiniciar');}
  if(c?.error){heading=t('cal.h.error');help=t('cal.ayuda_error');}
  if(changed){heading=t('cal.ui.sesion');help=t('cal.ui.sesion_ayuda');}
  text('cal-heading',heading);text('cal-help',help);
  const direction=work||state==='paused'?(next?.kind==='left'?'left':next?.kind==='right'?'right':'center'):'center';
  $('cal-picture').dataset.direction=direction;
  $('cal-picture').dataset.phase=holding?'holding':work?'positioning':'idle';
  text('cal-arrow',state==='complete'||state==='saved'?'✓':direction==='left'?'←':direction==='right'?'→':'↑');
  $('cal-start').hidden=!starting;$('cal-measuring').hidden=!holding;
  $('cal-progress').value=100*(g?.progress||0);
  text('cal-timer',t('cal.ui.segundos',{n:Math.max(0,Math.ceil((c?.capture?.duration_s||0)-(c?.capture?.statistics?.duration_s||0)))}));
  text('cal-feedback',!live?t('cal.esperando_lecturas'):work?srv(g?.message):state==='saved'?srv(g.message):'');
  const limit=live&&(c.raw===0||c.raw===4095),issue=c?.error||g?.detail||(!live?c?.reason:'')||(limit?t('cal.ui.limite_ayuda'):'');
  $('cal-issue').hidden=!issue;
  text('cal-issue-title',c?.error?t('cal.h.error'):limit||g?.issue==='limit'?t('cal.ui.limite'):g?.issue==='noise'?t('cal.ui.ruido'):g?.issue==='position'?t('cal.ui.posicion'):t('cal.ui.revisar'));
  text('cal-issue-detail',srv(issue)||'');
  text('cal-issue-next',state==='paused'&&!changed?t('cal.ui.conservadas'):'');
  $('cal-primary').hidden=holding;
  text('cal-primary',changed?t('cal.p.reiniciar'):work?t('cal.ui.medir',{n:next.duration_s}):state==='complete'?t('cal.p.usar'):state==='saved'?t('cal.p.terminar'):state==='paused'?t('cal.p.reintentar'):state==='failed'||state==='cancelled'?t('cal.p.reiniciar'):t('cal.empezar'));
  $('cal-primary').disabled=busy||(state!=='saved'&&(!live||starting&&!$('cal-ready').checked||work&&limit))||!!c?.error;
  $('cal-stop').hidden=!work&&state!=='paused';$('cal-stop').disabled=busy;
  $('cal-close').disabled=busy;
  text('cal-connection',!app.backendOK?t('cal.conn.abrir'):!live?t('cal.conn.wifi'):t('cal.conn.visual'));
  const stage=starting?0:state==='complete'||state==='saved'?4:done===0?1:done<=4?2:3;
  document.querySelectorAll('.cal-stages li').forEach((el,i)=>{el.classList.toggle('current',i===stage);el.classList.toggle('done',i<stage);if(i===stage)el.setAttribute('aria-current','step');else el.removeAttribute('aria-current');});
  $('cal-position-chips').hidden=!work||done===0;
  [...$('cal-position-chips').children].forEach((el,i)=>{el.classList.toggle('current',i===(done-1)%4);el.classList.toggle('done',i<(done-1)%4);});
  const measured=c?.capture?.statistics||(state==='paused'?c?.run?.attempts?.at(-1)?.statistics:null);
  text('cal-link',live?t('cal.ui.recibiendo'):t('cal.ui.sin_datos'));
  text('cal-raw',live?t('cal.ui.cuentas',{n:value(c.raw)}):'—');
  text('cal-noise',measured?t('cal.ui.cuentas',{n:value(measured.peak_to_peak)}):'—');
  text('cal-saved',`${done} / ${c?.total_steps||13}`);
  const profiles=c?.profiles||[],selectedId=$('cal-profile').value,stamp=profiles.map(p=>p.id).join();
  if($('cal-profile').dataset.stamp!==stamp){
    const placeholder=new Option(t('cal.selecciona'),'');placeholder.dataset.i18n='cal.selecciona';
    $('cal-profile').replaceChildren(placeholder);
    for(const p of profiles)$('cal-profile').append(new Option(p.name,p.id));
    $('cal-profile').value=selectedId;$('cal-profile').dataset.stamp=stamp;
  }
  const selected=profiles.find(p=>p.id===$('cal-profile').value);
  $('cal-activate').disabled=!live||busy||work||!selected||!$('cal-compatible').checked;
  $('cal-deactivate').disabled=busy||work||!c?.active;
  $('cal-export-profile').hidden=!selected;if(selected)$('cal-export-profile').href='/api/calibration/export?profile='+selected.id;
  text('cal-profile-detail',selected?`${srv(selected.metadata.configuration)} · ${new Date(selected.created_utc).toLocaleString(idioma.locale,{timeZone:'Europe/Lisbon'})}`:'');
  text('cal-active-name',c?.active?t('cal.activo',{nombre:c.active.name}):t('cal.sin_activa'));
  text('cal-technical',t('cal.tecnico',{valor:value(c?.raw),razon:srv(c?.reason)||'',error:srv(c?.error)||''}));
  const summaries=c?.run?.captures||[];
  text('cal-summary',t('cal.resumen',{n:summaries.length,detalle:srv(g?.detail)||''}));
  $('cal-export-run').hidden=!c?.run;if(c?.run)$('cal-export-run').href='/api/calibration/export?run='+c.run.id;
  if($('cal-details').open){
    $('cal-rows').replaceChildren();
    for(const row of c?.run?.attempts||[]){const tr=document.createElement('tr');for(const v of [NAMES[row.kind]?t(NAMES[row.kind]):row.kind,row.cycle||t('cal.inicial'),value(row.statistics?.median),value(row.statistics?.peak_to_peak),row.status==='completed'?t('cal.ronda_guardada'):srv(row.reason)]){const td=document.createElement('td');td.textContent=v;tr.append(td);}$('cal-rows').append(tr);}
  }
}
