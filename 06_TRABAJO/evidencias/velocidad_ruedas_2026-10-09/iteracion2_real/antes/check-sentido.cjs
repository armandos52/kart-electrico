// Observa las transformaciones reales mediante una referencia solo inyectada en QA.
const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs'),path=require('node:path');
const out=process.argv[2],label=process.argv[3]||'despues',url='http://127.0.0.1:8766';
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const checks=[],errors=[];const check=(name,ok)=>checks.push({name,pass:!!ok});
 try{
  const page=await browser.newPage({viewport:{width:1440,height:1000}});page.on('pageerror',e=>errors.push(e.message));
  await page.route('**/escena_kart.js',async route=>{
    const response=await route.fetch();const source=await response.text(),needle='const kart=buildKart();scene.add(kart.root);';
    if(!source.includes(needle))throw Error('No se localiza la escena para observación QA');
    await route.fulfill({response,body:source.replace(needle,needle+'window.__qaKart=kart;window.__qaT=T;window.__qaCamera=camera;')});
  });
  await page.goto(url+'/#3d');await page.waitForSelector('#escena-3d canvas[data-ready="true"]');
  await page.waitForFunction(()=>document.querySelector('#mode-badge').textContent==='Sin conexión');
  const geometry=await page.evaluate(()=>{const k=window.__qaKart,T=window.__qaT;return {power:new T.Box3().setFromObject(k.groups.power).getCenter(new T.Vector3()).x,wheels:Object.fromEntries(Object.entries(k.wheels).map(([id,w])=>[id,w.hub.position.x])),controllers:Object.fromEntries(['motor_left','motor_right'].map(id=>[id,new T.Box3().setFromObject(k.groups[id]).getCenter(new T.Vector3()).x]))};});
  check('Pulsador en el lado izquierdo del conductor (X negativo)',geometry.power<0);
  check('Ruedas e inspectores I/D coinciden con lados físicos',geometry.wheels.front_left<0&&geometry.wheels.rear_left<0&&geometry.wheels.front_right>0&&geometry.wheels.rear_right>0);
  check('Kelly I/D asociados a su lado',geometry.controllers.motor_left<0&&geometry.controllers.motor_right>0);
  await page.locator('#demo').click();await page.locator('#demo-pedal').fill('32');await page.locator('#demo-steer').fill('0');
  const roll=async()=>page.evaluate(()=>Object.fromEntries(Object.entries(window.__qaKart.wheels).map(([id,w])=>[id,w.spin.rotation.x])));
  let a=await roll();await page.waitForTimeout(140);let b=await roll();
  const delta=(end,start)=>Math.atan2(Math.sin(end-start),Math.cos(end-start));
  const motion=Object.fromEntries(Object.keys(a).map(id=>[id,delta(b[id],a[id])]));
  check('Avance: la parte superior de las cuatro ruedas va hacia el frontal (-Z)',Object.values(motion).every(v=>v<0));
  const svgStep=async()=>page.locator('#rim-rl').evaluate(e=>parseFloat(e.style.transform.match(/[-\d.]+/)?.[0]||'0'));
  a=await svgStep();await page.waitForTimeout(65);b=await svgStep();
  const svgDelta=((b-a+27)%18)-9;check('Rodadura SVG también va hacia el frontal',svgDelta<0);
  const steering=[];
  for(const angle of [-15,0,15]){
    await page.locator('#demo-steer').fill(String(angle));await page.waitForTimeout(550);
    const s=await page.evaluate(async()=>{
      const k=window.__qaKart,T=window.__qaT,n=await import('/nucleo.js');
      k.root.updateMatrixWorld(true);
      return {wheelX:['front_left','front_right'].map(id=>new T.Vector3(0,0,-1).transformDirection(k.wheels[id].hub.matrixWorld).x),svgAngle:Number(document.querySelector('#steer-left').getAttribute('transform').match(/rotate\(([-\d.]+)/)[1]),rpmL:n.app.demoFrame.signals.rpm_rl.value,rpmR:n.app.demoFrame.signals.rpm_rr.value};
    });steering.push({angle,...s});
    check(`Giro ${angle}: ambas ruedas y esquema coinciden`,s.wheelX.every(x=>angle===0?Math.abs(x)<.001:Math.sign(x)===Math.sign(angle))&&Math.abs(s.svgAngle-angle)<.1);
    check(`Giro ${angle}: rueda exterior más rápida, recto igual`,angle<0?s.rpmR>s.rpmL:angle>0?s.rpmL>s.rpmR:Math.abs(s.rpmL-s.rpmR)<.01);
    await page.locator('[data-camera="superior"]').click();await page.waitForTimeout(100);
    await page.locator('.lamina').screenshot({path:path.join(out,`${label}-giro-${angle}.png`)});
  }
  await page.locator('#demo-pedal').fill('0');await page.waitForTimeout(220);a=await roll();await page.waitForTimeout(140);b=await roll();
  check('Pedal cero no genera RPM al girar el volante',Object.keys(a).every(id=>Math.abs(delta(b[id],a[id]))<.000001));
  await page.locator('#demo-pedal').fill('65');await page.waitForTimeout(200);await page.locator('#motion').click();a=await roll();await page.waitForTimeout(140);b=await roll();check('Pausa detiene las cuatro ruedas',Object.keys(a).every(id=>b[id]===a[id]));await page.locator('#motion').click();
  await page.emulateMedia({reducedMotion:'reduce'});await page.waitForTimeout(150);a=await roll();await page.waitForTimeout(150);b=await roll();check('Movimiento reducido mantiene las ruedas quietas',Object.keys(a).every(id=>a[id]===b[id]));await page.emulateMedia({reducedMotion:'no-preference'});
  await page.locator('#demo').click();const fixture=await(await page.request.get(url+'/api/state')).json();fixture.status='connected';fixture.stats.valid_frames=1;
  for(const [key,value]of Object.entries({rpm_rl:110,rpm_rr:140,gear:'R',can_rx_age_ms:5}))fixture.signals[key]={...fixture.signals[key],value,quality:'fresh',age_ms:0,node:'esp32',sample_id:1};
  await page.route('**/api/state',r=>r.fulfill({json:fixture}));await page.waitForFunction(()=>document.querySelector('#travel-map').textContent.includes('ATRÁS'));
  a=await roll();await page.waitForTimeout(140);b=await roll();check('Marcha atrás invierte las dos ruedas traseras',delta(b.rear_left,a.rear_left)>0&&delta(b.rear_right,a.rear_right)>0);
  fixture.signals.rpm_rl.quality='stale';fixture.signals.rpm_rr.quality='stale';await page.waitForTimeout(600);a=await roll();await page.waitForTimeout(140);b=await roll();check('Sin RPM vigentes la rodadura se detiene',a.rear_left===b.rear_left&&a.rear_right===b.rear_right);
  await page.unroute('**/api/state');await page.waitForFunction(()=>document.querySelector('#mode-badge').textContent==='Sin conexión');
  await page.locator('#modelo-pieza').selectOption('power');await page.locator('[data-camera="superior"]').click();await page.waitForTimeout(200);await page.locator('.lamina').screenshot({path:path.join(out,`${label}-pulsador.png`)});
  const orientation=await page.evaluate(()=>{const T=window.__qaT,c=window.__qaCamera;const p=(x,z)=>new T.Vector3(x,.4,z).project(c);return {front:p(0,-1).y,rear:p(0,1).y,left:p(-.6,0).x,right:p(.6,0).x};});
  check('Cámara superior: frontal arriba y lado izquierdo a la izquierda, igual al esquema',orientation.front>orientation.rear&&orientation.left<orientation.right);
  check('Sin errores JavaScript',!errors.length);check('Servidor sin conexión al kart',(await(await page.request.get(url+'/api/state')).json()).status==='disconnected');
  const result={status:checks.every(c=>c.pass)?'PASS':'FAIL',scope:'Geometría y animación visual; datos sintéticos, sin hardware',checks,geometry,motion,steering,errors};fs.writeFileSync(path.join(out,`${label}.json`),JSON.stringify(result,null,2));console.log(JSON.stringify(result,null,2));if(result.status==='FAIL')process.exitCode=1;
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
