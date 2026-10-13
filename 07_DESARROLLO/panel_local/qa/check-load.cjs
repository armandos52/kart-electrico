// Aperturas independientes del panel local. Nunca conecta al kart.
const {chromium}=require('C:/Users/nandi/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
const fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
 const loads=[];
 try{
  for(let i=0;i<3;i++){
   const page=await browser.newPage({viewport:{width:1366,height:900}}),failed=[],errors=[];
   page.on('requestfailed',r=>{if(/\.(js|css)(\?|$)/.test(r.url()))failed.push({url:r.url(),error:r.failure()?.errorText});});
   page.on('pageerror',e=>errors.push(e.message));
   await page.goto('http://127.0.0.1:8766/');
   await page.waitForSelector('#calibracion',{state:'attached'});
   await page.locator('#abrir-calibracion').click();
   const state=await page.evaluate(async()=>await(await fetch('/api/state')).json());
   loads.push({iteration:i+1,dialog:await page.locator('#calibracion').isVisible(),status:state.status,version:state.calibration.panel_version,failed,errors});
   await page.close();
  }
 }finally{await browser.close();fs.writeFileSync(process.argv[2],JSON.stringify(loads,null,2));}
 if(loads.length!==3||loads.some(x=>!x.dialog||x.status!=='disconnected'||x.failed.length||x.errors.length))throw Error('Fallo de carga: '+JSON.stringify(loads));
 console.log('3 aperturas completas, sin errores de recursos ni JavaScript, sin conexión al kart');
})().catch(e=>{console.error(e);process.exitCode=1;});
