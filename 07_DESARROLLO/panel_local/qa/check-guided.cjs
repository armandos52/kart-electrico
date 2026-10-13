// Navegador real + servicio QA aislado. No conecta ni transmite al kart.
const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs'),path=require('node:path');
const out=process.argv[2],url='http://127.0.0.1:8877';
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const checks=[],errors=[];
 const check=(name,ok)=>{checks.push({name,ok});if(!ok)throw Error(name);};
 try{
  let page=await browser.newPage({viewport:{width:1366,height:1000}});
  page.on('pageerror',e=>errors.push(e.message));
  await page.goto(url);
  await page.waitForFunction(async()=>!!(await import('/nucleo.js')).app.token);
  const qa=async action=>page.evaluate(async action=>{
    const {app}=await import('/nucleo.js');
    const r=await fetch('/qa/'+action,{method:'POST',headers:{'X-Kart-Token':app.token}});
    if(!r.ok)throw Error(await r.text());
  },action);
  const state=()=>page.evaluate(async()=>(await(await fetch('/api/state')).json()).calibration);
  const wait=async predicate=>page.waitForFunction(predicate);
  await qa('reset-workflow');
  await page.waitForFunction(async()=>!(await import('/nucleo.js')).app.state?.calibration?.guide);
  await page.locator('#abrir-calibracion').click();
  check('Sin confirmar preparación no empieza',await page.locator('#cal-primary').isDisabled());
  await page.locator('#cal-ready').check();await page.locator('#cal-primary').click();
  await wait(()=>document.querySelector('#cal-primary').textContent.includes('medir 10'));
  await qa('limit');
  await wait(()=>!document.querySelector('#cal-issue').hidden);
  check('Mover antes de medir no cancela',(await state()).guide.state==='running');
  check('Límite visible y medición bloqueada',await page.locator('#cal-primary').isDisabled());
  await qa('resume');
  await wait(()=>!document.querySelector('#cal-primary').disabled);
  await page.locator('#cal-primary').click();await qa('partial');
  await wait(()=>document.querySelector('#cal-progress').value>0);
  check('Cuenta atrás y avance visibles',await page.locator('#cal-measuring').isVisible());
  await page.screenshot({path:path.join(out,'midiendo.png')});
  await qa('noise');
  await wait(()=>document.querySelector('#cal-issue-title').textContent.includes('señal varía'));
  check('Variación visible sin abrir detalles',!(await page.locator('#cal-details').getAttribute('open'))&&await page.locator('#cal-issue').isVisible());
  check('Medición ruidosa no guardada',(await state()).run.captures.length===0);
  await page.screenshot({path:path.join(out,'senal-inestable.png')});
  await page.locator('#cal-primary').click();
  await wait(()=>document.querySelector('#cal-primary').textContent.includes('medir 10'));
  await page.locator('#cal-primary').click();await qa('finish-capture');
  await wait(()=>document.querySelector('#cal-heading').textContent.includes('izquierdo'));
  check('Centro guardado y siguiente paso espera clic',(await state()).run.captures.length===1&&(await state()).capture===null);
  await page.waitForTimeout(900); // terminar la transición ilustrativa para la captura
  await page.screenshot({path:path.join(out,'giro-izquierda.png')});
  await page.locator('#cal-primary').click();await qa('limit');
  await wait(()=>document.querySelector('#cal-issue-title').textContent.includes('límite'));
  check('Fallo de límite conserva centro',(await state()).run.captures.length===1);
  await qa('resume');await page.locator('#cal-primary').click();
  await wait(()=>document.querySelector('#cal-primary').textContent.includes('medir 5'));
  for(let i=1;i<13;i++){
    await page.locator('#cal-primary').click();await qa('finish-capture');
    await page.waitForFunction(n=>document.querySelector('#cal-saved').textContent===`${n} / 13`,i+1);
  }
  check('13 posiciones completas',(await state()).guide.state==='complete');
  await page.locator('#cal-primary').click();
  await wait(()=>document.querySelector('#cal-heading').textContent==='Todo listo');
  check('Perfil guardado y activo en PC',!!(await state()).active);
  await page.locator('#cal-details').evaluate(e=>e.open=true);
  const download=await Promise.all([page.waitForEvent('download'),page.locator('#cal-export-run').click()]);
  await download[0].saveAs(path.join(out,'ensayo-sintetico.zip'));
  check('Exportación ZIP disponible',fs.statSync(path.join(out,'ensayo-sintetico.zip')).size>1000);
  await page.locator('#cal-details').evaluate(e=>e.open=false);
  await page.screenshot({path:path.join(out,'completa.png')});
  for(const width of [1366,768,375,320]){
    await page.setViewportSize({width,height:900});
    check('Sin desborde del asistente a '+width,await page.locator('#calibracion').evaluate(e=>e.scrollWidth<=e.clientWidth+1));
    if(width===375)await page.screenshot({path:path.join(out,'movil.png')});
  }
  await page.emulateMedia({reducedMotion:'reduce'});
  check('Movimiento reducido respetado',await page.locator('.cal-orbit').evaluate(e=>getComputedStyle(e).animationName==='none'));
  await page.keyboard.press('Escape');
  check('Escape cierra y devuelve foco',!(await page.locator('#calibracion').evaluate(e=>e.open))&&await page.locator('#abrir-calibracion').evaluate(e=>e===document.activeElement));
  for(const lang of ['pt','en','es']){
    await page.locator(`[data-lang="${lang}"]`).click();await page.locator('#abrir-calibracion').click();
    check('Traducción de etapas '+lang,!(await page.locator('.cal-stages').innerText()).includes('cal.ui'));
    await page.locator('#cal-close').click();
  }
  // Aislar el panel normal del contexto QA (descarga, idioma, viewport y WebGL).
  await page.close();
  page=await browser.newPage({viewport:{width:1366,height:1000}});
  page.on('pageerror',e=>errors.push(e.message));
  page.on('requestfailed',r=>errors.push(r.url()+': '+r.failure()?.errorText));
  await page.goto('http://127.0.0.1:8766/');
  await page.waitForSelector('#calibracion',{state:'attached'});
  await page.locator('#abrir-calibracion').click();
  check('Panel real desconectado bloquea calibración',await page.locator('#cal-primary').isDisabled());
  await page.setViewportSize({width:1366,height:1000});
  await page.screenshot({path:path.join(out,'panel-desconectado.png')});
  await page.locator('#cal-close').click();await page.locator('#demo').click();
  check('Demo no permite calibrar',await page.locator('#abrir-calibracion').isDisabled());
  check('Sin errores JavaScript',errors.length===0);
 }finally{
  fs.writeFileSync(path.join(out,'browser-checks.json'),JSON.stringify({checks,errors},null,2));
  await browser.close();
 }
 console.log(`${checks.length} comprobaciones satisfactorias`);
})().catch(e=>{console.error(e);process.exitCode=1;});
