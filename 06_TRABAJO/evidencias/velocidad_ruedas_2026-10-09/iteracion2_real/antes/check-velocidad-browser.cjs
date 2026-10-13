// Mide transformaciones renderizadas con la demo; no envía órdenes al servidor.
const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs'),path=require('node:path');
const out=process.argv[2], url=process.env.KART_PANEL_URL||'http://127.0.0.1:8766';
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const checks=[],errors=[],rows=[],writes=[];
 const check=(name,ok)=>checks.push({name,pass:!!ok});
 try{
  const context=await browser.newContext({viewport:{width:1440,height:1000}});
  const page=await context.newPage();page.on('pageerror',e=>errors.push(e.message));
  await page.route('**/*',route=>{
    if(route.request().method()!=='GET'){writes.push(route.request().url());return route.abort();}
    return route.fallback();
  });
  await page.route('**/escena_kart.js',async route=>{
    const response=await route.fetch(), source=await response.text(), needle='const kart=buildKart();scene.add(kart.root);';
    if(!source.includes(needle))throw Error('No se localiza la escena para observación QA');
    await route.fulfill({response,body:source.replace(needle,needle+'window.__qaKart=kart;')});
  });
  await page.goto(url);await page.waitForSelector('#escena-3d canvas[data-ready="true"]');
  await page.waitForFunction(()=>document.querySelector('#mode-badge').textContent==='Sin conexión');
  await page.locator('#demo').click();await page.locator('#demo-steer').fill('0');
  check('Una marca única unida a cada rueda 3D',await page.evaluate(()=>Object.values(window.__qaKart.wheels).every(w=>w.spin.getObjectByName('marca-rodadura'))));
  check('Cuatro marcas equivalentes en el esquema',await page.locator('.plan-rotation-marker').count()===4);
  const measure=()=>page.evaluate(async()=>{
    const wheel=window.__qaKart.wheels.rear_left.spin;
    const svg=()=>parseFloat(document.querySelector('#rim-rl').style.transform.match(/[-\d.]+/)[0]);
    let previous3d=wheel.rotation.x,previousSvg=svg(),angle=0,pixels=0,frames=0;
    const start=performance.now();
    await new Promise(resolve=>{
      function sample(now){
        const current3d=wheel.rotation.x,currentSvg=svg();
        angle+=Math.atan2(Math.sin(current3d-previous3d),Math.cos(current3d-previous3d));
        pixels+=((currentSvg-previousSvg+27)%18)-9;
        previous3d=current3d;previousSvg=currentSvg;frames++;
        if(now-start>=1100)resolve();else requestAnimationFrame(sample);
      }
      requestAnimationFrame(sample);
    });
    const seconds=(performance.now()-start)/1000;
    return {seconds,frames,rad_s:angle/seconds,svg_px_s:pixels/seconds};
  });
  for(const gear of ['D','R']){
    await page.locator('#demo-gear').selectOption(gear);
    for(const pedal of [30,60,100]){
      await page.locator('#demo-pedal').fill(String(pedal));
      const measured=await measure();rows.push({gear,pedal,rpm:pedal*4,...measured});
      const sign=gear==='D'?-1:1,expected=pedal*4*2*Math.PI/60*.03;
      check(`${gear} ${pedal} %: 3D proporcional a RPM`,Math.abs(measured.rad_s-sign*expected)<expected*.18);
      check(`${gear} ${pedal} %: SVG proporcional y mismo sentido`,Math.abs(measured.svg_px_s-sign*expected*20)<expected*20*.18);
    }
    const speeds=rows.filter(row=>row.gear===gear);
    check(gear+': se distinguen los tres escalones 3D',speeds.every((v,i)=>!i||Math.abs(v.rad_s)>Math.abs(speeds[i-1].rad_s)*1.4));
  }
  await page.locator('#demo-gear').selectOption('D');await page.locator('#demo-pedal').fill('60');
  await page.locator('#modelo-pieza').selectOption('rear_right');await page.locator('[data-camera="lateral"]').click();
  await page.locator('#motion').click();
  const stopped=await measure();check('Pausa conserva 3D y esquema',stopped.rad_s===0&&stopped.svg_px_s===0);
  await page.screenshot({path:path.join(out,'3d-marcas.png'),fullPage:true});
  await page.locator('#ver-esquema').click();await page.screenshot({path:path.join(out,'esquema-marcas.png'),fullPage:true});
  await page.locator('#motion').click();const visibleSvg=await measure();
  check('Esquema visible acelera con la misma escala',Math.abs(visibleSvg.svg_px_s+60*4*2*Math.PI/60*.03*20)<2);
  await page.emulateMedia({reducedMotion:'reduce'});const reduced=await measure();
  check('Movimiento reducido detiene el esquema',reduced.svg_px_s===0);
  await page.locator('#ver-3d').click();const reduced3d=await measure();check('Movimiento reducido detiene el 3D',reduced3d.rad_s===0);
  await page.emulateMedia({reducedMotion:'no-preference'});
  await page.locator('#demo-pedal').fill('0');const zero=await measure();check('Pedal cero detiene las dos vistas',zero.rad_s===0&&zero.svg_px_s===0);
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(800);// WebGL se borra al redimensionar y se redibuja en el siguiente fotograma
  await page.screenshot({path:path.join(out,'movil-demo.png'),fullPage:true});
  check('Selector D/R y mandos sin desbordamiento móvil',await page.evaluate(()=>{
    const control=document.querySelector('#demo-controls').getBoundingClientRect();
    return [...document.querySelectorAll('#demo-controls input,#demo-controls select')].every(e=>{const r=e.getBoundingClientRect();return r.width>0&&r.left>=control.left&&r.right<=control.right;});
  }));
  await page.locator('#demo').click();
  check('Sin escrituras al servidor',writes.length===0);
  check('Sin errores JavaScript',errors.length===0);
  const state=await(await page.request.get(url+'/api/state')).json();check('Servidor desconectado del kart',state.status==='disconnected');
  const result={scope:'Chrome, demo y transformaciones renderizadas; sin hardware',status:checks.every(c=>c.pass)?'PASS':'FAIL',rows,checks,errors,writes};
  fs.writeFileSync(path.join(out,'browser.json'),JSON.stringify(result,null,2));
  console.log(JSON.stringify(result,null,2));if(result.status==='FAIL')process.exitCode=1;
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
