// QA de la copia pública. Solo servidor local y datos sintéticos; ningún puerto del kart.
const {chromium} = require('playwright');
const fs = require('node:fs');
const path = require('node:path');
const url = process.env.KART_PANEL_URL || 'http://127.0.0.1:8878';
const out = process.argv[2];
if (!out) throw Error('Indica carpeta de evidencia como primer argumento');
fs.mkdirSync(out, {recursive:true});
const checks=[], errors=[];
const check=(name,ok)=>{checks.push({name,ok:!!ok});if(!ok)throw Error(name);};
(async()=>{
  const options={headless:true};
  if(process.env.KART_CHROME)options.executablePath=process.env.KART_CHROME;
  const browser=await chromium.launch(options);
  try{
    const page=await browser.newPage({viewport:{width:1440,height:1000}});
    page.on('pageerror',e=>errors.push(e.message));
    await page.goto(url);
    await page.waitForSelector('#escena-3d canvas[data-ready="true"]');
    await page.waitForFunction(()=>document.querySelector('#mode-badge').textContent==='Sin conexión');
    const state=await page.evaluate(async()=>await(await fetch('/api/state')).json());
    check('Inicio desconectado',state.status==='disconnected');
    check('3D disponible sin hardware',await page.locator('#escena-3d canvas').isVisible());
    await page.locator('#ver-fotos').click();
    await page.waitForSelector('[data-photo-local]');
    check('Frontal privada ausente con aviso',await page.locator('[data-photo-local]').isVisible());
    const loaded=await page.locator('#fotos-kart img').evaluateAll(imgs=>imgs.filter(i=>!i.hidden).map(i=>i.complete&&i.naturalWidth>0));
    check('Tres fotos públicas visibles',loaded.length===3&&loaded.every(Boolean));
    await page.screenshot({path:path.join(out,'galeria-publica.png')});
    await page.locator('#cerrar-fotos').click();
    await page.locator('#abrir-calibracion').click();
    check('Calibración bloqueada sin conexión',await page.locator('#cal-primary').isDisabled());
    await page.locator('#cal-close').click();
    await page.locator('#demo').click();
    await page.waitForFunction(()=>!document.querySelector('#demo-banner').hidden);
    check('Demo rotulada como sintética',await page.locator('#demo-banner').isVisible());
    check('Demo bloquea calibración real',await page.locator('#abrir-calibracion').isDisabled());
    await page.screenshot({path:path.join(out,'panel-demo-publico.png')});
    await page.locator('#ver-esquema').click();
    check('Esquema accesible',await page.locator('#kart').isVisible());
    await page.screenshot({path:path.join(out,'esquema-publico.png')});
    await page.setViewportSize({width:390,height:844});
    await page.screenshot({path:path.join(out,'panel-movil.png')});
    check('Sin errores JavaScript',errors.length===0);
  }finally{
    fs.writeFileSync(path.join(out,'browser.json'),JSON.stringify({checks,errors},null,2));
    await browser.close();
  }
  console.log(`${checks.length} comprobaciones; ${errors.length} errores JavaScript`);
})().catch(e=>{console.error(e);process.exitCode=1;});
