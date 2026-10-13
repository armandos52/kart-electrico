const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs');
const path=require('node:path');
const {pathToFileURL}=require('node:url');
(async()=>{
  const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  const checks=[],errors=[];
  try{
    const page=await browser.newPage({viewport:{width:800,height:1100}});
    page.on('pageerror',e=>errors.push(e.message));
    await page.goto(pathToFileURL(path.join(__dirname,'preview.html')).href);
    const f=page.frameLocator('iframe');
    await f.locator('#dd-angle').waitFor();
    const assert=async(name,condition)=>{if(!await condition)throw Error(name);checks.push(name);};
    await assert('Centro de ejemplo cero; estimación anterior desplazada', (async()=>await f.locator('#dd-angle').innerText()==='0,0°'&&await f.locator('#dd-old').innerText()==='2,8°')());
    await f.locator('[data-position="left"]').click();
    await assert('Tope izquierdo conserva −24° y −100 %',(async()=>await f.locator('#dd-angle').innerText()==='-24,0°'&&await f.locator('#dd-percent').innerText()==='-100,0 %')());
    await f.locator('[data-position="right"]').click();
    await assert('Tope derecho independiente +19° y +100 %',(async()=>await f.locator('#dd-angle').innerText()==='19,0°'&&await f.locator('#dd-percent').innerText()==='100,0 %')());
    await f.locator('[data-capture="center"]').click();
    await assert('Captura inválida rechazada sin alterar perfil',(async()=>await f.locator('#dd-error').isVisible()&&(await f.locator('[data-value="center"]').innerText()).includes('2180'))());
    await f.locator('#dd-reading').fill('2300');
    await f.locator('[data-capture="center"]').click();
    await assert('Centro candidato se actualiza y recalcula',(async()=>await f.locator('#dd-angle').innerText()==='0,0°'&&(await f.locator('[data-value="leftSpan"]').innerText()).includes('1800'))());
    await f.locator('#dd-reading').fill('4095');
    await assert('Fuera de rango visible sin grados extrapolados',f.locator('#dd-angle').innerText().then(t=>t==='Fuera de rango'));
    await f.locator('#dd-reset').click();
    await f.locator('#dd-jitter').click();
    await assert('Variación sigue visible después de centrar',(async()=>await f.locator('#dd-angle').innerText()!=='0,0°'&&(await f.locator('#dd-status').innerText()).includes('96 cuentas'))());
    await f.locator('#dd-reset').click();
    const frame=page.frames().find(x=>x!==page.mainFrame());
    for(const width of [800,375,320]){
      await page.setViewportSize({width,height:1200});
      await page.waitForTimeout(250);
      const geometry=await frame.evaluate(()=>{
        const root=document.getElementById('direccion-demo');
        const rect=root.getBoundingClientRect();
        const labels=[...root.querySelectorAll('svg text')].map(e=>{const r=e.getBoundingClientRect();return{x:r.x,y:r.y,right:r.right,bottom:r.bottom,text:e.textContent};});
        const overlap=labels.some((a,i)=>labels.some((b,j)=>j>i&&a.x<b.right&&a.right>b.x&&a.y<b.bottom&&a.bottom>b.y));
        return{rootWidth:rect.width,client:document.documentElement.clientWidth,scroll:document.documentElement.scrollWidth,overlap};
      });
      await assert(`Diseño ${width}px sin desborde ni etiquetas superpuestas`,geometry.scroll<=geometry.client+1&&!geometry.overlap);
      await page.screenshot({path:path.join(__dirname,`demo-${width}.png`),fullPage:true});
    }
    await assert('Sin errores JavaScript',errors.length===0);
    fs.writeFileSync(path.join(__dirname,'qa_resultado.json'),JSON.stringify({tipo:'prueba_automatica_datos_sinteticos',fecha_local:'2026-10-08',checks,errors,hardware_conectado:false,panel_modificado:false},null,2));
    process.stdout.write(JSON.stringify({ok:true,checks,errors},null,2));
  }catch(e){fs.writeFileSync(path.join(__dirname,'qa_fallo.txt'),e.stack);throw e;}
  finally{await browser.close();}
})();
