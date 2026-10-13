// Asistente de calibración del PC: las peticiones nunca se transmiten al kart.
import {$,app,f} from './nucleo.js';
let busy=false, refresh, plotPoints=[],lastSample=null,profileStamp='',lastActive=null,lastConnection='';
const labels={center:'Centro',left:'Izquierda',right:'Derecha',center_left:'Centro desde I',center_right:'Centro desde D'};
const show=v=>typeof v==='number'?f(v,2):'—';
export function initCalibration(poll){
  refresh=poll;
  const dialog=document.createElement('dialog');dialog.id='calibracion';dialog.setAttribute('aria-labelledby','cal-title');
  dialog.innerHTML=`<div class="cal-cab"><div><h2 id="cal-title">Calibración de dirección</h2><p>Perfiles en el portátil · sin cambios en el control de motores</p></div><button id="cal-close" type="button">Cerrar</button></div>
  <div class="cal-body">
  <p id="cal-availability" role="status"></p><p id="cal-error" role="alert" hidden></p><p id="cal-notice" role="status"></p>
  <section><h3>1. Preparación</h3><p>Electrónica alimentada, kart inmóvil y tracción inhabilitada. En el kart instalado utiliza Wi-Fi. Alinea las ruedas con una referencia física.</p>
  <div class="cal-fields"><label>Configuración del kart / sensor<input id="cal-config" maxlength="600" placeholder="Identificación del montaje, sensor y ajustes"></label><label>Método de alineación del centro<input id="cal-alignment" maxlength="600" placeholder="Cómo compruebas que las ruedas están rectas"></label><label>Condiciones del ensayo<input id="cal-conditions" maxlength="600" placeholder="Apoyo, carga, alimentación y temperatura si se conoce"></label></div>
  <label class="cal-check"><input id="cal-ready" type="checkbox"> Confirmo kart inmóvil, tracción inhabilitada y preparación física.</label>
  <button id="cal-begin" type="button">Preparar nuevo ensayo</button><span class="nota"> Los ensayos anteriores se conservan en el portátil.</span></section>
  <section><h3>2. Centro y tres ciclos de repetición</h3><p id="cal-next">Prepara el ensayo para empezar.</p>
  <div class="cal-actions"><button id="cal-capture" type="button" class="primary" disabled>Capturar posición</button><button id="cal-cancel" type="button" disabled>Cancelar captura</button><a id="cal-export-run" download="ensayo-direccion.zip" hidden>Descargar ensayo y muestras</a></div>
  <progress id="cal-progress" value="0" max="100" aria-label="Avance de captura"></progress><p id="cal-progress-text" role="status"></p>
  <div class="cal-live"><div>CAN normalizado<strong id="cal-raw">—</strong><span>cuentas · no ADC original</span></div><div>Estimación anterior<strong id="cal-old">—</strong><span>centro fijo / limitada a ±20°</span></div><div>Perfil activo<strong id="cal-value">—</strong><span id="cal-unit">Sin perfil activo</span></div></div>
  <canvas id="cal-trace" height="110" role="img" aria-label="Últimas lecturas CAN en cuentas, sin suavizado; vista de 30 segundos"></canvas><p class="nota">Traza de pantalla sin suavizado · tiempo desde la última muestra (s). El archivo conserva cada trama recibida.</p>
  <div class="cal-table"><table><thead><tr><th>Posición</th><th>Ciclo</th><th>Estado</th><th>Muestras</th><th>Mediana</th><th>Mín.–máx.</th><th>Pico a pico</th><th>Desv. estándar</th></tr></thead><tbody id="cal-rows"></tbody></table></div>
  <p id="cal-comparison"></p><p>El centro se obtiene con las ruedas alineadas. Cada extremo conserva su propio recorrido. No fuerces los topes.</p></section>
  <section><h3>3. Revisar y guardar perfil</h3><p>Elige tolerancias en cuentas según el ensayo; no se asignan límites físicos automáticamente. Un perfil guarda el centro y los extremos, sin ocultar la fluctuación.</p>
  <div class="cal-fields"><label>Nombre del perfil<input id="cal-name" maxlength="100" placeholder="Dirección · fecha / montaje"></label><label>Variación máxima por captura (cuentas)<input id="cal-max-noise" type="number" min="0.01" step="any"></label><label>Error máximo de retorno al centro (cuentas)<input id="cal-max-return" type="number" min="0.01" step="any"></label><label>Recorrido mínimo por lado (cuentas)<input id="cal-min-span" type="number" min="0.01" step="any"></label></div>
  <details><summary>Referencias físicas opcionales para mostrar grados</summary><p>Sin estas medidas, el perfil muestra porcentaje de recorrido. Los grados se interpolan entre referencias; quedan pendientes de comprobar en puntos intermedios.</p>
  <div class="cal-fields"><label>Elemento medido<select id="cal-target"><option value="">Solo porcentaje</option><option value="wheel_left">Rueda delantera izquierda</option><option value="wheel_right">Rueda delantera derecha</option><option value="steering_wheel">Volante</option></select></label><label>Giro máximo a izquierda, magnitud (°)<input id="cal-angle-left" type="number" min="0.01" step="any"></label><label>Giro máximo a derecha, magnitud (°)<input id="cal-angle-right" type="number" min="0.01" step="any"></label><label>Instrumento de referencia<input id="cal-instrument" maxlength="200"></label><label>Resolución del instrumento (°)<input id="cal-resolution" type="number" min="0.001" step="any"></label></div></details>
  <label class="cal-check"><input id="cal-reviewed" type="checkbox"> He revisado capturas, variación, retornos y tolerancias. Las referencias angulares, si las añado, son medidas del elemento indicado.</label>
  <button id="cal-save" type="button" disabled>Guardar perfil nuevo en PC</button></section>
  <section><h3>4. Perfiles guardados</h3><label>Perfil<select id="cal-profile"><option value="">Sin perfiles</option></select></label><p id="cal-profile-detail"></p>
  <label class="cal-check"><input id="cal-compatible" type="checkbox"> Confirmo el mismo sensor, geometría y configuración física que en el perfil seleccionado.</label>
  <div class="cal-actions"><button id="cal-activate" type="button" disabled>Activar solo en esta sesión del panel</button><button id="cal-deactivate" type="button" disabled>Volver a estimación anterior</button><a id="cal-export-profile" download="perfil-direccion.zip" hidden>Descargar perfil y evidencia</a></div>
  <p id="cal-active-name"></p><p class="nota">Tras pérdida de enlace, reinicio o nueva conexión hay que confirmar el perfil otra vez. Los perfiles anteriores siguen disponibles.</p></section>
  </div>`;
  document.body.appendChild(dialog);
  $('abrir-calibracion').addEventListener('click',()=>{dialog.showModal();renderCalibration();});
  $('cal-close').addEventListener('click',()=>dialog.close());
  const val=id=>$(id).value, num=id=>val(id).trim()===''?null:Number(val(id));
  $('cal-begin').addEventListener('click',()=>send('begin',{prepared:$('cal-ready').checked,configuration:val('cal-config'),alignment:val('cal-alignment'),conditions:val('cal-conditions')}));
  $('cal-capture').addEventListener('click',()=>send('capture',{}));
  $('cal-cancel').addEventListener('click',()=>send('cancel',{}));
  $('cal-save').addEventListener('click',()=>send('save',{name:val('cal-name'),reviewed:$('cal-reviewed').checked,criteria:{max_peak_to_peak:num('cal-max-noise'),max_return_error:num('cal-max-return'),min_span:num('cal-min-span')},
    angles:val('cal-target')?{target:val('cal-target'),left:num('cal-angle-left'),right:num('cal-angle-right'),instrument:val('cal-instrument'),resolution:num('cal-resolution')}:null}));
  $('cal-activate').addEventListener('click',()=>send('activate',{id:val('cal-profile'),compatible:$('cal-compatible').checked}));
  $('cal-deactivate').addEventListener('click',()=>send('deactivate',{}));
  $('cal-profile').addEventListener('change',()=>{$('cal-compatible').checked=false;renderCalibration();});
  for(const id of ['cal-ready','cal-reviewed','cal-compatible'])$(id).addEventListener('change',renderCalibration);
  new ResizeObserver(drawTrace).observe($('cal-trace'));
}
async function send(action,payload){
  if(busy||app.demo)return;
  busy=true;$('cal-error').hidden=true;$('cal-notice').textContent='';renderCalibration();
  try{
    const response=await fetch('/api/calibration/'+action,{method:'POST',headers:{'Content-Type':'application/json','X-Kart-Token':app.token},body:JSON.stringify(payload)});
    const data=await response.json();if(!response.ok)throw Error(data.error||'No se pudo completar');
    if(action==='save')$('cal-notice').textContent='Perfil guardado. Selecciónalo y confirma el montaje para activarlo.';
    if(action==='begin'){$('cal-reviewed').checked=false;$('cal-notice').textContent='Ensayo preparado. Coloca las ruedas en el centro físico.';}
    await refresh();
    if(data.id){$('cal-profile').value=data.id;$('cal-compatible').checked=false;}
  }catch(error){$('cal-error').textContent=error.message;$('cal-error').hidden=false;}
  finally{busy=false;renderCalibration();}
}
function drawTrace(){
  const canvas=$('cal-trace');if(!canvas||!canvas.clientWidth)return;
  const w=canvas.clientWidth,h=110,dpr=window.devicePixelRatio||1;
  canvas.width=w*dpr;canvas.height=h*dpr;
  const ctx=canvas.getContext('2d');ctx.scale(dpr,dpr);
  const css=getComputedStyle(document.documentElement);ctx.strokeStyle=css.getPropertyValue('--borde');ctx.strokeRect(48,12,w-60,h-38);
  ctx.fillStyle=css.getPropertyValue('--texto');ctx.font='12px Segoe UI';
  if(!plotPoints.length){ctx.fillText('Sin lecturas vigentes',60,55);return;}
  const values=plotPoints.map(p=>p.v),lo=Math.min(...values)-1,hi=Math.max(...values)+1,t=Date.now();
  ctx.fillText(f(hi,0),2,20);ctx.fillText(f(lo,0),2,h-26);ctx.fillText('−30 s',48,h-6);ctx.fillText('0 s',w-28,h-6);
  ctx.strokeStyle=css.getPropertyValue('--serie-1');ctx.beginPath();
  plotPoints.forEach((p,i)=>{const x=48+(w-60)*(1-(t-p.t)/30000),y=12+(h-38)*(1-(p.v-lo)/(hi-lo));i?ctx.lineTo(x,y):ctx.moveTo(x,y);});ctx.stroke();
}
export function renderCalibration(){
  if(!$('calibracion'))return;
  const c=app.state?.calibration,enabled=!app.demo&&app.backendOK&&c?.ready;
  const connection=[app.state?.port,app.state?.nodes?.esp32?.session,app.state?.nodes?.esp32?.fw].join('|');
  if((lastActive&&!c?.active)||connection!==lastConnection)$('cal-compatible').checked=false;
  lastActive=c?.active?.id||null;lastConnection=connection;
  $('abrir-calibracion').disabled=app.demo;
  const id=app.state?.signals?.steer_raw?.sample_id;
  if(enabled&&id!==lastSample){plotPoints.push({t:Date.now(),v:c.raw});lastSample=id;}
  if(!enabled)plotPoints=[];
  plotPoints=plotPoints.filter(p=>p.t>Date.now()-30000);
  if(!$('calibracion').open)return;
  $('cal-availability').textContent=app.demo?'Calibración real deshabilitada en demostración':!app.backendOK?'Servicio local no disponible':c?.error||c?.reason||'Telemetría disponible · '+(app.state.port||'');
  $('cal-begin').disabled=!enabled||busy||!!c?.capture||!$('cal-ready').checked;
  $('cal-capture').disabled=!enabled||busy||!c?.run||!!c?.capture||!c?.next;
  $('cal-cancel').disabled=busy||!c?.capture;
  $('cal-save').disabled=busy||!!c?.capture||c?.run?.captures?.length!==c?.total_steps||!$('cal-reviewed').checked;
  const next=c?.next;
  $('cal-next').textContent=!c?.run?'Prepara el ensayo para empezar.':next?`${c.run.captures.length+1}/${c.total_steps} · ${next.label}${next.cycle?' · ciclo '+next.cycle+'/3':''}. Mantén la posición durante al menos ${next.duration_s} s y ${next.min_samples} muestras.`:'Capturas completas. Revisa la variación y los retornos antes de guardar.';
  $('cal-capture').textContent=next?'Capturar '+next.label.toLocaleLowerCase('es'):'Capturas completas';
  const cap=c?.capture;
  $('cal-progress').value=cap?Math.min(100,100*Math.min((cap.statistics?.duration_s||0)/cap.duration_s,cap.n/cap.min_samples)):0;
  $('cal-progress-text').textContent=cap?`Capturando · ${cap.n}/${cap.min_samples} muestras · ${show(cap.statistics?.duration_s||0)} s · variación ${show(cap.statistics?.peak_to_peak)} cuentas`:'Sin captura en curso.';
  $('cal-raw').textContent=enabled?show(c.raw):'—';$('cal-old').textContent=enabled?show(c.old_angle)+'°':'—';
  const out=enabled?c.calibrated:null;
  $('cal-value').textContent=!out?'—':out.outside?'Fuera de rango':show(out.angle??out.percent)+(out.angle===null?' %':'°');
  $('cal-unit').textContent=!out?'Sin perfil activo':out.angle===null?'Recorrido por lado · grados sin calibrar':'Interpolación · '+({wheel_left:'rueda izquierda',wheel_right:'rueda derecha',steering_wheel:'volante'}[out.target]);
  $('cal-rows').replaceChildren();
  for(const row of (c?.run?.attempts||[])){
    const s=row.statistics||{},tr=document.createElement('tr');
    for(const text of [labels[row.kind],row.cycle||'Inicial',row.status==='completed'?'Capturada':'Rechazada: '+row.reason,s.n||0,show(s.median),show(s.min)+'–'+show(s.max),show(s.peak_to_peak),show(s.stdev)]){const td=document.createElement('td');td.textContent=text;tr.append(td);}
    $('cal-rows').append(tr);
  }
  const captures=c?.run?.captures||[],center=captures.find(x=>x.kind==='center')?.statistics.median,left=captures.filter(x=>x.kind==='left').at(-1)?.statistics.median,right=captures.filter(x=>x.kind==='right').at(-1)?.statistics.median;
  $('cal-comparison').textContent=`Últimas capturas: I ${show(left)} · C ${show(center)} · D ${show(right)} cuentas. Perfil activo: ${c?.active?`I ${show(c.active.references.left)} · C ${show(c.active.references.center)} · D ${show(c.active.references.right)}`:'ninguno'}.`;
  $('cal-export-run').hidden=!c?.run;if(c?.run)$('cal-export-run').href='/api/calibration/export?run='+c.run.id;
  const profiles=c?.profiles||[],stamp=profiles.map(p=>p.id).join();
  if(stamp!==profileStamp||!$('cal-profile').options.length){
    const old=$('cal-profile').value;$('cal-profile').replaceChildren();
    const empty=document.createElement('option');empty.value='';empty.textContent='Selecciona un perfil';$('cal-profile').append(empty);
    for(const p of profiles){const option=document.createElement('option');option.value=p.id;option.textContent=p.name+' · '+new Date(p.created_utc).toLocaleString('es-ES',{timeZone:'Europe/Lisbon'});$('cal-profile').append(option);}
    $('cal-profile').value=old;profileStamp=stamp;
  }
  const selected=profiles.find(p=>p.id===$('cal-profile').value);
  $('cal-profile-detail').textContent=selected?`${selected.metadata.configuration} · I ${show(selected.references.left)} / C ${show(selected.references.center)} / D ${show(selected.references.right)} cuentas · variación máx. ${show(selected.max_peak_to_peak)} · error retorno ${show(selected.max_return_error)} cuentas · ${selected.context.firmware}`:'Los perfiles guardados no se activan automáticamente.';
  $('cal-activate').disabled=!enabled||busy||!!cap||!selected||!$('cal-compatible').checked;
  $('cal-deactivate').disabled=busy||!!cap||!c?.active;
  $('cal-export-profile').hidden=!selected;if(selected)$('cal-export-profile').href='/api/calibration/export?profile='+selected.id;
  $('cal-active-name').textContent=c?.active?'Activo en PC: '+c.active.name:'Sin perfil activo en esta sesión.';
  drawTrace();
}
