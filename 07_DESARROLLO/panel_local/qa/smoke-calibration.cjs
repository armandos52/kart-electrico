const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs');
const path=require('node:path');
(async()=>{
const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
try{
 const page=await browser.newPage({viewport:{width:1280,height:960}}),errors=[];
 page.on('pageerror',e=>errors.push(e.message));
 await page.goto('http://127.0.0.1:8766/');
 await page.waitForFunction(()=>document.querySelector('#mode-badge').textContent==='Sin conexión');
 await page.locator('#abrir-calibracion').click();
 if(!await page.locator('#cal-begin').isDisabled())throw Error('Permite iniciar sin datos');
 const sizes=[];
 for(const width of [1280,375,320]){
  await page.setViewportSize({width,height:960});
  const info=await page.evaluate(()=>({client:innerWidth,scroll:document.documentElement.scrollWidth,overflow:[...document.querySelectorAll('body *')].filter(e=>e.getBoundingClientRect().right>innerWidth+1&&!e.closest('.cal-table')).slice(0,12).map(e=>({tag:e.tagName,id:e.id,class:e.className,right:e.getBoundingClientRect().right,width:e.getBoundingClientRect().width}))}));
  sizes.push(info);
  if(info.scroll>width+1)throw Error('Desborde del panel a '+width+': '+JSON.stringify(info));
 }
 console.log(JSON.stringify({sizes,errors},null,2));
 await page.setViewportSize({width:1280,height:960});
 await page.screenshot({path:path.join(process.argv[2],'panel-real-desconectado.png')});
 await page.locator('#cal-close').click();await page.locator('#demo').click();
 if(!await page.locator('#abrir-calibracion').isDisabled())throw Error('Demo permite calibración real');
 await page.locator('#demo').click();
 if(errors.length)throw Error(errors.join('\n'));
 fs.writeFileSync(path.join(process.argv[2],'smoke_panel.json'),JSON.stringify({sizes,errors,disconnected_guard:true,demo_guard:true},null,2));
}finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
