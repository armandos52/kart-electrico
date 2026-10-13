const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs'),path=require('node:path');
const evidence=process.argv[2],url='http://127.0.0.1:8766';
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const checks=[],errors=[];const assert=(name,ok)=>{if(!ok)throw Error(name);checks.push(name);};
 try{
  const page=await browser.newPage({viewport:{width:1440,height:1000}});page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>{if(m.type()==='error')errors.push(m.text());});
  await page.goto(url+'/#esquema');await page.waitForFunction(()=>document.querySelector('#mode-badge').textContent==='Sin conexión');
  assert('Enlace directo abre el esquema',await page.locator('#kart').isVisible()&&!await page.locator('#modelo-3d').isVisible());
  assert('Doce componentes accesibles',await page.locator('#kart [data-part]').count()===12);
  for(const id of ['front_left','front_right','rear_left','rear_right','stm32','power','esp32','battery','can','imu','motor_left','motor_right']){
    await page.locator(`#kart [data-part="${id}"]`).focus();await page.keyboard.press('Enter');assert('Selección con teclado '+id,await page.locator('#modelo-pieza').inputValue()===id);
  }
  await page.locator('#modelo-pieza').selectOption('esp32');
  assert('Selección inversa del inspector al esquema',await page.locator('#kart [data-part="esp32"]').evaluate(e=>e.classList.contains('selected')));
  await page.locator('#foto-superior').click();await page.waitForFunction(()=>document.querySelector('#referencia-superior img').naturalWidth>0);
  assert('Fotografía superior original disponible',await page.locator('#referencia-superior').isVisible());await page.keyboard.press('Escape');
  await page.screenshot({path:path.join(evidence,'esquema-desconectado.png'),fullPage:true});await page.locator('.lamina').screenshot({path:path.join(evidence,'esquema-superior.png')});
  await page.locator('#demo').click();await page.locator('#demo-steer').fill('15');await page.waitForFunction(()=>document.querySelector('#steer-left').getAttribute('transform').startsWith('rotate(14')||document.querySelector('#steer-left').getAttribute('transform').startsWith('rotate(15'));
  assert('Dirección gira sobre el centro de las dos ruedas',await page.locator('#steer-left').getAttribute('transform').then(v=>v.endsWith('261 196)'))&&await page.locator('#steer-right').getAttribute('transform').then(v=>v.endsWith('579 196)')));
  const first=await page.locator('#rim-rl').getAttribute('style');await page.waitForTimeout(200);assert('Rodadura responde a RPM sintéticas',first!==await page.locator('#rim-rl').getAttribute('style'));
  await page.locator('#motion').click();const paused=await page.locator('#rim-rl').getAttribute('style');await page.waitForTimeout(200);assert('Pausa mantiene inmóvil la rodadura',paused===await page.locator('#rim-rl').getAttribute('style'));
  await page.screenshot({path:path.join(evidence,'esquema-demo.png'),fullPage:true});await page.locator('#demo').click();await page.locator('#motion').click();
  assert('Salir de demo retira las RPM',await page.locator('#rpm-rl').textContent()==='— rpm');
  const fixture=await (await page.request.get(url+'/api/state')).json();fixture.status='connected';fixture.stats.valid_frames=1;
  for(const [key,value] of Object.entries({steer_raw:2800,can_rx_age_ms:5}))fixture.signals[key]={...fixture.signals[key],value,quality:'fresh',age_ms:0,node:'esp32',sample_id:1};
  await page.route('**/api/state',r=>r.fulfill({json:fixture}));await page.waitForFunction(()=>document.querySelector('#steering-map').textContent.includes('ESTIMADOS'));fixture.signals.can_rx_age_ms.value=220;
  await page.waitForFunction(()=>document.querySelector('#steering-map').textContent==='DIRECCIÓN SIN DATOS');assert('CAN caducado retira la dirección del esquema',true);await page.unroute('**/api/state');
  await page.locator('#ver-3d').click();await page.waitForSelector('#escena-3d canvas[data-ready="true"]');assert('La vista 3D sigue disponible',await page.locator('#modelo-3d').isVisible());await page.locator('#ver-esquema').click();
  const sizes=[];for(const width of [1440,1024,760,375,320]){
    await page.setViewportSize({width,height:1000});await page.waitForTimeout(100);const result=await page.evaluate(()=>({width:innerWidth,scroll:document.documentElement.scrollWidth,viewBox:document.querySelector('#kart').getAttribute('viewBox'),compact:document.querySelector('#kart').classList.contains('plan-compact')}));sizes.push(result);assert('Sin desborde a '+width,result.scroll<=width+1);
    if(width===375)await page.locator('.vista').screenshot({path:path.join(evidence,'esquema-movil.png')});
  }
  assert('Modo compacto en móvil',sizes.at(-1).compact);assert('Sin errores de JavaScript ni SVG',errors.length===0);
  const state=await (await page.request.get(url+'/api/state')).json();assert('Ningún hardware conectado',state.status==='disconnected');
  const result={status:'PASS',scope:'QA sintético del esquema; sin hardware',checks,errors,sizes};fs.writeFileSync(path.join(evidence,'qa_esquema.json'),JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));
 }catch(e){fs.writeFileSync(path.join(evidence,'qa_esquema_fallo.json'),JSON.stringify({checks,errors,error:e.message},null,2));throw e;}finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
