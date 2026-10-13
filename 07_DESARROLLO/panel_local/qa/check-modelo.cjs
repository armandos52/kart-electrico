// Prueba de integración visual: servidor local, telemetría simulada, ningún puerto del kart.
const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const evidence=process.argv[2],url=process.env.KART_PANEL_URL||'http://127.0.0.1:8766';
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const checks=[],errors=[],external=[];const assert=(name,ok)=>{if(!ok)throw Error(name);checks.push(name);};
 try{
  const page=await browser.newPage({viewport:{width:1440,height:1000}});
  page.on('pageerror',e=>errors.push(e.message));
  await page.route('**/*',route=>{if(!route.request().url().startsWith(url)){external.push(route.request().url());return route.abort();}return route.continue();});
  await page.goto(url);await page.waitForSelector('#escena-3d canvas[data-ready="true"]');
  await page.waitForFunction(()=>document.querySelector('#mode-badge').textContent==='Sin conexión');
  assert('3D disponible sin solicitudes externas',external.length===0);
  assert('Inicio desconectado; sin RPM inventadas',(await page.locator('#modelo-rpm').textContent())==='I — · D —');
  await page.locator('#escena-3d canvas').click({position:{x:510,y:385}});
  assert('Selección por clic en rueda 3D',(await page.locator('#selection-title').textContent())==='Rueda delantera D');
  await page.locator('#modelo-pieza').selectOption('rear_left');
  assert('Inspector compartido con el modelo',(await page.locator('#selection-title').textContent())==='Rueda trasera I');
  await page.locator('#ver-esquema').click();assert('Esquema 2D disponible',await page.locator('#kart').isVisible());
  await page.locator('[data-part="rear_right"]').click();assert('Selección 2D sincroniza selector',(await page.locator('#modelo-pieza').inputValue())==='rear_right');
  await page.locator('#ver-3d').click();
  for(const view of ['frontal','trasera','superior','lateral','perspectiva']){await page.locator(`[data-camera="${view}"]`).click();assert('Cámara '+view,await page.locator(`[data-camera="${view}"]`).getAttribute('aria-pressed')==='true');}
  await page.locator('#ver-fotos').click();
  await page.waitForFunction(()=>[...document.querySelectorAll('#fotos-kart img')].every(i=>i.complete&&i.naturalWidth>0));
  assert('Las cuatro fotografías locales se muestran',await page.locator('#fotos-kart img').count()===4);await page.keyboard.press('Escape');
  await page.locator('#demo').click();await page.locator('#demo-steer').fill('-15');
  await page.waitForFunction(()=>document.querySelector('#modelo-steering').textContent.includes('-15'));
  assert('Demo rotulada y conexión bloqueada',await page.locator('#connect').isDisabled()&&(await page.locator('#modelo-status').textContent()).includes('DEMO'));
  // Un recorte de la geometría excluye los rótulos DOM de RPM: estos siguen
  // refrescando durante una pausa, que solo debe detener la animación mecánica.
  const canvas=page.locator('#escena-3d canvas'),hash=async()=>{
    const b=await canvas.boundingBox();const clip={x:b.x+b.width*.26,y:b.y+85,width:b.width*.68,height:b.height-120};
    return crypto.createHash('sha256').update(await page.screenshot({clip})).digest('hex');
  };
  let a=await hash();await page.waitForTimeout(250);assert('RPM producen movimiento renderizado',a!==await hash());
  await page.locator('#motion').click();await page.waitForTimeout(250);a=await hash();await page.waitForTimeout(250);assert('Pausa detiene la rodadura 3D',a===await hash());
  await page.locator('#motion').click();await page.emulateMedia({reducedMotion:'reduce'});await page.waitForTimeout(500);a=await hash();await page.waitForTimeout(500);const reducedHash=await hash();
  if(a!==reducedHash)console.log(await page.evaluate(async()=>{const n=await import('/nucleo.js');return {reduced:n.reducedMotion.matches,paused:n.app.motionPaused,steer:n.steeringPresentation(),media:matchMedia('(prefers-reduced-motion: reduce)').matches};}));
  assert('Movimiento reducido respetado',a===reducedHash);await page.emulateMedia({reducedMotion:'no-preference'});
  await page.screenshot({path:path.join(evidence,'panel-3d-demo.png'),fullPage:true});
  await page.locator('#demo').click();await page.waitForFunction(()=>document.querySelector('#modelo-rpm').textContent==='I — · D —');
  const actual=await (await page.request.get(url+'/api/state')).json(),fixture=structuredClone(actual);fixture.status='connected';fixture.port='socket://192.168.4.1:3333';fixture.stats.valid_frames=5;
  for(const [key,value] of Object.entries({rpm_rl:120,steer_raw:2600,can_rx_age_ms:5,gear:'D'}))fixture.signals[key]={...fixture.signals[key],value,quality:'fresh',node:'esp32',age_ms:0,sample_id:50};
  await page.route('**/api/state',r=>r.fulfill({json:fixture}));
  await page.waitForFunction(()=>document.querySelector('#modelo-rpm').textContent==='I 120 · D —');
  assert('Solo se muestran RPM recibidas',true);
  await page.waitForFunction(()=>document.querySelector('#modelo-steering').textContent.includes('estimados del mando'));
  fixture.signals.can_rx_age_ms.value=220;
  await page.waitForFunction(()=>document.querySelector('#modelo-steering').textContent.includes('Sin dirección vigente'));
  assert('Timeout CAN retira dirección vigente',true);
  fixture.signals.rpm_rl.quality='stale';await page.waitForFunction(()=>document.querySelector('#modelo-rpm').textContent==='I — · D —');
  await page.waitForTimeout(250);a=await hash();await page.waitForTimeout(250);assert('Datos caducados detienen rodadura renderizada',a===await hash());
  await page.unroute('**/api/state');await page.waitForFunction(()=>document.querySelector('#mode-badge').textContent==='Sin conexión');
  await page.locator('#abrir-calibracion').click();assert('Calibración conservada, bloqueada sin datos',await page.locator('#cal-primary').isDisabled());await page.locator('#cal-close').click();
  await page.locator('#ampliar-modelo').click();assert('Vista ampliada disponible',!(await page.locator('.tira').isVisible()));await page.locator('#ampliar-modelo').click();
  const sizes=[];
  for(const width of [1920,1440,1280,1024,760,375,320]){
    await page.setViewportSize({width,height:1000});await page.waitForTimeout(120);
    const result=await page.evaluate(()=>({width:innerWidth,scroll:document.documentElement.scrollWidth,canvas:document.querySelector('#escena-3d canvas').width}));sizes.push(result);assert('Sin desborde a '+width,result.scroll<=width+1&&result.canvas>0);
    if(width===375)await page.screenshot({path:path.join(evidence,'panel-3d-movil.png'),fullPage:true});
  }
  await page.setViewportSize({width:1440,height:1000});await page.locator('#modelo-pieza').selectOption('esp32');
  await page.locator('[data-camera="perspectiva"]').click();await page.waitForTimeout(250);
  await page.screenshot({path:path.join(evidence,'panel-3d-final.png'),fullPage:true});
  await page.locator('[data-camera="lateral"]').click();await page.waitForTimeout(150);await page.locator('.lamina').screenshot({path:path.join(evidence,'modelo-lateral-final.png')});
  const response=await page.request.get(url+'/vendor/../server.py');assert('No se sirven archivos fuera de rutas cerradas',response.status()===404);
  const state=await (await page.request.get(url+'/api/state')).json();assert('No se conectó ningún hardware',state.status==='disconnected');
  assert('Sin errores JavaScript',errors.length===0);
  // Recuperación: un fallo del recurso WebGL deja el resto de la app operativo.
  const fallback=await browser.newPage();await fallback.route('**/vendor/three.module.min.js',r=>r.abort());
  await fallback.goto(url);await fallback.waitForFunction(()=>document.querySelector('#ver-3d').disabled);
  assert('Fallo de 3D conserva esquema y conexión',await fallback.locator('#kart').isVisible()&&await fallback.locator('#mode-badge').textContent()==='Sin conexión');await fallback.close();
  const result={status:'PASS',scope:'Software y señales sintéticas; sin ensayo físico',checks,errors,external,sizes};fs.writeFileSync(path.join(evidence,'qa_modelo.json'),JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));
 }catch(e){fs.writeFileSync(path.join(evidence,'qa_modelo_fallo.json'),JSON.stringify({checks,errors,message:e.message},null,2));throw e;}finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
