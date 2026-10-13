// Mide en Chrome el giro real 1:1, el desenfoque de movimiento y los mandos; no envía órdenes al servidor.
const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs'),path=require('node:path');
const out=process.argv[2], url=process.env.KART_PANEL_URL||'http://127.0.0.1:8766';
const RPM_POR_PCT=5.5, RAD_POR_RPM=2*Math.PI/60, R_SVG_TRASERA=109/2, DIAMETRO_M=0.48;
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
  check('Cada rueda del esquema tiene su estela',await page.locator('.plan-rotation-trail').count()===4);
  check('La estela 3D no se puede seleccionar',await page.evaluate(()=>Object.values(window.__qaKart.wheels).every(w=>{let n=0;w.trailGroup.traverse(o=>{if(o.isMesh&&window.__qaKart.pickables.includes(o))n++;});return n===0;})));
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
  // Estado de desenfoque observado tal como lo ve el usuario: materiales 3D y atributos SVG.
  const blurState=()=>page.evaluate(()=>{
    const w=window.__qaKart.wheels.rear_left, rect=[...document.querySelectorAll('#estela-rl rect')].filter(r=>Number(r.getAttribute('opacity'))>.02);
    return {detail3d:w.blur.detail,arc3d:w.blur.arc,uArc:w.trail.uniforms.uArc.value,uDir:w.trail.uniforms.uDir.value,uPeak:w.trail.uniforms.uPeak.value,
      detailVisible:w.detail.visible,trailVisible:w.trailGroup.visible,
      detailSvg:Number(document.querySelector('#rim-rl').getAttribute('opacity')),
      trailSvgVisible:document.querySelector('#estela-rl').getAttribute('display')!=='none',trailSvgSegments:rect.length};
  });
  // Energía media de la estela del esquema durante 1,1 s: no depende de en qué fase de la vuelta se mire.
  const trailEnergy=()=>page.evaluate(async()=>{
    const rects=[...document.querySelectorAll('#estela-rl rect')];let sum=0,frames=0;const start=performance.now();
    await new Promise(resolve=>{function sample(now){sum+=rects.reduce((a,r)=>a+Number(r.getAttribute('opacity'))*Number(r.getAttribute('height')),0);frames++;if(now-start>=1100)resolve();else requestAnimationFrame(sample);}requestAnimationFrame(sample);});
    return sum/frames;
  });
  const blurRows=[];
  for(const gear of ['D','R']){
    await page.locator('#demo-gear').selectOption(gear);
    const mine=[];
    for(const pedal of [30,60,100]){
      await page.locator('#demo-pedal').fill(String(pedal));
      const measured=await measure();const rpm=pedal*RPM_POR_PCT;
      // svg_px_s no es fiable aquí: a 60 fps solo se mide una banda que avance menos de 9 px por fotograma.
      const {svg_px_s,...visible}=measured;rows.push({gear,pedal,rpm,...visible});mine.push(measured);
      const sign=gear==='D'?-1:1,expected=rpm*RAD_POR_RPM;
      check(`${gear} ${pedal} %: el 3D gira a la velocidad real (${rpm} RPM)`,Math.abs(measured.rad_s-sign*expected)<expected*.15);
    }
    check(gear+': se distinguen los tres escalones 3D (30/60/100 %)',mine.every((v,i)=>!i||Math.abs(v.rad_s)>Math.abs(mine[i-1].rad_s)*1.4));
    // Una rueda real a 100 % debe parecer ~50 km/h: v = ω·r con Ø 0,48 m.
    const kmh=Math.abs(mine[2].rad_s)*DIAMETRO_M/2*3.6;
    check(gear+': a 100 % la rueda gira como a 50 km/h',kmh>42&&kmh<58);
  }
  await page.locator('#demo-gear').selectOption('D');
  check('El mando muestra el equivalente en km/h',(await page.locator('#demo-pedal-value').textContent()).includes('≈ 50 km/h')||(await page.locator('#demo-pedal').fill('100'),(await page.locator('#demo-pedal-value').textContent()).includes('≈ 50 km/h')));
  // Tabla de desenfoque por pedal (D y R).
  for(const gear of ['D','R']){
    await page.locator('#demo-gear').selectOption(gear);
    for(const pedal of [0,3,10,30,60,100]){
      await page.locator('#demo-pedal').fill(String(pedal));await page.waitForTimeout(220);
      blurRows.push({gear,pedal,...await blurState()});
    }
  }
  for(const gear of ['D','R']){
    const t=blurRows.filter(r=>r.gear===gear), by=p=>t.find(r=>r.pedal===p);
    check(gear+': parado y a 3 % las ruedas son nítidas y sin estela perceptible',by(0).detail3d===1&&by(0).arc3d===0&&!by(0).trailVisible&&!by(0).trailSvgVisible&&by(3).detail3d===1&&by(3).detailVisible);
    check(gear+': a 30 % y más los detalles 3D desaparecen',[30,60,100].every(p=>by(p).detail3d===0&&!by(p).detailVisible));
    check(gear+': a 10 % están a medio camino',by(10).detail3d>0&&by(10).detail3d<1&&by(10).detailVisible);
    check(gear+': el esquema usa el mismo desenfoque que el 3D',t.every(r=>Math.abs(r.detailSvg-r.detail3d)<.01||r.pedal===0));
    check(gear+': la estela 3D crece con el pedal',[10,30,60,100].every((p,i,a)=>!i||by(p).uArc>by(a[i-1]).uArc));
    check(gear+': la estela mide lo que gira la rueda en 1/30 s',[30,60,100].every(p=>Math.abs(by(p).uArc-p*RPM_POR_PCT*RAD_POR_RPM/30)<.02));
    check(gear+': la estela va detrás del sentido de giro',[30,60,100].every(p=>by(p).uDir===(gear==='D'?-1:1)));
    check(gear+': la estela del esquema está activa a 30/60/100 % y oculta parado',[30,60,100].every(p=>by(p).trailSvgVisible)&&!by(0).trailSvgVisible);
    await page.locator('#demo-gear').selectOption(gear);
    const energy=[];for(const pedal of [30,60,100]){await page.locator('#demo-pedal').fill(String(pedal));await page.waitForTimeout(150);energy.push(await trailEnergy());}
    blurRows.push({gear,energia_estela_svg:energy});
    check(gear+': la estela del esquema crece con la velocidad (energía media 30<60<100 %)',energy[0]>0&&energy[1]>energy[0]*1.3&&energy[2]>energy[1]*1.15);
    check(gear+': marca y estela más tenues a máxima velocidad',by(100).uPeak<by(3).uPeak);
  }
  // Rodadura SVG a baja velocidad (la banda aún es legible): sentido y velocidad reales.
  await page.locator('#demo-gear').selectOption('D');await page.locator('#demo-pedal').fill('3');
  const slow=await measure(), slowExpected=3*RPM_POR_PCT*RAD_POR_RPM*R_SVG_TRASERA;
  rows.push({gear:'D',pedal:3,rpm:16.5,svg_expected_px_s:-slowExpected,...slow});
  check('D 3 %: la banda del esquema avanza a ω·R (px reales)',Math.abs(slow.svg_px_s+slowExpected)<slowExpected*.15);
  await page.locator('#demo-gear').selectOption('R');
  const slowR=await measure();rows.push({gear:'R',pedal:3,rpm:16.5,...slowR});
  check('R 3 %: la banda del esquema avanza al revés',Math.abs(slowR.svg_px_s-slowExpected)<slowExpected*.15);
  await page.locator('#demo-gear').selectOption('D');await page.locator('#demo-pedal').fill('60');
  await page.locator('#modelo-pieza').selectOption('rear_right');await page.locator('[data-camera="lateral"]').click();
  // Pausa: ruedas quietas y nítidas.
  await page.locator('#motion').click();await page.waitForTimeout(200);
  const stopped=await measure(),paused=await blurState();
  check('Pausa conserva 3D y esquema',stopped.rad_s===0&&stopped.svg_px_s===0);
  check('Pausa: ruedas nítidas y sin estela',paused.detail3d===1&&paused.detailVisible&&!paused.trailVisible&&!paused.trailSvgVisible&&paused.detailSvg===1);
  await page.screenshot({path:path.join(out,'3d-pausa.png'),fullPage:true});
  await page.locator('#motion').click();await page.waitForTimeout(300);
  await page.screenshot({path:path.join(out,'3d-60.png'),fullPage:true});
  await page.locator('#demo-pedal').fill('100');await page.waitForTimeout(300);
  await page.screenshot({path:path.join(out,'3d-100.png'),fullPage:true});
  await page.locator('#ver-esquema').click();await page.waitForTimeout(200);await page.screenshot({path:path.join(out,'esquema-100.png'),fullPage:true});
  await page.emulateMedia({reducedMotion:'reduce'});await page.waitForTimeout(200);
  const reduced=await measure(),reducedSvg=await blurState();
  check('Movimiento reducido detiene el esquema',reduced.svg_px_s===0);
  check('Movimiento reducido: esquema nítido y sin estela',reducedSvg.detailSvg===1&&!reducedSvg.trailSvgVisible);
  await page.locator('#ver-3d').click();await page.waitForTimeout(250);const reduced3d=await measure(),reducedBlur=await blurState();
  check('Movimiento reducido detiene el 3D',reduced3d.rad_s===0);
  check('Movimiento reducido: ruedas 3D nítidas y sin estela',reducedBlur.detail3d===1&&reducedBlur.detailVisible&&!reducedBlur.trailVisible);
  await page.emulateMedia({reducedMotion:'no-preference'});
  await page.locator('#demo-pedal').fill('0');await page.waitForTimeout(200);const zero=await measure(),zeroBlur=await blurState();
  check('Pedal cero detiene las dos vistas sin estela',zero.rad_s===0&&zero.svg_px_s===0&&!zeroBlur.trailVisible&&!zeroBlur.trailSvgVisible);
  // Selector Real / Lento.
  check('El giro real es el valor por defecto',await page.locator('#giro-escala').inputValue()==='1');
  await page.locator('#demo-pedal').fill('100');await page.locator('#giro-escala').selectOption('0.03');await page.waitForTimeout(250);
  const lento=await measure(),lentoBlur=await blurState(),expectedSlow=100*RPM_POR_PCT*RAD_POR_RPM*.03;
  rows.push({modo:'lento',pedal:100,...lento});
  check('Modo Lento: 3 % de la velocidad real, nítido',Math.abs(Math.abs(lento.rad_s)-expectedSlow)<expectedSlow*.15&&lentoBlur.detail3d===1);
  check('Modo Lento se recuerda en el navegador',await page.evaluate(()=>localStorage.getItem('kart.giro'))==='0.03');
  await page.reload();await page.waitForSelector('#escena-3d canvas[data-ready="true"]');
  check('Modo Lento se conserva al recargar',await page.locator('#giro-escala').inputValue()==='0.03');
  await page.locator('#giro-escala').selectOption('1');
  check('Volver a Real se recuerda',await page.evaluate(()=>localStorage.getItem('kart.giro'))==='1');
  // Móvil.
  await page.locator('#demo').click();await page.locator('#demo-pedal').fill('60');
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(800);// WebGL se borra al redimensionar y se redibuja en el siguiente fotograma
  await page.screenshot({path:path.join(out,'movil-demo.png'),fullPage:true});
  check('Selectores y mandos sin desbordamiento móvil',await page.evaluate(()=>{
    const view=document.documentElement.clientWidth, control=document.querySelector('#demo-controls').getBoundingClientRect();
    const demoOK=[...document.querySelectorAll('#demo-controls input,#demo-controls select')].every(e=>{const r=e.getBoundingClientRect();return r.width>0&&r.left>=control.left&&r.right<=control.right;});
    const giro=document.querySelector('#giro-escala').getBoundingClientRect();
    return demoOK&&giro.width>0&&giro.left>=0&&giro.right<=view&&document.documentElement.scrollWidth<=view+1;
  }));
  await page.locator('#demo').click();
  check('Sin escrituras al servidor',writes.length===0);
  check('Sin errores JavaScript',errors.length===0);
  const state=await(await page.request.get(url+'/api/state')).json();check('Servidor desconectado del kart',state.status==='disconnected');
  const result={scope:'Chrome, demo y transformaciones renderizadas; sin hardware',status:checks.every(c=>c.pass)?'PASS':'FAIL',rows,blurRows,checks,errors,writes};
  fs.writeFileSync(path.join(out,'browser.json'),JSON.stringify(result,null,2));
  console.log(JSON.stringify({status:result.status,fails:checks.filter(c=>!c.pass).map(c=>c.name),total:checks.length,errors},null,1));if(result.status==='FAIL')process.exitCode=1;
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
